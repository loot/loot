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

TEST(PlainTextMessage, shouldEscapeMarkdownSpecialCharacters) {
  auto message =
      PlainTextMessage(MessageType::say, "normal text\\`*_{}[]()#+-.!");

  auto expectedText = R"raw(normal text\\\`\*\_\{\}\[\]\(\)\#\+\-\.\!)raw";
  EXPECT_EQ(expectedText, message.GetContent()[0].GetText());
}

TEST(ToMessage, shouldOutputAllNonZeroCounts) {
  const auto detail = std::vector<MessageContent>({
      MessageContent("detail"),
  });

  Message message =
      ToMessage(PluginCleaningData(0x12345678, "cleaner", detail, 2, 10, 30));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(
      "cleaner found 2 ITM records, 10 deleted references and 30 deleted "
      "navmeshes. detail",
      SelectMessageContent(message.GetContent(),
                           MessageContent::DEFAULT_LANGUAGE)
          .value()
          .GetText());

  message =
      ToMessage(PluginCleaningData(0x12345678, "cleaner", detail, 0, 0, 0));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found dirty edits. detail",
            SelectMessageContent(message.GetContent(),
                                 MessageContent::DEFAULT_LANGUAGE)
                .value()
                .GetText());

  message =
      ToMessage(PluginCleaningData(0x12345678, "cleaner", detail, 0, 10, 30));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(
      "cleaner found 10 deleted references and 30 deleted navmeshes. detail",
      SelectMessageContent(message.GetContent(),
                           MessageContent::DEFAULT_LANGUAGE)
          .value()
          .GetText());

  message =
      ToMessage(PluginCleaningData(0x12345678, "cleaner", detail, 0, 0, 30));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 30 deleted navmeshes. detail",
            SelectMessageContent(message.GetContent(),
                                 MessageContent::DEFAULT_LANGUAGE)
                .value()
                .GetText());

  message =
      ToMessage(PluginCleaningData(0x12345678, "cleaner", detail, 0, 10, 0));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 10 deleted references. detail",
            SelectMessageContent(message.GetContent(),
                                 MessageContent::DEFAULT_LANGUAGE)
                .value()
                .GetText());

  message =
      ToMessage(PluginCleaningData(0x12345678, "cleaner", detail, 2, 0, 30));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 2 ITM records and 30 deleted navmeshes. detail",
            SelectMessageContent(message.GetContent(),
                                 MessageContent::DEFAULT_LANGUAGE)
                .value()
                .GetText());

  message =
      ToMessage(PluginCleaningData(0x12345678, "cleaner", detail, 2, 0, 0));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 2 ITM records. detail",
            SelectMessageContent(message.GetContent(),
                                 MessageContent::DEFAULT_LANGUAGE)
                .value()
                .GetText());

  message =
      ToMessage(PluginCleaningData(0x12345678, "cleaner", detail, 2, 10, 0));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 2 ITM records and 10 deleted references. detail",
            SelectMessageContent(message.GetContent(),
                                 MessageContent::DEFAULT_LANGUAGE)
                .value()
                .GetText());
}

TEST(ToMessage, shouldDistinguishBetweenSingularAndPluralCounts) {
  const auto detail = std::vector<MessageContent>({
      MessageContent("detail"),
  });

  Message message =
      ToMessage(PluginCleaningData(0x12345678, "cleaner", detail, 1, 2, 3));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(
      "cleaner found 1 ITM record, 2 deleted references and 3 deleted "
      "navmeshes. detail",
      SelectMessageContent(message.GetContent(),
                           MessageContent::DEFAULT_LANGUAGE)
          .value()
          .GetText());

  message =
      ToMessage(PluginCleaningData(0x12345678, "cleaner", detail, 2, 1, 3));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(
      "cleaner found 2 ITM records, 1 deleted reference and 3 deleted "
      "navmeshes. detail",
      SelectMessageContent(message.GetContent(),
                           MessageContent::DEFAULT_LANGUAGE)
          .value()
          .GetText());

  message =
      ToMessage(PluginCleaningData(0x12345678, "cleaner", detail, 3, 2, 1));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(
      "cleaner found 3 ITM records, 2 deleted references and 1 deleted "
      "navmesh. detail",
      SelectMessageContent(message.GetContent(),
                           MessageContent::DEFAULT_LANGUAGE)
          .value()
          .GetText());
}

TEST(ToMessage,
     shouldReturnAMessageWithCountsButNoDetailStringIfDetailIsAnEmptyVector) {
  Message message = ToMessage(PluginCleaningData(
      0x12345678, "cleaner", std::vector<MessageContent>(), 1, 2, 3));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(
      "cleaner found 1 ITM record, 2 deleted references and 3 deleted "
      "navmeshes.",
      SelectMessageContent(message.GetContent(),
                           MessageContent::DEFAULT_LANGUAGE)
          .value()
          .GetText());
}

TEST(SplitRegistryPath, shouldAssumeHKLMIfNoRootKeyIsGiven) {
  auto [rootKey, subKey, value] = SplitRegistryPath("sub\\key\\value");

  EXPECT_EQ("HKEY_LOCAL_MACHINE", rootKey);
  EXPECT_EQ("sub\\key", subKey);
  EXPECT_EQ("value", value);
}

TEST(SplitRegistryPath, shouldUseRootKeyIfSpecified) {
  auto [rootKey, subKey, value] =
      SplitRegistryPath("HKEY_DUMMY\\sub\\key\\value");

  EXPECT_EQ("HKEY_DUMMY", rootKey);
  EXPECT_EQ("sub\\key", subKey);
  EXPECT_EQ("value", value);
}

TEST(SplitRegistryPath, shouldThrowIfNoValueIsGiven) {
  EXPECT_THROW(SplitRegistryPath("HKEY_DUMMY\\subkey\\"),
               std::invalid_argument);
  EXPECT_THROW(SplitRegistryPath("HKEY_DUMMY\\subkey"), std::invalid_argument);
}

TEST(SplitRegistryPath, shouldThrowIfNoSubkeyOrValueAreGiven) {
  EXPECT_THROW(SplitRegistryPath("HKEY_DUMMY\\"), std::invalid_argument);
  EXPECT_THROW(SplitRegistryPath("HKEY_DUMMY"), std::invalid_argument);
}

TEST(SplitRegistryPath, shouldThrowIfNoRootKeyOrValueAreGiven) {
  EXPECT_THROW(SplitRegistryPath("subkey\\"), std::invalid_argument);
  EXPECT_THROW(SplitRegistryPath("subkey"), std::invalid_argument);
}

TEST(SplitRegistryPath, shouldThrowIfInputIsEmptyOrOnlyBackslashes) {
  EXPECT_THROW(SplitRegistryPath("\\\\"), std::invalid_argument);
  EXPECT_THROW(SplitRegistryPath("\\"), std::invalid_argument);
  EXPECT_THROW(SplitRegistryPath(""), std::invalid_argument);
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
}
}

#endif
