/*  LOOT

    A modding utility for Starfield and some Elder Scrolls and Fallout games.

    Copyright (C) 2013-2026 Oliver Hamlet
    Copyright (C) 2021 Daniel Coelho
    Copyright (C) 2021 IllusiveMan196
    Copyright (C) 2021 sibir-ine
    Copyright (C) 2024 Ilker Binzet

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
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "gui/state/game/game_settings.h"

namespace loot {
extern const std::string DEFAULT_PRELUDE_SOURCE;

struct CheckSettingsResult {
  bool preludeMigrationFailed{false};
  bool masterlistMigrationFailed{false};
};

CheckSettingsResult checkSettingsFile(const std::filesystem::path& filePath);

struct WindowPosition {
  int top{0};
  int bottom{0};
  int left{0};
  int right{0};
  bool maximised{false};
};

struct Language {
  std::string locale;
  std::string name;
};

struct FilterSettings {
  bool hideVersionNumbers{false};
  bool hideCRCs{false};
  bool hideBashTags{true};
  bool hideLocations{true};
  bool hideNotes{false};
  bool hideOfficialPluginsCleaningMessages{false};
  bool hideAllPluginMessages{false};
  bool hideInactivePlugins{false};
  bool hideMessagelessPlugins{false};
  bool hideCreationClubPlugins{false};
  bool showOnlyNewMessages{false};
  bool showOnlyEmptyPlugins{false};
  bool showOnlyPluginsWithLoadAfterMetadata{false};
  bool showOnlyPluginsWithLoadAfterUserMetadata{false};
  bool showOnlyPluginsWithoutLoadOrderMetadata{false};
};

struct GeneralSettings {
  bool enableDebugLogging{false};
  bool updateMasterlistBeforeSort{true};
  bool enableLootUpdateCheck{true};
  bool useNoSortingChangesDialog{true};
  bool warnOnCaseSensitiveGamePaths{true};
  std::string defaultGame{"auto"};
  std::string language{"en"};
  std::string preludeSource{DEFAULT_PRELUDE_SOURCE};
  std::string theme{"default"};
};

class LootSettings {
public:
  void load(const std::filesystem::path& file,
            const std::vector<std::string>& preferredUILanguages);
  void save(const std::filesystem::path& file);

  bool isAutoSortEnabled() const;
  bool isDebugLoggingEnabled() const;
  bool isMasterlistUpdateBeforeSortEnabled() const;
  bool isLootUpdateCheckEnabled() const;
  bool isNoSortingChangesDialogEnabled() const;
  bool isWarnOnCaseSensitiveGamePathsEnabled() const;
  std::string getDefaultGame() const;
  std::string getLastGame() const;
  std::string getLastVersion() const;
  std::string getLanguage() const;
  std::string getTheme() const;
  std::string getPreludeSource() const;
  std::optional<WindowPosition> getMainWindowPosition() const;
  std::optional<WindowPosition> getGroupsEditorWindowPosition() const;
  std::optional<WindowPosition> getCompareLoadOrdersWindowPosition() const;
  const std::vector<GameSettings>& getGameSettings() const;
  const FilterSettings& getFilters() const;
  const std::vector<Language>& getLanguages() const;

  void setDefaultGame(const std::string& gameFolderName);
  void setLanguage(const std::string& languageLocaleCode);
  void setTheme(const std::string& theme);
  void setPreludeSource(const std::string& source);
  void enableAutoSort(bool enable);
  void enableDebugLogging(bool enable);
  void enableMasterlistUpdateBeforeSort(bool enable);
  void enableLootUpdateCheck(bool enable);
  void enableNoSortingChangesDialog(bool enable);
  void enableWarnOnCaseSensitiveGamePaths(bool enable);

  void storeGeneralSettings(const GeneralSettings& generalSettings);
  void storeLastGame(const std::string& lastGame);
  void storeMainWindowPosition(const WindowPosition& position);
  void storeGroupsEditorWindowPosition(const WindowPosition& position);
  void storeCompareLoadOrdersWindowPosition(const WindowPosition& position);
  void storeGameSettings(const std::vector<GameSettings>& gameSettings);
  void storeFilters(const FilterSettings& filters);
  void updateLastVersion();

private:
  bool autoSort_{false};
  std::string lastGame_{"auto"};
  std::string lastVersion_;
  GeneralSettings generalSettings_;
  std::optional<WindowPosition> mainWindowPosition_;
  std::optional<WindowPosition> groupsEditorWindowPosition_;
  std::optional<WindowPosition> compareLoadOrdersWindowPosition_;
  std::vector<GameSettings> gameSettings_;
  FilterSettings filters_;
  std::vector<Language> languages_{
      Language({"en", "English"}),
      Language({"bg", "Български"}),
      Language({"cs", "Čeština"}),
      Language({"da", "Dansk"}),
      Language({"de", "Deutsch"}),
      Language({"es", "Español"}),
      Language({"fi", "Suomi"}),
      Language({"fr", "Français"}),
      Language({"it", "Italiano"}),
      Language({"ja", "日本語"}),
      Language({"ko", "한국어"}),
      Language({"pl", "Polski"}),
      Language({"pt_BR", "Português do Brasil"}),
      Language({"pt_PT", "Português de Portugal"}),
      Language({"ru", "Русский"}),
      Language({"sv", "Svenska"}),
      Language({"ta", "தமிழ்"}),
      Language({"tr_TR", "Türkçe"}),
      Language({"uk_UA", "Українська"}),
      Language({"zh_CN", "简体中文"}),
  };

  mutable std::recursive_mutex mutex_;
};

// Find the first of the given preferred UI languages that matches one of the
// given supported languages. Preferred UI languages may be given as BCP 47
// language tags (e.g. pt-BR) or as POSIX locale names (e.g. pt_BR.UTF-8).
std::optional<std::string> findPreferredLanguage(
    const std::vector<std::string>& preferredUILanguages,
    const std::vector<Language>& supportedLanguages);
}

#endif
