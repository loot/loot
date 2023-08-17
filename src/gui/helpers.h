/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2014 WrinklyNinja

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
#ifndef LOOT_GUI_HELPERS
#define LOOT_GUI_HELPERS

#include <loot/enum/message_type.h>

#include <filesystem>
#include <optional>
#include <vector>

#include "gui/sourced_message.h"

namespace loot {
void OpenInDefaultApplication(const std::filesystem::path& file);

#ifdef _WIN32
std::wstring ToWinWide(const std::string& str);

std::string FromWinWide(const std::wstring& wstr);
#endif

std::vector<std::string> GetPreferredUILanguages();

std::vector<std::filesystem::path> GetDriveRootPaths();

std::optional<std::filesystem::path> FindXboxGamingRootPath(
    const std::filesystem::path& driveRootPath);

// Compare strings as if they're filenames, respecting filesystem case
// insensitivity on Windows. Returns -1 if lhs < rhs, 0 if lhs == rhs, and 1 if
// lhs > rhs. The comparison may give different results on Linux, but is still
// locale-invariant.
int CompareFilenames(const std::string& lhs, const std::string& rhs);

std::filesystem::path getExecutableDirectory();

std::filesystem::path getLocalAppDataPath();

std::string crcToString(uint32_t crc);
}
#endif
