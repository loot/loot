/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013-2016    WrinklyNinja

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

#include "api/game.h"

#include "api/api_database.h"
#include "api/plugin/plugin_sorter.h"

namespace loot {
namespace api {
Game::Game(const GameType gameType,
           const std::string& gamePath,
           const std::string& gameLocalDataPath)
           : game_(gameType, gamePath, gameLocalDataPath) {
  game_.Init();

  database_ = std::make_shared<ApiDatabase>(game_);
}

std::shared_ptr<DatabaseInterface> Game::GetDatabase() {
  return database_;
}

bool Game::IsValidPlugin(const std::string& plugin) {
  return Plugin::IsValid(plugin, game_);
}

void Game::LoadPlugins(const std::vector<std::string>& plugins, bool loadHeadersOnly) {
  game_.LoadPlugins(plugins, masterFile_, loadHeadersOnly);
}

std::shared_ptr<const PluginInterface> Game::GetPlugin(const std::string& pluginName) {
  return std::static_pointer_cast<const PluginInterface>(game_.GetPlugin(pluginName));
}

std::set<std::shared_ptr<const PluginInterface>> Game::GetLoadedPlugins() {
  auto pointers = game_.GetPlugins();
  std::set<std::shared_ptr<const PluginInterface>> interfacePointers;
  for (auto& plugin : game_.GetPlugins()) {
    interfacePointers.insert(std::static_pointer_cast<const PluginInterface>(plugin));
  }

  return interfacePointers;
}

void Game::IdentifyMainMasterFile(const std::string& masterFile) {
  masterFile_ = masterFile;
}

std::vector<std::string> Game::SortPlugins(const std::vector<std::string>& plugins) {
  game_.LoadPlugins(plugins, masterFile_, false);

  //Sort plugins into their load order.
  PluginSorter sorter;
  return sorter.Sort(game_);
}

bool Game::IsPluginActive(const std::string& plugin) {
  return game_.IsPluginActive(plugin);
}

std::vector<std::string> Game::GetLoadOrder() {
  return game_.GetLoadOrder();
}

void Game::SetLoadOrder(const std::vector<std::string>& loadOrder) {
  game_.SetLoadOrder(loadOrder);
}
}
}
