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

#ifndef LOOT_TEST_LOOT_GET_PLUGIN_TAGS
#define LOOT_TEST_LOOT_GET_PLUGIN_TAGS

#include "../include/loot/api.h"
#include "api_game_operations_test.h"

namespace loot {
    namespace test {
        class loot_get_plugin_tags_test : public ApiGameOperationsTest {
        protected:
            loot_get_plugin_tags_test() :
                tagMap(nullptr),
                numTags(0),
                added(nullptr),
                removed(nullptr),
                numAdded(0),
                numRemoved(0),
                modified(false) {}

            const char * const * tagMap;
            size_t numTags;

            const unsigned int * added;
            const unsigned int * removed;
            size_t numAdded;
            size_t numRemoved;
            bool modified;

            void getTagMap() {
                ASSERT_EQ(loot_ok, loot_get_tag_map(db, &tagMap, &numTags));

                ASSERT_EQ(3, numTags);
                ASSERT_STREQ("Actors.ACBS", tagMap[0]);
                ASSERT_STREQ("Actors.AIData", tagMap[1]);
                ASSERT_STREQ("C.Water", tagMap[2]);
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
            EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(NULL, blankEsm.c_str(), &added, &numAdded, &removed, &numRemoved, &modified));
            EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(db, NULL, &added, &numAdded, &removed, &numRemoved, &modified));
            EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(db, blankEsm.c_str(), NULL, &numAdded, &removed, &numRemoved, &modified));
            EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(db, blankEsm.c_str(), &added, NULL, &removed, &numRemoved, &modified));
            EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(db, blankEsm.c_str(), &added, &numAdded, NULL, &numRemoved, &modified));
            EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(db, blankEsm.c_str(), &added, &numAdded, &removed, NULL, &modified));
            EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(db, blankEsm.c_str(), &added, &numAdded, &removed, &numRemoved, NULL));
        }

        TEST_P(loot_get_plugin_tags_test, shouldReturnANoTagMapIfCalledBeforeATagMapHasBeenGotten) {
            EXPECT_EQ(loot_error_no_tag_map, loot_get_plugin_tags(db, blankEsm.c_str(), &added, &numAdded, &removed, &numRemoved, &modified));
        }

        TEST_P(loot_get_plugin_tags_test, shouldReturnOkAndOutputEmptyNonModifiedArraysIfAPluginWithoutTagsIsQueried) {
            ASSERT_NO_THROW(generateMasterlist());
            ASSERT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), NULL));
            getTagMap();

            EXPECT_EQ(loot_ok, loot_get_plugin_tags(db, blankEsp.c_str(), &added, &numAdded, &removed, &numRemoved, &modified));

            EXPECT_EQ(0, numAdded);
            EXPECT_EQ(NULL, added);
            EXPECT_EQ(0, numRemoved);
            EXPECT_EQ(NULL, removed);
            EXPECT_FALSE(modified);
        }

        TEST_P(loot_get_plugin_tags_test, shouldReturnOkAndNonEmptyNonModifiedArraysIfAPluginWithTagsIsQueried) {
            ASSERT_NO_THROW(generateMasterlist());
            ASSERT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), NULL));
            getTagMap();

            EXPECT_EQ(loot_ok, loot_get_plugin_tags(db, blankEsm.c_str(), &added, &numAdded, &removed, &numRemoved, &modified));

            // The values are tag map indices, check they match up as expected.
            ASSERT_EQ(2, numAdded);
            EXPECT_EQ(0, added[0]);
            EXPECT_EQ(1, added[1]);

            ASSERT_EQ(1, numRemoved);
            EXPECT_EQ(2, removed[0]);

            EXPECT_FALSE(modified);
        }

        TEST_P(loot_get_plugin_tags_test, shouldReturnOkAndNonEmptyModifiedArraysIfAPluginWithTagsIsQueriedAndMetadataWasAlsoLoadedFromAUserlist) {
            ASSERT_NO_THROW(generateMasterlist());
            ASSERT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), masterlistPath.string().c_str()));
            getTagMap();

            EXPECT_EQ(loot_ok, loot_get_plugin_tags(db, blankEsm.c_str(), &added, &numAdded, &removed, &numRemoved, &modified));

            // The values are tag map indices, check they match up as expected.
            ASSERT_EQ(2, numAdded);
            EXPECT_EQ(0, added[0]);
            EXPECT_EQ(1, added[1]);

            ASSERT_EQ(1, numRemoved);
            EXPECT_EQ(2, removed[0]);

            EXPECT_TRUE(modified);
        }
    }
}

#endif
