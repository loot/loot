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
class Location {
public:
  LOOT_API Location();
  LOOT_API Location(const std::string& url, const std::string& name = "");

  LOOT_API bool operator < (const Location& rhs) const;
  LOOT_API bool operator == (const Location& rhs) const;

  LOOT_API std::string URL() const;
  LOOT_API std::string Name() const;
private:
  std::string url_;
  std::string name_;
};
}

#endif
