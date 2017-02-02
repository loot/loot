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

#ifndef LOOT_TESTS_BACKEND_METADATA_MESSAGE_TEST
#define LOOT_TESTS_BACKEND_METADATA_MESSAGE_TEST

#include "loot/metadata/message.h"

#include "api/game/game.h"
#include "loot/yaml/message.h"
#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class MessageTest : public CommonGameTestFixture {
protected:

  typedef std::vector<MessageContent> MessageContents;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        MessageTest,
                        ::testing::Values(
                          GameType::tes4));

TEST_P(MessageTest, defaultConstructorShouldCreateNoteWithNoContent) {
  Message message;
  EXPECT_EQ(MessageType::say, message.GetType());
  EXPECT_EQ(MessageContents(), message.GetContent());
}

TEST_P(MessageTest, scalarContentConstructorShouldCreateAMessageWithASingleContentString) {
  MessageContent content = MessageContent("content1", LanguageCode::english);
  Message message(MessageType::warn, content.GetText(), "condition1");

  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(MessageContents({content}), message.GetContent());
  EXPECT_EQ("condition1", message.GetCondition());
}

TEST_P(MessageTest, vectorContentConstructorShouldCreateAMessageWithGivenContentStrings) {
  MessageContents contents({
      MessageContent("content1", LanguageCode::english),
      MessageContent("content2", LanguageCode::french),
  });
  Message message(MessageType::error, contents, "condition1");

  EXPECT_EQ(MessageType::error, message.GetType());
  EXPECT_EQ(contents, message.GetContent());
  EXPECT_EQ("condition1", message.GetCondition());
}

TEST_P(MessageTest, vectorContentConstructorShouldThrowIfMultipleContentStringsAreGivenAndNoneAreEnglish) {
  MessageContents contents({
      MessageContent("content1", LanguageCode::german),
      MessageContent("content2", LanguageCode::french),
  });
  EXPECT_THROW(Message(MessageType::error, contents, "condition1"), std::invalid_argument);
}

TEST_P(MessageTest, messagesWithDifferentContentStringsShouldBeUnequal) {
  Message message1(MessageType::say, "content1", "condition1");
  Message message2(MessageType::say, "content2", "condition1");

  EXPECT_FALSE(message1 == message2);
}

TEST_P(MessageTest, messagesWithEqualContentStringsShouldBeEqual) {
  Message message1(MessageType::say, MessageContents({MessageContent("content1", LanguageCode::english)}), "condition1");
  Message message2(MessageType::warn, MessageContents({MessageContent("content1", LanguageCode::french)}), "condition2");

  EXPECT_TRUE(message1 == message2);
}

TEST_P(MessageTest, lessThanOperatorShouldUseCaseInsensitiveLexicographicalContentStringComparison) {
  Message message1(MessageType::say, MessageContents({MessageContent("content1", LanguageCode::english)}), "condition1");
  Message message2(MessageType::warn, MessageContents({MessageContent("content1", LanguageCode::french)}), "condition2");
  EXPECT_FALSE(message1 < message2);
  EXPECT_FALSE(message2 < message1);

  message1 = Message(MessageType::say, "content1", "condition1");
  message2 = Message(MessageType::say, "content2", "condition1");
  EXPECT_TRUE(message1 < message2);
  EXPECT_FALSE(message2 < message1);
}

TEST_P(MessageTest, getContentShouldReturnADefaultContentObjectIfNoneExists) {
  Message message;
  EXPECT_EQ(MessageContent(), message.GetContent(LanguageCode::english));
}

TEST_P(MessageTest, getContentShouldSelectTheEnglishStringIfThereIsNoStringForTheGivenLanguage) {
  Message message(MessageType::say, MessageContents({
    MessageContent("content1", LanguageCode::german),
    MessageContent("content2", LanguageCode::english),
    MessageContent("content3", LanguageCode::french),
  }));

  EXPECT_EQ("content2", message.GetContent(LanguageCode::korean).GetText());
}

TEST_P(MessageTest, getContentShouldSelectTheGivenLanguageStringIfItExists) {
  Message message(MessageType::say, MessageContents({
    MessageContent("content1", LanguageCode::german),
    MessageContent("content2", LanguageCode::english),
    MessageContent("content3", LanguageCode::french),
  }));

  EXPECT_EQ("content3", message.GetContent(LanguageCode::french).GetText());
}

TEST_P(MessageTest, getContentShouldSelectTheContentStringIfOnlyOneExists) {
  Message message(MessageType::say, MessageContents({
    MessageContent("content1", LanguageCode::german),
  }));

  EXPECT_EQ("content1", message.GetContent(LanguageCode::english).GetText());
}

TEST_P(MessageTest, toSimpleMessageShouldSelectTextAndLanguageUsingGetContent) {
  Message message(MessageType::warn, MessageContents({
    MessageContent("content1", LanguageCode::german),
    MessageContent("content2", LanguageCode::english),
    MessageContent("content3", LanguageCode::french),
  }));

  SimpleMessage simpleMessage = message.ToSimpleMessage(LanguageCode::french);

  EXPECT_EQ(MessageType::warn, simpleMessage.type);
  EXPECT_EQ("content3", simpleMessage.text);
  EXPECT_EQ(LanguageCode::french, simpleMessage.language);
}

TEST_P(MessageTest, emittingAsYamlShouldOutputNoteMessageTypeCorrectly) {
  Message message(MessageType::say, "content1");
  YAML::Emitter emitter;
  emitter << message;

  EXPECT_STREQ("type: say\n"
               "content: 'content1'", emitter.c_str());
}

TEST_P(MessageTest, emittingAsYamlShouldOutputWarnMessageTypeCorrectly) {
  Message message(MessageType::warn, "content1");
  YAML::Emitter emitter;
  emitter << message;

  EXPECT_STREQ("type: warn\n"
               "content: 'content1'", emitter.c_str());
}

TEST_P(MessageTest, emittingAsYamlShouldOutputErrorMessageTypeCorrectly) {
  Message message(MessageType::error, "content1");
  YAML::Emitter emitter;
  emitter << message;

  EXPECT_STREQ("type: error\n"
               "content: 'content1'", emitter.c_str());
}

TEST_P(MessageTest, emittingAsYamlShouldOutputConditionIfItIsNotEmpty) {
  Message message(MessageType::say, "content1", "condition1");
  YAML::Emitter emitter;
  emitter << message;

  EXPECT_STREQ("type: say\n"
               "content: 'content1'\n"
               "condition: 'condition1'", emitter.c_str());
}

TEST_P(MessageTest, emittingAsYamlShouldOutputMultipleContentStringsAsAList) {
  Message message(MessageType::say, MessageContents({
      MessageContent("content1", LanguageCode::english),
      MessageContent("content2", LanguageCode::german)
  }));
  YAML::Emitter emitter;
  emitter << message;

  EXPECT_STREQ("type: say\n"
               "content:\n"
               "  - lang: en\n"
               "    text: 'content1'\n"
               "  - lang: de\n"
               "    text: 'content2'", emitter.c_str());
}

TEST_P(MessageTest, encodingAsYamlShouldStoreNoteMessageTypeCorrectly) {
  Message message(MessageType::say, "content1");
  YAML::Node node;
  node = message;

  EXPECT_EQ("say", node["type"].as<std::string>());
}

TEST_P(MessageTest, encodingAsYamlShouldStoreWarningMessageTypeCorrectly) {
  Message message(MessageType::warn, "content1");
  YAML::Node node;
  node = message;

  EXPECT_EQ("warn", node["type"].as<std::string>());
}

TEST_P(MessageTest, encodingAsYamlShouldStoreErrorMessageTypeCorrectly) {
  Message message(MessageType::error, "content1");
  YAML::Node node;
  node = message;

  EXPECT_EQ("error", node["type"].as<std::string>());
}

TEST_P(MessageTest, encodingAsYamlShouldOmitConditionFieldIfItIsEmpty) {
  Message message(MessageType::say, "content1");
  YAML::Node node;
  node = message;

  EXPECT_FALSE(node["condition"]);
}

TEST_P(MessageTest, encodingAsYamlShouldStoreConditionFieldIfItIsNotEmpty) {
  Message message(MessageType::say, "content1", "condition1");
  YAML::Node node;
  node = message;

  EXPECT_EQ("condition1", node["condition"].as<std::string>());
}

TEST_P(MessageTest, encodingAsYamlShouldStoreASingleContentStringInAVector) {
  Message message(MessageType::say, "content1");
  YAML::Node node;
  node = message;

  EXPECT_EQ(message.GetContent(), node["content"].as<MessageContents>());
}

TEST_P(MessageTest, encodingAsYamlShouldMultipleContentStringsInAVector) {
  MessageContents contents({
      MessageContent("content1", LanguageCode::english),
      MessageContent("content2", LanguageCode::french),
  });
  Message message(MessageType::say, contents);
  YAML::Node node;
  node = message;

  EXPECT_EQ(contents, node["content"].as<MessageContents>());
}

TEST_P(MessageTest, decodingFromYamlShouldSetNoteTypeCorrectly) {
  YAML::Node node = YAML::Load("type: say\n"
                               "content: content1");
  Message message = node.as<Message>();

  EXPECT_EQ(MessageType::say, message.GetType());
}

TEST_P(MessageTest, decodingFromYamlShouldSetWarningTypeCorrectly) {
  YAML::Node node = YAML::Load("type: warn\n"
                               "content: content1");
  Message message = node.as<Message>();

  EXPECT_EQ(MessageType::warn, message.GetType());
}

TEST_P(MessageTest, decodingFromYamlShouldSetErrorTypeCorrectly) {
  YAML::Node node = YAML::Load("type: error\n"
                               "content: content1");
  Message message = node.as<Message>();

  EXPECT_EQ(MessageType::error, message.GetType());
}

TEST_P(MessageTest, decodingFromYamlShouldHandleAnUnrecognisedTypeAsANote) {
  YAML::Node node = YAML::Load("type: invalid\n"
                               "content: content1");
  Message message = node.as<Message>();

  EXPECT_EQ(MessageType::say, message.GetType());
}

TEST_P(MessageTest, decodingFromYamlShouldLeaveTheConditionEmptyIfNoneIsPresent) {
  YAML::Node node = YAML::Load("type: say\n"
                               "content: content1");
  Message message = node.as<Message>();

  EXPECT_TRUE(message.GetCondition().empty());
}

TEST_P(MessageTest, decodingFromYamlShouldStoreANonEmptyConditionField) {
  YAML::Node node = YAML::Load("type: say\n"
                               "content: content1\n"
                               "condition: 'file(\"Foo.esp\")'");
  Message message = node.as<Message>();

  EXPECT_EQ("file(\"Foo.esp\")", message.GetCondition());
}

TEST_P(MessageTest, decodingFromYamlShouldStoreAScalarContentValueCorrectly) {
  YAML::Node node = YAML::Load("type: say\n"
                               "content: content1\n");
  Message message = node.as<Message>();
  MessageContents expectedContent({MessageContent("content1", LanguageCode::english)});

  EXPECT_EQ(expectedContent, message.GetContent());
}

TEST_P(MessageTest, decodingFromYamlShouldStoreAListOfContentStringsCorrectly) {
  YAML::Node node = YAML::Load("type: say\n"
                               "content:\n"
                               "  - lang: en\n"
                               "    text: content1\n"
                               "  - lang: de\n"
                               "    text: content2");
  Message message = node.as<Message>();

  EXPECT_EQ(MessageContents({
      MessageContent("content1", LanguageCode::english),
      MessageContent("content2", LanguageCode::german),
  }), message.GetContent());
}

TEST_P(MessageTest, decodingFromYamlShouldNotThrowIfTheOnlyContentStringIsNotEnglish) {
  YAML::Node node = YAML::Load("type: say\n"
                               "content:\n"
                               "  - lang: fr\n"
                               "    text: content1");

  EXPECT_NO_THROW(Message message = node.as<Message>());
}

TEST_P(MessageTest, decodingFromYamlShouldThrowIfMultipleContentStringsAreGivenAndNoneAreEnglish) {
  YAML::Node node = YAML::Load("type: say\n"
                               "content:\n"
                               "  - lang: de\n"
                               "    text: content1\n"
                               "  - lang: fr\n"
                               "    text: content2");

  EXPECT_THROW(node.as<Message>(), YAML::RepresentationException);
}

TEST_P(MessageTest, decodingFromYamlShouldApplySubstitutionsWhenThereIsOnlyOneContentString) {
  YAML::Node node = YAML::Load("type: say\n"
                               "content: con%1%tent1\n"
                               "subs:\n"
                               "  - sub1");
  Message message = node.as<Message>();

  EXPECT_EQ(MessageContents({MessageContent("consub1tent1", LanguageCode::english)}), message.GetContent());
}

TEST_P(MessageTest, decodingFromYamlShouldApplySubstitutionsToAllContentStrings) {
  YAML::Node node = YAML::Load("type: say\n"
                               "content:\n"
                               "  - lang: en\n"
                               "    text: content1 %1%\n"
                               "  - lang: de\n"
                               "    text: content2 %1%\n"
                               "subs:\n"
                               "  - sub");
  Message message = node.as<Message>();

  EXPECT_EQ(MessageContents({
      MessageContent("content1 sub", LanguageCode::english),
      MessageContent("content2 sub", LanguageCode::german),
  }), message.GetContent());
}

TEST_P(MessageTest, decodingFromYamlShouldThrowIfTheContentStringExpectsMoreSubstitutionsThanExist) {
  YAML::Node node = YAML::Load("type: say\n"
                               "content: '%1% %2%'\n"
                               "subs:\n"
                               "  - sub1");

  EXPECT_THROW(node.as<Message>(), YAML::RepresentationException);
}

// Don't throw because no subs are given, so none are expected in the content string.
TEST_P(MessageTest, decodingFromYamlShouldIgnoreSubstitutionSyntaxIfNoSubstitutionsExist) {
  YAML::Node node = YAML::Load("type: say\n"
                               "content: con%1%tent1\n");
  Message message = node.as<Message>();

  EXPECT_EQ(MessageContents({MessageContent("con%1%tent1", LanguageCode::english)}), message.GetContent());
}

TEST_P(MessageTest, decodingFromYamlShouldThrowIfAnInvalidConditionIsGiven) {
  YAML::Node node = YAML::Load("type: say\n"
                               "content: content1\n"
                               "condition: invalid");

  EXPECT_THROW(node.as<Message>(), YAML::RepresentationException);
}

TEST_P(MessageTest, decodingFromYamlShouldThrowIfAScalarIsGiven) {
  YAML::Node node = YAML::Load("scalar");

  EXPECT_THROW(node.as<Message>(), YAML::RepresentationException);
}

TEST_P(MessageTest, decodingFromYamlShouldThrowIfAListIsGiven) {
  YAML::Node node = YAML::Load("[0, 1, 2]");

  EXPECT_THROW(node.as<Message>(), YAML::RepresentationException);
}
}
}

#endif
