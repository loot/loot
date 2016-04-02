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

#ifndef LOOT_TEST_BACKEND_HELPERS_LANGUAGE
#define LOOT_TEST_BACKEND_HELPERS_LANGUAGE

#include "backend/helpers/language.h"

#include <gtest/gtest.h>

namespace loot {
    namespace test {
        TEST(Language, codeConstructorShouldSetTheCorrectData) {
            Language lang(Language::english);
            EXPECT_EQ(Language::english, lang.Code());
            EXPECT_EQ("English", lang.Name());
            EXPECT_EQ("en", lang.Locale());

            lang = Language(Language::polish);
            EXPECT_EQ(Language::polish, lang.Code());
            EXPECT_EQ("Polski", lang.Name());
            EXPECT_EQ("pl", lang.Locale());
        }

        TEST(Language, localeConstructorShouldSetTheCorrectData) {
            Language lang("en");
            EXPECT_EQ(Language::english, lang.Code());
            EXPECT_EQ("English", lang.Name());
            EXPECT_EQ("en", lang.Locale());

            lang = Language("de");
            EXPECT_EQ(Language::german, lang.Code());
            EXPECT_EQ("Deutsch", lang.Name());
            EXPECT_EQ("de", lang.Locale());
        }

        TEST(Language, codeConstructorShouldTreatAnInvalidCodeAsEnglish) {
            Language lang(1000);
            EXPECT_EQ(Language::english, lang.Code());
            EXPECT_EQ("English", lang.Name());
            EXPECT_EQ("en", lang.Locale());
        }

        TEST(Language, localeConstructorShouldTreatAnInvalidLocaleAsEnglish) {
            Language lang("foo");
            EXPECT_EQ(Language::english, lang.Code());
            EXPECT_EQ("English", lang.Name());
            EXPECT_EQ("en", lang.Locale());
        }

        TEST(Language, codesShouldContainAllExpectedLanguageCodes) {
            std::vector<unsigned int> codes = {
                Language::english,
                Language::spanish,
                Language::russian,
                Language::french,
                Language::chinese,
                Language::polish,
                Language::brazilian_portuguese,
                Language::finnish,
                Language::german,
                Language::danish,
                Language::korean
            };

            EXPECT_EQ(codes, Language::Codes);
        }
    }
}

#endif
