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
along with LOOT.  If not, see
<https://www.gnu.org/licenses/>.
*/

#ifndef LOOT_TESTS_GUI_STATE_GAME_DETECTION_STEAM_TEST
#define LOOT_TESTS_GUI_STATE_GAME_DETECTION_STEAM_TEST

#include "gui/state/game/detection/steam.h"
#include "tests/common_game_test_fixture.h"

namespace loot::test {
class Steam_FindGameInstallsTest : public ::testing::TestWithParam<GameId> {};

INSTANTIATE_TEST_SUITE_P(,
                         Steam_FindGameInstallsTest,
                         ::testing::ValuesIn(ALL_GAME_IDS));

TEST_P(Steam_FindGameInstallsTest, shouldNotThrowForAnyValidGameId) {
  EXPECT_NO_THROW(loot::steam::FindGameInstalls(GetParam()));
}
}

#endif
