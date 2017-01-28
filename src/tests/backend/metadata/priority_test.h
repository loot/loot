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

#ifndef LOOT_TESTS_BACKEND_METADATA_PRIORITY_TEST
#define LOOT_TESTS_BACKEND_METADATA_PRIORITY_TEST

#include "loot/metadata/priority.h"

#include <gtest/gtest.h>

namespace loot {
namespace test {
TEST(Priority, defaultConstructorShouldInitialiseAnImplicitZeroPriority) {
  Priority priority;

  EXPECT_EQ(0, priority.getValue());
  EXPECT_FALSE(priority.isExplicit());
}

TEST(Priority, valueConstructorShouldInitialseAnExplicitValue) {
  Priority priority(0);

  EXPECT_EQ(0, priority.getValue());
  EXPECT_TRUE(priority.isExplicit());

  priority = Priority(5);

  EXPECT_EQ(5, priority.getValue());
  EXPECT_TRUE(priority.isExplicit());
}

TEST(Priority, valueConstructorTruncatesOutOfRangeValues) {
  EXPECT_EQ(127, Priority(127).getValue());
  EXPECT_EQ(127, Priority(128).getValue());

  EXPECT_EQ(-127, Priority(-127).getValue());
  EXPECT_EQ(-127, Priority(-128).getValue());
}

TEST(Priority, lessThanOperatorShouldCompareValues) {
  Priority priority1(1);
  Priority priority2(2);

  EXPECT_TRUE(priority1 < priority2);
  EXPECT_FALSE(priority2 < priority1);

  EXPECT_FALSE(priority1 < priority1);
}

TEST(Priority, greaterThanOperatorShouldCompareValues) {
  Priority priority1(1);
  Priority priority2(2);

  EXPECT_FALSE(priority1 > priority2);
  EXPECT_TRUE(priority2 > priority1);

  EXPECT_FALSE(priority1 > priority1);
}

TEST(Priority, greaterThanOrEqualToOperatorShouldCompareValues) {
  Priority priority1(1);
  Priority priority2(2);

  EXPECT_FALSE(priority1 >= priority2);
  EXPECT_TRUE(priority2 >= priority1);

  EXPECT_TRUE(priority1 >= priority1);
}

TEST(Priority, equalityOperatorShouldCompareValues) {
  Priority priority1(1);
  Priority priority2(2);

  EXPECT_FALSE(priority1 == priority2);
  EXPECT_FALSE(priority2 == priority1);

  EXPECT_TRUE(priority1 == priority1);
}

TEST(Priority, greaterThanUint8OperatorShouldCompareValue) {
  Priority priority1(1);
  Priority priority2(3);
  uint8_t value = 2;

  EXPECT_FALSE(priority1 > value);
  EXPECT_TRUE(priority2 > value);
}
}
}

#endif
