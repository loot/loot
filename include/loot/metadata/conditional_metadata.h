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
#ifndef LOOT_METADATA_CONDITIONAL_METADATA
#define LOOT_METADATA_CONDITIONAL_METADATA

#include <string>

#include "loot/api_decorator.h"

namespace loot {
class ConditionalMetadata {
public:
  LOOT_API ConditionalMetadata();
  LOOT_API ConditionalMetadata(const std::string& condition);

  LOOT_API bool IsConditional() const;
  LOOT_API void ParseCondition() const;

  LOOT_API std::string GetCondition() const;
private:
  std::string condition_;
};
}
#endif
