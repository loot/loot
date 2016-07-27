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

#ifndef LOOT_TESTS_API_TEST_API
#define LOOT_TESTS_API_TEST_API

#include "loot/api.h"

#include <gtest/gtest.h>

namespace loot {
namespace test {
TEST(loot_get_version, shouldReturnAnInvalidArgsErrorIfPassedNullPointers) {
  unsigned int vMajor, vMinor, vPatch;
  EXPECT_EQ(loot_error_invalid_args, loot_get_version(&vMajor, NULL, NULL));
  EXPECT_EQ(loot_error_invalid_args, loot_get_version(NULL, &vMinor, NULL));
  EXPECT_EQ(loot_error_invalid_args, loot_get_version(NULL, NULL, &vPatch));
  EXPECT_EQ(loot_error_invalid_args, loot_get_version(NULL, NULL, NULL));
}

TEST(loot_get_version, shouldReturnOkIfPassedNonNullPointers) {
  unsigned int vMajor, vMinor, vPatch;
  EXPECT_EQ(loot_ok, loot_get_version(&vMajor, &vMinor, &vPatch));
}

TEST(loot_get_build_id, shouldReturnAnInvalidArgsErrorIfPassedANullPointer) {
  EXPECT_EQ(loot_error_invalid_args, loot_get_build_id(NULL));
}

TEST(loot_get_build_id, shouldReturnOkAndOutputANonNullNonPlaceholderRevisionString) {
  const char * revision;
  EXPECT_EQ(loot_ok, loot_get_build_id(&revision));
  EXPECT_STRNE(NULL, revision);
  EXPECT_STRNE("@GIT_COMMIT_STRING@", revision);  // The CMake placeholder.
}

TEST(loot_is_compatible, shouldReturnTrueWithEqualMajorAndMinorVersionsAndUnequalPatchVersion) {
  unsigned int vMajor, vMinor, vPatch;
  EXPECT_EQ(loot_ok, loot_get_version(&vMajor, &vMinor, &vPatch));

  EXPECT_TRUE(loot_is_compatible(vMajor, vMinor, vPatch + 1));
}

TEST(loot_is_compatible, shouldReturnFalseWithEqualMajorVersionAndUnequalMinorAndPatchVersions) {
  unsigned int vMajor, vMinor, vPatch;
  EXPECT_EQ(loot_ok, loot_get_version(&vMajor, &vMinor, &vPatch));

  EXPECT_FALSE(loot_is_compatible(vMajor, vMinor + 1, vPatch + 1));
}

TEST(loot_get_error_message, shouldReturnAnInvalidArgsErrorIfPassedANullPointer) {
  EXPECT_EQ(loot_error_invalid_args, loot_get_error_message(NULL));

  const char * error;
  EXPECT_EQ(loot_ok, loot_get_error_message(&error));
  ASSERT_STREQ("Null message pointer passed.", error);
}

TEST(loot_get_error_message, shouldReturnOkIfPassedANonNullPointer) {
  const char * error;
  EXPECT_EQ(loot_ok, loot_get_error_message(&error));
}

TEST(loot_get_error_message, shouldOutputAnErrorMessageDetailingTheLastErrorIfAnErrorHasOccurred) {
  EXPECT_EQ(loot_error_invalid_args, loot_get_error_message(NULL));

  const char * error;
  EXPECT_EQ(loot_ok, loot_get_error_message(&error));
  ASSERT_STREQ("Null message pointer passed.", error);
}

TEST(loot_destroy_db, shouldNotThrowIfPassedANullPointer) {
  ASSERT_NO_THROW(loot_destroy_db(NULL));
}
}
}

#endif
