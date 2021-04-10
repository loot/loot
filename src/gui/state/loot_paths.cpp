/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2012 WrinklyNinja

    This file is part of LOOT.

    LOOT is free software : you can redistribute
    it and / or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    LOOT is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with LOOT.If not, see
    <https://www.gnu.org/licenses/>.
*/

#include "loot_paths.h"

#include <locale>

#ifdef _WIN32
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <shlobj.h>
#include <windows.h>
#endif

#include <boost/locale.hpp>

#include "gui/helpers.h"
#include "gui/state/logging.h"
#include "loot/api.h"

namespace loot {
LootPaths::LootPaths(const std::filesystem::path& lootAppPath,
                     const std::filesystem::path& lootDataPath) {
  // Set the locale to get UTF-8 conversions working correctly.
  std::locale::global(boost::locale::generator().generate(""));

  if (lootAppPath.empty()) {
    lootAppPath_ = getExecutableDirectory();
  } else {
    lootAppPath_ = lootAppPath;
  }

  if (!lootDataPath.empty())
    lootDataPath_ = lootDataPath;
  else
    lootDataPath_ = getLocalAppDataPath() / "LOOT";
}

std::filesystem::path LootPaths::getReadmePath() const {
  return lootAppPath_ / "docs";
}

std::filesystem::path LootPaths::getResourcesPath() const {
  return lootAppPath_ / "resources";
}

std::filesystem::path LootPaths::getL10nPath() const {
  return getResourcesPath() / "l10n";
}

std::filesystem::path LootPaths::getLootDataPath() const {
  return lootDataPath_;
}

std::filesystem::path LootPaths::getSettingsPath() const {
  return lootDataPath_ / "settings.toml";
}

std::filesystem::path LootPaths::getLogPath() const {
  return lootDataPath_ / "LOOTDebugLog.txt";
}
}
