/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2026 Oliver Hamlet

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
#ifndef LOOT_GUI_TRANSLATE
#define LOOT_GUI_TRANSLATE

#include <string>

namespace loot {
// Can't take a string_view because boost::locale::translate only accepts C
// strings and std::string.
std::string translate(const char* text);

std::string translate(const char* singularText,
                      const char* pluralText,
                      unsigned int count);

std::string translate(const char* singularText,
                      const char* pluralText,
                      size_t count);
}

#endif
