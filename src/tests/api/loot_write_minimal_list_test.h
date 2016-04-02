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

#ifndef LOOT_TEST_LOOT_WRITE_MINIMAL_LIST
#define LOOT_TEST_LOOT_WRITE_MINIMAL_LIST

#include "../include/loot/api.h"
#include "api_game_operations_test.h"

namespace loot {
    namespace test {
        class loot_write_minimal_list_test : public ApiGameOperationsTest {
        protected:
            loot_write_minimal_list_test() :
                outputPath(localPath / "minimal.yml") {}

            void SetUp() {
                ApiGameOperationsTest::SetUp();

                ASSERT_FALSE(boost::filesystem::exists(outputPath));
            }

            void TearDown() {
                ApiGameOperationsTest::TearDown();

                ASSERT_NO_THROW(boost::filesystem::remove(outputPath));
            }

            std::string getExpectedContent() const {
                using std::endl;

                std::stringstream expectedContent;
                expectedContent
                    << "plugins:" << endl
                    << "  - name: '" << blankEsm << "'" << endl
                    << "    tag:" << endl
                    << "      - Actors.ACBS" << endl
                    << "      - Actors.AIData" << endl
                    << "      - -C.Water" << endl
                    << "  - name: '" << blankDifferentEsm << "'" << endl
                    << "    dirty:" << endl
                    << "      - crc: 0x7d22f9df" << endl
                    << "        util: 'TES4Edit'" << endl
                    << "        udr: 4";

                return expectedContent.str();
            }

            const boost::filesystem::path outputPath;
        };

        // Pass an empty first argument, as it's a prefix for the test instantation,
        // but we only have the one so no prefix is necessary.
        INSTANTIATE_TEST_CASE_P(,
                                loot_write_minimal_list_test,
                                ::testing::Values(
                                    loot_game_tes4,
                                    loot_game_tes5,
                                    loot_game_fo3,
                                    loot_game_fonv,
                                    loot_game_fo4));

        TEST_P(loot_write_minimal_list_test, shouldReturnAnInvalidArgsErrorIfAPointerArgumentIsNull) {
            EXPECT_EQ(loot_error_invalid_args, loot_write_minimal_list(NULL, outputPath.string().c_str(), false));
            EXPECT_EQ(loot_error_invalid_args, loot_write_minimal_list(db, NULL, false));
        }

        TEST_P(loot_write_minimal_list_test, shouldReturnAFileWriteErrorIfThePathGivenIsInvalid) {
            EXPECT_EQ(loot_error_file_write_fail, loot_write_minimal_list(db, "/:?*", false));
        }

        TEST_P(loot_write_minimal_list_test, shouldReturnOkAndWriteToFileIfArgumentsGivenAreValid) {
            EXPECT_EQ(loot_ok, loot_write_minimal_list(db, outputPath.string().c_str(), false));
            EXPECT_TRUE(boost::filesystem::exists(outputPath));
        }

        TEST_P(loot_write_minimal_list_test, shouldReturnAFileWriteErrorIfTheFileAlreadyExistsAndTheOverwriteArgumentIsFalse) {
            ASSERT_EQ(loot_ok, loot_write_minimal_list(db, outputPath.string().c_str(), false));
            ASSERT_TRUE(boost::filesystem::exists(outputPath));

            EXPECT_EQ(loot_error_file_write_fail, loot_write_minimal_list(db, outputPath.string().c_str(), false));
        }

        TEST_P(loot_write_minimal_list_test, shouldReturnOkAndWriteToFileIfTheArgumentsAreValidAndTheOverwriteArgumentIsTrue) {
            EXPECT_EQ(loot_ok, loot_write_minimal_list(db, outputPath.string().c_str(), true));
            EXPECT_TRUE(boost::filesystem::exists(outputPath));
        }

        TEST_P(loot_write_minimal_list_test, shouldReturnOkIfTheFileAlreadyExistsAndTheOverwriteArgumentIsTrue) {
            ASSERT_EQ(loot_ok, loot_write_minimal_list(db, outputPath.string().c_str(), false));
            ASSERT_TRUE(boost::filesystem::exists(outputPath));

            EXPECT_EQ(loot_ok, loot_write_minimal_list(db, outputPath.string().c_str(), true));
        }

        TEST_P(loot_write_minimal_list_test, shouldWriteOnlyBashTagsAndDirtyInfo) {
            ASSERT_NO_THROW(generateMasterlist());
            ASSERT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), NULL));

            EXPECT_EQ(loot_ok, loot_write_minimal_list(db, outputPath.string().c_str(), true));

            boost::filesystem::ifstream in(outputPath);
            std::stringstream content;
            content << in.rdbuf();

            EXPECT_EQ(getExpectedContent(), content.str());
        }
    }
}

#endif
