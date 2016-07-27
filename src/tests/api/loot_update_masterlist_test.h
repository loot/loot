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

#ifndef LOOT_TESTS_API_LOOT_UPDATE_MASTERLIST_TEST
#define LOOT_TESTS_API_LOOT_UPDATE_MASTERLIST_TEST

#include "loot/api.h"

#include "tests/api/api_game_operations_test.h"

namespace loot {
namespace test {
class loot_update_masterlist_test : public ApiGameOperationsTest {
protected:
  loot_update_masterlist_test() :
    updated_(false) {}

  inline void TearDown() {
    ApiGameOperationsTest::TearDown();

    // Also remove the ".git" folder if it has been created.
    ASSERT_NO_THROW(boost::filesystem::remove_all(masterlistPath.parent_path() / ".git"));
  }

  bool updated_;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        loot_update_masterlist_test,
                        ::testing::Values(
                          loot_game_tes4,
                          loot_game_tes5,
                          loot_game_fo3,
                          loot_game_fonv,
                          loot_game_fo4));

TEST_P(loot_update_masterlist_test, shouldReturnAnInvalidArgsErrorIfAnyOfTheArgumentsAreNull) {
  EXPECT_EQ(loot_error_invalid_args, loot_update_masterlist(NULL, masterlistPath.string().c_str(), "https://github.com/loot/testing-metadata.git", "master", &updated_));
  EXPECT_EQ(loot_error_invalid_args, loot_update_masterlist(db_, NULL, "https://github.com/loot/testing-metadata.git", "master", &updated_));
  EXPECT_EQ(loot_error_invalid_args, loot_update_masterlist(db_, masterlistPath.string().c_str(), NULL, "master", &updated_));
  EXPECT_EQ(loot_error_invalid_args, loot_update_masterlist(db_, masterlistPath.string().c_str(), "https://github.com/loot/testing-metadata.git", NULL, &updated_));
  EXPECT_EQ(loot_error_invalid_args, loot_update_masterlist(db_, masterlistPath.string().c_str(), "https://github.com/loot/testing-metadata.git", "master", NULL));
}

TEST_P(loot_update_masterlist_test, shouldReturnAnInvalidArgsErrorIfTheMasterlistPathGivenIsInvalid) {
  EXPECT_EQ(loot_error_invalid_args, loot_update_masterlist(db_, ";//\?", "https://github.com/loot/testing-metadata.git", "master", &updated_));
}

TEST_P(loot_update_masterlist_test, shouldReturnAnInvalidArgsErrorIfTheMasterlistPathGivenIsEmpty) {
  EXPECT_EQ(loot_error_invalid_args, loot_update_masterlist(db_, "", "https://github.com/loot/testing-metadata.git", "master", &updated_));
}

TEST_P(loot_update_masterlist_test, shouldReturnAGitErrorIfTheRepositoryUrlGivenCannotBeFound) {
  EXPECT_EQ(loot_error_git_error, loot_update_masterlist(db_, masterlistPath.string().c_str(), "https://github.com/loot/oblivion-does-not-exist.git", "master", &updated_));
}

TEST_P(loot_update_masterlist_test, shouldReturnAnInvalidArgsErrorIfTheRepositoryUrlGivenIsEmpty) {
  EXPECT_EQ(loot_error_invalid_args, loot_update_masterlist(db_, masterlistPath.string().c_str(), "", "master", &updated_));
}

TEST_P(loot_update_masterlist_test, shouldReturnAGitErrorIfTheRepositoryBranchGivenCannotBeFound) {
  EXPECT_EQ(loot_error_git_error, loot_update_masterlist(db_, masterlistPath.string().c_str(), "https://github.com/loot/testing-metadata.git", "missing-branch", &updated_));
}

TEST_P(loot_update_masterlist_test, shouldReturnAnInvalidArgsErrorIfTheRepositoryBranchGivenIsEmpty) {
  EXPECT_EQ(loot_error_invalid_args, loot_update_masterlist(db_, masterlistPath.string().c_str(), "https://github.com/loot/testing-metadata.git", "", &updated_));
}

TEST_P(loot_update_masterlist_test, shouldSucceedIfPassedValidParametersAndOutputTrueIfTheMasterlistWasUpdated) {
  EXPECT_EQ(loot_ok, loot_update_masterlist(db_, masterlistPath.string().c_str(), "https://github.com/loot/testing-metadata.git", "master", &updated_));
  EXPECT_TRUE(updated_);
  EXPECT_TRUE(boost::filesystem::exists(masterlistPath));
}

TEST_P(loot_update_masterlist_test, shouldSucceedIfCalledRepeatedlyButOnlyOutputTrueForTheFirstCall) {
  EXPECT_EQ(loot_ok, loot_update_masterlist(db_, masterlistPath.string().c_str(), "https://github.com/loot/testing-metadata.git", "master", &updated_));
  EXPECT_TRUE(updated_);

  EXPECT_EQ(loot_ok, loot_update_masterlist(db_, masterlistPath.string().c_str(), "https://github.com/loot/testing-metadata.git", "master", &updated_));
  EXPECT_FALSE(updated_);
  EXPECT_TRUE(boost::filesystem::exists(masterlistPath));
}
}
}

#endif
