/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2016    WrinklyNinja

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

#ifndef LOOT_TESTS_BACKEND_LOAD_ORDER_HANDLER_TEST
#define LOOT_TESTS_BACKEND_LOAD_ORDER_HANDLER_TEST

#include "backend/game/load_order_handler.h"

#include "loot/error.h"
#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class LoadOrderHandlerTest : public CommonGameTestFixture {
protected:
  LoadOrderHandler loadOrderHandler_;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        LoadOrderHandlerTest,
                        ::testing::Values(
                          GameType::tes4,
                          GameType::tes5,
                          GameType::fo3,
                          GameType::fonv,
                          GameType::fo4));

TEST_P(LoadOrderHandlerTest, initShouldThrowIfNoGamePathIsSet) {
  GameSettings game(GetParam());

  EXPECT_THROW(loadOrderHandler_.Init(game), std::invalid_argument);
  EXPECT_THROW(loadOrderHandler_.Init(game), std::invalid_argument);
  EXPECT_THROW(loadOrderHandler_.Init(game, localPath), std::invalid_argument);
  EXPECT_THROW(loadOrderHandler_.Init(game, localPath), std::invalid_argument);
}

#ifndef _WIN32
TEST_P(LoadOrderHandlerTest, initShouldThrowOnLinuxIfNoLocalPathIsSet) {
  GameSettings game(GetParam());
  game.SetGamePath(dataPath.parent_path());

  EXPECT_THROW(loadOrderHandler_.Init(game), Error);
}
#endif

TEST_P(LoadOrderHandlerTest, initShouldNotThrowIfAValidGameIdAndGamePathAndLocalPathAreSet) {
  GameSettings game(GetParam());
  game.SetGamePath(dataPath.parent_path());

  EXPECT_NO_THROW(loadOrderHandler_.Init(game, localPath));
}

TEST_P(LoadOrderHandlerTest, isPluginActiveShouldThrowIfTheHandlerHasNotBeenInitialised) {
  EXPECT_THROW(loadOrderHandler_.IsPluginActive(masterFile), Error);
}

TEST_P(LoadOrderHandlerTest, isPluginActiveShouldReturnCorrectPluginStatesAfterInitialisation) {
  GameSettings game(GetParam());
  game.SetGamePath(dataPath.parent_path());
  ASSERT_NO_THROW(loadOrderHandler_.Init(game, localPath));

  EXPECT_TRUE(loadOrderHandler_.IsPluginActive(masterFile));
  EXPECT_TRUE(loadOrderHandler_.IsPluginActive(blankEsm));
  EXPECT_FALSE(loadOrderHandler_.IsPluginActive(blankEsp));
}

TEST_P(LoadOrderHandlerTest, getLoadOrderShouldThrowIfTheHandlerHasNotBeenInitialised) {
  EXPECT_THROW(loadOrderHandler_.GetLoadOrder(), Error);
}

TEST_P(LoadOrderHandlerTest, getLoadOrderShouldReturnTheCurrentLoadOrder) {
  GameSettings game(GetParam());
  game.SetGamePath(dataPath.parent_path());
  ASSERT_NO_THROW(loadOrderHandler_.Init(game, localPath));

  ASSERT_EQ(getLoadOrder(), loadOrderHandler_.GetLoadOrder());
}

TEST_P(LoadOrderHandlerTest, setLoadOrderShouldThrowIfTheHandlerHasNotBeenInitialised) {
  std::vector<std::string> loadOrder({
      masterFile,
      blankEsm,
      blankMasterDependentEsm,
      blankDifferentEsm,
      blankDifferentMasterDependentEsm,
      blankDifferentEsp,
      blankDifferentPluginDependentEsp,
      blankEsp,
      blankMasterDependentEsp,
      blankDifferentMasterDependentEsp,
      blankPluginDependentEsp,
  });

  EXPECT_THROW(loadOrderHandler_.SetLoadOrder(loadOrder), Error);
}

TEST_P(LoadOrderHandlerTest, setLoadOrderShouldSetTheLoadOrder) {
  GameSettings game(GetParam());
  game.SetGamePath(dataPath.parent_path());
  ASSERT_NO_THROW(loadOrderHandler_.Init(game, localPath));

  std::vector<std::string> loadOrder({
      masterFile,
      blankEsm,
      blankMasterDependentEsm,
      blankDifferentEsm,
      blankDifferentMasterDependentEsm,
      blankDifferentEsp,
      blankDifferentPluginDependentEsp,
      blankEsp,
      blankMasterDependentEsp,
      blankDifferentMasterDependentEsp,
      blankPluginDependentEsp,
  });
  EXPECT_NO_THROW(loadOrderHandler_.SetLoadOrder(loadOrder));

  if (GetParam() == GameType::fo4)
    loadOrder.erase(begin(loadOrder));

  EXPECT_EQ(loadOrder, getLoadOrder());
}
}
}

#endif
