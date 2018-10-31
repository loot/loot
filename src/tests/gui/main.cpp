/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2014-2018    WrinklyNinja

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

#include <boost/locale.hpp>

#include "tests/gui/cef/query/json_test.h"
#include "tests/gui/state/game/game_settings_test.h"
#include "tests/gui/state/game/game_test.h"
#include "tests/gui/state/game/helpers_test.h"
#include "tests/gui/state/loot_paths_test.h"
#include "tests/gui/state/loot_settings_test.h"
#include "tests/gui/state/loot_state_test.h"

int main(int argc, char **argv) {
  // Set the locale to get encoding conversions working correctly.
  std::locale::global(boost::locale::generator().generate(""));
  loot::InitialiseLocale("");

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
