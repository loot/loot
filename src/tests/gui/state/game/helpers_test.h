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

#include "gui/state/game/helpers.h"

#include <gtest/gtest.h>

namespace loot {
namespace test {

TEST(EscapeMarkdownSpecialChars, shouldEscapeBackslash) {
  EXPECT_EQ("\\\\", EscapeMarkdownSpecialChars("\\"));
}

TEST(EscapeMarkdownSpecialChars, shouldEscapeBacktick) {
  EXPECT_EQ("\\`", EscapeMarkdownSpecialChars("`"));
}

TEST(EscapeMarkdownSpecialChars, shouldEscapeAsterisk) {
  EXPECT_EQ("\\*", EscapeMarkdownSpecialChars("*"));
}

TEST(EscapeMarkdownSpecialChars, shouldEscapeUnderscore) {
  EXPECT_EQ("\\_", EscapeMarkdownSpecialChars("_"));
}

TEST(EscapeMarkdownSpecialChars, shouldEscapeCurlyBraces) {
  EXPECT_EQ("\\{", EscapeMarkdownSpecialChars("{"));
  EXPECT_EQ("\\}", EscapeMarkdownSpecialChars("}"));
}

TEST(EscapeMarkdownSpecialChars, shouldEscapeSquareBrackets) {
  EXPECT_EQ("\\[", EscapeMarkdownSpecialChars("["));
  EXPECT_EQ("\\]", EscapeMarkdownSpecialChars("]"));
}

TEST(EscapeMarkdownSpecialChars, shouldEscapeParentheses) {
  EXPECT_EQ("\\(", EscapeMarkdownSpecialChars("("));
  EXPECT_EQ("\\)", EscapeMarkdownSpecialChars(")"));
}

TEST(EscapeMarkdownSpecialChars, shouldEscapeHash) {
  EXPECT_EQ("\\#", EscapeMarkdownSpecialChars("#"));
}

TEST(EscapeMarkdownSpecialChars, shouldEscapePlus) {
  EXPECT_EQ("\\+", EscapeMarkdownSpecialChars("+"));
}

TEST(EscapeMarkdownSpecialChars, shouldEscapeHyphen) {
  EXPECT_EQ("\\-", EscapeMarkdownSpecialChars("-"));
}

TEST(EscapeMarkdownSpecialChars, shouldEscapePeriod) {
  EXPECT_EQ("\\.", EscapeMarkdownSpecialChars("."));
}

TEST(EscapeMarkdownSpecialChars, shouldEscapeExclamationMark) {
  EXPECT_EQ("\\!", EscapeMarkdownSpecialChars("!"));
}

TEST(EscapeMarkdownSpecialChars, shouldNotEscapeNonSpecialCharacters) {
  auto text = "normal, text & \"symbols\"";
  EXPECT_EQ(text, EscapeMarkdownSpecialChars(text));
}

TEST(PlainTextMessage, shouldEscapeMarkdownSpecialCharacters) {
  auto message = PlainTextMessage(MessageType::say, "normal text\\`*_{}[]()#+-.!");

  auto expectedText = R"raw(normal text\\\`\*\_\{\}\[\]\(\)\#\+\-\.\!)raw";
  EXPECT_EQ(expectedText, message.GetContent()[0].GetText());
}

TEST(ToMessage, shouldOutputAllNonZeroCounts) {
  const auto info = std::vector<MessageContent>({
      MessageContent("info"),
  });

  Message message =
      ToMessage(PluginCleaningData(0x12345678, "cleaner", info, 2, 10, 30));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(
      "cleaner found 2 ITM records, 10 deleted references and 30 deleted "
      "navmeshes. info",
      message.GetContent(MessageContent::defaultLanguage).GetText());

  message = ToMessage(PluginCleaningData(0x12345678, "cleaner", info, 0, 0, 0));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found dirty edits. info",
            message.GetContent(MessageContent::defaultLanguage).GetText());

  message =
      ToMessage(PluginCleaningData(0x12345678, "cleaner", info, 0, 10, 30));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(
      "cleaner found 10 deleted references and 30 deleted navmeshes. info",
      message.GetContent(MessageContent::defaultLanguage).GetText());

  message =
      ToMessage(PluginCleaningData(0x12345678, "cleaner", info, 0, 0, 30));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 30 deleted navmeshes. info",
            message.GetContent(MessageContent::defaultLanguage).GetText());

  message =
      ToMessage(PluginCleaningData(0x12345678, "cleaner", info, 0, 10, 0));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 10 deleted references. info",
            message.GetContent(MessageContent::defaultLanguage).GetText());

  message =
      ToMessage(PluginCleaningData(0x12345678, "cleaner", info, 2, 0, 30));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 2 ITM records and 30 deleted navmeshes. info",
            message.GetContent(MessageContent::defaultLanguage).GetText());

  message = ToMessage(PluginCleaningData(0x12345678, "cleaner", info, 2, 0, 0));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 2 ITM records. info",
            message.GetContent(MessageContent::defaultLanguage).GetText());

  message =
      ToMessage(PluginCleaningData(0x12345678, "cleaner", info, 2, 10, 0));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 2 ITM records and 10 deleted references. info",
            message.GetContent(MessageContent::defaultLanguage).GetText());
}

TEST(ToMessage, shouldDistinguishBetweenSingularAndPluralCounts) {
  const auto info = std::vector<MessageContent>({
      MessageContent("info"),
  });

  Message message =
      ToMessage(PluginCleaningData(0x12345678, "cleaner", info, 1, 2, 3));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(
      "cleaner found 1 ITM record, 2 deleted references and 3 deleted "
      "navmeshes. info",
      message.GetContent(MessageContent::defaultLanguage).GetText());

  message = ToMessage(PluginCleaningData(0x12345678, "cleaner", info, 2, 1, 3));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(
      "cleaner found 2 ITM records, 1 deleted reference and 3 deleted "
      "navmeshes. info",
      message.GetContent(MessageContent::defaultLanguage).GetText());

  message = ToMessage(PluginCleaningData(0x12345678, "cleaner", info, 3, 2, 1));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(
      "cleaner found 3 ITM records, 2 deleted references and 1 deleted "
      "navmesh. info",
      message.GetContent(MessageContent::defaultLanguage).GetText());
}

TEST(ToMessage,
     shouldReturnAMessageWithCountsButNoInfoStringIfInfoIsAnEmptyString) {
  Message message = ToMessage(PluginCleaningData(
      0x12345678, "cleaner", std::vector<MessageContent>(), 1, 2, 3));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(
      "cleaner found 1 ITM record, 2 deleted references and 3 deleted "
      "navmeshes.",
      message.GetContent(MessageContent::defaultLanguage).GetText());
}

TEST(SplitRegistryPath, shouldAssumeHKLMIfNoRootKeyIsGiven) {
  auto[rootKey, subKey, value] = SplitRegistryPath("sub\\key\\value");

  EXPECT_EQ("HKEY_LOCAL_MACHINE", rootKey);
  EXPECT_EQ("sub\\key", subKey);
  EXPECT_EQ("value", value);
}

TEST(SplitRegistryPath, shouldUseRootKeyIfSpecified) {
  auto[rootKey, subKey, value] = SplitRegistryPath("HKEY_DUMMY\\sub\\key\\value");

  EXPECT_EQ("HKEY_DUMMY", rootKey);
  EXPECT_EQ("sub\\key", subKey);
  EXPECT_EQ("value", value);
}

TEST(SplitRegistryPath, shouldThrowIfNoValueIsGiven) {
  EXPECT_THROW(SplitRegistryPath("HKEY_DUMMY\\subkey\\"), std::invalid_argument);
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
}
}

#endif
