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

#ifdef TRAVIS
#pragma message("This is a Travis build, so defining BOOST_NO_CXX11_SCOPED_ENUMS to avoid boost::filesystem::copy_file() linking errors.")
#define BOOST_NO_CXX11_SCOPED_ENUMS
#endif

#include "api/loot_apply_load_order_test.h"
#include "api/loot_create_db_test.h"
#include "api/loot_db_test.h"
#include "api/loot_eval_lists_test.h"
#include "api/loot_get_dirty_info_test.h"
#include "api/loot_get_masterlist_revision_test.h"
#include "api/loot_get_plugin_messages_test.h"
#include "api/loot_get_plugin_tags_test.h"
#include "api/loot_get_tag_map_test.h"
#include "api/loot_load_lists_test.h"
#include "api/loot_sort_plugins_test.h"
#include "api/loot_update_masterlist_test.h"
#include "api/loot_write_minimal_list_test.h"
#include "api/test_api.h"
#include "backend/game/game_test.h"
#include "backend/game/game_cache_test.h"
#include "backend/game/game_settings_test.h"
#include "backend/game/load_order_handler_test.h"
#include "backend/helpers/git_helper_test.h"
#include "backend/helpers/helpers_test.h"
#include "backend/helpers/language_test.h"
#include "backend/helpers/version_test.h"
#include "backend/helpers/yaml_set_helpers_test.h"
#include "backend/metadata/condition_grammar_test.h"
#include "backend/metadata/conditional_metadata_test.h"
#include "backend/metadata/file_test.h"
#include "backend/metadata/location_test.h"
#include "backend/metadata/message_test.h"
#include "backend/metadata/message_content_test.h"
#include "backend/metadata/plugin_dirty_info_test.h"
#include "backend/metadata/test_plugin_metadata.h"
#include "backend/metadata/test_tag.h"
#include "backend/plugin/test_plugin.h"
#include "backend/test_metadata_list.h"
#include "backend/test_masterlist.h"
#include "backend/test_plugin_sorter.h"
#include "gui/loot_settings_test.h"
#include "gui/loot_state_test.h"

#include <boost/log/core.hpp>

TEST(ModuloOperator, shouldConformToTheCpp11Standard) {
    // C++11 defines the modulo operator more strongly
    // (only x % 0 is left undefined), whereas C++03
    // only defined the operator for positive first operand.
    // Test that the modulo operator has been implemented
    // according to C++11.

    EXPECT_EQ(0, 20 % 5);
    EXPECT_EQ(0, 20 % -5);
    EXPECT_EQ(0, -20 % 5);
    EXPECT_EQ(0, -20 % -5);

    EXPECT_EQ(2, 9 % 7);
    EXPECT_EQ(2, 9 % -7);
    EXPECT_EQ(-2, -9 % 7);
    EXPECT_EQ(-2, -9 % -7);
}

int main(int argc, char **argv) {
    //Set the locale to get encoding conversions working correctly.
    std::locale::global(boost::locale::generator().generate(""));
    boost::filesystem::path::imbue(std::locale());

    //Disable logging or else stdout will get overrun.
    boost::log::core::get()->set_logging_enabled(false);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
