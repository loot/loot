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

#ifndef LOOT_GUI_QUERY_REDATE_PLUGINS_QUERY
#define LOOT_GUI_QUERY_REDATE_PLUGINS_QUERY

#include "gui/cef/query/query.h"
#include "gui/state/game/game.h"

namespace loot {
template<typename G = gui::Game>
class RedatePluginsQuery : public Query {
public:
  RedatePluginsQuery(G& game) :
      game_(game) {}

  std::string executeLogic() {
    game_.RedatePlugins();
    return "";
  }

private:
  G& game_;
};
}

#endif
