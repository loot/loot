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

#ifndef LOOT_TEST_BACKEND_HELPERS
#define LOOT_TEST_BACKEND_HELPERS

#include "backend/helpers/helpers.h"
#include "backend/error.h"
#include "tests/fixtures.h"

class GetCrc32 : public SkyrimTest {};

TEST_F(GetCrc32, MissingFile) {
    EXPECT_THROW(loot::GetCrc32(dataPath / "Blank.missing.esp"), loot::error);
}

TEST_F(GetCrc32, ValidFile) {
    EXPECT_EQ(0x0B5B7B90, loot::GetCrc32(dataPath / "Blank.esp"));
}

TEST(IntToHexString, PositiveNegativeZeroValues) {
    EXPECT_EQ("14", loot::IntToHexString(20));
    EXPECT_EQ("-14", loot::IntToHexString(-20));
    EXPECT_EQ("0", loot::IntToHexString(0));
}

#endif
