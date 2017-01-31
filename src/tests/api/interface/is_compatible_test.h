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

#ifndef LOOT_TESTS_API_IS_COMPATIBLE_TEST
#define LOOT_TESTS_API_IS_COMPATIBLE_TEST

#include "loot/api.h"

#include <gtest/gtest.h>

namespace loot {
namespace test {
TEST(IsCompatible, shouldReturnTrueWithEqualMajorAndMinorVersionsAndUnequalPatchVersion) {
  EXPECT_TRUE(IsCompatible(LootVersion::major, LootVersion::minor, LootVersion::patch + 1));
}

TEST(IsCompatible, shouldReturnFalseWithEqualMajorVersionAndUnequalMinorAndPatchVersions) {
  EXPECT_FALSE(IsCompatible(LootVersion::major, LootVersion::minor + 1, LootVersion::patch + 1));
}
}
}

#endif
