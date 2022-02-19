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

#include "gui/helpers.h"
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
  const auto loadOrderFile = [&]() {
    // This isn't 100% accurate (it doesn't handle when Oblivion is configured
    // to use the install path instead of the local data path), but it's good
    // enough for a general advisory. It's not extracted into a separate
    // function to avoid it being misused somewhere else.
    const GameSettings settings = game.GetSettings();
    if (settings.Type() == GameType::tes3) {
      return settings.GamePath() / "Morrowind.ini";
    }

    // This is a misuse of FolderName(), but it works because LOOT's game
    // folder names happen to match the names of the folders used by the games
    // themselves.
    const auto gameLocalPath =
        settings.GameLocalPath().empty()
            ? getLocalAppDataPath() / settings.FolderName()
            : settings.GameLocalPath();
    return gameLocalPath / "plugins.txt";
  }();

  return (boost::format(boost::locale::translate(
              "Oh no, something went wrong! This is usually because \"%1%\" "
              "is set to be read-only. If it is, unset it and try again. If "
              "it isn't, you can check your LOOTDebugLog.txt (you can get to "
              "it through the main menu) for more information.")) %
          loadOrderFile.u8string())
      .str();
}
}

#endif
