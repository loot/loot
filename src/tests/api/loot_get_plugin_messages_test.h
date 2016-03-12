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

#ifndef LOOT_TEST_LOOT_GET_PLUGIN_MESSAGES
#define LOOT_TEST_LOOT_GET_PLUGIN_MESSAGES

#include "../include/loot/api.h"
#include "api_game_operations_test.h"

namespace loot {
    namespace test {
        class loot_get_plugin_messages_test : public ApiGameOperationsTest {
        protected:
            loot_get_plugin_messages_test() :
                messages(nullptr),
                numMessages(0) {}

            const loot_message * messages;
            size_t numMessages;
        };

        // Pass an empty first argument, as it's a prefix for the test instantation,
        // but we only have the one so no prefix is necessary.
        INSTANTIATE_TEST_CASE_P(,
                                loot_get_plugin_messages_test,
                                ::testing::Values(
                                    loot_game_tes4,
                                    loot_game_tes5,
                                    loot_game_fo3,
                                    loot_game_fonv,
                                    loot_game_fo4));

        TEST_P(loot_get_plugin_messages_test, shouldReturnAnInvalidArgsErrorIfAnyOfTheArgumentsAreNull) {
            EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_messages(NULL, blankEsp.c_str(), &messages, &numMessages));
            EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_messages(db, NULL, &messages, &numMessages));
            EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_messages(db, blankEsp.c_str(), NULL, &numMessages));
            EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_messages(db, blankEsp.c_str(), &messages, NULL));
        }

        TEST_P(loot_get_plugin_messages_test, shouldReturnOkAndOutputANullArrayIfAPluginWithNoMessagesIsQueried) {
            EXPECT_EQ(loot_ok, loot_get_plugin_messages(db, blankEsp.c_str(), &messages, &numMessages));
            EXPECT_EQ(0, numMessages);
            EXPECT_EQ(NULL, messages);
        }

        TEST_P(loot_get_plugin_messages_test, shouldReturnOkAndOutputANoteIfAPluginWithANoteMessageIsQueried) {
            ASSERT_NO_THROW(generateMasterlist());
            ASSERT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), NULL));

            EXPECT_EQ(loot_ok, loot_get_plugin_messages(db, blankEsm.c_str(), &messages, &numMessages));
            ASSERT_EQ(1, numMessages);
            EXPECT_EQ(loot_message_say, messages[0].type);
            EXPECT_STREQ(noteMessage.c_str(), messages[0].message);
        }

        TEST_P(loot_get_plugin_messages_test, shouldReturnOkAndOutputAWarningIfAPluginWithAWarningMessageIsQueried) {
            ASSERT_NO_THROW(generateMasterlist());
            ASSERT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), NULL));

            EXPECT_EQ(loot_ok, loot_get_plugin_messages(db, blankDifferentEsm.c_str(), &messages, &numMessages));
            ASSERT_EQ(1, numMessages);
            EXPECT_EQ(loot_message_warn, messages[0].type);
            EXPECT_STREQ(warningMessage.c_str(), messages[0].message);
        }

        TEST_P(loot_get_plugin_messages_test, shouldReturnOkAndOutputAnErrorIfAPluginWithAnErrorMessageIsQueried) {
            ASSERT_NO_THROW(generateMasterlist());
            ASSERT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), NULL));

            EXPECT_EQ(loot_ok, loot_get_plugin_messages(db, blankDifferentEsp.c_str(), &messages, &numMessages));
            ASSERT_EQ(1, numMessages);
            EXPECT_EQ(loot_message_error, messages[0].type);
            EXPECT_STREQ(errorMessage.c_str(), messages[0].message);
        }

        TEST_P(loot_get_plugin_messages_test, shouldReturnOkAndOutputMultipleMessagesIfAPluginWithMultipleMessagesIsQueried) {
            ASSERT_NO_THROW(generateMasterlist());
            ASSERT_EQ(loot_ok, loot_load_lists(db, masterlistPath.string().c_str(), NULL));

            EXPECT_EQ(loot_ok, loot_get_plugin_messages(db, blankDifferentMasterDependentEsp.c_str(), &messages, &numMessages));
            ASSERT_EQ(3, numMessages);
            EXPECT_EQ(loot_message_say, messages[0].type);
            EXPECT_STREQ(noteMessage.c_str(), messages[0].message);
            EXPECT_EQ(loot_message_warn, messages[1].type);
            EXPECT_STREQ(warningMessage.c_str(), messages[1].message);
            EXPECT_EQ(loot_message_error, messages[2].type);
            EXPECT_STREQ(errorMessage.c_str(), messages[2].message);
        }
    }
}

#endif
