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

#ifndef LOOT_GUI_QUERY_QUERY
#define LOOT_GUI_QUERY_QUERY

#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <optional>
#include <string>
#include <variant>

#include "gui/plugin_item.h"
#include "gui/state/logging.h"
#include "gui/state/loot_paths.h"
#include "gui/state/loot_state.h"

namespace loot {
typedef std::vector<std::pair<std::string, std::optional<short>>>
    CancelSortResult;
typedef std::vector<PluginItem> PluginItems;
typedef std::vector<std::pair<PluginItem, bool>> GetConflictingPluginsResult;

typedef std::variant<std::monostate,
                     CancelSortResult,
                     PluginItems,
                     PluginItem,
                     GetConflictingPluginsResult>
    QueryResult;

class Query {
public:
  Query() = default;
  Query(const Query&) = delete;
  Query(Query&&) = delete;
  virtual ~Query() = default;

  Query& operator=(const Query&) = delete;
  Query& operator=(Query&&) = delete;

  virtual QueryResult executeLogic() = 0;
  virtual std::string getErrorMessage() const {
    return boost::locale::translate(
               "Oh no, something went wrong! You can check your "
               "LOOTDebugLog.txt (you can get to it through the "
               "main menu) for more information.")
        .str();
  };
};

template<typename G>
inline std::string getSortingErrorMessage(const G& game) {
  return (boost::format(boost::locale::translate(
              "Oh no, something went wrong! This is usually because \"%1%\" "
              "is set to be read-only. If it is, unset it and try again. If "
              "it isn't, you can check your LOOTDebugLog.txt (you can get to "
              "it through the main menu) for more information.")) %
          game.PluginsTxtPath().u8string())
      .str();
}
}

#endif
