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

#ifndef LOOT_GUI_STATE_LOOT_SETTINGS
#define LOOT_GUI_STATE_LOOT_SETTINGS

#include <filesystem>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "gui/state/game/game_settings.h"

namespace loot {
class LootSettings {
public:
  struct WindowPosition {
    WindowPosition();

    long top;
    long bottom;
    long left;
    long right;
    bool maximised;
  };

  struct Language {
    std::string locale;
    std::string name;
    std::optional<std::string> fontFamily;
  };

  struct Filters {
    Filters();

    bool hideVersionNumbers;
    bool hideCRCs;
    bool hideBashTags;
    bool hideNotes;
    bool hideAllPluginMessages;
    bool hideInactivePlugins;
    bool hideMessagelessPlugins;
  };

  LootSettings();

  void load(const std::filesystem::path& file,
            const std::filesystem::path& lootDataPath);
  void save(const std::filesystem::path& file);

  bool shouldAutoSort() const;
  bool isDebugLoggingEnabled() const;
  bool updateMasterlist() const;
  bool isLootUpdateCheckEnabled() const;
  std::string getGame() const;
  std::string getLastGame() const;
  std::string getLastVersion() const;
  std::string getLanguage() const;
  std::string getTheme() const;
  std::string getPreludeRepositoryURL() const;
  std::string getPreludeRepositoryBranch() const;
  std::optional<WindowPosition> getWindowPosition() const;
  const std::vector<GameSettings>& getGameSettings() const;
  const Filters& getFilters() const;
  const std::vector<Language>& getLanguages() const;

  void setDefaultGame(const std::string& game);
  void setLanguage(const std::string& language);
  void setTheme(const std::string& theme);
  void setPreludeRepositoryURL(const std::string& url);
  void setPreludeRepositoryBranch(const std::string& branch);
  void setAutoSort(bool autSort);
  void enableDebugLogging(bool enable);
  void updateMasterlist(bool update);
  void enableLootUpdateCheck(bool enable);

  void storeLastGame(const std::string& lastGame);
  void storeWindowPosition(const WindowPosition& position);
  void storeGameSettings(const std::vector<GameSettings>& gameSettings);
  void storeFilters(const Filters& filters);
  void updateLastVersion();

private:
  bool autoSort_;
  bool enableDebugLogging_;
  bool updateMasterlist_;
  bool enableLootUpdateCheck_;
  std::string game_;
  std::string lastGame_;
  std::string lastVersion_;
  std::string language_;
  std::string preludeRepositoryBranch_;
  std::string preludeRepositoryURL_;
  std::string theme_;
  std::optional<WindowPosition> windowPosition_;
  std::vector<GameSettings> gameSettings_;
  Filters filters_;
  std::vector<Language> languages_;

  mutable std::recursive_mutex mutex_;

  void appendBaseGames();
};
}

#endif
