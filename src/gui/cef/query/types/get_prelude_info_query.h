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

#ifndef LOOT_GUI_QUERY_GET_PRELUDE_INFO_QUERY
#define LOOT_GUI_QUERY_GET_PRELUDE_INFO_QUERY

#include "gui/cef/query/query.h"
#include "loot/api.h"

namespace loot {
FileRevision GetMasterlistPreludeRevision(
    const std::filesystem::path& filePath) {
  using boost::locale::translate;

  auto logger = getLogger();

  FileRevision revision;
  try {
    revision = GetFileRevision(filePath, true);
    AddSuffixIfModified(revision);
  } catch (FileAccessError&) {
    if (logger) {
      logger->warn("No masterlist prelude present at {}", filePath.u8string());
    }
    auto text =
        /* translators: N/A is an abbreviation for Not Applicable. A masterlist is a database that contains information for various mods. */
        translate("N/A: No masterlist prelude present").str();
    revision.id = text;
    revision.date = text;
  } catch (GitStateError&) {
    if (logger) {
      logger->warn("Not a Git repository: {}",
                   filePath.parent_path().u8string());
    }
    auto text =
        /* translators: Git is the software LOOT uses to track changes to the source code. */
        translate("Unknown: Git repository missing").str();
    revision.id = text;
    revision.date = text;
  }

  return revision;
}

class GetPreludeInfoQuery : public Query {
public:
  GetPreludeInfoQuery(std::filesystem::path filePath) : filePath_(filePath) {}

  std::string executeLogic() {
    auto logger = getLogger();
    if (logger) {
      logger->debug("Getting the masterlist prelude's revision.");
    }

    nlohmann::json json = GetMasterlistPreludeRevision(filePath_);

    return json.dump();
  }

private:
  const std::filesystem::path filePath_;
  const std::string remoteURL_;
  const std::string remoteBranch_;
};
}

#endif
