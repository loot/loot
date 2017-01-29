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

#ifndef LOOT_TESTS_API_GAME_INTERFACE_TEST
#define LOOT_TESTS_API_GAME_INTERFACE_TEST

#include "loot/api.h"

#include "tests/api/api_game_operations_test.h"

namespace loot {
namespace test {
class GameInterfaceTest : public ApiGameOperationsTest {};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        GameInterfaceTest,
                        ::testing::Values(
                          GameType::tes4,
                          GameType::tes5,
                          GameType::fo3,
                          GameType::fonv,
                          GameType::fo4,
                          GameType::tes5se));



TEST_P(GameInterfaceTest, sortPluginsShouldSucceedIfPassedValidArguments) {
  std::vector<std::string> expectedOrder = {
    masterFile,
    blankEsm,
    blankMasterDependentEsm,
    blankDifferentEsm,
    blankDifferentMasterDependentEsm,
    blankMasterDependentEsp,
    blankDifferentMasterDependentEsp,
    blankEsp,
    blankPluginDependentEsp,
    blankDifferentEsp,
    blankDifferentPluginDependentEsp,
  };

  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(handle_->GetDatabase()->LoadLists(masterlistPath.string(), ""));

  std::vector<std::string> actualOrder = handle_->SortPlugins({
    blankEsp,
    blankPluginDependentEsp,
    blankDifferentMasterDependentEsm,
    blankMasterDependentEsp,
    blankDifferentMasterDependentEsp,
    blankDifferentEsp,
    blankDifferentPluginDependentEsp,
    masterFile,
    blankEsm,
    blankMasterDependentEsm,
    blankDifferentEsm,
  });

  EXPECT_EQ(expectedOrder, actualOrder);
}

TEST_P(GameInterfaceTest, isPluginActiveShouldReturnFalseIfTheGivenPluginIsNotActive) {
  EXPECT_TRUE(handle_->IsPluginActive(blankEsm));
}

TEST_P(GameInterfaceTest, isPluginActiveShouldReturnTrueIfTheGivenPluginIsActive) {
  EXPECT_FALSE(handle_->IsPluginActive(blankEsp));
}

TEST_P(GameInterfaceTest, getLoadOrderShouldReturnTheCurrentLoadOrder) {
  ASSERT_EQ(getLoadOrder(), handle_->GetLoadOrder());
}

TEST_P(GameInterfaceTest, setLoadOrderShouldSetTheLoadOrder) {
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

  EXPECT_NO_THROW(handle_->SetLoadOrder(loadOrder));


  EXPECT_EQ(loadOrder, handle_->GetLoadOrder());

  if (GetParam() == GameType::fo4 || GetParam() == GameType::tes5se)
    loadOrder.erase(std::begin(loadOrder));

  EXPECT_EQ(loadOrder, getLoadOrder());
}

}
}

#endif
