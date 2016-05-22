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

#ifndef LOOT_TEST_GUI_LOOT_STATE
#define LOOT_TEST_GUI_LOOT_STATE

#include "backend/app/loot_state.h"

#include <gtest/gtest.h>

namespace loot {
    namespace test {
        class LootStateTest : public ::testing::Test {
        protected:
            LootState lootState;
        };

        TEST_F(LootStateTest, hasUnappliedChangesShouldBeFalseByDefault) {
            EXPECT_FALSE(lootState.hasUnappliedChanges());
        }

        TEST_F(LootStateTest, shouldNotHaveUnappliedChangesIfCounterIsDeccremented) {
            lootState.decrementUnappliedChangeCounter();
            EXPECT_FALSE(lootState.hasUnappliedChanges());
        }

        TEST_F(LootStateTest, shouldHaveUnappliedChangesIfCounterIsIncremented) {
            lootState.incrementUnappliedChangeCounter();
            EXPECT_TRUE(lootState.hasUnappliedChanges());
        }

        TEST_F(LootStateTest, incrementingTheChangeCounterMoreThanItIsDecrementedShouldLeaveUnappliedChanges) {
            lootState.incrementUnappliedChangeCounter();
            lootState.incrementUnappliedChangeCounter();
            lootState.decrementUnappliedChangeCounter();
            EXPECT_TRUE(lootState.hasUnappliedChanges());
        }

        TEST_F(LootStateTest, incrementingTheChangeCounterLessThanItIsDecrementedShouldLeaveNoUnappliedChanges) {
            lootState.incrementUnappliedChangeCounter();
            lootState.decrementUnappliedChangeCounter();
            lootState.decrementUnappliedChangeCounter();
            EXPECT_FALSE(lootState.hasUnappliedChanges());
        }

        TEST_F(LootStateTest, incrementingTheChangeCounterThenDecrementingItEquallyShouldLeaveNoUnappliedChanges) {
            lootState.incrementUnappliedChangeCounter();
            lootState.incrementUnappliedChangeCounter();
            lootState.decrementUnappliedChangeCounter();
            lootState.decrementUnappliedChangeCounter();
            EXPECT_FALSE(lootState.hasUnappliedChanges());
        }
    }
}

#endif
