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

#ifndef LOOT_TESTS_API_LOOT_EVAL_LISTS_TEST
#define LOOT_TESTS_API_LOOT_EVAL_LISTS_TEST

#include "loot/api.h"

#include "tests/api/api_game_operations_test.h"

namespace loot {
namespace test {
class loot_eval_lists_test : public ApiGameOperationsTest {};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        loot_eval_lists_test,
                        ::testing::Values(
                          loot_game_tes4,
                          loot_game_tes5,
                          loot_game_fo3,
                          loot_game_fonv,
                          loot_game_fo4));

TEST_P(loot_eval_lists_test, shouldReturnAnInvalidArgsErrorIfPassedANullPointer) {
  EXPECT_EQ(loot_error_invalid_args, loot_eval_lists(NULL, loot_lang_english));
}

TEST_P(loot_eval_lists_test, shouldReturnAnInvalidArgsErrorIfPassedAnInvalidLanguageCode) {
  EXPECT_EQ(loot_error_invalid_args, loot_eval_lists(db_, UINT_MAX));
}

TEST_P(loot_eval_lists_test, shouldReturnOkForAllLanguagesWithNoListsLoaded) {
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_english));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_english));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_spanish));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_russian));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_french));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_chinese));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_polish));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_brazilian_portuguese));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_finnish));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_german));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_danish));
}

TEST_P(loot_eval_lists_test, shouldReturnOKForAllLanguagesWithAMasterlistLoaded) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_EQ(loot_ok, loot_load_lists(db_, masterlistPath.string().c_str(), NULL));

  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_english));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_english));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_spanish));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_russian));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_french));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_chinese));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_polish));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_brazilian_portuguese));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_finnish));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_german));
  EXPECT_EQ(loot_ok, loot_eval_lists(db_, loot_lang_danish));
}
}
}

#endif
