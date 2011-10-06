/* Copyright (c) 2009 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "maidsafe/common/crypto.h"
#include <memory>
#include <algorithm>

#ifdef __MSVC__
#  pragma warning(push, 1)
#  pragma warning(disable: 4702)
#endif

#include "boost/scoped_array.hpp"
#include "cryptopp/gzip.h"
#include "cryptopp/hex.h"
#include "cryptopp/aes.h"
#include "cryptopp/modes.h"
#include "cryptopp/osrng.h"
#include "cryptopp/pwdbased.h"
#include "cryptopp/cryptlib.h"

#ifdef __MSVC__
#  pragma warning(pop)
#endif
#include "boost/thread/mutex.hpp"

#include "maidsafe/common/log.h"
#include "maidsafe/common/platform_config.h"

namespace maidsafe {

namespace crypto {

namespace {

boost::mutex g_rng_mutex, g_keygen_mutex;

CryptoPP::RandomNumberGenerator &g_srandom_number_generator() {
  static CryptoPP::AutoSeededX917RNG<CryptoPP::AES> rng;
  return rng;
}

}  // Unnamed namespace


std::string XOR(const std::string &first, const std::string &second) {
  size_t common_size(first.size());
  if ((common_size != second.size()) || (common_size == 0)) {
    DLOG(WARNING) << "Size mismatch or zero.";
    return "";
  }

  boost::scoped_array<char> first_char(new char[common_size]);
  std::copy(first.begin(), first.end(), first_char.get());
  boost::scoped_array<char> second_char(new char[common_size]);
  std::copy(second.begin(), second.end(), second_char.get());

  boost::scoped_array<char> buffer(new char[common_size]);
  for (size_t i = 0; i < common_size; ++i)
    buffer[i] = first_char[i] ^ second_char[i];

  std::string result(buffer.get(), common_size);
  return result;
}

std::string SecurePassword(const std::string &password,
                           const std::string &salt,
                           const uint32_t &pin,
                           const std::string &label) {
  if (password.empty() || salt.empty() || pin == 0 || label.empty()) {
    DLOG(WARNING) << "Invalid parameter.";
    return "";
  }
  uint16_t iter = (pin % 10000) + 10000;
  CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA512> pbkdf;
  CryptoPP::SecByteBlock derived(AES256_KeySize + AES256_IVSize);
  byte purpose = 0;  // unused in this pbkdf implementation
  CryptoPP::SecByteBlock context(salt.size() + label.size());
  std::copy_n(salt.data(), salt.size(), &context[0]);
  std::copy_n(label.data(),  label.size(), &context[salt.size()]);
  pbkdf.DeriveKey(derived, derived.size(), purpose,
                  reinterpret_cast<const byte*>(password.data()),
                  password.size(), context.data(), context.size(), iter);
  std::string derived_password;
  CryptoPP::StringSink string_sink(derived_password);
  string_sink.Put(derived, derived.size());
  return derived_password;
}

std::string SymmEncrypt(const std::string &input,
                        const std::string &key,
                        const std::string &initialisation_vector) {
  if (key.size() < AES256_KeySize ||
      initialisation_vector.size() < AES256_IVSize) {
    DLOG(WARNING) << "Undersized key or IV.";
    return "";
  }

  try {
    byte byte_key[AES256_KeySize], byte_iv[AES256_IVSize];

    CryptoPP::StringSource(key.substr(0, AES256_KeySize), true,
        new CryptoPP::ArraySink(byte_key, sizeof(byte_key)));

    CryptoPP::StringSource(initialisation_vector.substr(0, AES256_IVSize), true,
        new CryptoPP::ArraySink(byte_iv, sizeof(byte_iv)));

    std::string result;
    CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption encryptor(byte_key,
        sizeof(byte_key), byte_iv);
    CryptoPP::StringSource(input, true,
        new CryptoPP::StreamTransformationFilter(encryptor,
            new CryptoPP::StringSink(result)));
    return result;
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "Failed symmetric encryption: " << e.what();
    return "";
  }
}

std::string SymmDecrypt(const std::string &input,
                        const std::string &key,
                        const std::string &initialisation_vector) {
  if (key.size() < AES256_KeySize ||
      initialisation_vector.size() < AES256_IVSize ||
      input.empty()) {
    DLOG(WARNING) << "Undersized key or IV or input.";
    return "";
  }

  try {
    byte byte_key[AES256_KeySize], byte_iv[AES256_IVSize];

    CryptoPP::StringSource(key.substr(0, AES256_KeySize), true,
        new CryptoPP::ArraySink(byte_key, sizeof(byte_key)));

    CryptoPP::StringSource(initialisation_vector.substr(0, AES256_IVSize), true,
        new CryptoPP::ArraySink(byte_iv, sizeof(byte_iv)));

    std::string result;
    CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decryptor(byte_key,
        sizeof(byte_key), byte_iv);
    CryptoPP::StringSource(input, true,
        new CryptoPP::StreamTransformationFilter(decryptor,
            new CryptoPP::StringSink(result)));
    return result;
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "Failed symmetric decryption: " << e.what();
    return "";
  }
}

std::string AsymEncrypt(const std::string &input,
                        const std::string &public_key) {
  if (input.empty() || public_key.empty()) {
    DLOG(WARNING) << "Empty key or input.";
    return "";
  }
  try {
    CryptoPP::StringSource key(public_key, true);
    CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(key);
    std::string result;
    CryptoPP::StringSource(input, true, new CryptoPP::PK_EncryptorFilter(
        g_srandom_number_generator(), encryptor,
        new CryptoPP::StringSink(result)));
    return result;
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "Failed asymmetric encryption: " << e.what();
    return (e.GetErrorType() == CryptoPP::Exception::IO_ERROR) ?
           AsymEncrypt(input, public_key) : "";
  }
}

std::string AsymDecrypt(const std::string &input,
                        const std::string &private_key) {
  if (input.empty() || private_key.empty()) {
    DLOG(WARNING) << "Empty key or input.";
    return "";
  }
  try {
    CryptoPP::StringSource key(private_key, true);
    CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(key);
    std::string result;
    CryptoPP::StringSource(input, true, new CryptoPP::PK_DecryptorFilter(
        g_srandom_number_generator(), decryptor,
        new CryptoPP::StringSink(result)));
    return result;
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "Failed asymmetric decryption: " << e.what();
    return (e.GetErrorType() == CryptoPP::Exception::IO_ERROR) ?
           AsymDecrypt(input, private_key) : "";
  }
}

std::string AsymSign(const std::string &input, const std::string &private_key) {
  if (input.empty() || private_key.empty()) {
    DLOG(WARNING) << "Empty key or input.";
    return "";
  }

  try {
    CryptoPP::StringSource key(private_key, true);
    CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA512>::Signer signer(key);
    std::string result;
    CryptoPP::StringSource(input, true, new CryptoPP::SignerFilter(
                           g_srandom_number_generator(), signer,
                           new CryptoPP::StringSink(result)));
    return result;
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "Failed asymmetric signing: " << e.what();
    return (e.GetErrorType() == CryptoPP::Exception::IO_ERROR) ?
           AsymSign(input, private_key) : "";
  }
}

bool AsymCheckSig(const std::string &input_data,
                  const std::string &input_signature,
                  const std::string &public_key) {
  try {
    CryptoPP::StringSource key(public_key, true);
    CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA512>::Verifier
        verifier(key);
    CryptoPP::StringSource signature_string(input_signature, true);
    if (signature_string.MaxRetrievable() != verifier.SignatureLength())
      return false;
    const std::unique_ptr<CryptoPP::SecByteBlock> kSignature(
        new CryptoPP::SecByteBlock(verifier.SignatureLength()));
    signature_string.Get(*kSignature, kSignature->size());
    CryptoPP::SignatureVerificationFilter *verifier_filter(
        new CryptoPP::SignatureVerificationFilter(verifier));
    verifier_filter->Put(*kSignature, verifier.SignatureLength());
    CryptoPP::StringSource ssource(input_data, true, verifier_filter);
    return verifier_filter->GetLastResult();
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "Error validating asymmetric signature: " << e.what();
    return false;
  }
}

std::string Compress(const std::string &input,
                     const uint16_t &compression_level) {
  if (compression_level > kMaxCompressionLevel)
    return "";
  try {
    std::string result;
    CryptoPP::StringSource(input, true, new CryptoPP::Gzip(
        new CryptoPP::StringSink(result), compression_level));
    return result;
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "Failed compressing: " << e.what();
    return "";
  }
}

std::string Uncompress(const std::string &input) {
  try {
    std::string result;
    CryptoPP::StringSource(input, true, new CryptoPP::Gunzip(
        new CryptoPP::StringSink(result)));
    return result;
  }
  catch(const CryptoPP::Exception &e) {
    DLOG(ERROR) << "Failed uncompressing: " << e.what();
    return "";
  }
}

CryptoPP::Integer RandomNumber(size_t bit_count) {
  boost::mutex::scoped_lock rng_lock(g_rng_mutex);
  return CryptoPP::Integer(g_srandom_number_generator(), bit_count);
}

void RandomBlock(byte *output, size_t size) {
  boost::mutex::scoped_lock rng_lock(g_rng_mutex);
  g_srandom_number_generator().GenerateBlock(output, size);
}

void RsaKeyPair::GenerateKeys(const uint16_t &key_size) {
  ClearKeys();
  CryptoPP::RandomPool rand_pool;
  boost::scoped_array<byte> seed(new byte[key_size]);
  RandomBlock(seed.get(), key_size);
  {
    boost::mutex::scoped_lock rng_lock(g_keygen_mutex);
    rand_pool.IncorporateEntropy(seed.get(), key_size);
  }
  CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(rand_pool, key_size);
  CryptoPP::StringSink private_key(private_key_);
  decryptor.DEREncode(private_key);
  private_key.MessageEnd();
  CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(decryptor);
  CryptoPP::StringSink public_key(public_key_);
  encryptor.DEREncode(public_key);
  public_key.MessageEnd();
}

}  // namespace crypto

}  // namespace maidsafe
