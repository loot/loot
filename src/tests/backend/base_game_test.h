/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2013-2016    WrinklyNinja

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

#ifndef LOOT_TEST_BASE_GAME_TEST
#define LOOT_TEST_BASE_GAME_TEST

#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>

#include <map>
#include <unordered_set>

#include "tests/common_game_test_fixture.h"

namespace loot {
    namespace test {
        class BaseGameTest : 
            public ::testing::TestWithParam<GameType>,
            public CommonGameTestFixture {
        protected:
            BaseGameTest() :
                CommonGameTestFixture(static_cast<unsigned int>(GetParam())) {}

            inline virtual void SetUp() {
                setUp();
            }

            inline virtual void TearDown() {
                tearDown();
            }
        };
    }
}

#endif
