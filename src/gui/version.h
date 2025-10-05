/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2017 WrinklyNinja

    This file is part of the LOOT metadata validator.

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

#ifndef LOOT_GUI_VERSION
#define LOOT_GUI_VERSION

#include <string>

namespace loot {
inline constexpr unsigned int LOOT_VERSION_MAJOR = 0;
inline constexpr unsigned int LOOT_VERSION_MINOR = 26;
inline constexpr unsigned int LOOT_VERSION_PATCH = 1;

std::string getLootVersion();

std::string getLootRevision();
}

#endif
