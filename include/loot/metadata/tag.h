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
#ifndef LOOT_METADATA_TAG
#define LOOT_METADATA_TAG

#include <string>

#include "loot/api_decorator.h"
#include "loot/metadata/conditional_metadata.h"

namespace loot {
class Tag : public ConditionalMetadata {
public:
  LOOT_API Tag();
  LOOT_API Tag(const std::string& tag,
               const bool isAddition = true,
               const std::string& condition = "");

  LOOT_API bool operator < (const Tag& rhs) const;
  LOOT_API bool operator == (const Tag& rhs) const;

  LOOT_API bool IsAddition() const;
  LOOT_API std::string Name() const;
private:
  std::string name_;
  bool addTag_;
};
}

#endif
