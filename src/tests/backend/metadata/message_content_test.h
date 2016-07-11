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

#ifndef LOOT_TEST_BACKEND_METADATA_MESSAGE_CONTENT
#define LOOT_TEST_BACKEND_METADATA_MESSAGE_CONTENT

#include "backend/metadata/message_content.h"

#include <gtest/gtest.h>

namespace loot {
    namespace test {
        TEST(MessageContent, defaultConstructorShouldSetEmptyEnglishLanguageString) {
            MessageContent content;

            EXPECT_TRUE(content.GetText().empty());
            EXPECT_EQ(Language::Code::english, content.GetLanguage());
        }

        TEST(MessageContent, contentConstructorShouldStoreGivenStringAndLanguage) {
            MessageContent content("content", Language::Code::french);

            EXPECT_EQ("content", content.GetText());
            EXPECT_EQ(Language::Code::french, content.GetLanguage());
        }

        TEST(MessageContent, contentShouldBeEqualIfStringsAreCaseInsensitivelyEqual) {
            MessageContent content1("content", Language::Code::english);
            MessageContent content2("Content", Language::Code::french);

            EXPECT_TRUE(content1 == content2);
        }

        TEST(MessageContent, contentShouldBeUnequalIfStringsAreNotCaseInsensitivelyEqual) {
            MessageContent content1("content1", Language::Code::french);
            MessageContent content2("content2", Language::Code::french);

            EXPECT_FALSE(content1 == content2);
        }

        TEST(MessageContent, LessThanOperatorShouldUseCaseInsensitiveLexicographicalComparison) {
            MessageContent content1("content", Language::Code::english);
            MessageContent content2("Content", Language::Code::french);

            EXPECT_FALSE(content1 < content2);
            EXPECT_FALSE(content2 < content1);

            content1 = MessageContent("content1", Language::Code::french);
            content2 = MessageContent("content2", Language::Code::english);

            EXPECT_TRUE(content1 < content2);
            EXPECT_FALSE(content2 < content1);
        }

        TEST(MessageContent, emittingAsYamlShouldOutputDataCorrectly) {
            MessageContent content("content", Language::Code::french);
            YAML::Emitter emitter;
            emitter << content;

            EXPECT_EQ("lang: " + Language(content.GetLanguage()).GetLocale() +
                      "\nstr: '" + content.GetText() + "'", emitter.c_str());
        }

        TEST(MessageContent, encodingAsYamlShouldOutputDataCorrectly) {
            MessageContent content("content", Language::Code::french);
            YAML::Node node;
            node = content;

            EXPECT_EQ(content.GetText(), node["str"].as<std::string>());
            EXPECT_EQ(Language(Language::Code::french).GetLocale(), node["lang"].as<std::string>());
        }

        TEST(MessageContent, decodingFromYamlShouldSetDataCorrectly) {
            YAML::Node node = YAML::Load("{str: content, lang: de}");
            MessageContent content = node.as<MessageContent>();

            EXPECT_EQ("content", content.GetText());
            EXPECT_EQ(Language::Code::german, content.GetLanguage());
        }

        TEST(MessageContent, decodingFromYamlScalarShouldThrow) {
            YAML::Node node = YAML::Load("scalar");

            EXPECT_ANY_THROW(node.as<MessageContent>());
        }

        TEST(MessageContent, decodingFromYamlListShouldThrow) {
            YAML::Node node = YAML::Load("[0, 1, 2]");

            EXPECT_ANY_THROW(node.as<MessageContent>());
        }
    }
}

#endif
