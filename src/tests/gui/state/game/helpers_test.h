/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2018    Oliver Hamlet

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

#ifndef LOOT_TESTS_GUI_STATE_GAME_HELPERS_TEST
#define LOOT_TESTS_GUI_STATE_GAME_HELPERS_TEST

#include <gtest/gtest.h>

#include <fstream>

#include "gui/state/game/helpers.h"
#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeBackslash) {
  EXPECT_EQ("\\\\", escapeMarkdownASCIIPunctuation("\\"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeBacktick) {
  EXPECT_EQ("\\`", escapeMarkdownASCIIPunctuation("`"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeAsterisk) {
  EXPECT_EQ("\\*", escapeMarkdownASCIIPunctuation("*"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeUnderscore) {
  EXPECT_EQ("\\_", escapeMarkdownASCIIPunctuation("_"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeCurlyBraces) {
  EXPECT_EQ("\\{", escapeMarkdownASCIIPunctuation("{"));
  EXPECT_EQ("\\}", escapeMarkdownASCIIPunctuation("}"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeSquareBrackets) {
  EXPECT_EQ("\\[", escapeMarkdownASCIIPunctuation("["));
  EXPECT_EQ("\\]", escapeMarkdownASCIIPunctuation("]"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeParentheses) {
  EXPECT_EQ("\\(", escapeMarkdownASCIIPunctuation("("));
  EXPECT_EQ("\\)", escapeMarkdownASCIIPunctuation(")"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeHash) {
  EXPECT_EQ("\\#", escapeMarkdownASCIIPunctuation("#"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapePlus) {
  EXPECT_EQ("\\+", escapeMarkdownASCIIPunctuation("+"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeHyphen) {
  EXPECT_EQ("\\-", escapeMarkdownASCIIPunctuation("-"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapePeriod) {
  EXPECT_EQ("\\.", escapeMarkdownASCIIPunctuation("."));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeExclamationMark) {
  EXPECT_EQ("\\!", escapeMarkdownASCIIPunctuation("!"));
}

TEST(CheckForRemovedPlugins, shouldCompareFilenamesBeforeAndAfter) {
  const auto plugins = checkForRemovedPlugins(
      {"test1.esp", "test2.esp", "test3.esp"}, {"test1.esp", "test3.esp"});

  EXPECT_EQ(std::vector<std::string>{"test2.esp"}, plugins);
}

TEST(ReadBashTagsFile, shouldCorrectlyReadTheExampleFileContent) {
  // From the Wrye Bash Advanced Readme
  // <https://wrye-bash.github.io/docs/Wrye%20Bash%20Advanced%20Readme.html#patch-tags>
  const auto fileContent = R"(
# Everything after a '#' is a comment
# Every line that is not a comment
# or empty will add or remove tags
# from the plugin

Delev, Relev # This line will add two
             # new tags to the plugin...

-C.Water     # ...while this line removes
             # a tag from the plugin

# Addition and removal can also be
# done in one line:
C.Location, -C.LockList

# The result of this file would be:
# Added: C.Location, Delev, Relev
# Removed: C.LockList, C.Water)";

  std::stringstream in(fileContent);

  const auto tags = readBashTagsFile(in);
  const std::vector<Tag> expectedTags{Tag("Delev"),
                                      Tag("Relev"),
                                      Tag("C.Water", false),
                                      Tag("C.Location"),
                                      Tag("C.LockList", false)};

  EXPECT_EQ(expectedTags, tags);
}

TEST(ReadBashTagsFile, shouldReturnAnEmptyVectorIfThePathDoesNotExist) {
  EXPECT_TRUE(
      readBashTagsFile(std::filesystem::temp_directory_path() / "missing",
                       "Blank.esp")
          .empty());
}

TEST(ReadBashTagsFile, shouldReadFromAPluginFile) {
  const auto dataPath = getTempPath();
  const auto bashTagsDir = dataPath / "BashTags";

  std::filesystem::create_directories(bashTagsDir);

  std::ofstream out(bashTagsDir / "Blank.txt");
  out << "C.Location, Delev, -Relev";
  out.close();

  const auto tags = readBashTagsFile(dataPath, "Blank.esp");

  const std::vector<Tag> expectedTags{
      Tag("C.Location"), Tag("Delev"), Tag("Relev", false)};

  EXPECT_EQ(expectedTags, tags);

  std::filesystem::remove_all(dataPath);
}

TEST(GetTagConflicts,
     shouldReturnTagNamesAddedByOneSourceAndRemovedByTheOther) {
  const auto conflicts =
      getTagConflicts({Tag("A", false), Tag("B"), Tag("C", false), Tag("D")},
                      {Tag("A"), Tag("B", false), Tag("C", false)});

  const std::vector<std::string> expectedConflicts{"A", "B"};

  EXPECT_EQ(expectedConflicts, conflicts);
}

class ResolveGameFilePathTest : public CommonGameTestFixture,
                                public ::testing::WithParamInterface<GameId> {
protected:
  ResolveGameFilePathTest() : CommonGameTestFixture(GetParam()) {}
};

INSTANTIATE_TEST_SUITE_P(,
                         ResolveGameFilePathTest,
                         ::testing::Values(GameId::tes5se, GameId::openmw));

TEST_P(ResolveGameFilePathTest,
       shouldReturnFilenameInExternalDataPathIfItExistsThere) {
  const auto filePath = localPath / blankEsm;
  std::filesystem::copy(dataPath / blankEsm, filePath);

  const auto pluginPath =
      resolveGameFilePath(GetParam(), {localPath}, dataPath, blankEsm);

  EXPECT_EQ(filePath, pluginPath);
}

TEST_P(ResolveGameFilePathTest, shouldCheckExternalDataPathsInOrder) {
  const auto filePath = localPath / blankEsm;
  std::filesystem::copy(dataPath / blankEsm, filePath);

  const auto otherDataPath = localPath.parent_path() / "other";
  std::filesystem::create_directories(otherDataPath);
  std::filesystem::copy(dataPath / blankEsm, otherDataPath / blankEsm);

  const auto pluginPath = resolveGameFilePath(
      GetParam(), {localPath, otherDataPath}, dataPath, blankEsm);

  if (GetParam() == GameId::openmw) {
    // Should check in reverse order for OpenMW.
    EXPECT_EQ(otherDataPath / blankEsm, pluginPath);
  } else {
    EXPECT_EQ(filePath, pluginPath);
  }
}

TEST_P(
    ResolveGameFilePathTest,
    shouldReturnGhostedPluginNameInExternalDataPathIfItExistsAsAGhostedPluginThere) {
  const std::string filename = "external.esp";
  const auto ghostedFilename = filename + ".ghost";
  std::filesystem::copy(dataPath / blankEsm, localPath / ghostedFilename);

  const auto pluginPath =
      resolveGameFilePath(GetParam(), {localPath}, dataPath, filename);

  if (GetParam() == GameId::openmw) {
    EXPECT_FALSE(pluginPath.has_value());
  } else {
    EXPECT_EQ(localPath / ghostedFilename, pluginPath);
  }
}

TEST_P(ResolveGameFilePathTest,
       shouldReturnFilenameInDataPathIfTheFileOnlyExistsThere) {
  const auto pluginPath =
      resolveGameFilePath(GetParam(), {localPath}, dataPath, blankEsm);

  EXPECT_EQ(dataPath / blankEsm, pluginPath);
}

TEST_P(
    ResolveGameFilePathTest,
    shouldReturnGhostedFilenameInDataPathIfTheFileOnlyExistsThereAsAGhostedPlugin) {
  const auto pluginPath = resolveGameFilePath(
      GetParam(), {localPath}, dataPath, blankMasterDependentEsm);

  if (GetParam() == GameId::openmw) {
    EXPECT_FALSE(pluginPath.has_value());
  } else {
    EXPECT_EQ(dataPath / (std::string(blankMasterDependentEsm) + ".ghost"),
              pluginPath);
  }
}

TEST_P(ResolveGameFilePathTest,
       shouldReturnNulloptIfTheFileDoesNotExistInAnyOfTheDataPaths) {
  const auto pluginPath =
      resolveGameFilePath(GetParam(), {localPath}, dataPath, "missing.esp");

  EXPECT_FALSE(pluginPath.has_value());
}
}
}

#endif
