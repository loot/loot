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

namespace loot {
    namespace test {
        class MasterlistTest : public SkyrimTest {
        protected:
#ifndef _WIN32
            void SetUp() {
                SkyrimTest::SetUp();

                ASSERT_NO_THROW(boost::filesystem::create_directories(g_path_local / "Skyrim"));
            }

            void TearDown() {
                SkyrimTest::TearDown();

                ASSERT_NO_THROW(boost::filesystem::remove_all(g_path_local));
            }
#endif
        };

        TEST_F(MasterlistTest, Update_Game) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            game.SetRepoURL("https://github.com/loot/testing-metadata.git");
            game.SetRepoBranch("master");
            ASSERT_NO_THROW(game.Init(false, localPath));

            // This may fail on Windows if a 'real' LOOT install is also present.
            Masterlist masterlist;
            EXPECT_TRUE(masterlist.Update(game));
            EXPECT_TRUE(boost::filesystem::exists(game.MasterlistPath()));

            EXPECT_FALSE(masterlist.Update(game));
            EXPECT_TRUE(boost::filesystem::exists(game.MasterlistPath()));
        }

        TEST_F(MasterlistTest, Update_NonGame_InvalidPath) {
            Masterlist masterlist;
            EXPECT_ANY_THROW(masterlist.Update(";//\?",
                                               "https://github.com/loot/testing-metadata.git",
                                               "master"));
        }

        TEST_F(MasterlistTest, Update_NonGame_EmptyPath) {
            Masterlist masterlist;
            EXPECT_ANY_THROW(masterlist.Update("",
                                               "https://github.com/loot/testing-metadata.git",
                                               "master"));
        }

        TEST_F(MasterlistTest, Update_NonGame_InvalidBranch) {
            Masterlist masterlist;
            EXPECT_ANY_THROW(masterlist.Update(masterlistPath,
                                               "https://github.com/loot/testing-metadata.git",
                                               "missing-branch"));
        }

        TEST_F(MasterlistTest, Update_NonGame_EmptyBranch) {
            Masterlist masterlist;
            EXPECT_ANY_THROW(masterlist.Update(masterlistPath,
                                               "https://github.com/loot/testing-metadata.git",
                                               ""));
        }

        TEST_F(MasterlistTest, Update_NonGame_InvalidUrl) {
            Masterlist masterlist;
            EXPECT_ANY_THROW(masterlist.Update(masterlistPath,
                                               "https://github.com/loot/does-not-exist.git",
                                               "master"));
        }

        TEST_F(MasterlistTest, Update_NonGame_EmptyUrl) {
            Masterlist masterlist;
            EXPECT_ANY_THROW(masterlist.Update(masterlistPath,
                                               "",
                                               "master"));
        }

        TEST_F(MasterlistTest, Update_NonGame) {
            Masterlist masterlist;
            EXPECT_TRUE(masterlist.Update(masterlistPath,
                                          "https://github.com/loot/testing-metadata.git",
                                          "master"));

            EXPECT_FALSE(masterlist.Update(masterlistPath,
                                           "https://github.com/loot/testing-metadata.git",
                                           "master"));
        }

        TEST_F(MasterlistTest, GetInfo_NoMasterlist) {
            Masterlist masterlist;
            EXPECT_ANY_THROW(masterlist.GetInfo(masterlistPath, false));
        }

        TEST_F(MasterlistTest, GetInfo_NoRepository) {
            ASSERT_NO_THROW(boost::filesystem::copy("./testing-metadata/masterlist.yaml", masterlistPath));

            Masterlist masterlist;
            EXPECT_ANY_THROW(masterlist.GetInfo(masterlistPath, false));
        }

        TEST_F(MasterlistTest, GetInfo_LongID) {
            Masterlist masterlist;
            ASSERT_TRUE(masterlist.Update(masterlistPath,
                                          "https://github.com/loot/testing-metadata.git",
                                          "master"));

            Masterlist::Info info = masterlist.GetInfo(masterlistPath, false);
            EXPECT_EQ(40, info.revision.length());
            EXPECT_EQ(10, info.date.length());
        }

        TEST_F(MasterlistTest, GetInfo_ShortID) {
            Masterlist masterlist;
            ASSERT_TRUE(masterlist.Update(masterlistPath,
                                          "https://github.com/loot/testing-metadata.git",
                                          "master"));

            Masterlist::Info info = masterlist.GetInfo(masterlistPath, true);
            EXPECT_GE((unsigned)40, info.revision.length());
            EXPECT_LE((unsigned)7, info.revision.length());
            EXPECT_EQ(10, info.date.length());
        }

        TEST_F(MasterlistTest, GetInfo_Edited) {
            Masterlist masterlist;
            ASSERT_TRUE(masterlist.Update(masterlistPath,
                                          "https://github.com/loot/testing-metadata.git",
                                          "master"));
            boost::filesystem::ofstream out(masterlistPath);
            out.close();

            Masterlist::Info info = masterlist.GetInfo(masterlistPath, false);
            EXPECT_EQ(49, info.revision.length());
            EXPECT_EQ(" (edited)", info.revision.substr(40));
            EXPECT_EQ(19, info.date.length());
            EXPECT_EQ(" (edited)", info.date.substr(10));
        }
    }
}

#endif
