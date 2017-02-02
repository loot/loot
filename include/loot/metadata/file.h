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
class File : public ConditionalMetadata {
public:
  LOOT_API File();
  LOOT_API File(const std::string& name, const std::string& display = "",
                const std::string& condition = "");

  LOOT_API bool operator < (const File& rhs) const;
  LOOT_API bool operator == (const File& rhs) const;

  LOOT_API std::string GetName() const;
  LOOT_API std::string GetDisplayName() const;
private:
  std::string name_;
  std::string display_;
};
}

#endif
