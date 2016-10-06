/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2016    WrinklyNinja

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

#ifndef LOOT_LOOT_VERSION
#define LOOT_LOOT_VERSION

#include <string>

#include "loot/api_decorator.h"

namespace loot {
/**
 * @brief A purely static class that provides information about the version of
 *        the LOOT API that is being run.
 */
class LootVersion {
public:
  /** @brief The major version number. */
  LOOT_API static const unsigned int major;

  /** @brief The minor version number. */
  LOOT_API static const unsigned int minor;

  /** @brief The patch version number. */
  LOOT_API static const unsigned int patch;

  /** @brief The source control revision that the API was built from. */
  LOOT_API static const std::string revision;

  /**
   * @brief Get the API version as a string.
   * @return A string of the form "major.minor.patch".
   */
  LOOT_API static std::string string();
};
}

#endif
