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
  EXPECT_EQ("\\\\", EscapeMarkdownASCIIPunctuation("\\"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeBacktick) {
  EXPECT_EQ("\\`", EscapeMarkdownASCIIPunctuation("`"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeAsterisk) {
  EXPECT_EQ("\\*", EscapeMarkdownASCIIPunctuation("*"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeUnderscore) {
  EXPECT_EQ("\\_", EscapeMarkdownASCIIPunctuation("_"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeCurlyBraces) {
  EXPECT_EQ("\\{", EscapeMarkdownASCIIPunctuation("{"));
  EXPECT_EQ("\\}", EscapeMarkdownASCIIPunctuation("}"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeSquareBrackets) {
  EXPECT_EQ("\\[", EscapeMarkdownASCIIPunctuation("["));
  EXPECT_EQ("\\]", EscapeMarkdownASCIIPunctuation("]"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeParentheses) {
  EXPECT_EQ("\\(", EscapeMarkdownASCIIPunctuation("("));
  EXPECT_EQ("\\)", EscapeMarkdownASCIIPunctuation(")"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeHash) {
  EXPECT_EQ("\\#", EscapeMarkdownASCIIPunctuation("#"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapePlus) {
  EXPECT_EQ("\\+", EscapeMarkdownASCIIPunctuation("+"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeHyphen) {
  EXPECT_EQ("\\-", EscapeMarkdownASCIIPunctuation("-"));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapePeriod) {
  EXPECT_EQ("\\.", EscapeMarkdownASCIIPunctuation("."));
}

TEST(EscapeMarkdownASCIIPunctuation, shouldEscapeExclamationMark) {
  EXPECT_EQ("\\!", EscapeMarkdownASCIIPunctuation("!"));
}

TEST(CheckForRemovedPlugins, shouldCompareFilenamesBeforeAndAfter) {
  const auto messages =
      CheckForRemovedPlugins({"dir/test1.esp", "dir/test2.esp", "test3.esp"},
                             {"test1.esp", "test3.esp"});

  ASSERT_EQ(1, messages.size());
  EXPECT_EQ(
      "LOOT has detected that \\\"dir\\/test2\\.esp\\\" is invalid "
      "and is now "
      "ignoring it\\.",
      messages[0].text);
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

  const auto tags = ReadBashTagsFile(in);
  const std::vector<Tag> expectedTags{Tag("Delev"),
                                      Tag("Relev"),
                                      Tag("C.Water", false),
                                      Tag("C.Location"),
                                      Tag("C.LockList", false)};

  EXPECT_EQ(expectedTags, tags);
}

TEST(ReadBashTagsFile, shouldReturnAnEmptyVectorIfThePathDoesNotExist) {
  EXPECT_TRUE(
      ReadBashTagsFile(std::filesystem::temp_directory_path() / "missing",
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

  const auto tags = ReadBashTagsFile(dataPath, "Blank.esp");

  const std::vector<Tag> expectedTags{
      Tag("C.Location"), Tag("Delev"), Tag("Relev", false)};

  EXPECT_EQ(expectedTags, tags);

  std::filesystem::remove_all(dataPath);
}

TEST(GetTagConflicts,
     shouldReturnTagNamesAddedByOneSourceAndRemovedByTheOther) {
  const auto conflicts =
      GetTagConflicts({Tag("A", false), Tag("B"), Tag("C", false), Tag("D")},
                      {Tag("A"), Tag("B", false), Tag("C", false)});

  const std::vector<std::string> expectedConflicts{"A", "B"};

  EXPECT_EQ(expectedConflicts, conflicts);
}

class ResolveGameFilePathTest : public CommonGameTestFixture {
protected:
  ResolveGameFilePathTest() : CommonGameTestFixture(GameId::tes5se) {}
};

TEST_F(ResolveGameFilePathTest,
       shouldReturnFilenameInExternalDataPathIfItExistsThere) {
  const auto filename = "external.esp";
  const auto filePath = localPath / filename;
  std::filesystem::copy(dataPath / blankEsm, filePath);

  const auto pluginPath = ResolveGameFilePath({localPath}, dataPath, filename);

  EXPECT_EQ(filePath, pluginPath);
}

TEST_F(
    ResolveGameFilePathTest,
    shouldReturnPluginNameInExternalDataPathIfItExistsAsAGhostedPluginThere) {
  const std::string filename = "external.esp";
  std::filesystem::copy(dataPath / blankEsm, localPath / (filename + ".ghost"));

  const auto pluginPath = ResolveGameFilePath({localPath}, dataPath, filename);

  EXPECT_EQ(localPath / filename, pluginPath);
}

TEST_F(
    ResolveGameFilePathTest,
    shouldReturnFilenameInDataPathIfTheFileDoesNotExistInAnExternalDataPath) {
  const auto filename = "test.esp";
  const auto pluginPath = ResolveGameFilePath({localPath}, dataPath, filename);

  EXPECT_EQ(dataPath / filename, pluginPath);
}

TEST(GetExternalDataPaths,
     shouldReturnAnEmptyVectorIfTheGameIsNotAMicrosoftStoreInstall) {
  const auto dataPath = std::filesystem::u8path("data");
  const auto paths = GetExternalDataPaths(GameId::fo4, false, dataPath);

  EXPECT_TRUE(paths.empty());
}

TEST(GetExternalDataPaths, shouldReturnAnEmptyVectorIfTheGameIsNotFallout4) {
  const auto dataPath = std::filesystem::u8path("data");
  const auto paths = GetExternalDataPaths(GameId::tes5se, true, dataPath);

  EXPECT_TRUE(paths.empty());
}

TEST(GetExternalDataPaths,
     shouldReturnDlcPluginPathsIfTheGameIsAMicrosoftStoreInstallOfFallout4) {
  const auto dataPath = std::filesystem::u8path("data");
  const auto paths = GetExternalDataPaths(GameId::fo4, true, dataPath);

  EXPECT_EQ(std::vector<std::filesystem::path>(
                {dataPath / "../../../Fallout 4- Automatron (PC)/Content/Data",
                 dataPath / "../../../Fallout 4- Nuka-World "
                            "(PC)/Content/Data",
                 dataPath / "../../../Fallout 4- Wasteland Workshop "
                            "(PC)/Content/Data",
                 dataPath / "../../../Fallout 4- High Resolution Texture "
                            "Pack/Content/Data",
                 dataPath / "../../../Fallout 4- Vault-Tec Workshop "
                            "(PC)/Content/Data",
                 dataPath / "../../../Fallout 4- Far Harbor (PC)/Content/Data",
                 dataPath / "../../../Fallout 4- Contraptions Workshop "
                            "(PC)/Content/Data"}),
            paths);
}
}
}

#endif
