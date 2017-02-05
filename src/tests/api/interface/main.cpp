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

#include <gtest/gtest.h>

#include "tests/api/interface/create_game_handle_test.h"
#include "tests/api/interface/database_interface_test.h"
#include "tests/api/interface/game_interface_test.h"
#include "tests/api/interface/is_compatible_test.h"

#include <boost/log/core.hpp>
#include <boost/locale.hpp>

int main(int argc, char **argv) {
    //Set the locale to get encoding conversions working correctly.
  std::locale::global(boost::locale::generator().generate(""));
  boost::filesystem::path::imbue(std::locale());

  //Disable logging or else stdout will get overrun.
  boost::log::core::get()->set_logging_enabled(false);
  loot::SetLoggingVerbosity(loot::LogVerbosity::off);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
