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

#ifndef LOOT_GUI_STATE_LOOT_PATHS
#define LOOT_GUI_STATE_LOOT_PATHS

#include <filesystem>

namespace loot {
class LootPaths {
public:
  LootPaths(const std::filesystem::path& lootAppPath, 
            const std::filesystem::path& lootDataPath);

  std::filesystem::path getReadmePath() const;
  std::filesystem::path getResourcesPath() const;
  std::filesystem::path getL10nPath() const;
  std::filesystem::path getLootDataPath() const;
  std::filesystem::path getSettingsPath() const;
  std::filesystem::path getLogPath() const;

private:
  std::filesystem::path lootAppPath_;
  std::filesystem::path lootDataPath_;
};
}

#endif
