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

#ifndef LOOT_ERROR
#define LOOT_ERROR

#include <exception>
#include <string>

/**
 * @file
 * @brief Contains the Error class used for some exceptions thrown.
 */

namespace loot {
/**
 * @brief A class that defines the type of objects thrown as errors by the LOOT
 *        API.
 * @details Note that not all exceptions thrown from the LOOT API use this type,
 *          so catch `std::exception` too.
 */
class Error : public std::exception {
public:
  /**
   * @brief A code indicating the type of error that occurred.
   */
  enum struct Code : unsigned int {
    /**
     * No error occurred. This is used internally to indicate that an operation
     * failed, but its failure was not fatal to the task being performed.
     */
    ok = 0,
    /** An error was encountered when reading or writing the load order. */
    liblo_error = 1,
    /** An error was encountered when writing a file. */
    path_write_fail = 2,
    /** An error was encountered when reading a file. */
    path_read_fail = 3,
    /**
     * An error was encountered when attempting to evaluate a metadata
     * condition.
     */
    condition_eval_fail = 4,
    /** An error was encountered when parsing a regular expression. */
    regex_eval_fail = 5,
    /** An error was encountered when trying to allocate memory. */
    no_mem = 6,
    /** An error was encountered due to invalid function arguments. */
    invalid_args = 7,
    /**
     * @deprecated
     * @brief An error was encountered when getting plugin tags as no tag map
     * was built. **This code is obsolete and no longer used.**
     */
    no_tag_map = 8,
    /** A path could not be found. */
    path_not_found = 9,
    /** None of LOOT's supported games could be detected. */
    no_game_detected = 10,
    /**
     * @deprecated
     * @brief An error was encountered while trying to run a Subversion command.
     * **This code is obsolete and no longer used.**
     */
    subversion_error = 11,
    /**
     * An error was encountered while trying to create or interact with a Git
     * repository.
     */
    git_error = 12,
    /** A miscellaneous error occurred when using an operating system API. */
    windows_error = 13,
    /** An error occurred while trying to sort the load order. */
    sorting_error = 14,
  };

  /**
   * @brief Construct an error object giving a code and error message.
   */
  Error(const Code code_arg, const std::string& what_arg) : code_(code_arg), what_(what_arg) {}
  ~Error() throw() {};

  /**
   * @brief Get the code describing the type of error that occurred.
   * @return An error code.
   */
  Code code() const { return code_; }

  /**
   * @brief A utility function that casts the code to an unsigned int.
   * @return An error code's unsigned integer value.
   */
  unsigned int codeAsUnsignedInt() const {
    return asUnsignedInt(code_);
  }

  /**
   * @brief Get the message that describes the error details.
   * @return A string describing the error that occurred.
   */
  const char * what() const throw() { return what_.c_str(); }

  /**
   * A utility function that converts a given error code to an unsigned int.
   * @param  code The code to convert.
   * @return      The error code's unsigned integer value.
   */
  static unsigned int asUnsignedInt(Code code) {
    return static_cast<unsigned int>(code);
  }
private:
  Code code_;
  std::string what_;
};
}

#endif
