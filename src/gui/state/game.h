/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2016    WrinklyNinja

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

#ifndef LOOT_GUI_STATE_GAME
#define LOOT_GUI_STATE_GAME

#include <string>

#include <boost/filesystem.hpp>

#include "gui/state/game_settings.h"
#include "loot/api.h"

namespace loot {
namespace gui {
class Game : public GameSettings {
public:
  Game(const GameSettings& gameSettings,
       const boost::filesystem::path& lootDataPath,
       const boost::filesystem::path& localDataPath = "");

  using GameSettings::Type;

  static bool IsInstalled(const GameSettings& gameSettings);
  void Init();

  std::shared_ptr<const PluginInterface> GetPlugin(const std::string& name) const;
  std::set<std::shared_ptr<const PluginInterface>> GetPlugins() const;

  void RedatePlugins();  //Change timestamps to match load order (Skyrim only).

  void LoadAllInstalledPlugins(bool headersOnly);  //Loads all installed plugins.
  bool ArePluginsFullyLoaded() const;  // Checks if the game's plugins have already been loaded.

  boost::filesystem::path DataPath() const;
  boost::filesystem::path MasterlistPath() const;
  boost::filesystem::path UserlistPath() const;

  std::vector<std::string> GetLoadOrder() const;
  void SetLoadOrder(const std::vector<std::string>& loadOrder);

  short GetActiveLoadOrderIndex(const std::string & pluginName) const;
  short GetActiveLoadOrderIndex(const std::string & pluginName, const std::vector<std::string>& loadOrder) const;
private:
#ifdef _WIN32
  static std::string RegKeyStringValue(const std::string& keyStr, const std::string& subkey, const std::string& value);
#endif
  static boost::filesystem::path DetectGamePath(const GameSettings& gameSettings);
  static void BackupLoadOrder(const std::vector<std::string>& loadOrder,
                              const boost::filesystem::path& backupDirectory);

  const boost::filesystem::path lootDataPath_;

  std::shared_ptr<GameInterface> gameHandle_;
  bool pluginsFullyLoaded_;
};
}
}

#endif
