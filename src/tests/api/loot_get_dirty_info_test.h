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

#ifndef LOOT_TEST_API_LOOT_GET_DIRTY_INFO_TEST
#define LOOT_TEST_API_LOOT_GET_DIRTY_INFO_TEST

#include "loot/api.h"

#include "tests/api/api_game_operations_test.h"

namespace loot {
namespace test {
class loot_get_dirty_info_test : public ApiGameOperationsTest {
protected:
  loot_get_dirty_info_test() :
    needsCleaning_(0) {}

  unsigned int needsCleaning_;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        loot_get_dirty_info_test,
                        ::testing::Values(
                          loot_game_tes4,
                          loot_game_tes5,
                          loot_game_fo3,
                          loot_game_fonv,
                          loot_game_fo4));

TEST_P(loot_get_dirty_info_test, shouldReturnAnInvalidArgsErrorIfAnyOfTheArgumentsAreNull) {
  EXPECT_EQ(loot_error_invalid_args, loot_get_dirty_info(NULL, blankEsp.c_str(), &needsCleaning_));
  EXPECT_EQ(loot_error_invalid_args, loot_get_dirty_info(db_, NULL, &needsCleaning_));
  EXPECT_EQ(loot_error_invalid_args, loot_get_dirty_info(db_, blankEsp.c_str(), NULL));
}

TEST_P(loot_get_dirty_info_test, shouldReturnOkAndOutputUnknownForAPluginWithNoDirtyInfo) {
  EXPECT_EQ(loot_ok, loot_get_dirty_info(db_, blankEsp.c_str(), &needsCleaning_));
  EXPECT_EQ(loot_needs_cleaning_unknown, needsCleaning_);
}

TEST_P(loot_get_dirty_info_test, shouldReturnOkAndOutputYesForAPluginWithDirtyInfo) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_EQ(loot_ok, loot_load_lists(db_, masterlistPath.string().c_str(), NULL));

  EXPECT_EQ(loot_ok, loot_get_dirty_info(db_, blankDifferentEsm.c_str(), &needsCleaning_));
  EXPECT_EQ(loot_needs_cleaning_yes, needsCleaning_);
}

TEST_P(loot_get_dirty_info_test, shouldReturnOkAndOutputNoForAPluginWithADoNotCleanMessage) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_EQ(loot_ok, loot_load_lists(db_, masterlistPath.string().c_str(), NULL));

  EXPECT_EQ(loot_ok, loot_get_dirty_info(db_, blankEsm.c_str(), &needsCleaning_));
  EXPECT_EQ(loot_needs_cleaning_no, needsCleaning_);
}
}
}

#endif
