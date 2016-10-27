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

#ifndef LOOT_TESTS_BACKEND_PLUGIN_PLUGIN_TEST
#define LOOT_TESTS_BACKEND_PLUGIN_PLUGIN_TEST

#include "backend/plugin/plugin.h"

#include "backend/game/game.h"
#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class PluginTest : public CommonGameTestFixture {
protected:
  PluginTest() :
    emptyFile("EmptyFile.esm"),
    nonPluginFile("NotAPlugin.esm"),
    blankArchive("Blank" + Game(GetParam()).GetArchiveFileExtension()),
    blankSuffixArchive("Blank - Different - suffix" + Game(GetParam()).GetArchiveFileExtension()),
    game_(GetParam()) {}

  void SetUp() {
    CommonGameTestFixture::SetUp();

    game_.SetGamePath(dataPath.parent_path());
    game_.Init(false, localPath);

    // Write out an empty file.
    boost::filesystem::ofstream out(dataPath / emptyFile);
    out.close();
    ASSERT_TRUE(boost::filesystem::exists(dataPath / emptyFile));

    // Write out an non-empty, non-plugin file.
    out.open(dataPath / nonPluginFile);
    out << "This isn't a valid plugin file.";
    out.close();
    ASSERT_TRUE(boost::filesystem::exists(dataPath / nonPluginFile));

    // Create dummy archive files.
    out.open(dataPath / blankArchive);
    out.close();
    out.open(dataPath / blankSuffixArchive);
    out.close();
  }

  void TearDown() {
    CommonGameTestFixture::TearDown();

    boost::filesystem::remove(dataPath / emptyFile);
    boost::filesystem::remove(dataPath / nonPluginFile);
    boost::filesystem::remove(dataPath / blankArchive);
    boost::filesystem::remove(dataPath / blankSuffixArchive);
  }

  Game game_;

  const std::string emptyFile;
  const std::string nonPluginFile;
  const std::string blankArchive;
  const std::string blankSuffixArchive;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        PluginTest,
                        ::testing::Values(
                          GameType::tes4,
                          GameType::tes5,
                          GameType::fo3,
                          GameType::fonv,
                          GameType::fo4,
                          GameType::tes5se));

TEST_P(PluginTest, loadingHeaderOnlyShouldReadHeaderData) {
  Plugin plugin(game_, blankEsm, true);

  EXPECT_EQ(blankEsm, plugin.Name());
  EXPECT_TRUE(plugin.getMasters().empty());
  EXPECT_TRUE(plugin.isMasterFile());
  EXPECT_FALSE(plugin.IsEmpty());
  EXPECT_EQ("v5.0", plugin.getDescription());
}

TEST_P(PluginTest, loadingHeaderOnlyShouldNotReadFieldsOrCalculateCrc) {
  Plugin plugin(game_, blankEsm, true);

  EXPECT_TRUE(plugin.getFormIds().empty());
  EXPECT_EQ(0, plugin.Crc());
}

TEST_P(PluginTest, loadingWholePluginShouldReadHeaderData) {
  Plugin plugin(game_, blankEsm, true);

  EXPECT_EQ(blankEsm, plugin.Name());
  EXPECT_TRUE(plugin.getMasters().empty());
  EXPECT_TRUE(plugin.isMasterFile());
  EXPECT_FALSE(plugin.IsEmpty());
  EXPECT_EQ("v5.0", plugin.getDescription());
}

TEST_P(PluginTest, loadingWholePluginShouldReadFields) {
  Plugin plugin(game_, blankMasterDependentEsm, false);

  EXPECT_EQ(4, plugin.NumOverrideFormIDs());
}

TEST_P(PluginTest, loadingWholePluginShouldCalculateCrc) {
  Plugin plugin(game_, blankEsm, false);

  EXPECT_EQ(blankEsmCrc, plugin.Crc());
}

TEST_P(PluginTest, loadingANonMasterPluginShouldReadTheMasterFlagAsFalse) {
  Plugin plugin(game_, blankMasterDependentEsp, true);

  EXPECT_FALSE(plugin.isMasterFile());
}

TEST_P(PluginTest, loadingAPluginWithMastersShouldReadThemCorrectly) {
  Plugin plugin(game_, blankMasterDependentEsp, true);

  EXPECT_EQ(std::vector<std::string>({
      blankEsm
  }), plugin.getMasters());
}

TEST_P(PluginTest, loadsArchiveForAnArchiveThatExactlyMatchesAnEsmFileBasenameShouldReturnTrueForAllGamesExceptOblivion) {
  bool loadsArchive = Plugin(game_, blankEsm, true).LoadsArchive();

  if (GetParam() == GameType::tes4)
    EXPECT_FALSE(loadsArchive);
  else
    EXPECT_TRUE(loadsArchive);
}

TEST_P(PluginTest, loadsArchiveForAnArchiveThatExactlyMatchesAnEspFileBasenameShouldReturnTrue) {
  EXPECT_TRUE(Plugin(game_, blankEsp, true).LoadsArchive());
}

TEST_P(PluginTest, loadsArchiveForAnArchiveWithAFilenameWhichStartsWithTheEsmFileBasenameShouldReturnTrueForAllGamesExceptOblivionAndSkyrim) {
  bool loadsArchive = Plugin(game_, blankDifferentEsm, true).LoadsArchive();

  if (GetParam() == GameType::tes4 || GetParam() == GameType::tes5)
    EXPECT_FALSE(loadsArchive);
  else
    EXPECT_TRUE(loadsArchive);
}

TEST_P(PluginTest, loadsArchiveForAnArchiveWithAFilenameWhichStartsWithTheEspFileBasenameShouldReturnTrueForAllGamesExceptSkyrim) {
  bool loadsArchive = Plugin(game_, blankDifferentEsp, true).LoadsArchive();

  if (GetParam() == GameType::tes5)
    EXPECT_FALSE(loadsArchive);
  else
    EXPECT_TRUE(loadsArchive);
}

TEST_P(PluginTest, loadsArchiveShouldReturnFalseForAPluginThatDoesNotLoadAnArchive) {
  EXPECT_FALSE(Plugin(game_, blankMasterDependentEsp, true).LoadsArchive());
}

TEST_P(PluginTest, loadsArchiveShouldReturnFalseForAPluginWithARegexFilename) {
  EXPECT_FALSE(Plugin(game_, "Blank\\.esp", true).LoadsArchive());
}

TEST_P(PluginTest, isValidShouldReturnTrueForAValidPlugin) {
  EXPECT_TRUE(Plugin::IsValid(blankEsm, game_));
}

TEST_P(PluginTest, isValidShouldReturnFalseForANonPluginFile) {
  EXPECT_FALSE(Plugin::IsValid(nonPluginFile, game_));
}

TEST_P(PluginTest, isValidShouldReturnFalseForAnEmptyFile) {
  EXPECT_FALSE(Plugin::IsValid(emptyFile, game_));
}

TEST_P(PluginTest, isActiveShouldReturnTrueForAPluginThatIsActive) {
  EXPECT_TRUE(Plugin(game_, blankEsm, true).IsActive());
}

TEST_P(PluginTest, isActiveShouldReturnFalseForAPluginThatIsNotActive) {
  EXPECT_FALSE(Plugin(game_, blankEsp, true).IsActive());
}

TEST_P(PluginTest, lessThanOperatorShouldUseCaseInsensitiveLexicographicalNameComparison) {
  Plugin plugin1(game_, "Blank.esp", true);
  Plugin plugin2(game_, "blank.esp", true);

  EXPECT_FALSE(plugin1 < plugin2);
  EXPECT_FALSE(plugin2 < plugin1);

  plugin1 = Plugin(game_, "blank.esm", true);
  plugin2 = Plugin(game_, "blank.esp", true);

  EXPECT_TRUE(plugin1 < plugin2);
  EXPECT_FALSE(plugin2 < plugin1);
}

TEST_P(PluginTest, doFormIDsOverlapShouldReturnFalseForTwoPluginsWithOnlyHeadersLoaded) {
  Plugin plugin1(game_, blankEsm, true);
  Plugin plugin2(game_, blankMasterDependentEsm, true);

  EXPECT_FALSE(plugin1.DoFormIDsOverlap(plugin2));
  EXPECT_FALSE(plugin2.DoFormIDsOverlap(plugin1));
}

TEST_P(PluginTest, doFormIDsOverlapShouldReturnFalseIfThePluginsHaveUnrelatedRecords) {
  Plugin plugin1(game_, blankEsm, false);
  Plugin plugin2(game_, blankEsp, false);

  EXPECT_FALSE(plugin1.DoFormIDsOverlap(plugin2));
  EXPECT_FALSE(plugin2.DoFormIDsOverlap(plugin1));
}

TEST_P(PluginTest, doFormIDsOverlapShouldReturnTrueIfOnePluginOverridesTheOthersRecords) {
  Plugin plugin1(game_, blankEsm, false);
  Plugin plugin2(game_, blankMasterDependentEsm, false);

  EXPECT_TRUE(plugin1.DoFormIDsOverlap(plugin2));
  EXPECT_TRUE(plugin2.DoFormIDsOverlap(plugin1));
}

TEST_P(PluginTest, overlapFormIDsShouldReturnAnEmptySetForTwoPluginsWithOnlyHeadersLoaded) {
  Plugin plugin1(game_, blankEsm, true);
  Plugin plugin2(game_, blankMasterDependentEsm, true);

  EXPECT_TRUE(plugin1.OverlapFormIDs(plugin2).empty());
  EXPECT_TRUE(plugin2.OverlapFormIDs(plugin1).empty());
}

TEST_P(PluginTest, overlapFormIDsShouldReturnAnEmptySetIfThePluginsHaveUnrelatedRecords) {
  Plugin plugin1(game_, blankEsm, false);
  Plugin plugin2(game_, blankEsp, false);

  EXPECT_TRUE(plugin1.OverlapFormIDs(plugin2).empty());
  EXPECT_TRUE(plugin2.OverlapFormIDs(plugin1).empty());
}

TEST_P(PluginTest, overlapFormIDsShouldReturnTheFormIDsOfRecordsAddedByOnePluginAndOverriddenByTheOther) {
  Plugin plugin1(game_, blankEsm, false);
  Plugin plugin2(game_, blankMasterDependentEsm, false);

  std::set<libespm::FormId> expectedFormIds({
      libespm::FormId(blankEsm, std::vector<std::string>(), 0xCF0),
      libespm::FormId(blankEsm, std::vector<std::string>(), 0xCF1),
      libespm::FormId(blankEsm, std::vector<std::string>(), 0xCF2),
      libespm::FormId(blankEsm, std::vector<std::string>(), 0xCF3),
  });
  EXPECT_EQ(expectedFormIds, plugin1.OverlapFormIDs(plugin2));
  EXPECT_EQ(expectedFormIds, plugin2.OverlapFormIDs(plugin1));
}

TEST_P(PluginTest, checkInstallValidityShouldCheckThatRequirementsArePresent) {
  Plugin plugin(game_, blankEsm, false);
  plugin.Reqs({
      File(missingEsp),
      File(blankEsp),
  });

  plugin.CheckInstallValidity(game_);
  EXPECT_EQ(std::vector<Message>({
      Message(MessageType::error, "This plugin requires \"" + missingEsp + "\" to be installed, but it is missing."),
  }), plugin.Messages());
}

TEST_P(PluginTest, checkInstallValidityShouldCheckThatIncompatibilitiesAreAbsent) {
  Plugin plugin(game_, blankEsm, false);
  plugin.Incs({
      File(missingEsp),
      File(masterFile),
  });

  plugin.CheckInstallValidity(game_);
  EXPECT_EQ(std::vector<Message>({
      Message(MessageType::error, "This plugin is incompatible with \"" + masterFile + "\", but both are present."),
  }), plugin.Messages());
}

TEST_P(PluginTest, checkInstallValidityShouldGenerateMessagesFromDirtyInfo) {
  const std::vector<MessageContent> info = std::vector<MessageContent>({
    MessageContent("info", LanguageCode::english),
  });

  Plugin plugin(game_, blankEsm, false);
  plugin.DirtyInfo({
      PluginCleaningData(blankEsmCrc, "utility1", info, 0, 1, 2),
      PluginCleaningData(0xDEADBEEF, "utility2", info, 0, 5, 10),
  });

  plugin.CheckInstallValidity(game_);
  EXPECT_EQ(std::vector<Message>({
      PluginCleaningData(blankEsmCrc, "utility1", info, 0, 1, 2).AsMessage(),
      PluginCleaningData(0xDEADBEEF, "utility2", info, 0, 5, 10).AsMessage(),
  }), plugin.Messages());
}

TEST_P(PluginTest, checkInstallValidityShouldCheckIfAPluginsMastersAreAllPresentAndActiveIfNoFilterTagIsPresent) {
  Plugin plugin(game_, blankDifferentMasterDependentEsp, false);

  plugin.CheckInstallValidity(game_);
  EXPECT_EQ(std::vector<Message>({
      Message(MessageType::error, "This plugin requires \"" + blankDifferentEsm + "\" to be active, but it is inactive."),
  }), plugin.Messages());
}

TEST_P(PluginTest, checkInstallValidityShouldNotCheckIfAPluginsMastersAreAllActiveIfAFilterTagIsPresent) {
  Plugin plugin(game_, blankDifferentMasterDependentEsp, false);
  plugin.Tags({Tag("Filter")});

  plugin.CheckInstallValidity(game_);
  EXPECT_TRUE(plugin.Messages().empty());
}
}
}

#endif
