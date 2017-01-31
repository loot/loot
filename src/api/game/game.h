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

#include "api/game/game_cache.h"
#include "api/game/load_order_handler.h"

namespace loot {
class Game : public GameCache {
public:
  Game(const GameType gameType,
       const boost::filesystem::path& gamePath,
       const boost::filesystem::path& localDataPath = "");

  GameType Type() const;
  boost::filesystem::path DataPath() const;
  std::string GetArchiveFileExtension() const;

  void Init();

  void LoadPlugins(const std::vector<std::string>& plugins, const std::string& masterFile, bool headersOnly);

  bool IsPluginActive(const std::string& pluginName) const;

  std::vector<std::string> GetLoadOrder() const;
  void SetLoadOrder(const std::vector<std::string>& loadOrder);
private:
  const GameType type_;
  const boost::filesystem::path gamePath_;
  const boost::filesystem::path localDataPath_;

  LoadOrderHandler loadOrderHandler_;
};
}

#endif
