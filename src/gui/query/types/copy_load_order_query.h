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

#ifndef LOOT_GUI_QUERY_COPY_LOAD_ORDER_QUERY
#define LOOT_GUI_QUERY_COPY_LOAD_ORDER_QUERY

#include <iomanip>

#include "gui/helpers.h"
#include "gui/query/query.h"

namespace loot {
struct Counters {
  size_t activeNormal = 0;
  size_t activeLightPlugins = 0;
};

template<typename G = gui::Game>
class CopyLoadOrderQuery : public Query {
public:
  CopyLoadOrderQuery(const G& game, const std::vector<std::string>& plugins) :
      game_(game), plugins_(plugins) {}

  QueryResult executeLogic() override {
    Counters counters;
    std::stringstream stream;
    for (const auto& pluginName : plugins_) {
      writePluginLine(stream, pluginName, counters);
    }

    CopyToClipboard(stream.str());
    return std::monostate();
  }

private:
  void writePluginLine(std::ostream& stream,
                       const std::string& pluginName,
                       Counters& counters) {
    auto plugin = game_.GetPlugin(pluginName);
    if (!plugin) {
      return;
    }

    const auto isActive = game_.IsPluginActive(pluginName);

    if (isActive && plugin->IsLightPlugin()) {
      stream << "254 FE " << std::setw(3) << std::hex
             << counters.activeLightPlugins << std::dec << " ";
      counters.activeLightPlugins += 1;
    } else if (isActive) {
      stream << std::setw(3) << counters.activeNormal << " " << std::hex
             << std::setw(2) << counters.activeNormal << std::dec << "     ";
      counters.activeNormal += 1;
    } else {
      stream << "           ";
    }

    stream << pluginName << "\r\n";
  }

  const G& game_;
  const std::vector<std::string> plugins_;
};
}

#endif
