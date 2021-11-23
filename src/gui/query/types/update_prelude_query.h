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

#ifndef LOOT_GUI_QUERY_UPDATE_PRELUDE_QUERY
#define LOOT_GUI_QUERY_UPDATE_PRELUDE_QUERY

#include <loot/api.h>

#include "gui/state/game/helpers.h"

namespace loot {
class UpdatePreludeQuery : public Query {
public:
  UpdatePreludeQuery(std::filesystem::path filePath,
                     std::string remoteURL,
                     std::string remoteBranch) :
      filePath_(filePath), remoteURL_(remoteURL), remoteBranch_(remoteBranch) {}

  nlohmann::json executeLogic() {
    auto logger = getLogger();
    if (logger) {
      logger->debug("Updating the masterlist prelude.");
    }

    bool wasUpdated = UpdateFile(filePath_, remoteURL_, remoteBranch_);
    if (!wasUpdated) {
      return nullptr;
    }

    return GetFileRevisionToDisplay(filePath_, FileType::MasterlistPrelude);
  }

private:
  const std::filesystem::path filePath_;
  const std::string remoteURL_;
  const std::string remoteBranch_;
};
}

#endif
