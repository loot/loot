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

#ifndef LOOT_TEST_BACKEND_LOAD_ORDER_HANDLER
#define LOOT_TEST_BACKEND_LOAD_ORDER_HANDLER

#include "backend/error.h"
#include "backend/game/load_order_handler.h"

#include "tests/base_game_test.h"

namespace loot {
    namespace test {
        class LoadOrderHandlerTest : public BaseGameTest {
        protected:
            LoadOrderHandler loh;
        };

        // Pass an empty first argument, as it's a prefix for the test instantation,
        // but we only have the one so no prefix is necessary.
        INSTANTIATE_TEST_CASE_P(,
                                LoadOrderHandlerTest,
                                ::testing::Values(
                                    GameSettings::tes4,
                                    GameSettings::tes5,
                                    GameSettings::fo3,
                                    GameSettings::fonv,
                                    GameSettings::fo4));

        TEST_P(LoadOrderHandlerTest, initShouldThrowForAnInvalidGameId) {
            GameSettings game(GameSettings::autodetect);
            game.SetGamePath(dataPath.parent_path());

            EXPECT_THROW(loh.Init(game), Error);
            EXPECT_THROW(loh.Init(game), Error);
            EXPECT_THROW(loh.Init(game, localPath), Error);
            EXPECT_THROW(loh.Init(game, localPath), Error);
        }

        TEST_P(LoadOrderHandlerTest, initShouldThrowIfNoGamePathIsSet) {
            GameSettings game(GetParam());

            EXPECT_THROW(loh.Init(game), Error);
            EXPECT_THROW(loh.Init(game), Error);
            EXPECT_THROW(loh.Init(game, localPath), Error);
            EXPECT_THROW(loh.Init(game, localPath), Error);
        }

#ifndef _WIN32
        TEST_P(LoadOrderHandlerTest, initShouldThrowOnLinuxIfNoLocalPathIsSet) {
            GameSettings game(GetParam());
            game.SetGamePath(dataPath.parent_path());

            EXPECT_THROW(loh.Init(game), Error);
        }
#endif

        TEST_P(LoadOrderHandlerTest, initShouldNotThrowIfAValidGameIdAndGamePathAndLocalPathAreSet) {
            GameSettings game(GetParam());
            game.SetGamePath(dataPath.parent_path());

            EXPECT_NO_THROW(loh.Init(game, localPath));
        }

        TEST_P(LoadOrderHandlerTest, isPluginActiveShouldThrowIfTheHandlerHasNotBeenInitialised) {
            EXPECT_THROW(loh.IsPluginActive(masterFile), Error);
        }

        TEST_P(LoadOrderHandlerTest, isPluginActiveShouldReturnCorrectPluginStatesAfterInitialisation) {
            GameSettings game(GetParam());
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(loh.Init(game, localPath));

            EXPECT_TRUE(loh.IsPluginActive(masterFile));
            EXPECT_TRUE(loh.IsPluginActive(blankEsm));
            EXPECT_FALSE(loh.IsPluginActive(blankEsp));
        }

        TEST_P(LoadOrderHandlerTest, getLoadOrderShouldThrowIfTheHandlerHasNotBeenInitialised) {
            EXPECT_THROW(loh.GetLoadOrder(), Error);
        }

        TEST_P(LoadOrderHandlerTest, getLoadOrderShouldReturnTheCurrentLoadOrder) {
            GameSettings game(GetParam());
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(loh.Init(game, localPath));

            ASSERT_EQ(getLoadOrder(), loh.GetLoadOrder());
        }

        TEST_P(LoadOrderHandlerTest, setLoadOrderShouldThrowIfTheHandlerHasNotBeenInitialised) {
            std::list<std::string> loadOrder({
                masterFile,
                blankEsm,
                blankMasterDependentEsm,
                blankDifferentEsm,
                blankDifferentMasterDependentEsm,
                blankDifferentEsp,
                blankDifferentPluginDependentEsp,
                blankEsp,
                blankMasterDependentEsp,
                blankDifferentMasterDependentEsp,
                blankPluginDependentEsp,
            });

            EXPECT_THROW(loh.SetLoadOrder(std::list<std::string>()), Error);
        }

        TEST_P(LoadOrderHandlerTest, setLoadOrderShouldSetTheLoadOrder) {
            GameSettings game(GetParam());
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(loh.Init(game, localPath));

            std::list<std::string> loadOrder({
                masterFile,
                blankEsm,
                blankMasterDependentEsm,
                blankDifferentEsm,
                blankDifferentMasterDependentEsm,
                blankDifferentEsp,
                blankDifferentPluginDependentEsp,
                blankEsp,
                blankMasterDependentEsp,
                blankDifferentMasterDependentEsp,
                blankPluginDependentEsp,
            });
            EXPECT_NO_THROW(loh.SetLoadOrder(loadOrder));

            if (GetParam() == GameSettings::fo4)
                loadOrder.erase(begin(loadOrder));

            EXPECT_EQ(loadOrder, getLoadOrder());
        }
    }
}

#endif
