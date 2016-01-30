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

#ifndef LOOT_TEST_BACKEND_MASTERLIST
#define LOOT_TEST_BACKEND_MASTERLIST

#include "backend/masterlist.h"
#include "tests/fixtures.h"

class Masterlist : public SkyrimTest {};

TEST_F(Masterlist, Update_Game) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    game.SetRepoURL("https://github.com/loot/testing-metadata.git");
    game.SetRepoBranch("master");
    ASSERT_NO_THROW(game.Init(false, localPath));

    // This may fail on Windows if a 'real' LOOT install is also present.
    loot::Masterlist masterlist;
    EXPECT_TRUE(masterlist.Update(game));
    EXPECT_TRUE(boost::filesystem::exists(game.MasterlistPath()));

    EXPECT_FALSE(masterlist.Update(game));
    EXPECT_TRUE(boost::filesystem::exists(game.MasterlistPath()));
}

TEST_F(Masterlist, Update_NonGame_InvalidPath) {
    loot::Masterlist masterlist;
    EXPECT_ANY_THROW(masterlist.Update(";//\?",
        "https://github.com/loot/testing-metadata.git",
        "master"));
}

TEST_F(Masterlist, Update_NonGame_EmptyPath) {
    loot::Masterlist masterlist;
    EXPECT_ANY_THROW(masterlist.Update("",
        "https://github.com/loot/testing-metadata.git",
        "master"));
}

TEST_F(Masterlist, Update_NonGame_InvalidBranch) {
    loot::Masterlist masterlist;
    EXPECT_ANY_THROW(masterlist.Update(masterlistPath,
        "https://github.com/loot/testing-metadata.git",
        "missing-branch"));
}

TEST_F(Masterlist, Update_NonGame_EmptyBranch) {
    loot::Masterlist masterlist;
    EXPECT_ANY_THROW(masterlist.Update(masterlistPath,
        "https://github.com/loot/testing-metadata.git",
        ""));
}

TEST_F(Masterlist, Update_NonGame_InvalidUrl) {
    loot::Masterlist masterlist;
    EXPECT_ANY_THROW(masterlist.Update(masterlistPath,
        "https://github.com/loot/does-not-exist.git",
        "master"));
}

TEST_F(Masterlist, Update_NonGame_EmptyUrl) {
    loot::Masterlist masterlist;
    EXPECT_ANY_THROW(masterlist.Update(masterlistPath,
        "",
        "master"));
}

TEST_F(Masterlist, Update_NonGame) {
    loot::Masterlist masterlist;
    EXPECT_TRUE(masterlist.Update(masterlistPath,
        "https://github.com/loot/testing-metadata.git",
        "master"));

    EXPECT_FALSE(masterlist.Update(masterlistPath,
        "https://github.com/loot/testing-metadata.git",
        "master"));
}

TEST_F(Masterlist, GetInfo_NoMasterlist) {
    loot::Masterlist masterlist;
    EXPECT_ANY_THROW(masterlist.GetInfo(masterlistPath, false));
}

TEST_F(Masterlist, GetInfo_NoRepository) {
    ASSERT_NO_THROW(boost::filesystem::copy("./testing-metadata/masterlist.yaml", masterlistPath));

    loot::Masterlist masterlist;
    EXPECT_ANY_THROW(masterlist.GetInfo(masterlistPath, false));
}

TEST_F(Masterlist, GetInfo_LongID) {
    loot::Masterlist masterlist;
    ASSERT_TRUE(masterlist.Update(masterlistPath,
        "https://github.com/loot/testing-metadata.git",
        "master"));

    loot::Masterlist::Info info = masterlist.GetInfo(masterlistPath, false);
    EXPECT_EQ(40, info.revision.length());
    EXPECT_EQ(10, info.date.length());
}

TEST_F(Masterlist, GetInfo_ShortID) {
    loot::Masterlist masterlist;
    ASSERT_TRUE(masterlist.Update(masterlistPath,
        "https://github.com/loot/testing-metadata.git",
        "master"));

    loot::Masterlist::Info info = masterlist.GetInfo(masterlistPath, true);
    EXPECT_GE((unsigned)40, info.revision.length());
    EXPECT_LE((unsigned)7, info.revision.length());
    EXPECT_EQ(10, info.date.length());
}

TEST_F(Masterlist, GetInfo_Edited) {
    loot::Masterlist masterlist;
    ASSERT_TRUE(masterlist.Update(masterlistPath,
        "https://github.com/loot/testing-metadata.git",
        "master"));
    boost::filesystem::ofstream out(masterlistPath);
    out.close();

    loot::Masterlist::Info info = masterlist.GetInfo(masterlistPath, false);
    EXPECT_EQ(49, info.revision.length());
    EXPECT_EQ(" (edited)", info.revision.substr(40));
    EXPECT_EQ(19, info.date.length());
    EXPECT_EQ(" (edited)", info.date.substr(10));
}

#endif
