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

#ifndef LOOT_TEST_LOOT_GET_MASTERLIST_REVISION
#define LOOT_TEST_LOOT_GET_MASTERLIST_REVISION

#include "../include/loot/api.h"
#include "api_game_operations_test.h"

namespace loot {
    namespace test {
        class loot_get_masterlist_revision_test : public ApiGameOperationsTest {
        protected:
            loot_get_masterlist_revision_test() :
                revisionId("foo"),
                revisionDate("bar"),
                isModified(true),
                updated(false) {}

            const char * revisionId;
            const char * revisionDate;
            bool isModified;
            bool updated;
        };

        // Pass an empty first argument, as it's a prefix for the test instantation,
        // but we only have the one so no prefix is necessary.
        INSTANTIATE_TEST_CASE_P(,
                                loot_get_masterlist_revision_test,
                                ::testing::Values(
                                    loot_game_tes4,
                                    loot_game_tes5,
                                    loot_game_fo3,
                                    loot_game_fonv,
                                    loot_game_fo4));

        TEST_P(loot_get_masterlist_revision_test, shouldReturnAnInvalidArgsErrorIfAnyOfTheArgumentsAreNull) {
            EXPECT_EQ(loot_error_invalid_args, loot_get_masterlist_revision(NULL, masterlistPath.string().c_str(), false, &revisionId, &revisionDate, &isModified));
            EXPECT_EQ(loot_error_invalid_args, loot_get_masterlist_revision(db, NULL, false, &revisionId, &revisionDate, &isModified));
            EXPECT_EQ(loot_error_invalid_args, loot_get_masterlist_revision(db, masterlistPath.string().c_str(), false, NULL, &revisionDate, &isModified));
            EXPECT_EQ(loot_error_invalid_args, loot_get_masterlist_revision(db, masterlistPath.string().c_str(), false, &revisionId, NULL, &isModified));
            EXPECT_EQ(loot_error_invalid_args, loot_get_masterlist_revision(db, masterlistPath.string().c_str(), false, &revisionId, &revisionDate, NULL));
        }

        TEST_P(loot_get_masterlist_revision_test, shouldSucceedIfNoMasterlistIsPresent) {
            EXPECT_EQ(loot_ok, loot_get_masterlist_revision(db, masterlistPath.string().c_str(), false, &revisionId, &revisionDate, &isModified));
            EXPECT_EQ(NULL, revisionId);
            EXPECT_EQ(NULL, revisionDate);
            EXPECT_FALSE(isModified);
        }

        TEST_P(loot_get_masterlist_revision_test, shouldSucceedIfANonVersionControlledMasterlistIsPresent) {
            ASSERT_NO_THROW(generateMasterlist());
            EXPECT_EQ(loot_ok, loot_get_masterlist_revision(db, masterlistPath.string().c_str(), false, &revisionId, &revisionDate, &isModified));
            EXPECT_EQ(NULL, revisionId);
            EXPECT_EQ(NULL, revisionDate);
            EXPECT_FALSE(isModified);
        }

        TEST_P(loot_get_masterlist_revision_test, shouldOutputLongStringsAndBooleanFalseIfAVersionControlledMasterlistIsPresentAndGetShortIdParameterIsFalse) {
            ASSERT_EQ(loot_ok, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/testing-metadata.git", "master", &updated));

            EXPECT_EQ(loot_ok, loot_get_masterlist_revision(db, masterlistPath.string().c_str(), false, &revisionId, &revisionDate, &isModified));
            EXPECT_STRNE(NULL, revisionId);
            EXPECT_EQ(40, strlen(revisionId));
            EXPECT_STRNE(NULL, revisionDate);
            EXPECT_EQ(10, strlen(revisionDate));
            EXPECT_FALSE(isModified);
        }

        TEST_P(loot_get_masterlist_revision_test, shouldOutputShortStringsAndBooleanFalseIfAVersionControlledMasterlistIsPresentAndGetShortIdParameterIsTrue) {
            ASSERT_EQ(loot_ok, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/testing-metadata.git", "master", &updated));

            EXPECT_EQ(loot_ok, loot_get_masterlist_revision(db, masterlistPath.string().c_str(), false, &revisionId, &revisionDate, &isModified));
            EXPECT_STRNE(NULL, revisionId);
            EXPECT_GE(size_t(40), strlen(revisionId));
            EXPECT_LE(size_t(7), strlen(revisionId));
            EXPECT_STRNE(NULL, revisionDate);
            EXPECT_EQ(10, strlen(revisionDate));
            EXPECT_FALSE(isModified);
        }

        TEST_P(loot_get_masterlist_revision_test, shouldSucceedIfAnEditedVersionControlledMasterlistIsPresent) {
            ASSERT_EQ(loot_ok, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/testing-metadata.git", "master", &updated));

            ASSERT_NO_THROW(generateMasterlist());
            EXPECT_EQ(loot_ok, loot_get_masterlist_revision(db, masterlistPath.string().c_str(), false, &revisionId, &revisionDate, &isModified));

            EXPECT_STRNE(NULL, revisionId);
            EXPECT_EQ(40, strlen(revisionId));
            EXPECT_STRNE(NULL, revisionDate);
            EXPECT_EQ(10, strlen(revisionDate));
            EXPECT_TRUE(isModified);
        }
    }
}

#endif
