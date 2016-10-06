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

#ifndef LOOT_MESSAGE_TYPE
#define LOOT_MESSAGE_TYPE

/**
 * The namespace used by the LOOT API.
 */
namespace loot {
/** @brief Codes used to indicate the type of a message. */
enum struct MessageType : unsigned int {
  /** A notification message that is of no significant severity. */
  say,
  /**
   * A warning message, used to indicate that an issue may be present that the
   * user may wish to act on.
   */
  warn,
  /**
   * An error message, used to indicate that an issue that requires user action
   * is present.
   */
  error,
};
}

#endif
