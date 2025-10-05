/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014 WrinklyNinja

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

#ifndef LOOT_TESTS_GUI_STATE_CHANGE_COUNT_TEST
#define LOOT_TESTS_GUI_STATE_CHANGE_COUNT_TEST

#include <gtest/gtest.h>

#include "gui/state/change_count.h"

namespace loot {
namespace test {
TEST(ChangeCount, isNonZeroShouldBeFalseByDefault) {
  ChangeCount counter;
  EXPECT_FALSE(counter.isNonZero());
}

TEST(ChangeCount, isNonZeroShouldBeFalseIfCounterIsDecremented) {
  ChangeCount counter;
  counter.decrement();

  EXPECT_FALSE(counter.isNonZero());
}

TEST(ChangeCount, isNonZeroShouldBeTrueIfCounterIsIncremented) {
  ChangeCount counter;
  counter.increment();

  EXPECT_TRUE(counter.isNonZero());
}

TEST(
    ChangeCount,
    isNonZeroShouldBeTrueIfIncrementingTheChangeCounterMoreThanItIsDecremented) {
  ChangeCount counter;
  counter.increment();
  counter.increment();
  counter.decrement();

  EXPECT_TRUE(counter.isNonZero());
}

TEST(
    ChangeCount,
    isNonZeroShouldBeFalseIfIncrementingTheChangeCounterLessThanItIsDecremented) {
  ChangeCount counter;
  counter.increment();
  counter.decrement();
  counter.decrement();

  EXPECT_FALSE(counter.isNonZero());
}

TEST(
    ChangeCount,
    isNonZeroShouldBeFalseIfIncrementingTheChangeCounterThenDecrementingItEqually) {
  ChangeCount counter;
  counter.increment();
  counter.increment();
  counter.decrement();
  counter.decrement();

  EXPECT_FALSE(counter.isNonZero());
}
}
}

#endif
