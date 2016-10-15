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

#ifndef LOOT_TESTS_BACKEND_HELPERS_LANGUAGE_TEST
#define LOOT_TESTS_BACKEND_HELPERS_LANGUAGE_TEST

#include "backend/helpers/language.h"

#include <gtest/gtest.h>

namespace loot {
namespace test {
TEST(Language, codeConstructorShouldSetTheCorrectData) {
  Language lang(LanguageCode::english);
  EXPECT_EQ(LanguageCode::english, lang.GetCode());
  EXPECT_EQ("English", lang.GetName());
  EXPECT_EQ("en", lang.GetLocale());

  lang = Language(LanguageCode::polish);
  EXPECT_EQ(LanguageCode::polish, lang.GetCode());
  EXPECT_EQ("Polski", lang.GetName());
  EXPECT_EQ("pl", lang.GetLocale());
}

TEST(Language, localeConstructorShouldSetTheCorrectData) {
  Language lang("en");
  EXPECT_EQ(LanguageCode::english, lang.GetCode());
  EXPECT_EQ("English", lang.GetName());
  EXPECT_EQ("en", lang.GetLocale());

  lang = Language("de");
  EXPECT_EQ(LanguageCode::german, lang.GetCode());
  EXPECT_EQ("Deutsch", lang.GetName());
  EXPECT_EQ("de", lang.GetLocale());
}

TEST(Language, codeConstructorShouldTreatAnInvalidCodeAsEnglish) {
  Language lang(LanguageCode(1000));
  EXPECT_EQ(LanguageCode::english, lang.GetCode());
  EXPECT_EQ("English", lang.GetName());
  EXPECT_EQ("en", lang.GetLocale());
}

TEST(Language, localeConstructorShouldTreatAnInvalidLocaleAsEnglish) {
  Language lang("foo");
  EXPECT_EQ(LanguageCode::english, lang.GetCode());
  EXPECT_EQ("English", lang.GetName());
  EXPECT_EQ("en", lang.GetLocale());
}

TEST(Language, codesShouldContainAllExpectedLanguageCodes) {
  std::vector<LanguageCode> codes = {
      LanguageCode::english,
      LanguageCode::spanish,
      LanguageCode::russian,
      LanguageCode::french,
      LanguageCode::chinese,
      LanguageCode::polish,
      LanguageCode::brazilian_portuguese,
      LanguageCode::finnish,
      LanguageCode::german,
      LanguageCode::danish,
      LanguageCode::korean,
      LanguageCode::swedish,
  };

  EXPECT_EQ(codes, Language::codes);
}
}
}

#endif
