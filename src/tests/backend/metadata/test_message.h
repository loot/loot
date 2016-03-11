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

#include "backend/metadata/message.h"
#include "tests/fixtures.h"

namespace loot {
    namespace test {
        class MessageTest : public SkyrimTest {};

        typedef std::vector<MessageContent> MessageContents;

        TEST_F(MessageTest, ConstructorsAndDataAccess) {
            Message message;
            EXPECT_EQ(Message::say, message.Type());
            EXPECT_EQ(MessageContents(), message.Content());

            message = Message(Message::say, "content1", "condition1");
            EXPECT_EQ(Message::say, message.Type());
            EXPECT_EQ(MessageContents({
                MessageContent("content1", Language::english)
            }), message.Content());
            EXPECT_EQ("condition1", message.Condition());

            MessageContents mc({MessageContent("content1", Language::english)});
            message = Message(Message::say, mc, "condition1");
            EXPECT_EQ(Message::say, message.Type());
            EXPECT_EQ(mc, message.Content());
            EXPECT_EQ("condition1", message.Condition());
        }

        TEST_F(MessageTest, EqualityOperator) {
            Message message1, message2;
            EXPECT_TRUE(message1 == message2);

            message1 = Message(Message::say, "content1", "condition1");
            message2 = Message(Message::say, "content1", "condition2");
            EXPECT_TRUE(message1 == message2);

            message1 = Message(Message::say, "content1", "condition1");
            message2 = Message(Message::say, "content2", "condition1");
            EXPECT_FALSE(message1 == message2);

            message1 = Message(Message::say, "content1", "condition1");
            message2 = Message(Message::warn, "content1", "condition1");
            EXPECT_TRUE(message1 == message2);

            message1 = Message(Message::say, "content1", "condition1");
            message2 = Message(Message::say, MessageContents({MessageContent("content1", Language::english)}), "condition1");
            EXPECT_TRUE(message1 == message2);

            message1 = Message(Message::say, "content1", "condition1");
            message2 = Message(Message::say, MessageContents({MessageContent("content2", Language::english)}), "condition1");
            EXPECT_FALSE(message1 == message2);

            message1 = Message(Message::say, MessageContents({MessageContent("content1", Language::english)}), "condition1");
            message2 = Message(Message::say, MessageContents({MessageContent("content1", Language::english)}), "condition1");
            EXPECT_TRUE(message1 == message2);

            message1 = Message(Message::say, MessageContents({MessageContent("content1", Language::english)}), "condition1");
            message2 = Message(Message::say, MessageContents({MessageContent("content2", Language::english)}), "condition1");
            EXPECT_FALSE(message1 == message2);

            message1 = Message(Message::say, MessageContents({MessageContent("content1", Language::english)}), "condition1");
            message2 = Message(Message::say, MessageContents({MessageContent("content1", Language::french)}), "condition1");
            EXPECT_TRUE(message1 == message2);
        }

        TEST_F(MessageTest, LessThanOperator) {
            Message message1, message2;
            EXPECT_FALSE(message1 < message2);
            EXPECT_FALSE(message2 < message1);

            message1 = Message(Message::say, "content1", "condition1");
            message2 = Message(Message::say, "content1", "condition1");
            EXPECT_FALSE(message1 < message2);
            EXPECT_FALSE(message2 < message1);

            message1 = Message(Message::say, "content1", "condition1");
            message2 = Message(Message::say, "content1", "condition2");
            EXPECT_FALSE(message1 < message2);
            EXPECT_FALSE(message2 < message1);

            message1 = Message(Message::say, "content1", "condition1");
            message2 = Message(Message::warn, "content1", "condition1");
            EXPECT_FALSE(message1 < message2);
            EXPECT_FALSE(message2 < message1);

            message1 = Message(Message::say, "", "condition1");
            message2 = Message(Message::say, "content1", "condition1");
            EXPECT_TRUE(message1 < message2);
            EXPECT_FALSE(message2 < message1);

            message1 = Message(Message::say, "content1", "condition1");
            message2 = Message(Message::say, "", "condition1");
            EXPECT_FALSE(message1 < message2);
            EXPECT_TRUE(message2 < message1);

            message1 = Message(Message::say, "content1", "condition1");
            message2 = Message(Message::say, MessageContents({MessageContent("content1", Language::english)}), "condition1");
            EXPECT_FALSE(message1 < message2);
            EXPECT_FALSE(message2 < message1);

            message1 = Message(Message::say, "content1", "condition1");
            message2 = Message(Message::say, MessageContents({MessageContent("content2", Language::english)}), "condition1");
            EXPECT_TRUE(message1 < message2);
            EXPECT_FALSE(message2 < message1);

            message1 = Message(Message::say, MessageContents({MessageContent("content1", Language::english)}), "condition1");
            message2 = Message(Message::say, MessageContents({MessageContent("content1", Language::english)}), "condition1");
            EXPECT_FALSE(message1 < message2);
            EXPECT_FALSE(message2 < message1);

            message1 = Message(Message::say, MessageContents({MessageContent("content1", Language::english)}), "condition1");
            message2 = Message(Message::say, MessageContents({MessageContent("content2", Language::english)}), "condition1");
            EXPECT_TRUE(message1 < message2);
            EXPECT_FALSE(message2 < message1);

            message1 = Message(Message::say, MessageContents({MessageContent("content1", Language::english)}), "condition1");
            message2 = Message(Message::say, MessageContents({MessageContent("content1", Language::french)}), "condition1");
            EXPECT_FALSE(message1 < message2);
            EXPECT_FALSE(message2 < message1);
        }

        TEST_F(MessageTest, EvalCondition) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            Message message;
            EXPECT_TRUE(message.EvalCondition(game, Language::any));
            EXPECT_EQ(1, message.Content().size());

            message = Message(Message::say, MessageContents({
                MessageContent("content1", Language::german),
                MessageContent("content1", Language::english),
                MessageContent("content1", Language::french),
            }));
            EXPECT_TRUE(message.EvalCondition(game, Language::french));
            EXPECT_EQ(1, message.Content().size());
        }

        TEST_F(MessageTest, ChooseContent) {
            MessageContent mc;
            Message message;
            EXPECT_EQ(mc, message.ChooseContent(Language::any));
            EXPECT_EQ(mc, message.ChooseContent(Language::french));
            EXPECT_EQ(mc, message.ChooseContent(Language::english));

            mc = MessageContent("content1", Language::english);
            message = Message(Message::say, "content1");
            EXPECT_EQ(mc, message.ChooseContent(Language::any));
            EXPECT_EQ(mc, message.ChooseContent(Language::french));
            EXPECT_EQ(mc, message.ChooseContent(Language::english));

            mc = MessageContent("content1", Language::english);
            message = Message(Message::say, MessageContents({mc}));
            EXPECT_EQ(mc, message.ChooseContent(Language::any));
            EXPECT_EQ(mc, message.ChooseContent(Language::french));
            EXPECT_EQ(mc, message.ChooseContent(Language::english));

            mc = MessageContent("content1", Language::english);
            message = Message(Message::say, MessageContents({
                MessageContent("content1", Language::english),
                MessageContent("content1", Language::german),
            }));
            EXPECT_EQ(mc, message.ChooseContent(Language::any));
            EXPECT_EQ(mc, message.ChooseContent(Language::french));
            EXPECT_EQ(mc, message.ChooseContent(Language::english));

            message = Message(Message::say, MessageContents({
                MessageContent("content1", Language::german),
                MessageContent("content1", Language::english),
                MessageContent("content1", Language::french),
            }));
            EXPECT_EQ(MessageContent("content1", Language::german), message.ChooseContent(Language::any));
            EXPECT_EQ(MessageContent("content1", Language::french), message.ChooseContent(Language::french));
            EXPECT_EQ(MessageContent("content1", Language::english), message.ChooseContent(Language::english));

            mc = MessageContent("content1", Language::german);
            message = Message(Message::say, MessageContents({mc}));
            EXPECT_EQ(mc, message.ChooseContent(Language::any));
            EXPECT_EQ(mc, message.ChooseContent(Language::french));
        }

        TEST_F(MessageTest, YamlEmitter) {
            Message message(Message::say, "content1");
            YAML::Emitter e1;
            e1 << message;
            EXPECT_STREQ("type: say\n"
                         "content: 'content1'", e1.c_str());

            message = Message(Message::warn, "content1");
            YAML::Emitter e2;
            e2 << message;
            EXPECT_STREQ("type: warn\n"
                         "content: 'content1'", e2.c_str());

            message = Message(Message::error, "content1");
            YAML::Emitter e3;
            e3 << message;
            EXPECT_STREQ("type: error\n"
                         "content: 'content1'", e3.c_str());

            message = Message(Message::say, "content1", "condition1");
            YAML::Emitter e4;
            e4 << message;
            EXPECT_STREQ("type: say\n"
                         "content: 'content1'\n"
                         "condition: 'condition1'", e4.c_str());

            message = Message(Message::say, MessageContents({
                MessageContent("content1", Language::english),
            }));
            YAML::Emitter e5;
            e5 << message;
            EXPECT_STREQ("type: say\n"
                         "content: 'content1'", e5.c_str());

            message = Message(Message::say, MessageContents({
                MessageContent("content1", Language::english),
                MessageContent("content2", Language::german)
            }));
            YAML::Emitter e6;
            e6 << message;
            EXPECT_STREQ("type: say\n"
                         "content:\n"
                         "  - lang: en\n"
                         "    str: 'content1'\n"
                         "  - lang: de\n"
                         "    str: 'content2'", e6.c_str());
        }

        TEST_F(MessageTest, YamlEncode) {
            YAML::Node node;

            MessageContents mcs({MessageContent("content1", Language::english)});
            Message message(Message::say, "content1");
            node = message;
            EXPECT_EQ("say", node["type"].as<std::string>());
            EXPECT_EQ(mcs, node["content"].as<MessageContents>());
            EXPECT_FALSE(node["condition"]);

            message = Message(Message::warn, "content1");
            node = message;
            EXPECT_EQ("warn", node["type"].as<std::string>());
            EXPECT_EQ(mcs, node["content"].as<MessageContents>());
            EXPECT_FALSE(node["condition"]);

            message = Message(Message::error, "content1");
            node = message;
            EXPECT_EQ("error", node["type"].as<std::string>());
            EXPECT_EQ(mcs, node["content"].as<MessageContents>());
            EXPECT_FALSE(node["condition"]);

            message = Message(Message::say, "content1", "condition1");
            node = message;
            EXPECT_EQ("say", node["type"].as<std::string>());
            EXPECT_EQ(mcs, node["content"].as<MessageContents>());
            EXPECT_EQ("condition1", node["condition"].as<std::string>());

            mcs = MessageContents({
                MessageContent("content1", Language::english),
                MessageContent("content2", Language::french),
            });
            message = Message(Message::say, mcs);
            node = message;
            EXPECT_EQ("say", node["type"].as<std::string>());
            EXPECT_EQ(mcs, node["content"].as<MessageContents>());
            EXPECT_FALSE(node["condition"]);
        }

        TEST_F(MessageTest, YamlDecode) {
            YAML::Node node = YAML::Load("type: say\n"
                                         "content: content1");
            Message message = node.as<Message>();
            EXPECT_EQ(Message::say, message.Type());
            EXPECT_EQ(MessageContents({MessageContent("content1", Language::english)}), message.Content());
            EXPECT_EQ("", message.Condition());

            node = YAML::Load("type: warn\n"
                              "content: content1");
            message = node.as<Message>();
            EXPECT_EQ(Message::warn, message.Type());
            EXPECT_EQ(MessageContents({MessageContent("content1", Language::english)}), message.Content());
            EXPECT_EQ("", message.Condition());

            node = YAML::Load("type: error\n"
                              "content: content1");
            message = node.as<Message>();
            EXPECT_EQ(Message::error, message.Type());
            EXPECT_EQ(MessageContents({MessageContent("content1", Language::english)}), message.Content());
            EXPECT_EQ("", message.Condition());

            node = YAML::Load("type: invalid\n"
                              "content: content1");
            message = node.as<Message>();
            EXPECT_EQ(Message::say, message.Type());
            EXPECT_EQ(MessageContents({MessageContent("content1", Language::english)}), message.Content());
            EXPECT_EQ("", message.Condition());

            node = YAML::Load("type: say\n"
                              "content: content1\n"
                              "condition: 'file(\"Foo.esp\")'");
            message = node.as<Message>();
            EXPECT_EQ(Message::say, message.Type());
            EXPECT_EQ(MessageContents({MessageContent("content1", Language::english)}), message.Content());
            EXPECT_EQ("file(\"Foo.esp\")", message.Condition());

            node = YAML::Load("type: say\n"
                              "content:\n"
                              "  - lang: en\n"
                              "    str: content1\n"
                              "  - lang: de\n"
                              "    str: content2");
            message = node.as<Message>();
            EXPECT_EQ(Message::say, message.Type());
            EXPECT_EQ(MessageContents({
                MessageContent("content1", Language::english),
                MessageContent("content2", Language::german),
            }), message.Content());
            EXPECT_EQ("", message.Condition());

            node = YAML::Load("type: say\n"
                              "content:\n"
                              "  - lang: fr\n"
                              "    str: content1");
            message = node.as<Message>();
            EXPECT_EQ(Message::say, message.Type());
            EXPECT_EQ(MessageContents({MessageContent("content1", Language::french)}), message.Content());
            EXPECT_EQ("", message.Condition());

            // Throw because no English string is given in a multilingual message.
            node = YAML::Load("type: say\n"
                              "content:\n"
                              "  - lang: de\n"
                              "    str: content1\n"
                              "  - lang: fr\n"
                              "    str: content2");
            EXPECT_THROW(node.as<Message>(), YAML::RepresentationException);

            node = YAML::Load("type: say\n"
                              "content: con%1%tent1\n"
                              "subs:\n"
                              "  - sub1");
            message = node.as<Message>();
            EXPECT_EQ(Message::say, message.Type());
            EXPECT_EQ(MessageContents({MessageContent("consub1tent1", Language::english)}), message.Content());
            EXPECT_EQ("", message.Condition());

            // Check that subs apply to all languages of a multilingual message.
            node = YAML::Load("type: say\n"
                              "content:\n"
                              "  - lang: en\n"
                              "    str: content1 %1%\n"
                              "  - lang: de\n"
                              "    str: content2 %1%\n"
                              "subs:\n"
                              "  - sub");
            message = node.as<Message>();
            EXPECT_EQ(Message::say, message.Type());
            EXPECT_EQ(MessageContents({
                MessageContent("content1 sub", Language::english),
                MessageContent("content2 sub", Language::german),
            }), message.Content());
            EXPECT_EQ("", message.Condition());

            // Throw because there aren't the same number of subs as referred to in the content string.
            node = YAML::Load("type: say\n"
                              "content: '%1% %2%'\n"
                              "subs:\n"
                              "  - sub1");
            EXPECT_THROW(node.as<Message>(), YAML::RepresentationException);

            // Don't throw because no subs are given, so none are expected in the content string.
            node = YAML::Load("type: say\n"
                              "content: con%1%tent1\n");
            message = node.as<Message>();
            EXPECT_EQ(Message::say, message.Type());
            EXPECT_EQ(MessageContents({MessageContent("con%1%tent1", Language::english)}), message.Content());
            EXPECT_EQ("", message.Condition());

            node = YAML::Load("type: say\n"
                              "content: content1\n"
                              "condition: invalid");
            EXPECT_THROW(node.as<Message>(), YAML::RepresentationException);

            node = YAML::Load("scalar");
            EXPECT_THROW(node.as<Message>(), YAML::RepresentationException);

            node = YAML::Load("[0, 1, 2]");
            EXPECT_THROW(node.as<Message>(), YAML::RepresentationException);
        }
    }
}

#endif
