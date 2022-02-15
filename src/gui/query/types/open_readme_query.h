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

#ifndef LOOT_GUI_QUERY_OPEN_README_QUERY
#define LOOT_GUI_QUERY_OPEN_README_QUERY

#include <boost/algorithm/string.hpp>

#include "gui/helpers.h"
#include "gui/query/query.h"
#include "gui/state/loot_paths.h"

namespace loot {
class OpenReadmeQuery : public Query {
public:
  OpenReadmeQuery(const std::filesystem::path readmePath,
                  const std::string& relativeFilePath) :
      readmePath_(readmePath), relativeFilePath_(relativeFilePath) {}

  QueryResult executeLogic() override {
    auto logger = getLogger();
    if (logger) {
      logger->info("Opening LOOT's readme.");
    }

    auto canonicalPath =
        std::filesystem::canonical(readmePath_ / relativeFilePath_);
    auto canonicalReadmePath = std::filesystem::canonical(readmePath_);

    if (!boost::starts_with(canonicalPath.u8string(),
                            canonicalReadmePath.u8string())) {
      throw std::runtime_error(
          "Attempted to open readme file outside of recognised readme "
          "directory.");
    }

    OpenInDefaultApplication(canonicalPath);

    return std::monostate();
  }

private:
  const std::filesystem::path readmePath_;
  const std::string relativeFilePath_;
};
}

#endif
