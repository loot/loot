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
#ifndef LOOT_METADATA_LOCATION
#define LOOT_METADATA_LOCATION

#include <string>
#include <vector>

#include "loot/api_decorator.h"

namespace loot {
/**
 * Represents a URL at which the parent plugin can be found.
 */
class Location {
public:
  /**
   * Construct a Location with empty URL and name strings.
   * @return A Location object.
   */
  LOOT_API Location();

  /**
   * Construct a Location with the given URL and name.
   * @param  url
   *         The URL at which the plugin can be found.
   * @param  name
   *         A name for the URL, eg. the page or site name.
   * @return A Location object.
   */
  LOOT_API Location(const std::string& url, const std::string& name = "");

  /**
   * A less-than operator implemented with no semantics so that Location objects
   * can be stored in sets.
   * @returns True if this Location's URL is case-insensitively
   *          lexicographically less than the given Location's URL, false
   *          otherwise.
   */
  LOOT_API bool operator < (const Location& rhs) const;

  /**
   * Check if two Location objects are equal by comparing their URLs.
   * @returns True if the URLs are case-insensitively equal, false otherwise.
   */
  LOOT_API bool operator == (const Location& rhs) const;

  /**
   * Get the object's URL.
   * @return A URL string.
   */
  LOOT_API std::string GetURL() const;

  /**
   * Get the object's name.
   * @return The name of the location.
   */
  LOOT_API std::string GetName() const;
private:
  std::string url_;
  std::string name_;
};
}

#endif
