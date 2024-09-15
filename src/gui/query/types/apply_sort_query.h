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

#ifndef LOOT_GUI_QUERY_APPLY_SORT_QUERY
#define LOOT_GUI_QUERY_APPLY_SORT_QUERY

#include <fmt/base.h>

#include "gui/helpers.h"
#include "gui/query/query.h"

namespace loot {
template<typename G = gui::Game>
class ApplySortQuery : public Query {
public:
  ApplySortQuery(G& game,
                 UnappliedChangeCounter& counter,
                 const std::vector<std::string>& plugins) :
      game_(game), counter_(counter), plugins_(plugins) {}

  QueryResult executeLogic() override {
    auto logger = getLogger();
    if (logger) {
      logger->trace("User has accepted sorted load order, applying it.");
    }
    try {
      game_.SetLoadOrder(plugins_);
      counter_.DecrementUnappliedChangeCounter();
    } catch (...) {
      useSortingErrorMessage = true;
      throw;
    }

    return std::monostate();
  }

  std::string getErrorMessage() const override {
    if (useSortingErrorMessage) {
      return getSortingErrorMessage(game_);
    }

    return Query::getErrorMessage();
  }

private:
  G& game_;
  UnappliedChangeCounter& counter_;
  const std::vector<std::string> plugins_;
  bool useSortingErrorMessage{false};

  std::string getSortingErrorMessage(const G& game) const {
    return fmt::format(
        boost::locale::translate(
            "Oh no, something went wrong! This is usually because \"{0}\" "
            "is set to be read-only. If it is, unset it and try again. If "
            "it isn't, you can check your LOOTDebugLog.txt (you can get to "
            "it through the main menu) for more information.")
            .str(),
        game.GetActivePluginsFilePath().u8string());
  }
};
}

#endif
