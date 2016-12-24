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
  }),
  loadOrderBackupFile0(localPath / "loadorder.bak.0"),
  loadOrderBackupFile1(localPath / "loadorder.bak.1"),
  loadOrderBackupFile2(localPath / "loadorder.bak.2"),
  loadOrderBackupFile3(localPath / "loadorder.bak.3") {}

  void TearDown() {
    CommonGameTestFixture::TearDown();

    boost::filesystem::remove(loadOrderBackupFile0);
    boost::filesystem::remove(loadOrderBackupFile1);
    boost::filesystem::remove(loadOrderBackupFile2);
  }

  void initialiseHandler() {
    GameSettings game(GetParam());
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(loadOrderHandler_.Init(game, localPath));
  }

  LoadOrderHandler loadOrderHandler_;
  std::vector<std::string> loadOrderToSet_;
  const boost::filesystem::path loadOrderBackupFile0;
  const boost::filesystem::path loadOrderBackupFile1;
  const boost::filesystem::path loadOrderBackupFile2;
  const boost::filesystem::path loadOrderBackupFile3;
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

  EXPECT_THROW(loadOrderHandler_.Init(game), std::system_error);
}
#endif

TEST_P(LoadOrderHandlerTest, initShouldNotThrowIfAValidGameIdAndGamePathAndLocalPathAreSet) {
  GameSettings game(GetParam());
  game.SetGamePath(dataPath.parent_path());

  EXPECT_NO_THROW(loadOrderHandler_.Init(game, localPath));
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

TEST_P(LoadOrderHandlerTest, backupLoadOrderShouldCreateABackupOfTheCurrentLoadOrder) {
  ASSERT_FALSE(boost::filesystem::exists(loadOrderBackupFile0));
  ASSERT_FALSE(boost::filesystem::exists(loadOrderBackupFile1));
  ASSERT_FALSE(boost::filesystem::exists(loadOrderBackupFile2));
  ASSERT_FALSE(boost::filesystem::exists(loadOrderBackupFile3));

  ASSERT_NO_THROW(LoadOrderHandler::BackupLoadOrder(loadOrderToSet_, localPath));

  EXPECT_TRUE(boost::filesystem::exists(loadOrderBackupFile0));
  EXPECT_FALSE(boost::filesystem::exists(loadOrderBackupFile1));
  EXPECT_FALSE(boost::filesystem::exists(loadOrderBackupFile2));
  EXPECT_FALSE(boost::filesystem::exists(loadOrderBackupFile3));

  auto loadOrder = readFileLines(loadOrderBackupFile0);

  EXPECT_EQ(loadOrderToSet_, loadOrder);
}

TEST_P(LoadOrderHandlerTest, backupLoadOrderShouldRollOverExistingBackups) {
  ASSERT_FALSE(boost::filesystem::exists(loadOrderBackupFile0));
  ASSERT_FALSE(boost::filesystem::exists(loadOrderBackupFile1));
  ASSERT_FALSE(boost::filesystem::exists(loadOrderBackupFile2));
  ASSERT_FALSE(boost::filesystem::exists(loadOrderBackupFile3));

  ASSERT_NO_THROW(LoadOrderHandler::BackupLoadOrder(loadOrderToSet_, localPath));

  auto firstSetLoadOrder = loadOrderToSet_;

  ASSERT_NE(blankPluginDependentEsp, loadOrderToSet_[9]);
  ASSERT_NE(blankDifferentMasterDependentEsp, loadOrderToSet_[10]);
  loadOrderToSet_[9] = blankPluginDependentEsp;
  loadOrderToSet_[10] = blankDifferentMasterDependentEsp;

  ASSERT_NO_THROW(LoadOrderHandler::BackupLoadOrder(loadOrderToSet_, localPath));

  EXPECT_TRUE(boost::filesystem::exists(loadOrderBackupFile0));
  EXPECT_TRUE(boost::filesystem::exists(loadOrderBackupFile1));
  EXPECT_FALSE(boost::filesystem::exists(loadOrderBackupFile2));
  EXPECT_FALSE(boost::filesystem::exists(loadOrderBackupFile3));

  auto loadOrder = readFileLines(loadOrderBackupFile0);
  EXPECT_EQ(loadOrderToSet_, loadOrder);

  loadOrder = readFileLines(loadOrderBackupFile1);
  EXPECT_EQ(firstSetLoadOrder, loadOrder);
}

TEST_P(LoadOrderHandlerTest, backupLoadOrderShouldKeepUpToThreeBackups) {
  ASSERT_FALSE(boost::filesystem::exists(loadOrderBackupFile0));
  ASSERT_FALSE(boost::filesystem::exists(loadOrderBackupFile1));
  ASSERT_FALSE(boost::filesystem::exists(loadOrderBackupFile2));
  ASSERT_FALSE(boost::filesystem::exists(loadOrderBackupFile3));

  ASSERT_NO_THROW(LoadOrderHandler::BackupLoadOrder(loadOrderToSet_, localPath));

  auto firstSetLoadOrder = loadOrderToSet_;

  ASSERT_NE(blankPluginDependentEsp, loadOrderToSet_[9]);
  ASSERT_NE(blankDifferentMasterDependentEsp, loadOrderToSet_[10]);
  loadOrderToSet_[9] = blankPluginDependentEsp;
  loadOrderToSet_[10] = blankDifferentMasterDependentEsp;

  ASSERT_NO_THROW(LoadOrderHandler::BackupLoadOrder(loadOrderToSet_, localPath));

  auto secondSetLoadOrder = loadOrderToSet_;

  ASSERT_NE(blankMasterDependentEsp, loadOrderToSet_[7]);
  ASSERT_NE(blankEsp, loadOrderToSet_[8]);
  loadOrderToSet_[7] = blankMasterDependentEsp;
  loadOrderToSet_[8] = blankEsp;

  ASSERT_NO_THROW(LoadOrderHandler::BackupLoadOrder(loadOrderToSet_, localPath));

  auto thirdSetLoadOrder = loadOrderToSet_;

  ASSERT_NE(blankMasterDependentEsm, loadOrderToSet_[7]);
  ASSERT_NE(blankDifferentEsm, loadOrderToSet_[8]);
  loadOrderToSet_[7] = blankMasterDependentEsm;
  loadOrderToSet_[8] = blankDifferentEsm;

  ASSERT_NO_THROW(LoadOrderHandler::BackupLoadOrder(loadOrderToSet_, localPath));

  EXPECT_TRUE(boost::filesystem::exists(loadOrderBackupFile0));
  EXPECT_TRUE(boost::filesystem::exists(loadOrderBackupFile1));
  EXPECT_TRUE(boost::filesystem::exists(loadOrderBackupFile2));
  EXPECT_FALSE(boost::filesystem::exists(loadOrderBackupFile3));

  auto loadOrder = readFileLines(loadOrderBackupFile0);
  EXPECT_EQ(loadOrderToSet_, loadOrder);

  loadOrder = readFileLines(loadOrderBackupFile1);
  EXPECT_EQ(thirdSetLoadOrder, loadOrder);

  loadOrder = readFileLines(loadOrderBackupFile2);
  EXPECT_EQ(secondSetLoadOrder, loadOrder);
}
}
}

#endif
