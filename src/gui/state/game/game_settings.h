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

#ifndef LOOT_GUI_STATE_GAME_GAME_SETTINGS
#define LOOT_GUI_STATE_GAME_GAME_SETTINGS

#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "gui/state/game/detection/game_install.h"
#include "loot/enum/game_type.h"

namespace loot {
static constexpr const char* MASTERLIST_FILENAME = "masterlist.yaml";

std::string GetPluginsFolderName(GameId gameId);

std::string ToString(const GameId gameId);

bool ShouldAllowRedating(const GameType gameType);

class GameSettings {
public:
  GameSettings() = default;
  explicit GameSettings(const GameId gameId, const std::string& lootFolder);

  GameId Id() const;
  GameType Type() const;
  std::string Name() const;  // Returns the game's name, eg. "TES IV: Oblivion".
  std::string FolderName() const;
  std::string Master() const;
  float MinimumHeaderVersion() const;
  std::string MasterlistSource() const;
  std::filesystem::path GamePath() const;
  std::filesystem::path GameLocalPath() const;
  std::filesystem::path DataPath() const;

  std::string PluginsFolderName() const;

  GameSettings& SetName(const std::string& name);
  GameSettings& SetMaster(const std::string& masterFile);
  GameSettings& SetMinimumHeaderVersion(float minimumHeaderVersion);
  GameSettings& SetMasterlistSource(const std::string& source);
  GameSettings& SetGamePath(const std::filesystem::path& path);
  GameSettings& SetGameLocalPath(const std::filesystem::path& GameLocalPath);
  GameSettings& SetGameLocalFolder(const std::string& folderName);

private:
  GameId id_{GameId::tes4};
  GameType type_{GameType::tes4};
  std::string name_;
  std::string masterFile_;
  float minimumHeaderVersion_{0.0f};

  std::string lootFolderName_;

  std::string masterlistSource_;

  std::filesystem::path gamePath_;  // Path to the game's folder.
  std::filesystem::path gameLocalPath_;
};
}

#endif
