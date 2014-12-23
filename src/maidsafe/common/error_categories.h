/*  Copyright 2012 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */

#ifndef MAIDSAFE_COMMON_ERROR_CATEGORIES_H_
#define MAIDSAFE_COMMON_ERROR_CATEGORIES_H_

#include <string>
#include <system_error>

#include "maidsafe/common/config.h"

namespace maidsafe {

namespace detail {

class CommonCategory : public std::error_category {
 public:
  virtual const char* name() const MAIDSAFE_NOEXCEPT;
  virtual std::string message(int error_value) const MAIDSAFE_NOEXCEPT;
  virtual std::error_condition default_error_condition(int error_value) const MAIDSAFE_NOEXCEPT;
};

class AsymmCategory : public std::error_category {
 public:
  virtual const char* name() const MAIDSAFE_NOEXCEPT;
  virtual std::string message(int error_value) const MAIDSAFE_NOEXCEPT;
  virtual std::error_condition default_error_condition(int error_value) const MAIDSAFE_NOEXCEPT;
};

class PassportCategory : public std::error_category {
 public:
  virtual const char* name() const MAIDSAFE_NOEXCEPT;
  virtual std::string message(int error_value) const MAIDSAFE_NOEXCEPT;
  virtual std::error_condition default_error_condition(int error_value) const MAIDSAFE_NOEXCEPT;
};

class RudpCategory : public std::error_category {
 public:
  virtual const char* name() const MAIDSAFE_NOEXCEPT;
  virtual std::string message(int error_value) const MAIDSAFE_NOEXCEPT;
  virtual std::error_condition default_error_condition(int error_value) const MAIDSAFE_NOEXCEPT;
};

class EncryptCategory : public std::error_category {
 public:
  virtual const char* name() const MAIDSAFE_NOEXCEPT;
  virtual std::string message(int error_value) const MAIDSAFE_NOEXCEPT;
  virtual std::error_condition default_error_condition(int error_value) const MAIDSAFE_NOEXCEPT;
};

class RoutingCategory : public std::error_category {
 public:
  virtual const char* name() const MAIDSAFE_NOEXCEPT;
  virtual std::string message(int error_value) const MAIDSAFE_NOEXCEPT;
  virtual std::error_condition default_error_condition(int error_value) const MAIDSAFE_NOEXCEPT;
};

class NfsCategory : public std::error_category {
 public:
  virtual const char* name() const MAIDSAFE_NOEXCEPT;
  virtual std::string message(int error_value) const MAIDSAFE_NOEXCEPT;
  virtual std::error_condition default_error_condition(int error_value) const MAIDSAFE_NOEXCEPT;
};

class DriveCategory : public std::error_category {
 public:
  virtual const char* name() const MAIDSAFE_NOEXCEPT;
  virtual std::string message(int error_value) const MAIDSAFE_NOEXCEPT;
  virtual std::error_condition default_error_condition(int error_value) const MAIDSAFE_NOEXCEPT;
};

class VaultCategory : public std::error_category {
 public:
  virtual const char* name() const MAIDSAFE_NOEXCEPT;
  virtual std::string message(int error_value) const MAIDSAFE_NOEXCEPT;
  virtual std::error_condition default_error_condition(int error_value) const MAIDSAFE_NOEXCEPT;
};

class VaultManagerCategory : public std::error_category {
 public:
  virtual const char* name() const MAIDSAFE_NOEXCEPT;
  virtual std::string message(int error_value) const MAIDSAFE_NOEXCEPT;
  virtual std::error_condition default_error_condition(int error_value) const MAIDSAFE_NOEXCEPT;
};

class ApiCategory : public std::error_category {
 public:
  virtual const char* name() const MAIDSAFE_NOEXCEPT;
  virtual std::string message(int error_value) const MAIDSAFE_NOEXCEPT;
  virtual std::error_condition default_error_condition(int error_value) const MAIDSAFE_NOEXCEPT;
};

}  // namespace detail

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_ERROR_CATEGORIES_H_
