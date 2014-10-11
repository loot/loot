/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014    WrinklyNinja

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

#ifndef LOOT_TEST_API
#define LOOT_TEST_API

#include "../../api/api.h"
#include "tests/fixtures.h"

TEST(GetVersion, HandlesNullInput) {
    unsigned int vMajor, vMinor, vPatch;
    EXPECT_EQ(loot_error_invalid_args, loot_get_version(&vMajor, NULL, NULL));
    EXPECT_EQ(loot_error_invalid_args, loot_get_version(NULL, &vMinor, NULL));
    EXPECT_EQ(loot_error_invalid_args, loot_get_version(NULL, NULL, &vPatch));
    EXPECT_EQ(loot_error_invalid_args, loot_get_version(NULL, NULL, NULL));
}

TEST(GetVersion, HandlesValidInput) {
    unsigned int vMajor, vMinor, vPatch;
    EXPECT_EQ(loot_ok, loot_get_version(&vMajor, &vMinor, &vPatch));
}

TEST(IsCompatible, HandlesCompatibleVersion) {
    unsigned int vMajor, vMinor, vPatch;
    EXPECT_EQ(loot_ok, loot_get_version(&vMajor, &vMinor, &vPatch));

    EXPECT_TRUE(loot_is_compatible(vMajor, vMinor, vPatch));
    // Test somewhat arbitrary variations.
    EXPECT_TRUE(loot_is_compatible(vMajor, vMinor + 1, vPatch + 1));
    if (vMinor > 0 && vPatch > 0)
        EXPECT_TRUE(loot_is_compatible(vMajor, vMinor - 1, vPatch - 1));
}

TEST(IsCompatible, HandlesIncompatibleVersion) {
    unsigned int vMajor, vMinor, vPatch;
    EXPECT_EQ(loot_ok, loot_get_version(&vMajor, &vMinor, &vPatch));

    EXPECT_FALSE(loot_is_compatible(vMajor + 1, vMinor, vPatch));
    // Test somewhat arbitrary variations.
    EXPECT_FALSE(loot_is_compatible(vMajor + 1, vMinor + 1, vPatch + 1));
    if (vMinor > 0 && vPatch > 0)
        EXPECT_FALSE(loot_is_compatible(vMajor + 1, vMinor - 1, vPatch - 1));
}

TEST(GetErrorMessage, HandlesInputCorrectly) {
    EXPECT_EQ(loot_error_invalid_args, loot_get_error_message(NULL));

    const char * error;
    EXPECT_EQ(loot_ok, loot_get_error_message(&error));
    ASSERT_STREQ("Null message pointer passed.", error);
}

TEST(Cleanup, CleansUpAfterError) {
    // First generate an error.
    EXPECT_EQ(loot_error_invalid_args, loot_get_error_message(NULL));

    // Check that the error message is non-null.
    const char * error;
    EXPECT_EQ(loot_ok, loot_get_error_message(&error));
    ASSERT_STREQ("Null message pointer passed.", error);

    ASSERT_NO_THROW(loot_cleanup());

    // Now check that the error message pointer is null.
    error = nullptr;
    EXPECT_EQ(loot_ok, loot_get_error_message(&error));
    EXPECT_EQ(nullptr, error);
}

TEST(Cleanup, HandlesNoError) {
    ASSERT_NO_THROW(loot_cleanup());

    const char * error = nullptr;
    EXPECT_EQ(loot_ok, loot_get_error_message(&error));
    EXPECT_EQ(nullptr, error);
}

TEST_F(OblivionTest, CreateDbHandlesValidInputs) {
    EXPECT_EQ(loot_ok, loot_create_db(&db, loot_game_tes4, dataPath.parent_path().string().c_str(), localPath.string().c_str()));
    ASSERT_NO_THROW(loot_destroy_db(db));
    db = nullptr;

    // Also test absolute paths.
    boost::filesystem::path game = boost::filesystem::current_path() / dataPath.parent_path();
    boost::filesystem::path local = boost::filesystem::current_path() / localPath;
    EXPECT_EQ(loot_ok, loot_create_db(&db, loot_game_tes4, game.string().c_str(), local.string().c_str()));
}

TEST_F(OblivionTest, CreateDbHandlesInvalidHandleInput) {
    EXPECT_EQ(loot_error_invalid_args, loot_create_db(NULL, loot_game_tes4, dataPath.parent_path().string().c_str(), localPath.string().c_str()));
}

TEST_F(OblivionTest, CreateDbHandlesInvalidGameType) {
    EXPECT_EQ(loot_error_invalid_args, loot_create_db(&db, UINT_MAX, dataPath.parent_path().string().c_str(), localPath.string().c_str()));
}

TEST_F(OblivionTest, CreateDbHandlesInvalidGamePathInput) {
    EXPECT_EQ(loot_error_invalid_args, loot_create_db(&db, loot_game_tes4, missingPath.string().c_str(), localPath.string().c_str()));
}

TEST_F(OblivionTest, CreateDbHandlesInvalidLocalPathInput) {
    EXPECT_EQ(loot_error_invalid_args, loot_create_db(&db, loot_game_tes4, dataPath.parent_path().string().c_str(), missingPath.string().c_str()));
}

#ifdef _WIN32
TEST_F(OblivionTest, CreateDbHandlesNullLocalPath) {
    EXPECT_EQ(loot_ok, loot_create_db(&db, loot_game_tes4, dataPath.parent_path().string().c_str(), NULL));
}
#endif

TEST(GameHandleDestroyTest, HandledNullInput) {
    ASSERT_NO_THROW(loot_destroy_db(NULL));
}
#endif
