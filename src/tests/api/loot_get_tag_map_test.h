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

#ifndef LOOT_TEST_LOOT_GET_TAG_MAP
#define LOOT_TEST_LOOT_GET_TAG_MAP

#include "../include/loot/api.h"
#include "api_game_operations_test.h"

namespace loot {
    namespace test {
        class loot_get_tag_map_test : public ApiGameOperationsTest {
        protected:
            loot_get_tag_map_test() :
                tagMap(nullptr),
                numTags(0) {}

            const char * const * tagMap;
            size_t numTags;
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
            EXPECT_EQ(loot_error_invalid_args, loot_get_tag_map(NULL, &tagMap, &numTags));
            EXPECT_EQ(loot_error_invalid_args, loot_get_tag_map(db, NULL, &numTags));
            EXPECT_EQ(loot_error_invalid_args, loot_get_tag_map(db, &tagMap, NULL));
        }

        TEST_P(loot_get_tag_map_test, shouldReturnOkAndOutputAnEmptyTagMapIfNoMetadataHasBeenLoaded) {
            EXPECT_EQ(loot_ok, loot_get_tag_map(db, &tagMap, &numTags));
            EXPECT_EQ(0, numTags);
            EXPECT_EQ(NULL, tagMap);
        }

        TEST_P(loot_get_tag_map_test, shouldReturnOKAndOutputANonEmptyTagMapIfMetadataHasBeenLoaded) {
            ASSERT_NO_THROW(generateMasterlist());
            ASSERT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), NULL));

            EXPECT_EQ(loot_ok, loot_get_tag_map(db, &tagMap, &numTags));

            ASSERT_EQ(3, numTags);
            EXPECT_STREQ("Actors.ACBS", tagMap[0]);
            EXPECT_STREQ("Actors.AIData", tagMap[1]);
            EXPECT_STREQ("C.Water", tagMap[2]);
        }
    }
}

#endif
