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

#ifndef LOOT_TEST_BACKEND_LOAD_ORDER_HANDLER
#define LOOT_TEST_BACKEND_LOAD_ORDER_HANDLER

#include "backend/error.h"
#include "backend/game/load_order_handler.h"

#include "tests/fixtures.h"

class LoadOrderHandler : public SkyrimTest {};

TEST_F(LoadOrderHandler, Constructors) {
    loot::LoadOrderHandler * loh = nullptr;
    EXPECT_NO_THROW(loh = new loot::LoadOrderHandler);
    EXPECT_NO_THROW(delete loh);
}

TEST_F(LoadOrderHandler, Init) {
    // Should throw for invalid game settings ID.
    loot::LoadOrderHandler loh;
    loot::GameSettings game(loot::GameSettings::autodetect);
    EXPECT_THROW(loh.Init(game), loot::error);
    EXPECT_THROW(loh.Init(game), loot::error);
    EXPECT_THROW(loh.Init(game, localPath), loot::error);
    EXPECT_THROW(loh.Init(game, localPath), loot::error);

    // Should throw an exception if no game path has been set.
    game = loot::GameSettings(loot::GameSettings::tes5);
    EXPECT_THROW(loh.Init(game), loot::error);
    EXPECT_THROW(loh.Init(game), loot::error);
    EXPECT_THROW(loh.Init(game, localPath), loot::error);
    EXPECT_THROW(loh.Init(game, localPath), loot::error);

#ifndef _WIN32
    // Should throw on Linux if no game local app data path is given.
    game = loot::GameSettings(loot::GameSettings::tes5)
        .SetGamePath(dataPath.parent_path());
    EXPECT_THROW(loh.Init(game), loot::error);
#endif

    game = loot::GameSettings(loot::GameSettings::tes5)
        .SetGamePath(dataPath.parent_path());
    EXPECT_NO_THROW(loh.Init(game, localPath));
}

TEST_F(LoadOrderHandler, GetActivePlugins) {
    loot::LoadOrderHandler loh;
    loot::GameSettings game(loot::GameSettings::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(loh.Init(game, localPath));

    std::unordered_set<std::string> expected({
        "skyrim.esm",
        "blank.esm",
        "blank - different master dependent.esp",
    });

    EXPECT_EQ(expected, loh.GetActivePlugins());
}

TEST_F(LoadOrderHandler, GetLoadOrder) {
    loot::LoadOrderHandler loh;
    loot::GameSettings game(loot::GameSettings::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(loh.Init(game, localPath));

    std::list<std::string> expected({
        "Skyrim.esm",
        "Blank.esm",
        "Blank - Different.esm",
        "Blank - Master Dependent.esm",
        "Blank - Different Master Dependent.esm",
        "Blank.esp",
        "Blank - Different.esp",
        "Blank - Master Dependent.esp",
        "Blank - Different Master Dependent.esp",
        "Blank - Plugin Dependent.esp",
        "Blank - Different Plugin Dependent.esp"
    });

    EXPECT_EQ(expected, loh.GetLoadOrder());
}

TEST_F(LoadOrderHandler, SetLoadOrder) {
    loot::LoadOrderHandler loh;
    loot::GameSettings game(loot::GameSettings::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(loh.Init(game, localPath));

    std::list<std::string> loadOrder({
        "Skyrim.esm",
        "Blank.esm",
        "Blank - Master Dependent.esm",
        "Blank - Different.esm",
        "Blank - Different Master Dependent.esm",
        "Blank - Different.esp",
        "Blank - Different Plugin Dependent.esp",
        "Blank.esp",
        "Blank - Master Dependent.esp",
        "Blank - Different Master Dependent.esp",
        "Blank - Plugin Dependent.esp",
    });

    EXPECT_NO_THROW(loh.SetLoadOrder(loadOrder));

    std::list<std::string> actual;
    loot::ifstream in(localPath / "loadorder.txt");
    while (in) {
        std::string line;
        std::getline(in, line);

        if (!line.empty())
            actual.push_back(line);
    }

    EXPECT_EQ(loadOrder, actual);
}

#endif
