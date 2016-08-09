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

#ifndef LOOT_BACKEND_GAME_LOAD_ORDER_HANDLER
#define LOOT_BACKEND_GAME_LOAD_ORDER_HANDLER

#include <list>
#include <string>
#include <unordered_set>

#include <boost/filesystem.hpp>
#include <libloadorder/libloadorder.h>

#include "backend/game/game_settings.h"

namespace loot {
class LoadOrderHandler {
public:
  LoadOrderHandler();
  ~LoadOrderHandler();

  void Init(const GameSettings& game, const boost::filesystem::path& gameLocalAppData = "");

  std::vector<std::string> GetLoadOrder() const;

  bool IsPluginActive(const std::string& pluginName) const;

  //These modify game load order, even though const.
  void SetLoadOrder(const char * const * const loadOrder, const size_t numPlugins) const;  // For API.
  void SetLoadOrder(const std::vector<std::string>& loadOrder) const;
private:
  lo_game_handle gh_;
};
}

#endif
