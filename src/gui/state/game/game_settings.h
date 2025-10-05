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

#include "gui/state/game/game_id.h"

namespace loot {
static constexpr const char* MASTERLIST_FILENAME = "masterlist.yaml";
static constexpr const char* DEFAULT_MASTERLIST_BRANCH = "v0.26";

std::string getDefaultMasterlistUrl(const std::string& repositoryName);

std::string getDefaultMasterlistUrl(const GameId gameId);

struct HiddenMessage {
  std::optional<std::string> pluginName;
  std::string text;
};

bool operator==(const HiddenMessage& lhs, const HiddenMessage& rhs);

class GameSettings {
public:
  GameSettings() = default;
  GameSettings(const GameId gameId, const std::string& lootFolder);

  GameId id() const;
  std::string name() const;  // Returns the game's name, eg. "TES IV: Oblivion".
  std::string folderName() const;
  std::string master() const;
  float minimumHeaderVersion() const;
  std::string masterlistSource() const;
  std::filesystem::path gamePath() const;
  std::filesystem::path gameLocalPath() const;
  std::filesystem::path dataPath() const;

  const std::vector<HiddenMessage>& hiddenMessages() const;

  bool pluginHasHiddenMessages(const std::string& pluginName) const;

  GameSettings& setName(const std::string& name);
  GameSettings& setMaster(const std::string& masterFile);
  GameSettings& setMinimumHeaderVersion(float minimumHeaderVersion);
  GameSettings& setMasterlistSource(const std::string& source);
  GameSettings& setGamePath(const std::filesystem::path& path);
  GameSettings& setGameLocalPath(const std::filesystem::path& gameLocalPath);
  GameSettings& setGameLocalFolder(const std::string& folderName);

  GameSettings& setHiddenMessages(
      const std::vector<HiddenMessage>& hiddenMessages);

  void hideMessage(const std::string& pluginName,
                   const std::string& messageText);

  bool hasHiddenGeneralMessages();

private:
  GameId id_{GameId::tes4};
  std::string name_;
  std::string masterFile_;
  float minimumHeaderVersion_{0.0f};

  std::string lootFolderName_;

  std::string masterlistSource_;

  std::filesystem::path gamePath_;
  std::filesystem::path gameLocalPath_;

  std::vector<HiddenMessage> hiddenMessages_;
};
}

#endif
