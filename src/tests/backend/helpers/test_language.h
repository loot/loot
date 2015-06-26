/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2015    WrinklyNinja

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
#include "tests/fixtures.h"

using loot::Language;

TEST(Language, ConstructorsAndDataAccess) {
    // Test English code and locale.
    Language lang(Language::english);
    EXPECT_EQ(Language::english, lang.Code());
    EXPECT_EQ("English", lang.Name());
    EXPECT_EQ("en", lang.Locale());

    lang = Language("en");
    EXPECT_EQ(Language::english, lang.Code());
    EXPECT_EQ("English", lang.Name());
    EXPECT_EQ("en", lang.Locale());

    // Test code and locale for a couple of other languages, in case English
    // is wrongly being used for everything. No point testing all languages,
    // because that would just be copying the class code.
    lang = Language(Language::polish);
    EXPECT_EQ(Language::polish, lang.Code());
    EXPECT_EQ("Polski", lang.Name());
    EXPECT_EQ("pl", lang.Locale());

    lang = Language("de");
    EXPECT_EQ(Language::german, lang.Code());
    EXPECT_EQ("Deutsch", lang.Name());
    EXPECT_EQ("de", lang.Locale());

    // Test that invalid values get treated as English.
    lang = Language(1000);
    EXPECT_EQ(Language::english, lang.Code());
    EXPECT_EQ("English", lang.Name());
    EXPECT_EQ("en", lang.Locale());

    lang = Language("foo");
    EXPECT_EQ(Language::english, lang.Code());
    EXPECT_EQ("English", lang.Name());
    EXPECT_EQ("en", lang.Locale());
}

TEST(Language, Codes) {
    // Check that all the expected codes are given.
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
    EXPECT_EQ(Language::Codes, codes);
}

#endif
