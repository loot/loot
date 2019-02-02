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

#ifndef LOOT_TESTS_GUI_STATE_UNAPPLIED_CHANGE_COUNTER_TEST
#define LOOT_TESTS_GUI_STATE_UNAPPLIED_CHANGE_COUNTER_TEST

#include "gui/state/unapplied_change_counter.h"

#include <gtest/gtest.h>

namespace loot {
namespace test {
TEST(UnappliedChangeCounter, hasUnappliedChangesShouldBeFalseByDefault) {
  UnappliedChangeCounter counter;
  EXPECT_FALSE(counter.HasUnappliedChanges());
}

TEST(UnappliedChangeCounter, shouldNotHaveUnappliedChangesIfCounterIsDecremented) {
  UnappliedChangeCounter counter;
  counter.DecrementUnappliedChangeCounter();

  EXPECT_FALSE(counter.HasUnappliedChanges());
}

TEST(UnappliedChangeCounter, shouldHaveUnappliedChangesIfCounterIsIncremented) {
  UnappliedChangeCounter counter;
  counter.IncrementUnappliedChangeCounter();

  EXPECT_TRUE(counter.HasUnappliedChanges());
}

TEST(UnappliedChangeCounter,
    incrementingTheChangeCounterMoreThanItIsDecrementedShouldLeaveUnappliedChanges) {
  UnappliedChangeCounter counter;
  counter.IncrementUnappliedChangeCounter();
  counter.IncrementUnappliedChangeCounter();
  counter.DecrementUnappliedChangeCounter();

  EXPECT_TRUE(counter.HasUnappliedChanges());
}

TEST(UnappliedChangeCounter,
    incrementingTheChangeCounterLessThanItIsDecrementedShouldLeaveNoUnappliedChanges) {
  UnappliedChangeCounter counter;
  counter.IncrementUnappliedChangeCounter();
  counter.DecrementUnappliedChangeCounter();
  counter.DecrementUnappliedChangeCounter();

  EXPECT_FALSE(counter.HasUnappliedChanges());
}

TEST(UnappliedChangeCounter,
    incrementingTheChangeCounterThenDecrementingItEquallyShouldLeaveNoUnappliedChanges) {
  UnappliedChangeCounter counter;
  counter.IncrementUnappliedChangeCounter();
  counter.IncrementUnappliedChangeCounter();
  counter.DecrementUnappliedChangeCounter();
  counter.DecrementUnappliedChangeCounter();

  EXPECT_FALSE(counter.HasUnappliedChanges());
}
}
}

#endif
