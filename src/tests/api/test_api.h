/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2015    WrinklyNinja

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

#include "api/api.h"
#include "tests/fixtures.h"

#include <boost/algorithm/string/predicate.hpp>

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

TEST(GetBuildID, HandlesNullInput) {
    EXPECT_EQ(loot_error_invalid_args, loot_get_build_id(NULL));
}

TEST(GetBuildID, HandlesValidInput) {
    const char * revision;
    EXPECT_EQ(loot_ok, loot_get_build_id(&revision));
    EXPECT_STRNE(NULL, revision);
    EXPECT_STRNE("@GIT_COMMIT_STRING@", revision);  // The CMake placeholder.
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

TEST_F(SkyrimTest, CreateDbHandlesValidInputs) {
    EXPECT_EQ(loot_ok, loot_create_db(&db, loot_game_tes5, dataPath.parent_path().string().c_str(), localPath.string().c_str()));
    ASSERT_NO_THROW(loot_destroy_db(db));
    db = nullptr;

    // Also test absolute paths.
    boost::filesystem::path game = boost::filesystem::current_path() / dataPath.parent_path();
    boost::filesystem::path local = boost::filesystem::current_path() / localPath;
    EXPECT_EQ(loot_ok, loot_create_db(&db, loot_game_tes5, game.string().c_str(), local.string().c_str()));
}

TEST_F(SkyrimTest, CreateDbHandlesInvalidHandleInput) {
    EXPECT_EQ(loot_error_invalid_args, loot_create_db(NULL, loot_game_tes5, dataPath.parent_path().string().c_str(), localPath.string().c_str()));
}

TEST_F(SkyrimTest, CreateDbHandlesInvalidGameType) {
    EXPECT_EQ(loot_error_invalid_args, loot_create_db(&db, UINT_MAX, dataPath.parent_path().string().c_str(), localPath.string().c_str()));
}

TEST_F(SkyrimTest, CreateDbHandlesInvalidGamePathInput) {
    EXPECT_EQ(loot_error_invalid_args, loot_create_db(&db, loot_game_tes5, missingPath.string().c_str(), localPath.string().c_str()));
}

TEST_F(SkyrimTest, CreateDbHandlesInvalidLocalPathInput) {
    EXPECT_EQ(loot_error_invalid_args, loot_create_db(&db, loot_game_tes5, dataPath.parent_path().string().c_str(), missingPath.string().c_str()));
}

#ifdef _WIN32
TEST_F(SkyrimTest, CreateDbHandlesNullLocalPath) {
    EXPECT_EQ(loot_ok, loot_create_db(&db, loot_game_tes5, dataPath.parent_path().string().c_str(), NULL));
}
#endif

TEST(GameHandleDestroyTest, HandledNullInput) {
    ASSERT_NO_THROW(loot_destroy_db(NULL));
}

TEST_F(OblivionAPIOperationsTest, UpdateMasterlist) {
    bool updated;
    EXPECT_EQ(loot_error_invalid_args, loot_update_masterlist(NULL, masterlistPath.string().c_str(), "https://github.com/loot/oblivion.git", "master", &updated));
    EXPECT_EQ(loot_error_invalid_args, loot_update_masterlist(db, NULL, "https://github.com/loot/oblivion.git", "master", &updated));
    EXPECT_EQ(loot_error_invalid_args, loot_update_masterlist(db, masterlistPath.string().c_str(), NULL, "master", &updated));
    EXPECT_EQ(loot_error_invalid_args, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/oblivion.git", NULL, &updated));
    EXPECT_EQ(loot_error_invalid_args, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/oblivion.git", "master", NULL));

    EXPECT_EQ(loot_error_invalid_args, loot_update_masterlist(db, ";//\?", "https://github.com/loot/oblivion.git", "master", &updated));
    EXPECT_EQ(loot_error_invalid_args, loot_update_masterlist(db, "", "https://github.com/loot/oblivion.git", "master", &updated));
    EXPECT_EQ(loot_error_git_error, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/oblivion-does-not-exist.git", "master", &updated));
    EXPECT_EQ(loot_error_git_error, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/oblivion.git", "missing-branch", &updated));
    EXPECT_EQ(loot_error_git_error, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/oblivion.git", "", &updated));

    // Test actual masterlist update.
    EXPECT_EQ(loot_ok, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/oblivion.git", "master", &updated));
    EXPECT_TRUE(updated);

    // Test with an up-to-date masterlist.
    EXPECT_EQ(loot_ok, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/oblivion.git", "master", &updated));
    EXPECT_FALSE(updated);
}

TEST_F(OblivionAPIOperationsTest, GetMasterlistRevision) {
    char * revisionID;
    char * revisionDate;
    bool isModified;
    EXPECT_EQ(loot_error_invalid_args, loot_get_masterlist_revision(NULL, masterlistPath.string().c_str(), false, &revisionID, &revisionDate, &isModified));
    EXPECT_EQ(loot_error_invalid_args, loot_get_masterlist_revision(db, NULL, false, &revisionID, &revisionDate, &isModified));
    EXPECT_EQ(loot_error_invalid_args, loot_get_masterlist_revision(db, masterlistPath.string().c_str(), false, NULL, &revisionDate, &isModified));
    EXPECT_EQ(loot_error_invalid_args, loot_get_masterlist_revision(db, masterlistPath.string().c_str(), false, &revisionID, NULL, &isModified));
    EXPECT_EQ(loot_error_invalid_args, loot_get_masterlist_revision(db, masterlistPath.string().c_str(), false, &revisionID, &revisionDate, NULL));

    // Test with no masterlist.
    EXPECT_EQ(loot_ok, loot_get_masterlist_revision(db, masterlistPath.string().c_str(), false, &revisionID, &revisionDate, &isModified));
    EXPECT_EQ(NULL, revisionID);
    EXPECT_EQ(NULL, revisionDate);
    EXPECT_FALSE(isModified);

    // Test with a generated (non-revision-control) masterlist.
    ASSERT_NO_THROW(GenerateMasterlist());
    EXPECT_EQ(loot_ok, loot_get_masterlist_revision(db, masterlistPath.string().c_str(), false, &revisionID, &revisionDate, &isModified));
    EXPECT_EQ(NULL, revisionID);
    EXPECT_EQ(NULL, revisionDate);
    EXPECT_FALSE(isModified);

    // Update the masterlist and test.
    bool updated;
    ASSERT_EQ(loot_ok, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/oblivion.git", "master", &updated));
    ASSERT_TRUE(updated);
    EXPECT_EQ(loot_ok, loot_get_masterlist_revision(db, masterlistPath.string().c_str(), false, &revisionID, &revisionDate, &isModified));
    EXPECT_STRNE(NULL, revisionID);
    EXPECT_EQ(40, strlen(revisionID));
    EXPECT_STRNE(NULL, revisionDate);
    EXPECT_EQ(10, strlen(revisionDate));
    EXPECT_FALSE(isModified);

    // Test with a short revision string.
    EXPECT_EQ(loot_ok, loot_get_masterlist_revision(db, masterlistPath.string().c_str(), true, &revisionID, &revisionDate, &isModified));
    EXPECT_STRNE(NULL, revisionID);
    EXPECT_GE(size_t(40), strlen(revisionID));
    EXPECT_LE(size_t(7), strlen(revisionID));
    EXPECT_STRNE(NULL, revisionDate);
    EXPECT_EQ(10, strlen(revisionDate));
    EXPECT_FALSE(isModified);

    // Overwrite the masterlist with a generated one.
    ASSERT_NO_THROW(GenerateMasterlist());
    EXPECT_EQ(loot_ok, loot_get_masterlist_revision(db, masterlistPath.string().c_str(), false, &revisionID, &revisionDate, &isModified));
    EXPECT_STRNE(NULL, revisionID);
    EXPECT_EQ(40, strlen(revisionID));
    EXPECT_STRNE(NULL, revisionDate);
    EXPECT_EQ(10, strlen(revisionDate));
    EXPECT_TRUE(isModified);
}

TEST_F(OblivionAPIOperationsTest, LoadLists) {
    EXPECT_EQ(loot_error_invalid_args, loot_load_lists(NULL, masterlistPath.string().c_str(), NULL));
    EXPECT_EQ(loot_error_invalid_args, loot_load_lists(db, NULL, NULL));

    EXPECT_EQ(loot_error_path_not_found, loot_load_lists(db, masterlistPath.string().c_str(), NULL));
    bool updated;
    ASSERT_EQ(loot_ok, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/oblivion.git", "master", &updated));
    EXPECT_EQ(loot_error_path_not_found, loot_load_lists(db, masterlistPath.string().c_str(), userlistPath.string().c_str()));

    ASSERT_NO_THROW(boost::filesystem::copy(masterlistPath, userlistPath));
    EXPECT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), userlistPath.string().c_str()));
}

TEST_F(SkyrimAPIOperationsTest, LoadLists) {
    EXPECT_EQ(loot_error_invalid_args, loot_load_lists(NULL, masterlistPath.string().c_str(), NULL));
    EXPECT_EQ(loot_error_invalid_args, loot_load_lists(db, NULL, NULL));

    EXPECT_EQ(loot_error_path_not_found, loot_load_lists(db, masterlistPath.string().c_str(), NULL));
    bool updated;
    ASSERT_EQ(loot_ok, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/skyrim.git", "master", &updated));
    EXPECT_EQ(loot_error_path_not_found, loot_load_lists(db, masterlistPath.string().c_str(), userlistPath.string().c_str()));

    ASSERT_NO_THROW(boost::filesystem::copy(masterlistPath, userlistPath));
    EXPECT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), userlistPath.string().c_str()));
}

TEST_F(OblivionAPIOperationsTest, EvalLists) {
    // No lists loaded.
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_any));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_english));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_spanish));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_russian));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_french));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_chinese));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_polish));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_brazilian_portuguese));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_finnish));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_german));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_danish));

    // Invalid args.
    EXPECT_EQ(loot_error_invalid_args, loot_eval_lists(NULL, loot_lang_any));
    EXPECT_EQ(loot_error_invalid_args, loot_eval_lists(db, UINT_MAX));

    // Now test different languages with a list loaded.
    bool updated;
    ASSERT_EQ(loot_ok, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/oblivion.git", "master", &updated));
    ASSERT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), NULL));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_any));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_english));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_spanish));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_russian));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_french));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_chinese));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_polish));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_brazilian_portuguese));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_finnish));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_german));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_danish));
}

TEST_F(SkyrimAPIOperationsTest, EvalLists) {
    // No lists loaded.
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_any));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_english));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_spanish));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_russian));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_french));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_chinese));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_polish));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_brazilian_portuguese));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_finnish));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_german));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_danish));

    // Invalid args.
    EXPECT_EQ(loot_error_invalid_args, loot_eval_lists(NULL, loot_lang_any));
    EXPECT_EQ(loot_error_invalid_args, loot_eval_lists(db, UINT_MAX));

    // Now test different languages with a list loaded.
    bool updated;
    ASSERT_EQ(loot_ok, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/skyrim.git", "master", &updated));
    ASSERT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), NULL));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_any));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_english));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_spanish));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_russian));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_french));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_chinese));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_polish));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_brazilian_portuguese));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_finnish));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_german));
    EXPECT_EQ(loot_ok, loot_eval_lists(db, loot_lang_danish));
}

TEST_F(OblivionAPIOperationsTest, SortPlugins) {
    char ** sortedPlugins;
    size_t numPlugins;
    EXPECT_EQ(loot_error_invalid_args, loot_sort_plugins(NULL, &sortedPlugins, &numPlugins));
    EXPECT_EQ(loot_error_invalid_args, loot_sort_plugins(db, NULL, &numPlugins));
    EXPECT_EQ(loot_error_invalid_args, loot_sort_plugins(db, &sortedPlugins, NULL));

    EXPECT_EQ(loot_ok, loot_sort_plugins(db, &sortedPlugins, &numPlugins));

    // Expected order was obtained from running the API function once.
    std::list<std::string> expectedOrder = {
        "Oblivion.esm",
        "Blank.esm",
        "Blank - Different.esm",
        "Blank - Master Dependent.esm",
        "Blank - Different Master Dependent.esm",
        "Blank.esp",
        "Blank - Different.esp",
        "Blank - Master Dependent.esp",
        "Blank - Different Master Dependent.esp",
        "Blank - Plugin Dependent.esp",
        "Blank - Different Plugin Dependent.esp",
    };
    std::list<std::string> actualOrder;
    for (size_t i = 0; i < numPlugins; ++i) {
        actualOrder.push_back(sortedPlugins[i]);
    }
    EXPECT_EQ(11, numPlugins);
    EXPECT_EQ(expectedOrder, actualOrder);
}

TEST_F(SkyrimAPIOperationsTest, SortPlugins) {
    char ** sortedPlugins;
    size_t numPlugins;
    EXPECT_EQ(loot_error_invalid_args, loot_sort_plugins(NULL, &sortedPlugins, &numPlugins));
    EXPECT_EQ(loot_error_invalid_args, loot_sort_plugins(db, NULL, &numPlugins));
    EXPECT_EQ(loot_error_invalid_args, loot_sort_plugins(db, &sortedPlugins, NULL));

    EXPECT_EQ(loot_ok, loot_sort_plugins(db, &sortedPlugins, &numPlugins));

    // Expected order was obtained from running the API function once.
    std::list<std::string> expectedOrder = {
        "Skyrim.esm",
        "Blank.esm",
        "Blank - Different.esm",
        "Blank - Master Dependent.esm",
        "Blank - Different Master Dependent.esm",
        "Blank.esp",
        "Blank - Different.esp",
        "Blank - Master Dependent.esp",
        "Blank - Different Master Dependent.esp",
        "Blank - Plugin Dependent.esp",
        "Blank - Different Plugin Dependent.esp",
    };
    std::list<std::string> actualOrder;
    for (size_t i = 0; i < numPlugins; ++i) {
        actualOrder.push_back(sortedPlugins[i]);
    }
    EXPECT_EQ(11, numPlugins);
    EXPECT_EQ(expectedOrder, actualOrder);
}

TEST_F(OblivionAPIOperationsTest, ApplyLoadOrder) {
    const char * loadOrder[11] = {
        "Oblivion.esm",
        "Blank.esm",
        "Blank - Different.esm",
        "Blank - Different Master Dependent.esm",
        "Blank - Master Dependent.esm",
        "Blank.esp",
        "Blank - Different.esp",
        "Blank - Different Master Dependent.esp",
        "Blank - Different Plugin Dependent.esp",
        "Blank - Master Dependent.esp",
        "Blank - Plugin Dependent.esp",
    };
    size_t numPlugins = 11;
    EXPECT_EQ(loot_error_invalid_args, loot_apply_load_order(NULL, loadOrder, numPlugins));
    EXPECT_EQ(loot_error_invalid_args, loot_apply_load_order(db, NULL, numPlugins));

    EXPECT_EQ(loot_ok, loot_apply_load_order(db, loadOrder, numPlugins));
}

TEST_F(SkyrimAPIOperationsTest, ApplyLoadOrder) {
    const char * loadOrder[11] = {
        "Skyrim.esm",
        "Blank.esm",
        "Blank - Different.esm",
        "Blank - Different Master Dependent.esm",
        "Blank - Master Dependent.esm",
        "Blank.esp",
        "Blank - Different.esp",
        "Blank - Different Master Dependent.esp",
        "Blank - Different Plugin Dependent.esp",
        "Blank - Master Dependent.esp",
        "Blank - Plugin Dependent.esp",
    };
    size_t numPlugins = 11;
    EXPECT_EQ(loot_error_invalid_args, loot_apply_load_order(NULL, loadOrder, numPlugins));
    EXPECT_EQ(loot_error_invalid_args, loot_apply_load_order(db, NULL, numPlugins));

    EXPECT_EQ(loot_ok, loot_apply_load_order(db, loadOrder, numPlugins));
}

TEST_F(OblivionAPIOperationsTest, GetTagMap) {
    char ** tagMap;
    size_t numTags;
    EXPECT_EQ(loot_error_invalid_args, loot_get_tag_map(NULL, &tagMap, &numTags));
    EXPECT_EQ(loot_error_invalid_args, loot_get_tag_map(db, NULL, &numTags));
    EXPECT_EQ(loot_error_invalid_args, loot_get_tag_map(db, &tagMap, NULL));

    // Get tag map with no masterlist loaded: should have no tags.
    EXPECT_EQ(loot_ok, loot_get_tag_map(db, &tagMap, &numTags));
    EXPECT_EQ(0, numTags);
    EXPECT_EQ(NULL, tagMap);

    // Get a masterlist, then get tags from it.
    ASSERT_NO_THROW(GenerateMasterlist());
    ASSERT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), NULL));
    EXPECT_EQ(loot_ok, loot_get_tag_map(db, &tagMap, &numTags));
    ASSERT_EQ(22, numTags);
    EXPECT_STREQ("Actors.ACBS", tagMap[0]);
    EXPECT_STREQ("Actors.AIData", tagMap[1]);
    EXPECT_STREQ("Actors.AIPackages", tagMap[2]);
    EXPECT_STREQ("Actors.CombatStyle", tagMap[3]);
    EXPECT_STREQ("Actors.DeathItem", tagMap[4]);
    EXPECT_STREQ("Actors.Stats", tagMap[5]);
    EXPECT_STREQ("C.Climate", tagMap[6]);
    EXPECT_STREQ("C.Light", tagMap[7]);
    EXPECT_STREQ("C.Music", tagMap[8]);
    EXPECT_STREQ("C.Name", tagMap[9]);
    EXPECT_STREQ("C.Owner", tagMap[10]);
    EXPECT_STREQ("C.Water", tagMap[11]);
    EXPECT_STREQ("Creatures.Blood", tagMap[12]);
    EXPECT_STREQ("Delev", tagMap[13]);
    EXPECT_STREQ("Factions", tagMap[14]);
    EXPECT_STREQ("Invent", tagMap[15]);
    EXPECT_STREQ("NPC.Class", tagMap[16]);
    EXPECT_STREQ("Names", tagMap[17]);
    EXPECT_STREQ("Relations", tagMap[18]);
    EXPECT_STREQ("Relev", tagMap[19]);
    EXPECT_STREQ("Scripts", tagMap[20]);
    EXPECT_STREQ("Stats", tagMap[21]);
}

TEST_F(OblivionAPIOperationsTest, GetPluginTags) {
    unsigned int * added;
    unsigned int * removed;
    size_t numAdded, numRemoved;
    bool modified;
    EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(NULL, "Unofficial Oblivion Patch.esp", &added, &numAdded, &removed, &numRemoved, &modified));
    EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(db, NULL, &added, &numAdded, &removed, &numRemoved, &modified));
    EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(db, "Unofficial Oblivion Patch.esp", NULL, &numAdded, &removed, &numRemoved, &modified));
    EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(db, "Unofficial Oblivion Patch.esp", &added, NULL, &removed, &numRemoved, &modified));
    EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(db, "Unofficial Oblivion Patch.esp", &added, &numAdded, NULL, &numRemoved, &modified));
    EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(db, "Unofficial Oblivion Patch.esp", &added, &numAdded, &removed, NULL, &modified));
    EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_tags(db, "Unofficial Oblivion Patch.esp", &added, &numAdded, &removed, &numRemoved, NULL));

    // Get tags before getting a tag map.
    EXPECT_EQ(loot_error_no_tag_map, loot_get_plugin_tags(db, "Unofficial Oblivion Patch.esp", &added, &numAdded, &removed, &numRemoved, &modified));

    // Load tag map.
    char ** tagMap;
    size_t numTags;
    ASSERT_NO_THROW(GenerateMasterlist());
    ASSERT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), NULL));
    ASSERT_EQ(loot_ok, loot_get_tag_map(db, &tagMap, &numTags));

    // Get tags for a plugin without any.
    EXPECT_EQ(loot_ok, loot_get_plugin_tags(db, "Blank.esp", &added, &numAdded, &removed, &numRemoved, &modified));
    EXPECT_EQ(0, numAdded);
    EXPECT_EQ(NULL, added);
    EXPECT_EQ(0, numRemoved);
    EXPECT_EQ(NULL, removed);
    EXPECT_FALSE(modified);

    // Get tags for a plugin with some.
    EXPECT_EQ(loot_ok, loot_get_plugin_tags(db, "Unofficial Oblivion Patch.esp", &added, &numAdded, &removed, &numRemoved, &modified));
    ASSERT_EQ(21, numAdded);
    for (size_t i = 0; i < 11; ++i) {
        EXPECT_EQ(i, added[i]);
    }
    for (size_t i = 11; i < 21; ++i) {
        EXPECT_EQ(i + 1, added[i]);
    }

    ASSERT_EQ(1, numRemoved);
    EXPECT_EQ(11, removed[0]);
    EXPECT_FALSE(modified);

    // Now load the masterlist as the userlist too, and check the modified flag.
    ASSERT_NO_THROW(boost::filesystem::copy_file(masterlistPath, userlistPath));
    ASSERT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), userlistPath.string().c_str()));
    ASSERT_EQ(loot_ok, loot_get_tag_map(db, &tagMap, &numTags));

    EXPECT_EQ(loot_ok, loot_get_plugin_tags(db, "Unofficial Oblivion Patch.esp", &added, &numAdded, &removed, &numRemoved, &modified));
    ASSERT_EQ(21, numAdded);
    for (size_t i = 0; i < 11; ++i) {
        EXPECT_EQ(i, added[i]);
    }
    for (size_t i = 11; i < 21; ++i) {
        EXPECT_EQ(i + 1, added[i]);
    }
    EXPECT_EQ(0, added[0]);
    ASSERT_EQ(1, numRemoved);
    EXPECT_EQ(11, removed[0]);
    EXPECT_TRUE(modified);
}

TEST_F(OblivionAPIOperationsTest, GetPluginMessages) {
    loot_message * messages;
    size_t numMessages;
    EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_messages(NULL, "EnhancedWeatherSIOnly.esm", &messages, &numMessages));
    EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_messages(db, NULL, &messages, &numMessages));
    EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_messages(db, "EnhancedWeatherSIOnly.esm", NULL, &numMessages));
    EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_messages(db, "EnhancedWeatherSIOnly.esm", &messages, NULL));

    // Fetch and load the metadata.
    ASSERT_NO_THROW(GenerateMasterlist());
    ASSERT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), NULL));

    // Test for plugin with no messages.
    EXPECT_EQ(loot_ok, loot_get_plugin_messages(db, "Silgrad_Tower.esm", &messages, &numMessages));
    EXPECT_EQ(0, numMessages);
    EXPECT_EQ(NULL, messages);

    // Test for plugin with one note.
    EXPECT_EQ(loot_ok, loot_get_plugin_messages(db, "No Lights Flicker.esm", &messages, &numMessages));
    EXPECT_EQ(1, numMessages);
    EXPECT_EQ(loot_message_say, messages[0].type);
    EXPECT_STREQ("Use Wrye Bash Bashed Patch tweak instead.", messages[0].message);

    // Test for plugin with one warning.
    EXPECT_EQ(loot_ok, loot_get_plugin_messages(db, "bookplacing.esm", &messages, &numMessages));
    EXPECT_EQ(1, numMessages);
    EXPECT_EQ(loot_message_warn, messages[0].type);
    EXPECT_STREQ("Check you are using v2+. If not, Update. v1 has a severe bug with the Mystic Emporium disappearing.", messages[0].message);

    // Test for plugin with one error.
    EXPECT_EQ(loot_ok, loot_get_plugin_messages(db, "EnhancedWeatherSIOnly.esm", &messages, &numMessages));
    EXPECT_EQ(1, numMessages);
    EXPECT_EQ(loot_message_error, messages[0].type);
    EXPECT_STREQ("Obsolete. Remove this and install Enhanced Weather.", messages[0].message);

    // Test for plugin with more than one message.
    EXPECT_EQ(loot_ok, loot_get_plugin_messages(db, "nVidia Black Screen Fix.esp", &messages, &numMessages));
    EXPECT_EQ(2, numMessages);
    EXPECT_EQ(loot_message_say, messages[0].type);
    EXPECT_STREQ("Use Wrye Bash Bashed Patch tweak instead.", messages[0].message);
    EXPECT_EQ(loot_message_say, messages[1].type);
    EXPECT_STREQ("Alternatively, remove this and use UOP v3.0.1+ instead.", messages[1].message);
}

TEST_F(OblivionAPIOperationsTest, GetDirtyInfo) {
    unsigned int needsCleaning;
    EXPECT_EQ(loot_error_invalid_args, loot_get_dirty_info(NULL, "Oblivion.esm", &needsCleaning));
    EXPECT_EQ(loot_error_invalid_args, loot_get_dirty_info(db, NULL, &needsCleaning));
    EXPECT_EQ(loot_error_invalid_args, loot_get_dirty_info(db, "Oblivion.esm", NULL));

    // Fetch and load the metadata.
    ASSERT_NO_THROW(GenerateMasterlist());
    ASSERT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), NULL));

    // A plugin with no metadata.
    EXPECT_EQ(loot_ok, loot_get_dirty_info(db, "Oblivion.esm", &needsCleaning));
    EXPECT_EQ(loot_needs_cleaning_unknown, needsCleaning);

    // A plugin with dirty metadata.
    EXPECT_EQ(loot_ok, loot_get_dirty_info(db, "Hammerfell.esm", &needsCleaning));
    EXPECT_EQ(loot_needs_cleaning_yes, needsCleaning);

    // A plugin with a standard "Do not clean" message.
    EXPECT_EQ(loot_ok, loot_get_dirty_info(db, "Unofficial Oblivion Patch.esp", &needsCleaning));
    EXPECT_EQ(loot_needs_cleaning_no, needsCleaning);
}

TEST_F(OblivionAPIOperationsTest, WriteMinimalList) {
    std::string outputFile = (localPath / "minimal.yml").string();
    EXPECT_EQ(loot_error_invalid_args, loot_write_minimal_list(NULL, outputFile.c_str(), false));
    EXPECT_EQ(loot_error_invalid_args, loot_write_minimal_list(db, NULL, false));
    EXPECT_EQ(loot_error_file_write_fail, loot_write_minimal_list(db, "/:?*", false));

    ASSERT_FALSE(boost::filesystem::exists(outputFile));
    EXPECT_EQ(loot_ok, loot_write_minimal_list(db, outputFile.c_str(), false));
    EXPECT_TRUE(boost::filesystem::exists(outputFile));
    EXPECT_EQ(loot_error_file_write_fail, loot_write_minimal_list(db, outputFile.c_str(), false));

    EXPECT_EQ(loot_ok, loot_write_minimal_list(db, outputFile.c_str(), true));
    ASSERT_NO_THROW(boost::filesystem::remove(outputFile));
    EXPECT_EQ(loot_ok, loot_write_minimal_list(db, outputFile.c_str(), true));
    EXPECT_TRUE(boost::filesystem::exists(outputFile));
    ASSERT_NO_THROW(boost::filesystem::remove(outputFile));

    // Check that Bash Tag removals get outputted correctly.
    bool updated;
    ASSERT_EQ(loot_ok, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/oblivion.git", "master", &updated));
    EXPECT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), NULL));
    EXPECT_EQ(loot_ok, loot_write_minimal_list(db, outputFile.c_str(), false));

    loot::ifstream in(outputFile);
    std::string line;
    while (std::getline(in, line)) {
        EXPECT_FALSE(boost::contains(line, "- \"-\""));
    }
    in.close();
    ASSERT_NO_THROW(boost::filesystem::remove(outputFile));
}
#endif
