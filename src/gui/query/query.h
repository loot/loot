/*  LOOT

A modding utility for Starfield and some Elder Scrolls and Fallout games.

Copyright (C) 2013-2026 Oliver Hamlet

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

#include <string>
#include <variant>

#include "gui/plugin_item.h"

namespace loot {
typedef std::pair<std::string, bool> MasterlistUpdateResult;
typedef std::vector<PluginItem> PluginItems;
typedef std::vector<std::pair<PluginItem, bool>> GetOverlappingPluginsResult;

typedef std::variant<bool,
                     MasterlistUpdateResult,
                     PluginItems,
                     GetOverlappingPluginsResult>
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
};
}

#endif
