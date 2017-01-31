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
#ifndef LOOT_METADATA_PRIORITY
#define LOOT_METADATA_PRIORITY

#include <cstdint>

#include "loot/api_decorator.h"

namespace loot {
class Priority {
public:
  LOOT_API Priority();
  // Take an int to prevent literals that are too large for one byte from
  // wrapping around to negative values.
  LOOT_API explicit Priority(const int value);

  // Doesn't return an int8_t because it is commonly signed char, which
  // yaml-cpp interprets as a character rather than an integer.
  LOOT_API short getValue() const;
  LOOT_API bool isExplicit() const;

  LOOT_API bool operator < (const Priority& rhs) const;
  LOOT_API bool operator > (const Priority& rhs) const;
  LOOT_API bool operator >= (const Priority& rhs) const;
  LOOT_API bool operator == (const Priority& rhs) const;

  LOOT_API bool operator > (const uint8_t rhs) const;

private:
  bool isExplicitZeroValue_;
  int8_t value_;
};
}

#endif
