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

#ifdef TRAVIS
#pragma message("This is a Travis build, so defining BOOST_NO_CXX11_SCOPED_ENUMS to avoid boost::filesystem::copy_file() linking errors.")
#define BOOST_NO_CXX11_SCOPED_ENUMS
#endif

#include "api/test_api.h"
#include "backend/game/test_game.h"
#include "backend/game/test_game_settings.h"
#include "backend/game/test_load_order_handler.h"
#include "backend/helpers/test_git_helper.h"
#include "backend/helpers/test_helpers.h"
#include "backend/helpers/test_language.h"
#include "backend/helpers/test_version.h"
#include "backend/helpers/test_yaml_set_helpers.h"
#include "backend/metadata/test_condition_grammar.h"
#include "backend/metadata/test_conditional_metadata.h"
#include "backend/metadata/test_file.h"
#include "backend/metadata/test_formid.h"
#include "backend/metadata/test_location.h"
#include "backend/metadata/test_message.h"
#include "backend/metadata/test_message_content.h"
#include "backend/metadata/test_plugin_dirty_info.h"
#include "backend/metadata/test_plugin_metadata.h"
#include "backend/metadata/test_tag.h"
#include "backend/plugin/test_plugin.h"
#include "backend/plugin/test_plugin_loader.h"
#include "backend/test_metadata_list.h"
#include "backend/test_masterlist.h"
#include "backend/test_plugin_sorter.h"

#include <boost/log/core.hpp>

TEST(ModuloOperator, Cpp11Conformance) {
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
