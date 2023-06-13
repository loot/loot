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
#ifndef LOOT_TESTS_GUI_SOURCED_MESSAGE_TEST
#define LOOT_TESTS_GUI_SOURCED_MESSAGE_TEST

#include <gtest/gtest.h>

#include "gui/sourced_message.h"

namespace loot::test {
TEST(SourcedMessage, equalityOperatorShouldReturnTrueIfAllFieldsAreEqual) {
  const auto message1 =
      SourcedMessage{MessageType::say, MessageSource::init, ""};
  const auto message2 = message1;

  EXPECT_TRUE(message1 == message2);
}

TEST(SourcedMessage, equalityOperatorShouldReturnFalseIfAnyFieldIsNotEqual) {
  const auto message1 =
      SourcedMessage{MessageType::say, MessageSource::init, ""};
  auto message2 = SourcedMessage{MessageType::warn, MessageSource::init, ""};

  EXPECT_FALSE(message1 == message2);

  message2 =
      SourcedMessage{MessageType::say, MessageSource::inactiveMaster, ""};
  EXPECT_FALSE(message1 == message2);

  message2 = SourcedMessage{MessageType::say, MessageSource::init, "different"};
  EXPECT_FALSE(message1 == message2);
}

TEST(SourcedMessage, inequalityOperatorShouldReturnFalseIfAllFieldsAreEqual) {
  const auto message1 =
      SourcedMessage{MessageType::say, MessageSource::init, ""};
  const auto message2 = message1;

  EXPECT_FALSE(message1 != message2);
}

TEST(SourcedMessage, equalityOperatorShouldReturnTrueIfAnyFieldIsNotEqual) {
  const auto message1 =
      SourcedMessage{MessageType::say, MessageSource::init, ""};
  auto message2 = SourcedMessage{MessageType::warn, MessageSource::init, ""};

  EXPECT_TRUE(message1 != message2);

  message2 =
      SourcedMessage{MessageType::say, MessageSource::inactiveMaster, ""};
  EXPECT_TRUE(message1 != message2);

  message2 = SourcedMessage{MessageType::say, MessageSource::init, "different"};
  EXPECT_TRUE(message1 != message2);
}

TEST(CreatePlainTextSourcedMessage, shouldEscapeMarkdownSpecialCharacters) {
  const auto message = CreatePlainTextSourcedMessage(
      MessageType::say, MessageSource::init, "normal text\\`*_{}[]()#+-.!");

  const auto expectedText =
      R"raw(normal text\\\`\*\_\{\}\[\]\(\)\#\+\-\.\!)raw";
  EXPECT_EQ(expectedText, message.text);
}

TEST(ToSourcedMessages, shouldSkipAnyMessagesWithoutContentInTheGivenLanguage) {
  const std::vector<Message> messages{
      Message(MessageType::say,
              {MessageContent("1_en", "en"), MessageContent("1_fr", "fr")}),
      Message(MessageType::warn, std::vector<MessageContent>()),
      Message(MessageType::error,
              {MessageContent("3_en", "en"), MessageContent("3_fr", "fr")}),
  };

  const auto sourcedMessages =
      ToSourcedMessages(messages, MessageSource::init, "fr");

  ASSERT_EQ(2, sourcedMessages.size());
  EXPECT_EQ(MessageType::say, sourcedMessages[0].type);
  EXPECT_EQ(MessageSource::init, sourcedMessages[0].source);
  EXPECT_EQ("1_fr", sourcedMessages[0].text);
  EXPECT_EQ(MessageType::error, sourcedMessages[1].type);
  EXPECT_EQ(MessageSource::init, sourcedMessages[1].source);
  EXPECT_EQ("3_fr", sourcedMessages[1].text);
}
}

#endif
