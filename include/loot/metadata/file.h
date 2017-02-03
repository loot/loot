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
#ifndef LOOT_METADATA_FILE
#define LOOT_METADATA_FILE

#include <string>

#include "loot/api_decorator.h"
#include "loot/metadata/conditional_metadata.h"

namespace loot {
/**
 * Represents a file in a game's Data folder, including files in subdirectories.
 */
class File : public ConditionalMetadata {
public:
  /**
   * Construct a File with blank name, display and condition strings.
   * @return A File object.
   */
  LOOT_API File();

  /**
   * Construct a File with the given name, display name and condition strings.
   * @param  name
   *         The filename of the file.
   * @param  display
   *         The name to be displayed for the file in messages.
   * @param  condition
   *         The File's condition string.
   * @return A File object.
   */
  LOOT_API File(const std::string& name, const std::string& display = "",
                const std::string& condition = "");

  /**
   * A less-than operator implemented with no semantics so that File objects can
   * be stored in sets.
   * @returns True if this File's name is case-insensitively lexicographically
   *          less than the given File's name, false otherwise.
   */
  LOOT_API bool operator < (const File& rhs) const;

  /**
   * Check if two File objects are equal by comparing their filenames.
   * @returns True if the filenames are case-insensitively equal, false
   *          otherwise.
   */
  LOOT_API bool operator == (const File& rhs) const;

  /**
   * Get the filename of the file.
   * @return The file's filename.
   */
  LOOT_API std::string GetName() const;

  /**
   * Get the display name of the file.
   * @return The file's display name.
   */
  LOOT_API std::string GetDisplayName() const;
private:
  std::string name_;
  std::string display_;
};
}

#endif
