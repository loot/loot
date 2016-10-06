/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2016    WrinklyNinja

    This file is part of LOOT.

    LOOT is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    LOOT is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with LOOT.  If not, see
    <https://www.gnu.org/licenses/>.
    */

#include "loot/error_categories.h"

namespace loot {
namespace detail {
class libloadorder_category : public std::error_category {
  virtual const char* name() const noexcept {
    return "libloadorder";
  }

  virtual std::string message(int ev) const {
    return "Libloadorder error";
  }

  virtual bool equivalent(const std::error_code& code, int condition) const noexcept {
    return code.category().name() == name();
  }
};

class libgit2_category : public std::error_category {
  virtual const char* name() const noexcept {
    return "libgit2";
  }

  virtual std::string message(int ev) const {
    return "libgit2 error";
  }

  virtual bool equivalent(const std::error_code& code, int condition) const noexcept {
    return code.category().name() == name();
  }
};
}

LOOT_API const std::error_category& libloadorder_category() {
  static detail::libloadorder_category instance;
  return instance;
}

LOOT_API const std::error_category& libgit2_category() {
  static detail::libgit2_category instance;
  return instance;
}
}
