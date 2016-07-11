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

#ifndef LOOT_TESTS_API_LOOT_GET_TAG_MAP_TEST
#define LOOT_TESTS_API_LOOT_GET_TAG_MAP_TEST

#include "loot/api.h"

#include "tests/api/api_game_operations_test.h"

namespace loot {
namespace test {
class loot_get_tag_map_test : public ApiGameOperationsTest {
protected:
  loot_get_tag_map_test() :
    tagMap_(nullptr),
    numTags_(0) {}

  const char * const * tagMap_;
  size_t numTags_;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        loot_get_tag_map_test,
                        ::testing::Values(
                          loot_game_tes4,
                          loot_game_tes5,
                          loot_game_fo3,
                          loot_game_fonv,
                          loot_game_fo4));

TEST_P(loot_get_tag_map_test, shouldReturnAnInvalidArgsErrorIfAnyOfTheArgumentsAreNull) {
  EXPECT_EQ(loot_error_invalid_args, loot_get_tag_map(NULL, &tagMap_, &numTags_));
  EXPECT_EQ(loot_error_invalid_args, loot_get_tag_map(db_, NULL, &numTags_));
  EXPECT_EQ(loot_error_invalid_args, loot_get_tag_map(db_, &tagMap_, NULL));
}

TEST_P(loot_get_tag_map_test, shouldReturnOkAndOutputAnEmptyTagMapIfNoMetadataHasBeenLoaded) {
  EXPECT_EQ(loot_ok, loot_get_tag_map(db_, &tagMap_, &numTags_));
  EXPECT_EQ(0, numTags_);
  EXPECT_EQ(NULL, tagMap_);
}

TEST_P(loot_get_tag_map_test, shouldReturnOKAndOutputANonEmptyTagMapIfMetadataHasBeenLoaded) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_EQ(loot_ok, loot_load_lists(db_, masterlistPath.string().c_str(), NULL));

  EXPECT_EQ(loot_ok, loot_get_tag_map(db_, &tagMap_, &numTags_));

  ASSERT_EQ(3, numTags_);
  EXPECT_STREQ("Actors.ACBS", tagMap_[0]);
  EXPECT_STREQ("Actors.AIData", tagMap_[1]);
  EXPECT_STREQ("C.Water", tagMap_[2]);
}
}
}

#endif
