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

#include "api/game/load_order_handler.h"

#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class LoadOrderHandlerTest : public CommonGameTestFixture {
protected:
  LoadOrderHandlerTest() : loadOrderToSet_({
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
  }) {}

  void TearDown() {
    CommonGameTestFixture::TearDown();
  }

  void initialiseHandler() {
    ASSERT_NO_THROW(loadOrderHandler_.Init(GetParam(), dataPath.parent_path(), localPath));
  }

  LoadOrderHandler loadOrderHandler_;
  std::vector<std::string> loadOrderToSet_;
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
                          GameType::fo4,
                          GameType::tes5se));

TEST_P(LoadOrderHandlerTest, initShouldThrowIfNoGamePathIsSet) {
  EXPECT_THROW(loadOrderHandler_.Init(GetParam(), ""), std::invalid_argument);
  EXPECT_THROW(loadOrderHandler_.Init(GetParam(), ""), std::invalid_argument);
  EXPECT_THROW(loadOrderHandler_.Init(GetParam(), "", localPath), std::invalid_argument);
  EXPECT_THROW(loadOrderHandler_.Init(GetParam(), "", localPath), std::invalid_argument);
}

#ifndef _WIN32
TEST_P(LoadOrderHandlerTest, initShouldThrowOnLinuxIfNoLocalPathIsSet) {
  EXPECT_THROW(loadOrderHandler_.Init(GetParam(), dataPath.parent_path()), std::system_error);
}
#endif

TEST_P(LoadOrderHandlerTest, initShouldNotThrowIfAValidGameIdAndGamePathAndLocalPathAreSet) {
  EXPECT_NO_THROW(loadOrderHandler_.Init(GetParam(), dataPath.parent_path(), localPath));
}

TEST_P(LoadOrderHandlerTest, isPluginActiveShouldThrowIfTheHandlerHasNotBeenInitialised) {
  EXPECT_THROW(loadOrderHandler_.IsPluginActive(masterFile), std::system_error);
}

TEST_P(LoadOrderHandlerTest, isPluginActiveShouldReturnCorrectPluginStatesAfterInitialisation) {
  initialiseHandler();

  EXPECT_TRUE(loadOrderHandler_.IsPluginActive(masterFile));
  EXPECT_TRUE(loadOrderHandler_.IsPluginActive(blankEsm));
  EXPECT_FALSE(loadOrderHandler_.IsPluginActive(blankEsp));
}

TEST_P(LoadOrderHandlerTest, getLoadOrderShouldThrowIfTheHandlerHasNotBeenInitialised) {
  EXPECT_THROW(loadOrderHandler_.GetLoadOrder(), std::system_error);
}

TEST_P(LoadOrderHandlerTest, getLoadOrderShouldReturnTheCurrentLoadOrder) {
  initialiseHandler();

  ASSERT_EQ(getLoadOrder(), loadOrderHandler_.GetLoadOrder());
}

TEST_P(LoadOrderHandlerTest, setLoadOrderShouldThrowIfTheHandlerHasNotBeenInitialised) {
  EXPECT_THROW(loadOrderHandler_.SetLoadOrder(loadOrderToSet_), std::system_error);
}

TEST_P(LoadOrderHandlerTest, setLoadOrderShouldSetTheLoadOrder) {
  initialiseHandler();

  EXPECT_NO_THROW(loadOrderHandler_.SetLoadOrder(loadOrderToSet_));

  if (GetParam() == GameType::fo4 || GetParam() == GameType::tes5se)
    loadOrderToSet_.erase(begin(loadOrderToSet_));

  EXPECT_EQ(loadOrderToSet_, getLoadOrder());
}
}
}

#endif
