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

#ifndef LOOT_TESTS_API_LOOT_GET_PLUGIN_MESSAGES_TEST
#define LOOT_TESTS_API_LOOT_GET_PLUGIN_MESSAGES_TEST

#include "loot/api.h"

#include "tests/api/api_game_operations_test.h"

namespace loot {
namespace test {
class loot_get_plugin_messages_test : public ApiGameOperationsTest {
protected:
  loot_get_plugin_messages_test() :
    messages_(nullptr),
    numMessages_(0) {}

  const loot_message * messages_;
  size_t numMessages_;
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
  EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_messages(NULL, blankEsp.c_str(), &messages_, &numMessages_));
  EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_messages(db_, NULL, &messages_, &numMessages_));
  EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_messages(db_, blankEsp.c_str(), NULL, &numMessages_));
  EXPECT_EQ(loot_error_invalid_args, loot_get_plugin_messages(db_, blankEsp.c_str(), &messages_, NULL));
}

TEST_P(loot_get_plugin_messages_test, shouldReturnOkAndOutputANullArrayIfAPluginWithNoMessagesIsQueried) {
  EXPECT_EQ(loot_ok, loot_get_plugin_messages(db_, blankEsp.c_str(), &messages_, &numMessages_));
  EXPECT_EQ(0, numMessages_);
  EXPECT_EQ(NULL, messages_);
}

TEST_P(loot_get_plugin_messages_test, shouldReturnOkAndOutputANoteIfAPluginWithANoteMessageIsQueried) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_EQ(loot_ok, loot_load_lists(db_, masterlistPath.string().c_str(), NULL));

  EXPECT_EQ(loot_ok, loot_get_plugin_messages(db_, blankEsm.c_str(), &messages_, &numMessages_));
  ASSERT_EQ(1, numMessages_);
  EXPECT_EQ(loot_message_say, messages_[0].type);
  EXPECT_STREQ(noteMessage.c_str(), messages_[0].message);
}

TEST_P(loot_get_plugin_messages_test, shouldReturnOkAndOutputAWarningIfAPluginWithAWarningMessageIsQueried) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_EQ(loot_ok, loot_load_lists(db_, masterlistPath.string().c_str(), NULL));

  EXPECT_EQ(loot_ok, loot_get_plugin_messages(db_, blankDifferentEsm.c_str(), &messages_, &numMessages_));
  ASSERT_EQ(1, numMessages_);
  EXPECT_EQ(loot_message_warn, messages_[0].type);
  EXPECT_STREQ(warningMessage.c_str(), messages_[0].message);
}

TEST_P(loot_get_plugin_messages_test, shouldReturnOkAndOutputAnErrorIfAPluginWithAnErrorMessageIsQueried) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_EQ(loot_ok, loot_load_lists(db_, masterlistPath.string().c_str(), NULL));

  EXPECT_EQ(loot_ok, loot_get_plugin_messages(db_, blankDifferentEsp.c_str(), &messages_, &numMessages_));
  ASSERT_EQ(1, numMessages_);
  EXPECT_EQ(loot_message_error, messages_[0].type);
  EXPECT_STREQ(errorMessage.c_str(), messages_[0].message);
}

TEST_P(loot_get_plugin_messages_test, shouldReturnOkAndOutputMultipleMessagesIfAPluginWithMultipleMessagesIsQueried) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_EQ(loot_ok, loot_load_lists(db_, masterlistPath.string().c_str(), NULL));

  EXPECT_EQ(loot_ok, loot_get_plugin_messages(db_, blankDifferentMasterDependentEsp.c_str(), &messages_, &numMessages_));
  ASSERT_EQ(3, numMessages_);
  EXPECT_EQ(loot_message_say, messages_[0].type);
  EXPECT_STREQ(noteMessage.c_str(), messages_[0].message);
  EXPECT_EQ(loot_message_warn, messages_[1].type);
  EXPECT_STREQ(warningMessage.c_str(), messages_[1].message);
  EXPECT_EQ(loot_message_error, messages_[2].type);
  EXPECT_STREQ(errorMessage.c_str(), messages_[2].message);
}
}
}

#endif
