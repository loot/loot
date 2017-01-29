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
class GameInterfaceTest : public ApiGameOperationsTest {
protected:
  GameInterfaceTest() :
    emptyFile("EmptyFile.esm"),
    nonPluginFile("NotAPlugin.esm"),
    pluginsToLoad({
      masterFile,
      blankEsm,
      blankDifferentEsm,
      blankMasterDependentEsm,
      blankDifferentMasterDependentEsm,
      blankEsp,
      blankDifferentEsp,
      blankMasterDependentEsp,
      blankDifferentMasterDependentEsp,
      blankPluginDependentEsp,
      blankDifferentPluginDependentEsp,
    }) {}

  void TearDown() {
    ApiGameOperationsTest::TearDown();

    boost::filesystem::remove(dataPath / emptyFile);
    boost::filesystem::remove(dataPath / nonPluginFile);
  }

  const std::string emptyFile;
  const std::string nonPluginFile;
  const std::vector<std::string> pluginsToLoad;
};

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

TEST_P(GameInterfaceTest, isValidPluginShouldReturnTrueForAValidPlugin) {
  EXPECT_TRUE(handle_->IsValidPlugin(blankEsm));
}

TEST_P(GameInterfaceTest, isValidPluginShouldReturnFalseForANonPluginFile) {
  // Write out an non-empty, non-plugin file.
  boost::filesystem::ofstream out(dataPath / nonPluginFile);
  out << "This isn't a valid plugin file.";
  out.close();
  ASSERT_TRUE(boost::filesystem::exists(dataPath / nonPluginFile));

  EXPECT_FALSE(handle_->IsValidPlugin(nonPluginFile));
}

TEST_P(GameInterfaceTest, isValidPluginShouldReturnFalseForAnEmptyFile) {
  // Write out an empty file.
  boost::filesystem::ofstream out(dataPath / emptyFile);
  out.close();
  ASSERT_TRUE(boost::filesystem::exists(dataPath / emptyFile));

  EXPECT_FALSE(handle_->IsValidPlugin(emptyFile));
}

TEST_P(GameInterfaceTest, loadPluginsWithHeadersOnlyTrueShouldLoadTheHeadersOfAllInstalledPlugins) {
  handle_->LoadPlugins(pluginsToLoad, true);
  EXPECT_EQ(11, handle_->GetLoadedPlugins().size());

  // Check that one plugin's header has been read.
  ASSERT_NO_THROW(handle_->GetPlugin(masterFile));
  auto plugin = handle_->GetPlugin(masterFile);
  EXPECT_EQ("5.0", plugin->GetVersion());

  // Check that only the header has been read.
  EXPECT_EQ(0, plugin->GetCRC());
}

TEST_P(GameInterfaceTest, loadPluginsWithHeadersOnlyFalseShouldFullyLoadAllInstalledPlugins) {
  handle_->LoadPlugins(pluginsToLoad, false);
  EXPECT_EQ(11, handle_->GetLoadedPlugins().size());

  // Check that one plugin's header has been read.
  ASSERT_NO_THROW(handle_->GetPlugin(masterFile));
  auto plugin = handle_->GetPlugin(masterFile);
  EXPECT_EQ("5.0", plugin->GetVersion());

  // Check that not only the header has been read.
  EXPECT_EQ(blankEsmCrc, plugin->GetCRC());
}

TEST_P(GameInterfaceTest, getPluginThatIsNotCachedShouldThrow) {
  EXPECT_THROW(handle_->GetPlugin(blankEsm), std::invalid_argument);
}

TEST_P(GameInterfaceTest, gettingPluginsShouldReturnAnEmptySetIfNoneHaveBeenLoaded) {
  EXPECT_TRUE(handle_->GetLoadedPlugins().empty());
}

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
