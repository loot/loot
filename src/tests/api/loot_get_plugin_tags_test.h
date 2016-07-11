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

#ifndef LOOT_TESTS_API_LOOT_GET_PLUGIN_TAGS_TEST
#define LOOT_TESTS_API_LOOT_GET_PLUGIN_TAGS_TEST

#include "loot/api.h"

#include "tests/api/api_game_operations_test.h"

namespace loot {
namespace test {
class loot_get_plugin_tags_test : public ApiGameOperationsTest {
protected:
  loot_get_plugin_tags_test() :
    tagMap_(nullptr),
    numTags_(0),
    added_(nullptr),
    removed_(nullptr),
    numAdded_(0),
    numRemoved_(0),
    modified_(false) {}

  const char * const * tagMap_;
  size_t numTags_;

  const unsigned int * added_;
  const unsigned int * removed_;
  size_t numAdded_;
  size_t numRemoved_;
  bool modified_;

  void getTagMap() {
    ASSERT_EQ(loot_ok, loot_get_tag_map(db_, &tagMap_, &numTags_));

    ASSERT_EQ(3, numTags_);
    ASSERT_STREQ("Actors.ACBS", tagMap_[0]);
    ASSERT_STREQ("Actors.AIData", tagMap_[1]);
    ASSERT_STREQ("C.Water", tagMap_[2]);
  }
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        loot_get_plugin_tags_test,
                        ::testing::Values(
                          loot_game_tes4,
                          loot_game_tes5,
                          loot_game_fo3,
                          loot_game_fonv,
                          loot_game_fo4));

TEST_P(loot_get_plugin_tags_test, shouldReturnAnInvalidArgsErrorIfAnyOfTheArgumentsAreNull) {
  EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(NULL, blankEsm.c_str(), &added_, &numAdded_, &removed_, &numRemoved_, &modified_));
  EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(db_, NULL, &added_, &numAdded_, &removed_, &numRemoved_, &modified_));
  EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(db_, blankEsm.c_str(), NULL, &numAdded_, &removed_, &numRemoved_, &modified_));
  EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(db_, blankEsm.c_str(), &added_, NULL, &removed_, &numRemoved_, &modified_));
  EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(db_, blankEsm.c_str(), &added_, &numAdded_, NULL, &numRemoved_, &modified_));
  EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(db_, blankEsm.c_str(), &added_, &numAdded_, &removed_, NULL, &modified_));
  EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(db_, blankEsm.c_str(), &added_, &numAdded_, &removed_, &numRemoved_, NULL));
}

TEST_P(loot_get_plugin_tags_test, shouldReturnANoTagMapIfCalledBeforeATagMapHasBeenGotten) {
  EXPECT_EQ(loot_error_no_tag_map, loot_get_plugin_tags(db_, blankEsm.c_str(), &added_, &numAdded_, &removed_, &numRemoved_, &modified_));
}

TEST_P(loot_get_plugin_tags_test, shouldReturnOkAndOutputEmptyNonModifiedArraysIfAPluginWithoutTagsIsQueried) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_EQ(loot_ok, loot_load_lists(db_, masterlistPath.string().c_str(), NULL));
  getTagMap();

  EXPECT_EQ(loot_ok, loot_get_plugin_tags(db_, blankEsp.c_str(), &added_, &numAdded_, &removed_, &numRemoved_, &modified_));

  EXPECT_EQ(0, numAdded_);
  EXPECT_EQ(NULL, added_);
  EXPECT_EQ(0, numRemoved_);
  EXPECT_EQ(NULL, removed_);
  EXPECT_FALSE(modified_);
}

TEST_P(loot_get_plugin_tags_test, shouldReturnOkAndNonEmptyNonModifiedArraysIfAPluginWithTagsIsQueried) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_EQ(loot_ok, loot_load_lists(db_, masterlistPath.string().c_str(), NULL));
  getTagMap();

  EXPECT_EQ(loot_ok, loot_get_plugin_tags(db_, blankEsm.c_str(), &added_, &numAdded_, &removed_, &numRemoved_, &modified_));

  // The values are tag map indices, check they match up as expected.
  ASSERT_EQ(2, numAdded_);
  EXPECT_EQ(0, added_[0]);
  EXPECT_EQ(1, added_[1]);

  ASSERT_EQ(1, numRemoved_);
  EXPECT_EQ(2, removed_[0]);

  EXPECT_FALSE(modified_);
}

TEST_P(loot_get_plugin_tags_test, shouldReturnOkAndNonEmptyModifiedArraysIfAPluginWithTagsIsQueriedAndMetadataWasAlsoLoadedFromAUserlist) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_EQ(loot_ok, loot_load_lists(db_, masterlistPath.string().c_str(), masterlistPath.string().c_str()));
  getTagMap();

  EXPECT_EQ(loot_ok, loot_get_plugin_tags(db_, blankEsm.c_str(), &added_, &numAdded_, &removed_, &numRemoved_, &modified_));

  // The values are tag map indices, check they match up as expected.
  ASSERT_EQ(2, numAdded_);
  EXPECT_EQ(0, added_[0]);
  EXPECT_EQ(1, added_[1]);

  ASSERT_EQ(1, numRemoved_);
  EXPECT_EQ(2, removed_[0]);

  EXPECT_TRUE(modified_);
}

TEST_P(loot_get_plugin_tags_test, shouldOutputTheCorrectBashTagsForPluginsWhenMakingConsecutiveCalls) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_EQ(loot_ok, loot_load_lists(db_, masterlistPath.string().c_str(), NULL));
  getTagMap();

  EXPECT_EQ(loot_ok, loot_get_plugin_tags(db_, blankEsm.c_str(), &added_, &numAdded_, &removed_, &numRemoved_, &modified_));

  ASSERT_EQ(2, numAdded_);
  EXPECT_EQ(0, added_[0]);
  EXPECT_EQ(1, added_[1]);

  ASSERT_EQ(1, numRemoved_);
  EXPECT_EQ(2, removed_[0]);
  EXPECT_FALSE(modified_);

  EXPECT_EQ(loot_ok, loot_get_plugin_tags(db_, blankEsp.c_str(), &added_, &numAdded_, &removed_, &numRemoved_, &modified_));

  EXPECT_EQ(0, numAdded_);
  EXPECT_EQ(NULL, added_);
  EXPECT_EQ(0, numRemoved_);
  EXPECT_EQ(NULL, removed_);
  EXPECT_FALSE(modified_);
}
}
}

#endif
