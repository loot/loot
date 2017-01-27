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

#ifndef LOOT_BACKEND_GAME_GAME
#define LOOT_BACKEND_GAME_GAME

#include <string>

#include <boost/filesystem.hpp>

#include "backend/game/game_cache.h"
#include "backend/game/game_settings.h"
#include "backend/game/load_order_handler.h"

namespace loot {
class Game : public GameSettings, public GameCache {
public:
  Game(const GameSettings& gameSettings, 
       const boost::filesystem::path& lootDataPath,
       const boost::filesystem::path& localDataPath = "");

  bool IsInstalled();  //Sets gamePath if the current value is not valid and a valid path is found.
  void Init();

  void RedatePlugins();  //Change timestamps to match load order (Skyrim only).

  void LoadPlugins(const std::vector<std::string>& plugins, bool headersOnly);
  void LoadAllInstalledPlugins(bool headersOnly);  //Loads all installed plugins.
  bool ArePluginsFullyLoaded() const;  // Checks if the game's plugins have already been loaded.

  // Check if the plugin is active by using the cached value if
  // available, and otherwise asking the load order handler.
  bool IsPluginActive(const std::string& pluginName) const;
  short GetActiveLoadOrderIndex(const std::string & pluginName) const;
  short GetActiveLoadOrderIndex(const std::string & pluginName, const std::vector<std::string>& loadOrder) const;

  std::vector<std::string> GetLoadOrder() const;
  void SetLoadOrder(const std::vector<std::string>& loadOrder) const;

  boost::filesystem::path MasterlistPath() const;
  boost::filesystem::path UserlistPath() const;
private:
#ifdef _WIN32
  std::string RegKeyStringValue(const std::string& keyStr, const std::string& subkey, const std::string& value);
#endif

  const boost::filesystem::path lootDataPath_;
  const boost::filesystem::path localDataPath_;

  bool pluginsFullyLoaded_;
  LoadOrderHandler loadOrderHandler_;
  mutable std::vector<std::string> loadOrder_;
};
}

#endif
