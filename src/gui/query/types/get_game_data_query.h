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

#ifndef LOOT_GUI_QUERY_GET_GAME_DATA_QUERY
#define LOOT_GUI_QUERY_GET_GAME_DATA_QUERY

#include <boost/locale.hpp>

#include "gui/query/query.h"
#include "gui/state/game/game.h"
#include "loot/loot_version.h"

namespace loot {
class GetGameDataQuery : public Query {
public:
  GetGameDataQuery(gui::Game& game,
                   std::string language,
                   std::function<void(std::string)> sendProgressUpdate) :
      game_(game),
      language_(language),
      sendProgressUpdate_(sendProgressUpdate) {}

  QueryResult executeLogic() override {
    sendProgressUpdate_(boost::locale::translate(
        "Parsing, merging and evaluating metadata…"));

    /* If the game's plugins object is empty, this is the first time loading
       the game data, so also load the metadata lists. */
    bool isFirstLoad = game_.GetPlugins().empty();

    std::exception_ptr exceptionPointer;
    std::vector<std::thread> threads;

    threads.push_back(std::thread([&]() {
      try {
        game_.LoadAllInstalledPlugins(true);
      } catch (...) {
        if (exceptionPointer == nullptr) {
          exceptionPointer = std::current_exception();
        }
      }
    }));

    if (isFirstLoad) {
      threads.push_back(std::thread([&]() {
        try {
          game_.LoadMetadata();
        } catch (...) {
          if (exceptionPointer == nullptr) {
            exceptionPointer = std::current_exception();
          }
        }
      }));
    }

    threads.push_back(std::thread([&]() {
      try {
        game_.LoadCreationClubPluginNames();
      } catch (...) {
        if (exceptionPointer == nullptr) {
          exceptionPointer = std::current_exception();
        }
      }
    }));

    for (auto& thread : threads) {
      if (thread.joinable()) {
        thread.join();
        if (exceptionPointer != nullptr) {
          std::rethrow_exception(exceptionPointer);
        }
      }
    }

    // Sort plugins into their load order.
    return GetPluginItems(game_.GetLoadOrder(), game_, language_);
  }

private:
  gui::Game& game_;
  std::string language_;
  std::function<void(std::string)> sendProgressUpdate_;
};
}

#endif
