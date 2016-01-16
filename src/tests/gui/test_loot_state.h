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

#ifndef LOOT_TEST_GUI_LOOT_STATE
#define LOOT_TEST_GUI_LOOT_STATE

#include "gui/loot_state.h"

#include "tests/fixtures.h"

namespace loot {
    namespace test {
        class LootState : public ::testing::Test {
        protected:
            loot::LootState lootState;
        };

        TEST_F(LootState, hasUnappliedChangesShouldBeFalseByDefault) {
            EXPECT_FALSE(lootState.hasUnappliedChanges());
        }

        TEST_F(LootState, shouldNotHaveUnappliedChangesIfCounterIsDeccremented) {
            lootState.decrementUnappliedChangeCounter();
            EXPECT_FALSE(lootState.hasUnappliedChanges());
        }

        TEST_F(LootState, shouldHaveUnappliedChangesIfCounterIsIncremented) {
            lootState.incrementUnappliedChangeCounter();
            EXPECT_TRUE(lootState.hasUnappliedChanges());
        }

        TEST_F(LootState, incrementingTheChangeCounterMoreThanItIsDecrementedShouldLeaveUnappliedChanges) {
            lootState.incrementUnappliedChangeCounter();
            lootState.incrementUnappliedChangeCounter();
            lootState.decrementUnappliedChangeCounter();
            EXPECT_TRUE(lootState.hasUnappliedChanges());
        }

        TEST_F(LootState, incrementingTheChangeCounterLessThanItIsDecrementedShouldLeaveNoUnappliedChanges) {
            lootState.incrementUnappliedChangeCounter();
            lootState.decrementUnappliedChangeCounter();
            lootState.decrementUnappliedChangeCounter();
            EXPECT_FALSE(lootState.hasUnappliedChanges());
        }

        TEST_F(LootState, incrementingTheChangeCounterThenDecrementingItEquallyShouldLeaveNoUnappliedChanges) {
            lootState.incrementUnappliedChangeCounter();
            lootState.incrementUnappliedChangeCounter();
            lootState.decrementUnappliedChangeCounter();
            lootState.decrementUnappliedChangeCounter();
            EXPECT_FALSE(lootState.hasUnappliedChanges());
        }
    }
}

#endif
