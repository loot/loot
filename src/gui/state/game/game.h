/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2018    WrinklyNinja

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

#ifndef LOOT_GUI_STATE_GAME_GAME
#define LOOT_GUI_STATE_GAME_GAME

#define FMT_NO_FMT_STRING_ALIAS

#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_set>

#include <spdlog/spdlog.h>

#include "gui/state/game/game_settings.h"
#include "loot/api.h"

namespace loot {
namespace gui {
class Game : public GameSettings {
public:
  Game(const GameSettings& gameSettings,
       const std::filesystem::path& lootDataPath);
  Game(const Game& game);

  Game& operator=(const Game& game);

  using GameSettings::Type;

  static bool IsInstalled(const GameSettings& gameSettings);
  static Message ToMessage(const PluginCleaningData& cleaningData);
  void Init();

  std::shared_ptr<const PluginInterface> GetPlugin(
      const std::string& name) const;
  std::set<std::shared_ptr<const PluginInterface>> GetPlugins() const;
  std::vector<Message> CheckInstallValidity(
      const std::shared_ptr<const PluginInterface>& plugin,
      const PluginMetadata& metadata);

  void RedatePlugins();  // Change timestamps to match load order (Skyrim only).

  void LoadAllInstalledPlugins(
      bool headersOnly);  // Loads all installed plugins.
  bool ArePluginsFullyLoaded()
      const;  // Checks if the game's plugins have already been loaded.

  std::filesystem::path DataPath() const;
  std::filesystem::path MasterlistPath() const;
  std::filesystem::path UserlistPath() const;

  std::vector<std::string> GetLoadOrder() const;
  void SetLoadOrder(const std::vector<std::string>& loadOrder);

  bool IsPluginActive(const std::string& pluginName) const;
  std::optional<short> GetActiveLoadOrderIndex(
      const std::shared_ptr<const PluginInterface>& plugin,
      const std::vector<std::string>& loadOrder) const;

  std::vector<std::string> SortPlugins();
  void IncrementLoadOrderSortCount();
  void DecrementLoadOrderSortCount();

  std::vector<Message> GetMessages() const;
  void AppendMessage(const Message& message);
  void ClearMessages();

  bool UpdateMasterlist();
  MasterlistInfo GetMasterlistInfo() const;

  void LoadMetadata();
  std::set<std::string> GetKnownBashTags() const;

  std::unordered_set<Group> GetMasterlistGroups() const;
  std::unordered_set<Group> GetUserGroups() const;

  std::optional<PluginMetadata> GetMasterlistMetadata(const std::string& pluginName,
                                       bool evaluateConditions = false) const;
  std::optional<PluginMetadata> GetUserMetadata(const std::string& pluginName,
                                 bool evaluateConditions = false) const;

  void SetUserGroups(const std::unordered_set<Group>& groups);
  void AddUserMetadata(const PluginMetadata& metadata);
  void ClearUserMetadata(const std::string& pluginName);
  void ClearAllUserMetadata();
  void SaveUserMetadata();

private:
#ifdef _WIN32
  static std::string RegKeyStringValue(const std::string& keyStr,
                                       const std::string& subkey,
                                       const std::string& value);
#endif
  static bool ExecutableExists(const GameType& gameType,
                               const std::filesystem::path& gamePath);
  static std::optional<std::filesystem::path> DetectGamePath(
      const GameSettings& gameSettings);
  static void BackupLoadOrder(const std::vector<std::string>& loadOrder,
                              const std::filesystem::path& backupDirectory);
  std::vector<std::string> GetInstalledPluginNames();
  void warnAboutRemovedPlugins(const std::vector<std::string> pluginsBefore,
                               const std::vector<std::string> pluginsAfter);

  std::filesystem::path lootDataPath_;

  std::shared_ptr<GameInterface> gameHandle_;
  bool pluginsFullyLoaded_;

  std::vector<Message> messages_;
  unsigned short loadOrderSortCount_;

  std::shared_ptr<spdlog::logger> logger_;

  mutable std::mutex mutex_;
};
}
}

#endif
