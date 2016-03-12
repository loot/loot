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

#ifndef LOOT_TEST_LOOT_LOAD_LISTS
#define LOOT_TEST_LOOT_LOAD_LISTS

#include "../include/loot/api.h"
#include "api_game_operations_test.h"

namespace loot {
    namespace test {
        class loot_load_lists_test : public ApiGameOperationsTest {
        protected:
            loot_load_lists_test() :
                userlistPath(localPath / "userlist.yaml") {}

            inline virtual void TearDown() {
                ApiGameOperationsTest::TearDown();

                // The userlist may have been created during the test, so delete it.
                ASSERT_NO_THROW(boost::filesystem::remove(userlistPath));
            }

            boost::filesystem::path userlistPath;
        };

        // Pass an empty first argument, as it's a prefix for the test instantation,
        // but we only have the one so no prefix is necessary.
        INSTANTIATE_TEST_CASE_P(,
                                loot_load_lists_test,
                                ::testing::Values(
                                    loot_game_tes4,
                                    loot_game_tes5,
                                    loot_game_fo3,
                                    loot_game_fonv,
                                    loot_game_fo4));

        TEST_P(loot_load_lists_test, shouldReturnAnInvalidArgsErrorIfTheDbOrMasterlistPathArgumentsAreNull) {
            EXPECT_EQ(loot_error_invalid_args, loot_load_lists(NULL, masterlistPath.string().c_str(), NULL));
            EXPECT_EQ(loot_error_invalid_args, loot_load_lists(db, NULL, NULL));
        }

        TEST_P(loot_load_lists_test, shouldReturnAPathNotFoundErrorIfNoMasterlistIsPresent) {
            EXPECT_EQ(loot_error_path_not_found, loot_load_lists(db, masterlistPath.string().c_str(), NULL));
        }

        TEST_P(loot_load_lists_test, shouldReturnAPathNotFoundErrorIfAMasterlistIsPresentButAUserlistDoesNotExistAtTheGivenPath) {
            ASSERT_NO_THROW(generateMasterlist());
            EXPECT_EQ(loot_error_path_not_found, loot_load_lists(db, masterlistPath.string().c_str(), userlistPath.string().c_str()));
        }

        TEST_P(loot_load_lists_test, shouldReturnOkIfTheMasterlistAndUserlistAreBothPresent) {
            ASSERT_NO_THROW(generateMasterlist());
            ASSERT_NO_THROW(boost::filesystem::copy(masterlistPath, userlistPath));

            EXPECT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), userlistPath.string().c_str()));
        }
    }
}

#endif
