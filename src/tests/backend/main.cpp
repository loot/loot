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

#include "tests/api/loot_db_test.h"

#include "app/loot_paths_test.h"
#include "app/loot_settings_test.h"
#include "app/loot_state_test.h"
#include "game/game_test.h"
#include "game/game_cache_test.h"
#include "game/game_settings_test.h"
#include "game/load_order_handler_test.h"
#include "helpers/git_helper_test.h"
#include "helpers/helpers_test.h"
#include "helpers/language_test.h"
#include "helpers/version_test.h"
#include "helpers/yaml_set_helpers_test.h"
#include "metadata/condition_grammar_test.h"
#include "metadata/conditional_metadata_test.h"
#include "metadata/file_test.h"
#include "metadata/location_test.h"
#include "metadata/message_test.h"
#include "metadata/message_content_test.h"
#include "metadata/plugin_dirty_info_test.h"
#include "metadata/plugin_metadata_test.h"
#include "metadata/tag_test.h"
#include "plugin/plugin_test.h"
#include "plugin/plugin_sorter_test.h"
#include "masterlist_test.h"
#include "metadata_list_test.h"

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
