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

#ifndef LOOT_TESTS_API_INTERNALS_HELPERS_CRC_TEST
#define LOOT_TESTS_API_INTERNALS_HELPERS_CRC_TEST

#include "api/helpers/crc.h"

#include "loot/exception/file_access_error.h"
#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class GetCrc32Test : public CommonGameTestFixture {};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
// Just test with one game because if it works for one it will work for them
// all.
INSTANTIATE_TEST_CASE_P(,
                        GetCrc32Test,
                        ::testing::Values(
                          GameType::tes5));

TEST_P(GetCrc32Test, gettingTheCrcOfAMissingFileShouldThrow) {
  EXPECT_THROW(GetCrc32(dataPath / missingEsp), FileAccessError);
}

TEST_P(GetCrc32Test, gettingTheCrcOfAFileShouldReturnTheCorrectValue) {
  EXPECT_EQ(blankEsmCrc, GetCrc32(dataPath / blankEsm));
}
}
}

#endif
