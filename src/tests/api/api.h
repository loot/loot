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

    EXPECT_EQ(loot_ok, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/oblivion.git", "master", &updated));
    EXPECT_TRUE(updated);
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
    EXPECT_EQ(loot_error_invalid_args, loot_eval_lists(db, (unsigned int)-1));

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

TEST_F(OblivionAPIOperationsTest, SortPlugins) {
    char ** sortedPlugins;
    size_t numPlugins;
    EXPECT_EQ(loot_error_invalid_args, loot_sort_plugins(NULL, &sortedPlugins, &numPlugins));
    EXPECT_EQ(loot_error_invalid_args, loot_sort_plugins(db, NULL, &numPlugins));
    EXPECT_EQ(loot_error_invalid_args, loot_sort_plugins(db, &sortedPlugins, NULL));

    EXPECT_EQ(loot_ok, loot_sort_plugins(db, &sortedPlugins, &numPlugins));

    // Expected order was obtained from running the API function once.
    std::list<std::string> expectedOrder = {
        "Blank.esm",
        "Blank - Master Dependent.esm",
        "Oblivion.esm",
        "Blank - Different.esm",
        "Blank - Different Master Dependent.esm",
        "Blank - Master Dependent.esp",
        "Blank.esp",
        "Blank - Plugin Dependent.esp",
        "Blank - Different Master Dependent.esp",
        "Blank - Different.esp",
        "Blank - Different Plugin Dependent.esp"
    };
    std::list<std::string> actualOrder;
    for (size_t i = 0; i < numPlugins; ++i) {
        actualOrder.push_back(sortedPlugins[i]);
    }
    EXPECT_EQ(11, numPlugins);
    EXPECT_EQ(expectedOrder, actualOrder);
}

TEST_F(OblivionAPIOperationsTest, GetPluginMessages) {
    loot_message * messages;
    size_t numMessages;
    EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_messages(NULL, "EnhancedWeatherSIOnly.esm", &messages, &numMessages));
    EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_messages(db, NULL, &messages, &numMessages));
    EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_messages(db, "EnhancedWeatherSIOnly.esm", NULL, &numMessages));
    EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_messages(db, "EnhancedWeatherSIOnly.esm", &messages, NULL));

    // Fetch and load the metadata.
    bool updated;
    ASSERT_EQ(loot_ok, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/oblivion.git", "master", &updated));
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
    bool updated;
    ASSERT_EQ(loot_ok, loot_update_masterlist(db, masterlistPath.string().c_str(), "https://github.com/loot/oblivion.git", "master", &updated));
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
#endif
