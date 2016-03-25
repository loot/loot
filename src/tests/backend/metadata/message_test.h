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
<http://www.gnu.org/licenses/>.
*/

#ifndef LOOT_TEST_BACKEND_METADATA_MESSAGE
#define LOOT_TEST_BACKEND_METADATA_MESSAGE

#include "backend/game/game.h"
#include "backend/metadata/message.h"
#include "tests/base_game_test.h"

namespace loot {
    namespace test {
        class MessageTest : public BaseGameTest {
        protected:

            typedef std::vector<MessageContent> MessageContents;
        };

        // Pass an empty first argument, as it's a prefix for the test instantation,
        // but we only have the one so no prefix is necessary.
        INSTANTIATE_TEST_CASE_P(,
                                MessageTest,
                                ::testing::Values(
                                    GameSettings::tes4));

        TEST_P(MessageTest, defaultConstructorShouldCreateNoteWithNoContent) {
            Message message;
            EXPECT_EQ(Message::say, message.Type());
            EXPECT_EQ(MessageContents(), message.Content());
        }

        TEST_P(MessageTest, scalarContentConstructorShouldCreateAMessageWithASingleContentString) {
            MessageContent content = MessageContent("content1", Language::english);
            Message message(Message::warn, content.Str(), "condition1");

            EXPECT_EQ(Message::warn, message.Type());
            EXPECT_EQ(MessageContents({content}), message.Content());
            EXPECT_EQ("condition1", message.Condition());
        }

        TEST_P(MessageTest, vectorContentConstructorShouldCreateAMessageWithGivenContentStrings) {
            MessageContents contents({
                MessageContent("content1", Language::english),
                MessageContent("content2", Language::french),
            });
            Message message(Message::error, contents, "condition1");

            EXPECT_EQ(Message::error, message.Type());
            EXPECT_EQ(contents, message.Content());
            EXPECT_EQ("condition1", message.Condition());
        }

        TEST_P(MessageTest, messagesWithDifferentContentStringsShouldBeUnequal) {
            Message message1(Message::say, "content1", "condition1");
            Message message2(Message::say, "content2", "condition1");

            EXPECT_FALSE(message1 == message2);
        }

        TEST_P(MessageTest, messagesWithEqualContentStringsShouldBeEqual) {
            Message message1(Message::say, MessageContents({MessageContent("content1", Language::english)}), "condition1");
            Message message2(Message::warn, MessageContents({MessageContent("content1", Language::french)}), "condition2");

            EXPECT_TRUE(message1 == message2);
        }

        TEST_P(MessageTest, LessThanOperatorShouldUseCaseInsensitiveLexicographicalContentStringComparison) {
            Message message1(Message::say, MessageContents({MessageContent("content1", Language::english)}), "condition1");
            Message message2(Message::warn, MessageContents({MessageContent("content1", Language::french)}), "condition2");
            EXPECT_FALSE(message1 < message2);
            EXPECT_FALSE(message2 < message1);

            message1 = Message(Message::say, "content1", "condition1");
            message2 = Message(Message::say, "content2", "condition1");
            EXPECT_TRUE(message1 < message2);
            EXPECT_FALSE(message2 < message1);
        }

        TEST_P(MessageTest, evalConditionShouldCreateADefaultContentObjectIfNoneExists) {
            Game game(GetParam());
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            Message message;
            EXPECT_TRUE(message.EvalCondition(game, Language::english));
            EXPECT_EQ(MessageContents({MessageContent()}), message.Content());
        }

        TEST_P(MessageTest, evalConditionShouldPickOneContentStringIfMoreThanOneExists) {
            Game game(GetParam());
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            Message message(Message::say, MessageContents({
                MessageContent("content1", Language::german),
                MessageContent("content2", Language::english),
                MessageContent("content3", Language::french),
            }));

            EXPECT_TRUE(message.EvalCondition(game, Language::french));
            EXPECT_EQ(1, message.Content().size());
            EXPECT_EQ(MessageContent("content3", Language::french), message.Content()[0]);
        }

        TEST_P(MessageTest, chooseContentShouldCreateADefaultContentObjectIfNoneExists) {
            EXPECT_EQ(MessageContent(), Message().ChooseContent(Language::english));
        }

        TEST_P(MessageTest, chooseContentShouldLeaveTheContentUnchangedIfOnlyOneStringExists) {
            MessageContent content("content1", Language::english);
            Message message(Message::say, MessageContents({content}));

            EXPECT_EQ(content, message.ChooseContent(Language::french));
            EXPECT_EQ(content, message.ChooseContent(Language::english));
        }

        TEST_P(MessageTest, chooseContentShouldSelectTheEnglishStringIfNoStringExistsForTheGivenLanguage) {
            MessageContent content("content1", Language::english);
            Message message(Message::say, MessageContents({
                content,
                MessageContent("content1", Language::german),
            }));
            EXPECT_EQ(content, message.ChooseContent(Language::french));
        }

        TEST_P(MessageTest, chooseContentShouldSelectTheStringForTheGivenLanguageIfOneExists) {
            MessageContent french("content3", Language::french);
            Message message(Message::say, MessageContents({
                MessageContent("content1", Language::german),
                MessageContent("content2", Language::english),
                french,
            }));

            EXPECT_EQ(french, message.ChooseContent(Language::french));
        }

        TEST_P(MessageTest, emittingAsYamlShouldOutputNoteMessageTypeCorrectly) {
            Message message(Message::say, "content1");
            YAML::Emitter emitter;
            emitter << message;

            EXPECT_STREQ("type: say\n"
                         "content: 'content1'", emitter.c_str());
        }

        TEST_P(MessageTest, emittingAsYamlShouldOutputWarnMessageTypeCorrectly) {
            Message message(Message::warn, "content1");
            YAML::Emitter emitter;
            emitter << message;

            EXPECT_STREQ("type: warn\n"
                         "content: 'content1'", emitter.c_str());
        }

        TEST_P(MessageTest, emittingAsYamlShouldOutputErrorMessageTypeCorrectly) {
            Message message(Message::error, "content1");
            YAML::Emitter emitter;
            emitter << message;

            EXPECT_STREQ("type: error\n"
                         "content: 'content1'", emitter.c_str());
        }

        TEST_P(MessageTest, emittingAsYamlShouldOutputConditionIfItIsNotEmpty) {
            Message message(Message::say, "content1", "condition1");
            YAML::Emitter emitter;
            emitter << message;

            EXPECT_STREQ("type: say\n"
                         "content: 'content1'\n"
                         "condition: 'condition1'", emitter.c_str());
        }

        TEST_P(MessageTest, emittingAsYamlShouldOutputMultipleContentStringsAsAList) {
            Message message(Message::say, MessageContents({
                MessageContent("content1", Language::english),
                MessageContent("content2", Language::german)
            }));
            YAML::Emitter emitter;
            emitter << message;

            EXPECT_STREQ("type: say\n"
                         "content:\n"
                         "  - lang: en\n"
                         "    str: 'content1'\n"
                         "  - lang: de\n"
                         "    str: 'content2'", emitter.c_str());
        }

        TEST_P(MessageTest, encodingAsYamlShouldStoreNoteMessageTypeCorrectly) {
            Message message(Message::say, "content1");
            YAML::Node node;
            node = message;

            EXPECT_EQ("say", node["type"].as<std::string>());
        }

        TEST_P(MessageTest, encodingAsYamlShouldStoreWarningMessageTypeCorrectly) {
            Message message(Message::warn, "content1");
            YAML::Node node;
            node = message;

            EXPECT_EQ("warn", node["type"].as<std::string>());
        }

        TEST_P(MessageTest, encodingAsYamlShouldStoreErrorMessageTypeCorrectly) {
            Message message(Message::error, "content1");
            YAML::Node node;
            node = message;

            EXPECT_EQ("error", node["type"].as<std::string>());
        }

        TEST_P(MessageTest, encodingAsYamlShouldOmitConditionFieldIfItIsEmpty) {
            Message message(Message::say, "content1");
            YAML::Node node;
            node = message;

            EXPECT_FALSE(node["condition"]);
        }

        TEST_P(MessageTest, encodingAsYamlShouldStoreConditionFieldIfItIsNotEmpty) {
            Message message(Message::say, "content1", "condition1");
            YAML::Node node;
            node = message;

            EXPECT_EQ("condition1", node["condition"].as<std::string>());
        }

        TEST_P(MessageTest, encodingAsYamlShouldStoreASingleContentStringInAVector) {
            Message message(Message::say, "content1");
            YAML::Node node;
            node = message;

            EXPECT_EQ(message.Content(), node["content"].as<MessageContents>());
        }

        TEST_P(MessageTest, encodingAsYamlShouldMultipleContentStringsInAVector) {
            MessageContents contents({
                MessageContent("content1", Language::english),
                MessageContent("content2", Language::french),
            });
            Message message(Message::say, contents);
            YAML::Node node;
            node = message;

            EXPECT_EQ(contents, node["content"].as<MessageContents>());
        }

        TEST_P(MessageTest, decodingFromYamlShouldSetNoteTypeCorrectly) {
            YAML::Node node = YAML::Load("type: say\n"
                                         "content: content1");
            Message message = node.as<Message>();

            EXPECT_EQ(Message::say, message.Type());
        }

        TEST_P(MessageTest, decodingFromYamlShouldSetWarningTypeCorrectly) {
            YAML::Node node = YAML::Load("type: warn\n"
                                         "content: content1");
            Message message = node.as<Message>();

            EXPECT_EQ(Message::warn, message.Type());
        }

        TEST_P(MessageTest, decodingFromYamlShouldSetErrorTypeCorrectly) {
            YAML::Node node = YAML::Load("type: error\n"
                                         "content: content1");
            Message message = node.as<Message>();

            EXPECT_EQ(Message::error, message.Type());
        }

        TEST_P(MessageTest, decodingFromYamlShouldHandleAnUnrecognisedTypeAsANote) {
            YAML::Node node = YAML::Load("type: invalid\n"
                                         "content: content1");
            Message message = node.as<Message>();

            EXPECT_EQ(Message::say, message.Type());
        }

        TEST_P(MessageTest, decodingFromYamlShouldLeaveTheConditionEmptyIfNoneIsPresent) {
            YAML::Node node = YAML::Load("type: say\n"
                                         "content: content1");
            Message message = node.as<Message>();

            EXPECT_TRUE(message.Condition().empty());
        }

        TEST_P(MessageTest, decodingFromYamlShouldStoreANonEmptyConditionField) {
            YAML::Node node = YAML::Load("type: say\n"
                                         "content: content1\n"
                                         "condition: 'file(\"Foo.esp\")'");
            Message message = node.as<Message>();

            EXPECT_EQ("file(\"Foo.esp\")", message.Condition());
        }

        TEST_P(MessageTest, decodingFromYamlShouldStoreAScalarContentValueCorrectly) {
            YAML::Node node = YAML::Load("type: say\n"
                                         "content: content1\n");
            Message message = node.as<Message>();
            MessageContents expectedContent({MessageContent("content1", Language::english)});

            EXPECT_EQ(expectedContent, message.Content());
        }

        TEST_P(MessageTest, decodingFromYamlShouldStoreAListOfContentStringsCorrectly) {
            YAML::Node node = YAML::Load("type: say\n"
                                         "content:\n"
                                         "  - lang: en\n"
                                         "    str: content1\n"
                                         "  - lang: de\n"
                                         "    str: content2");
            Message message = node.as<Message>();

            EXPECT_EQ(MessageContents({
                MessageContent("content1", Language::english),
                MessageContent("content2", Language::german),
            }), message.Content());
        }

        TEST_P(MessageTest, decodingFromYamlShouldNotThrowIfTheOnlyContentStringIsNotEnglish) {
            YAML::Node node = YAML::Load("type: say\n"
                                         "content:\n"
                                         "  - lang: fr\n"
                                         "    str: content1");

            EXPECT_NO_THROW(Message message = node.as<Message>());
        }

        TEST_P(MessageTest, decodingFromYamlShouldThrowIfMultipleContentStringsAreGivenAndNoneAreEnglish) {
            YAML::Node node = YAML::Load("type: say\n"
                                         "content:\n"
                                         "  - lang: de\n"
                                         "    str: content1\n"
                                         "  - lang: fr\n"
                                         "    str: content2");

            EXPECT_THROW(node.as<Message>(), YAML::RepresentationException);
        }

        TEST_P(MessageTest, decodingFromYamlShouldApplySubstitutionsWhenThereIsOnlyOneContentString) {
            YAML::Node node = YAML::Load("type: say\n"
                                         "content: con%1%tent1\n"
                                         "subs:\n"
                                         "  - sub1");
            Message message = node.as<Message>();

            EXPECT_EQ(MessageContents({MessageContent("consub1tent1", Language::english)}), message.Content());
        }

        TEST_P(MessageTest, decodingFromYamlShouldApplySubstitutionsToAllContentStrings) {
            YAML::Node node = YAML::Load("type: say\n"
                                         "content:\n"
                                         "  - lang: en\n"
                                         "    str: content1 %1%\n"
                                         "  - lang: de\n"
                                         "    str: content2 %1%\n"
                                         "subs:\n"
                                         "  - sub");
            Message message = node.as<Message>();

            EXPECT_EQ(MessageContents({
                MessageContent("content1 sub", Language::english),
                MessageContent("content2 sub", Language::german),
            }), message.Content());
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

            EXPECT_EQ(MessageContents({MessageContent("con%1%tent1", Language::english)}), message.Content());
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
