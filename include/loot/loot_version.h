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

/* set up dll import/export decorators
when compiling the dll on windows, ensure LOOT_EXPORT is defined. clients
that use this header do not need to define anything to import the symbols
properly. */
#if defined(_WIN32)
#   ifdef LOOT_STATIC
#       define LOOT_API
#   elif defined LOOT_EXPORT
#       define LOOT_API __declspec(dllexport)
#   else
#       define LOOT_API __declspec(dllimport)
#   endif
#else
#   define LOOT_API
#endif

namespace loot {
class LootVersion {
public:
  LOOT_API static const unsigned int major;
  LOOT_API static const unsigned int minor;
  LOOT_API static const unsigned int patch;
  LOOT_API static const std::string revision;

  LOOT_API static std::string string();
};
}

#endif
