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

#ifndef LOOT_TESTS_API_LOOT_GET_MASTERLIST_REVISION_TEST
#define LOOT_TESTS_API_LOOT_GET_MASTERLIST_REVISION_TEST

#include "loot/api.h"

#include "tests/api/api_game_operations_test.h"

namespace loot {
namespace test {
class loot_get_masterlist_revision_test : public ApiGameOperationsTest {
protected:
  loot_get_masterlist_revision_test() :
    branch_("2.x"),
    revisionId_("foo"),
    revisionDate_("bar"),
    isModified_(true),
    updated_(false) {}

  const std::string branch_;
  const char * revisionId_;
  const char * revisionDate_;
  bool isModified_;
  bool updated_;
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
  EXPECT_EQ(loot_error_invalid_args, loot_get_masterlist_revision(NULL, masterlistPath.string().c_str(), false, &revisionId_, &revisionDate_, &isModified_));
  EXPECT_EQ(loot_error_invalid_args, loot_get_masterlist_revision(db_, NULL, false, &revisionId_, &revisionDate_, &isModified_));
  EXPECT_EQ(loot_error_invalid_args, loot_get_masterlist_revision(db_, masterlistPath.string().c_str(), false, NULL, &revisionDate_, &isModified_));
  EXPECT_EQ(loot_error_invalid_args, loot_get_masterlist_revision(db_, masterlistPath.string().c_str(), false, &revisionId_, NULL, &isModified_));
  EXPECT_EQ(loot_error_invalid_args, loot_get_masterlist_revision(db_, masterlistPath.string().c_str(), false, &revisionId_, &revisionDate_, NULL));
}

TEST_P(loot_get_masterlist_revision_test, shouldSucceedIfNoMasterlistIsPresent) {
  EXPECT_EQ(loot_ok, loot_get_masterlist_revision(db_, masterlistPath.string().c_str(), false, &revisionId_, &revisionDate_, &isModified_));
  EXPECT_EQ(NULL, revisionId_);
  EXPECT_EQ(NULL, revisionDate_);
  EXPECT_FALSE(isModified_);
}

TEST_P(loot_get_masterlist_revision_test, shouldSucceedIfANonVersionControlledMasterlistIsPresent) {
  ASSERT_NO_THROW(GenerateMasterlist());
  EXPECT_EQ(loot_ok, loot_get_masterlist_revision(db_, masterlistPath.string().c_str(), false, &revisionId_, &revisionDate_, &isModified_));
  EXPECT_EQ(NULL, revisionId_);
  EXPECT_EQ(NULL, revisionDate_);
  EXPECT_FALSE(isModified_);
}

TEST_P(loot_get_masterlist_revision_test, shouldOutputLongStringsAndBooleanFalseIfAVersionControlledMasterlistIsPresentAndGetShortIdParameterIsFalse) {
  ASSERT_EQ(loot_ok, loot_update_masterlist(db_, masterlistPath.string().c_str(), "https://github.com/loot/testing-metadata.git", branch_.c_str(), &updated_));

  EXPECT_EQ(loot_ok, loot_get_masterlist_revision(db_, masterlistPath.string().c_str(), false, &revisionId_, &revisionDate_, &isModified_));
  EXPECT_STRNE(NULL, revisionId_);
  EXPECT_EQ(40, strlen(revisionId_));
  EXPECT_STRNE(NULL, revisionDate_);
  EXPECT_EQ(10, strlen(revisionDate_));
  EXPECT_FALSE(isModified_);
}

TEST_P(loot_get_masterlist_revision_test, shouldOutputShortStringsAndBooleanFalseIfAVersionControlledMasterlistIsPresentAndGetShortIdParameterIsTrue) {
  ASSERT_EQ(loot_ok, loot_update_masterlist(db_, masterlistPath.string().c_str(), "https://github.com/loot/testing-metadata.git", branch_.c_str(), &updated_));

  EXPECT_EQ(loot_ok, loot_get_masterlist_revision(db_, masterlistPath.string().c_str(), false, &revisionId_, &revisionDate_, &isModified_));
  EXPECT_STRNE(NULL, revisionId_);
  EXPECT_GE(size_t(40), strlen(revisionId_));
  EXPECT_LE(size_t(7), strlen(revisionId_));
  EXPECT_STRNE(NULL, revisionDate_);
  EXPECT_EQ(10, strlen(revisionDate_));
  EXPECT_FALSE(isModified_);
}

TEST_P(loot_get_masterlist_revision_test, shouldSucceedIfAnEditedVersionControlledMasterlistIsPresent) {
  ASSERT_EQ(loot_ok, loot_update_masterlist(db_, masterlistPath.string().c_str(), "https://github.com/loot/testing-metadata.git", branch_.c_str(), &updated_));

  ASSERT_NO_THROW(GenerateMasterlist());
  EXPECT_EQ(loot_ok, loot_get_masterlist_revision(db_, masterlistPath.string().c_str(), false, &revisionId_, &revisionDate_, &isModified_));

  EXPECT_STRNE(NULL, revisionId_);
  EXPECT_EQ(40, strlen(revisionId_));
  EXPECT_STRNE(NULL, revisionDate_);
  EXPECT_EQ(10, strlen(revisionDate_));
  EXPECT_TRUE(isModified_);
}
}
}

#endif
