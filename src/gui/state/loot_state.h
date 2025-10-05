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

#ifndef LOOT_GUI_STATE_LOOT_STATE
#define LOOT_GUI_STATE_LOOT_STATE

#include "gui/state/change_count.h"
#include "gui/state/game/games_manager.h"
#include "gui/state/loot_settings.h"

namespace loot {
class LootState : public GamesManager {
public:
  LootState(LootPaths&& paths);

  void init(const std::string& cmdLineGame,
            const std::filesystem::path& cmdLineGamePath,
            bool autoSort);
  void initCurrentGame();

  std::vector<GameSettings> loadInstalledGames(
      const std::vector<GameSettings>& gamesSettings);

  const std::vector<SourcedMessage>& getInitMessages() const;

  const LootSettings& getSettings() const;
  LootSettings& getSettings();

  ChangeCount& getUnappliedChangeCount();

  const LootPaths& getPaths() const;

private:
  void createLootDataPath();
  void loadSettings(const std::string& cmdLineGame, bool autoSort);
  void checkSettingsFile();
  void findXboxGamingRootPaths();
  void createPreludeDirectory();
  void overrideGamePath(const std::string& gameFolderName,
                        const std::filesystem::path& gamePath);
  void setInitialGame(const std::string& cliGameValue);

  bool isInstalled(const GameSettings& gameSettings) const override;

  void initialiseGameData(gui::Game& game) override;

  std::optional<std::string> getPreferredGameFolderName(
      const std::string& cliGameValue) const;

  LootPaths paths_;
  std::vector<std::filesystem::path> xboxGamingRootPaths_;
  std::vector<std::string> preferredUILanguages_;
  std::vector<SourcedMessage> initMessages_;
  LootSettings settings_;
  ChangeCount unappliedChangeCount_;
};
}

#endif
