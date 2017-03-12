/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2017    WrinklyNinja

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

#ifndef LOOT_TESTS_GUI_STATE_LOOT_STATE_TEST
#define LOOT_TESTS_GUI_STATE_LOOT_STATE_TEST

#include "gui/state/loot_state.h"

#include <gtest/gtest.h>

namespace loot {
namespace test {
class LootStateTest : public ::testing::Test {
protected:
  LootState lootState_;
};

TEST_F(LootStateTest, hasUnappliedChangesShouldBeFalseByDefault) {
  EXPECT_FALSE(lootState_.hasUnappliedChanges());
}

TEST_F(LootStateTest, shouldNotHaveUnappliedChangesIfCounterIsDeccremented) {
  lootState_.decrementUnappliedChangeCounter();
  EXPECT_FALSE(lootState_.hasUnappliedChanges());
}

TEST_F(LootStateTest, shouldHaveUnappliedChangesIfCounterIsIncremented) {
  lootState_.incrementUnappliedChangeCounter();
  EXPECT_TRUE(lootState_.hasUnappliedChanges());
}

TEST_F(LootStateTest, incrementingTheChangeCounterMoreThanItIsDecrementedShouldLeaveUnappliedChanges) {
  lootState_.incrementUnappliedChangeCounter();
  lootState_.incrementUnappliedChangeCounter();
  lootState_.decrementUnappliedChangeCounter();
  EXPECT_TRUE(lootState_.hasUnappliedChanges());
}

TEST_F(LootStateTest, incrementingTheChangeCounterLessThanItIsDecrementedShouldLeaveNoUnappliedChanges) {
  lootState_.incrementUnappliedChangeCounter();
  lootState_.decrementUnappliedChangeCounter();
  lootState_.decrementUnappliedChangeCounter();
  EXPECT_FALSE(lootState_.hasUnappliedChanges());
}

TEST_F(LootStateTest, incrementingTheChangeCounterThenDecrementingItEquallyShouldLeaveNoUnappliedChanges) {
  lootState_.incrementUnappliedChangeCounter();
  lootState_.incrementUnappliedChangeCounter();
  lootState_.decrementUnappliedChangeCounter();
  lootState_.decrementUnappliedChangeCounter();
  EXPECT_FALSE(lootState_.hasUnappliedChanges());
}
}
}

#endif
