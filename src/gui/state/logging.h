/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2012 WrinklyNinja

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
#ifndef LOOT_GUI_STATE_LOGGING
#define LOOT_GUI_STATE_LOGGING

#ifdef _MSC_VER
// Qt typedef's a uint type in the global namespace that spdlog shadows, just
// disable the warning.
#pragma warning(disable : 4459)
#include <spdlog/spdlog.h>
#pragma warning(default : 4459)
#else
#include <spdlog/spdlog.h>
#endif

#include <filesystem>

namespace loot {
std::shared_ptr<spdlog::logger> getLogger();

void setLogPath(const std::filesystem::path& outputFile);

void enableDebugLogging(bool enable);
}

#endif
