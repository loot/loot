/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

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
    along with LOOT. If not, see
    <https://www.gnu.org/licenses/>.
    */

#ifdef _MSC_VER
// Qt typedef's a uint type in the global namespace that spdlog shadows, just
// disable the warning.
#pragma warning(disable : 4459)
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>
#pragma warning(default : 4459)
#else
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>
#endif

#include <QtCore/QCoreApplication>
#include <boost/locale.hpp>

#include "tests/gui/backup_test.h"
#include "tests/gui/helpers_test.h"
#include "tests/gui/qt/helpers_test.h"
#include "tests/gui/qt/tasks/tasks_test.h"
#include "tests/gui/sourced_message_test.h"
#include "tests/gui/state/change_count_test.h"
#include "tests/gui/state/game/detection/common_test.h"
#include "tests/gui/state/game/detection/detail_test.h"
#include "tests/gui/state/game/detection/epic_games_store_test.h"
#include "tests/gui/state/game/detection/generic_test.h"
#include "tests/gui/state/game/detection/gog_test.h"
#include "tests/gui/state/game/detection/heroic_test.h"
#include "tests/gui/state/game/detection/microsoft_store_test.h"
#include "tests/gui/state/game/detection/steam_test.h"
#include "tests/gui/state/game/detection_test.h"
#include "tests/gui/state/game/game_settings_test.h"
#include "tests/gui/state/game/game_test.h"
#include "tests/gui/state/game/games_manager_test.h"
#include "tests/gui/state/game/group_node_positions_test.h"
#include "tests/gui/state/game/helpers_test.h"
#include "tests/gui/state/loot_paths_test.h"
#include "tests/gui/state/loot_settings_test.h"

int main(int argc, char **argv) {
  // Set the logger to use a null sink.
  spdlog::create<spdlog::sinks::null_sink_st>("loot_logger");

  QCoreApplication app(argc, argv);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
