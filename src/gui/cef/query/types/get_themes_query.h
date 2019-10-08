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

#ifndef LOOT_GUI_QUERY_GET_THEMES_QUERY
#define LOOT_GUI_QUERY_GET_THEMES_QUERY

#undef min

#include <boost/algorithm/string.hpp>
#include <json.hpp>

#include "gui/cef/query/query.h"

namespace loot {
class GetThemesQuery : public Query {
public:
  GetThemesQuery(const std::filesystem::path resourcesPath) : resourcesPath_(resourcesPath) {}

  std::string executeLogic() {
    auto logger = getLogger();
    if (logger) {
      logger->info("Getting LOOT's installed themes.");
    }

    nlohmann::json json;
    json["themes"] = findThemes();

    return json.dump();
  }

private:
  std::vector<std::string> findThemes() const {
    std::vector<std::string> themes;

    auto logger = getLogger();
    for (std::filesystem::directory_iterator it(resourcesPath_ / "ui" / "css");
         it != std::filesystem::directory_iterator();
        ++it) {
      if (!std::filesystem::is_regular_file(it->status())) {
        continue;
      }

      auto filename = it->path().filename().u8string();
      if (!boost::iends_with(filename, ".theme.css")) {
        continue;
      }

      if (logger) {
        logger->info("Found theme CSS file: {}", filename);
      }

      auto themeName = filename.substr(0, filename.size() - 10);
      themes.push_back(themeName);
    }

    return themes;
  }

  const std::filesystem::path resourcesPath_;
};
}

#endif
