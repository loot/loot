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

#ifndef LOOT_GUI_QUERY_OPEN_LOG_LOCATION_QUERY
#define LOOT_GUI_QUERY_OPEN_LOG_LOCATION_QUERY

#include "gui/helpers.h"
#include "gui/query/query.h"
#include "gui/state/loot_paths.h"

namespace loot {
class OpenLogLocationQuery : public Query {
public:
  explicit OpenLogLocationQuery(std::filesystem::path logPath) :
      logPath_(logPath) {}

  QueryResult executeLogic() override {
    auto logger = getLogger();
    if (logger) {
      logger->info("Opening LOOT's local appdata folder.");
    }
    OpenInDefaultApplication(logPath_.parent_path());

    return std::monostate();
  }

private:
  const std::filesystem::path logPath_;
};
}

#endif
