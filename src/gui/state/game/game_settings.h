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

#include "loot/enum/game_type.h"

namespace loot {
inline constexpr std::string_view NEHRIM_STEAM_REGISTRY_KEY =
    "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App "
    "1014940\\InstallLocation";
static constexpr const char* DEFAULT_MASTERLIST_BRANCH = "v0.18";

std::string GetPluginsFolderName(GameType gameType);

std::string GetDefaultMasterlistUrl(std::string repoName);

std::string ToString(const GameType gameType);

class GameSettings {
public:
  GameSettings() = default;
  explicit GameSettings(const GameType gameType,
                        const std::string& lootFolder = "");

  bool operator==(
      const GameSettings& rhs) const;  // Compares names and folder names.

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
  GameType type_{GameType::tes4};
  std::string name_;
  std::string masterFile_;
  float mininumHeaderVersion_{0.0f};

  std::string lootFolderName_;

  std::string masterlistSource_;

  std::filesystem::path gamePath_;  // Path to the game's folder.
  std::filesystem::path gameLocalPath_;
};
}

#endif
