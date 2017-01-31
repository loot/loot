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
    along with LOOT. If not, see
    <https://www.gnu.org/licenses/>.
    */

#include <boost/log/core.hpp>

#include "tests/api/internals/game/game_test.h"
#include "tests/api/internals/game/game_cache_test.h"
#include "tests/api/internals/game/load_order_handler_test.h"
#include "tests/api/internals/helpers/crc_test.h"
#include "tests/api/internals/helpers/git_helper_test.h"
#include "tests/api/internals/helpers/language_test.h"
#include "tests/api/internals/helpers/version_test.h"
#include "tests/api/internals/helpers/yaml_set_helpers_test.h"
#include "tests/api/internals/metadata/condition_evaluator_test.h"
#include "tests/api/internals/metadata/condition_grammar_test.h"
#include "tests/api/internals/metadata/conditional_metadata_test.h"
#include "tests/api/internals/metadata/file_test.h"
#include "tests/api/internals/metadata/location_test.h"
#include "tests/api/internals/metadata/message_test.h"
#include "tests/api/internals/metadata/message_content_test.h"
#include "tests/api/internals/metadata/plugin_cleaning_data_test.h"
#include "tests/api/internals/metadata/plugin_metadata_test.h"
#include "tests/api/internals/metadata/priority_test.h"
#include "tests/api/internals/metadata/tag_test.h"
#include "tests/api/internals/plugin/plugin_test.h"
#include "tests/api/internals/plugin/plugin_sorter_test.h"
#include "tests/api/internals/masterlist_test.h"
#include "tests/api/internals/metadata_list_test.h"

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
