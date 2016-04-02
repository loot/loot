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

#ifndef LOOT_TEST_LOOT_APPLY_LOAD_ORDER
#define LOOT_TEST_LOOT_APPLY_LOAD_ORDER

#include "../include/loot/api.h"
#include "api_game_operations_test.h"

namespace loot {
    namespace test {
        class loot_apply_load_order_test : public ApiGameOperationsTest {};

        // Pass an empty first argument, as it's a prefix for the test instantation,
        // but we only have the one so no prefix is necessary.
        INSTANTIATE_TEST_CASE_P(,
                                loot_apply_load_order_test,
                                ::testing::Values(
                                    loot_game_tes4,
                                    loot_game_tes5,
                                    loot_game_fo3,
                                    loot_game_fonv,
                                    loot_game_fo4));

        TEST_P(loot_apply_load_order_test, shouldReturnAnInvalidArgsIfTheDbOrLoadOrderPointersAreNull) {
            const char * loadOrder[1] = {
                masterFile.c_str(),
            };
            size_t numPlugins = 0;

            EXPECT_EQ(loot_error_invalid_args, loot_apply_load_order(NULL, loadOrder, numPlugins));
            EXPECT_EQ(loot_error_invalid_args, loot_apply_load_order(db, NULL, numPlugins));
        }

        TEST_P(loot_apply_load_order_test, shouldReturnOkIfLoadOrderGivenIsNotEmpty) {
            const char * loadOrder[11] = {
                masterFile.c_str(),
                blankEsm.c_str(),
                blankMasterDependentEsm.c_str(),
                blankDifferentEsm.c_str(),
                blankDifferentMasterDependentEsm.c_str(),
                blankMasterDependentEsp.c_str(),
                blankDifferentMasterDependentEsp.c_str(),
                blankEsp.c_str(),
                blankPluginDependentEsp.c_str(),
                blankDifferentEsp.c_str(),
                blankDifferentPluginDependentEsp.c_str(),
            };
            size_t numPlugins = 11;

            EXPECT_EQ(loot_ok, loot_apply_load_order(db, loadOrder, numPlugins));
        }
    }
}

#endif
