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
#include "tests/base_game_test.h"

namespace loot {
    namespace test {
        class MasterlistTest : public BaseGameTest {
        protected:
            MasterlistTest() :
                repoBranch("master"),
                repoUrl("https://github.com/loot/testing-metadata.git"),
                masterlistPath(localPath / "masterlist.yaml") {}

            void SetUp() {
                BaseGameTest::SetUp();

                ASSERT_FALSE(boost::filesystem::exists(masterlistPath));
                ASSERT_FALSE(boost::filesystem::exists(localPath / ".git"));

                ASSERT_NO_THROW(boost::filesystem::create_directories(g_path_local / Game(GetParam()).FolderName()));
            }

            void TearDown() {
                BaseGameTest::TearDown();

                ASSERT_NO_THROW(boost::filesystem::remove(masterlistPath));
                ASSERT_NO_THROW(boost::filesystem::remove_all(localPath / ".git"));

                ASSERT_NO_THROW(boost::filesystem::remove(g_path_local / Game(GetParam()).FolderName() / "masterlist.yaml"));
                ASSERT_NO_THROW(boost::filesystem::remove_all(g_path_local / Game(GetParam()).FolderName() / ".git"));
            }

            const std::string repoUrl;
            const std::string repoBranch;

            const boost::filesystem::path masterlistPath;
        };

        // Pass an empty first argument, as it's a prefix for the test instantation,
        // but we only have the one so no prefix is necessary.
        INSTANTIATE_TEST_CASE_P(,
                                MasterlistTest,
                                ::testing::Values(
                                    GameSettings::tes4,
                                    GameSettings::tes5,
                                    GameSettings::fo3,
                                    GameSettings::fonv,
                                    GameSettings::fo4));

        TEST_P(MasterlistTest, updateWithGameParameterShouldReturnTrueIfNoMasterlistExists) {
            Game game(GetParam());
            game.SetGamePath(dataPath.parent_path());
            game.SetRepoURL(repoUrl);
            game.SetRepoBranch(repoBranch);
            ASSERT_NO_THROW(game.Init(false, localPath));

            // This may fail on Windows if a 'real' LOOT install is also present.
            Masterlist masterlist;
            EXPECT_TRUE(masterlist.Update(game));
            EXPECT_TRUE(boost::filesystem::exists(game.MasterlistPath()));
        }

        TEST_P(MasterlistTest, updateWithGameParameterShouldReturnFalseIfAnUpToDateMasterlistExists) {
            Game game(GetParam());
            game.SetGamePath(dataPath.parent_path());
            game.SetRepoURL(repoUrl);
            game.SetRepoBranch(repoBranch);
            ASSERT_NO_THROW(game.Init(false, localPath));

            // This may fail on Windows if a 'real' LOOT install is also present.
            Masterlist masterlist;
            EXPECT_TRUE(masterlist.Update(game));
            EXPECT_TRUE(boost::filesystem::exists(game.MasterlistPath()));

            EXPECT_FALSE(masterlist.Update(game));
            EXPECT_TRUE(boost::filesystem::exists(game.MasterlistPath()));
        }

        TEST_P(MasterlistTest, updateWithSeparateParametersShouldThrowIfAnInvalidPathIsGiven) {
            Masterlist masterlist;

            EXPECT_ANY_THROW(masterlist.Update(";//\?", repoUrl, repoBranch));
        }

        TEST_P(MasterlistTest, updateWithSeparateParametersShouldThrowIfABlankPathIsGiven) {
            Masterlist masterlist;

            EXPECT_ANY_THROW(masterlist.Update("", repoUrl, repoBranch));
        }

        TEST_P(MasterlistTest, updateWithSeparateParametersShouldThrowIfABranchThatDoesNotExistIsGiven) {
            Masterlist masterlist;

            EXPECT_ANY_THROW(masterlist.Update(masterlistPath,
                                               repoUrl,
                                               "missing-branch"));
        }

        TEST_P(MasterlistTest, updateWithSeparateParametersShouldThrowIfABlankBranchIsGiven) {
            Masterlist masterlist;

            EXPECT_ANY_THROW(masterlist.Update(masterlistPath, repoUrl, ""));
        }

        TEST_P(MasterlistTest, updateWithSeparateParametersShouldThrowIfAUrlThatDoesNotExistIsGiven) {
            Masterlist masterlist;

            EXPECT_ANY_THROW(masterlist.Update(masterlistPath,
                                               "https://github.com/loot/does-not-exist.git",
                                               repoBranch));
        }

        TEST_P(MasterlistTest, updateWithSeparateParametersShouldThrowIfABlankUrlIsGiven) {
            Masterlist masterlist;
            EXPECT_ANY_THROW(masterlist.Update(masterlistPath, "", repoBranch));
        }

        TEST_P(MasterlistTest, updateWithSeparateParametersShouldReturnTrueIfNoMasterlistExists) {
            Masterlist masterlist;
            EXPECT_TRUE(masterlist.Update(masterlistPath,
                                          repoUrl,
                                          repoBranch));
        }

        TEST_P(MasterlistTest, updateWithSeparateParametersShouldReturnFalseIfAnUpToDateMasterlistExists) {
            Masterlist masterlist;

            EXPECT_TRUE(masterlist.Update(masterlistPath,
                                          repoUrl,
                                          repoBranch));

            EXPECT_FALSE(masterlist.Update(masterlistPath,
                                           repoUrl,
                                           repoBranch));
        }

        TEST_P(MasterlistTest, getInfoShouldThrowIfNoMasterlistExistsAtTheGivenPath) {
            Masterlist masterlist;
            EXPECT_ANY_THROW(masterlist.GetInfo(masterlistPath, false));
        }

        TEST_P(MasterlistTest, getInfoShouldThrowIfTheGivenPathDoesNotBelongToAGitRepository) {
            ASSERT_NO_THROW(boost::filesystem::copy("./testing-metadata/masterlist.yaml", masterlistPath));

            Masterlist masterlist;
            EXPECT_ANY_THROW(masterlist.GetInfo(masterlistPath, false));
        }

        TEST_P(MasterlistTest, getInfoShouldReturnRevisionAndDateStringsOfTheCorrectLengthsWhenRequestingALongId) {
            Masterlist masterlist;
            ASSERT_TRUE(masterlist.Update(masterlistPath,
                                          repoUrl,
                                          repoBranch));

            Masterlist::Info info = masterlist.GetInfo(masterlistPath, false);
            EXPECT_EQ(40, info.revision.length());
            EXPECT_EQ(10, info.date.length());
        }

        TEST_P(MasterlistTest, getInfoShouldReturnRevisionAndDateStringsOfTheCorrectLengthsWhenRequestingAShortId) {
            Masterlist masterlist;
            ASSERT_TRUE(masterlist.Update(masterlistPath,
                                          repoUrl,
                                          repoBranch));

            Masterlist::Info info = masterlist.GetInfo(masterlistPath, true);
            EXPECT_GE((unsigned)40, info.revision.length());
            EXPECT_LE((unsigned)7, info.revision.length());
            EXPECT_EQ(10, info.date.length());
        }

        TEST_P(MasterlistTest, getInfoShouldAppendSuffixesToReturnedStringsIfTheMasterlistHasBeenEdited) {
            Masterlist masterlist;
            ASSERT_TRUE(masterlist.Update(masterlistPath,
                                          repoUrl,
                                          repoBranch));
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
