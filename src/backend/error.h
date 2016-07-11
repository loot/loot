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
    <http://www.gnu.org/licenses/>.
    */

#ifndef LOOT_BACKEND_ERROR
#define LOOT_BACKEND_ERROR

#include <exception>
#include <string>

namespace loot {
class Error : public std::exception {
public:
  enum struct Code : unsigned int {
      // These must not be changed for API stability.
    ok = 0,
    liblo_error = 1,
    path_write_fail = 2,
    path_read_fail = 3,
    condition_eval_fail = 4,
    regex_eval_fail = 5,
    no_mem = 6,
    invalid_args = 7,
    no_tag_map = 8,
    path_not_found = 9,
    no_game_detected = 10,
    //11 was subversion_error, and was removed along with svn support.
    git_error = 12,
    windows_error = 13,
    sorting_error = 14,
  };

  Error(const Code code_arg, const std::string& what_arg) : code_(code_arg), what_(what_arg) {}
  ~Error() throw() {};

  Code code() const { return code_; }

  unsigned int codeAsUnsignedInt() const {
    return asUnsignedInt(code_);
  }

  const char * what() const throw() { return what_.c_str(); }

  static unsigned int asUnsignedInt(Code code) {
    return static_cast<unsigned int>(code);
  }
private:
  Code code_;
  std::string what_;
};
}

#endif
