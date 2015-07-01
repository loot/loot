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

#ifndef LOOT_TEST_BACKEND_HELPERS_LANGUAGE
#define LOOT_TEST_BACKEND_HELPERS_LANGUAGE

#include "backend/error.h"
#include "backend/helpers/git_helper.h"
#include "tests/fixtures.h"

using loot::GitHelper;

#ifdef _WIN32
boost::filesystem::path parentRepoRoot = boost::filesystem::current_path().parent_path().parent_path();
#else
boost::filesystem::path parentRepoRoot = boost::filesystem::current_path().parent_path();
#endif

TEST(GitHelper, ConstructorAndDestructor) {
    GitHelper * git = new GitHelper();
    EXPECT_EQ(nullptr, git->repo);

    EXPECT_EQ(2, git_libgit2_init());
    delete git;
    EXPECT_EQ(0, git_libgit2_shutdown());
}

TEST(GitHelper, Call) {
    GitHelper git;
    EXPECT_NO_THROW(git.Call(0));
    EXPECT_THROW(git.Call(1), loot::error);
    EXPECT_THROW(git.Call(-1), loot::error);
}

TEST(GitHelper, SetErrorMessage) {
    GitHelper git;
    git.SetErrorMessage("test message");

    try {
        git.Call(1);
        ADD_FAILURE() << "An exception should have been thrown.";
    }
    catch (loot::error& e) {
        EXPECT_NE(nullptr, strstr(e.what(), "test message"));
    }
}

TEST(GitHelper, Free) {
    // Initialise buffer member, it's simplest to test with.
    GitHelper git;
    git_buf_set(&git.buf, "foo", 4);
    EXPECT_NE(nullptr, git.buf.ptr);
    EXPECT_EQ(4, git.buf.size);

    git.Free();
    EXPECT_EQ(nullptr, git.buf.ptr);
    EXPECT_EQ(0, git.buf.size);
}

TEST(GitHelper, IsRepository) {
    GitHelper git;

    ASSERT_TRUE(boost::filesystem::exists(parentRepoRoot));
    EXPECT_TRUE(git.IsRepository(parentRepoRoot));
    EXPECT_FALSE(git.IsRepository(boost::filesystem::current_path()));
}

class IsFileDifferent : public ::testing::Test {
    inline virtual void SetUp() {
        GitHelper git;
        ASSERT_TRUE(git.IsRepository(parentRepoRoot));
        ASSERT_FALSE(git.IsRepository(boost::filesystem::current_path()));

        ASSERT_TRUE(boost::filesystem::exists(parentRepoRoot / "README.md"));

        // Create a backup of CONTRIBUTING.md.
        ASSERT_TRUE(boost::filesystem::exists(parentRepoRoot / "CONTRIBUTING.md"));
        ASSERT_FALSE(boost::filesystem::exists(parentRepoRoot / "CONTRIBUTING.md.copy"));
        ASSERT_NO_THROW(boost::filesystem::copy(parentRepoRoot / "CONTRIBUTING.md", parentRepoRoot / "CONTRIBUTING.md.copy"));
        ASSERT_TRUE(boost::filesystem::exists(parentRepoRoot / "CONTRIBUTING.md.copy"));

        // Edit CONTRIBUTING.md
        loot::ofstream out(parentRepoRoot / "CONTRIBUTING.md");
        out.close();
    }

    inline virtual void TearDown() {
        // Restore original CONTRIBUTING.md
        ASSERT_NO_THROW(boost::filesystem::remove(parentRepoRoot / "CONTRIBUTING.md"));
        ASSERT_NO_THROW(boost::filesystem::rename(parentRepoRoot / "CONTRIBUTING.md.copy", parentRepoRoot / "CONTRIBUTING.md"));
        ASSERT_TRUE(boost::filesystem::exists(parentRepoRoot / "CONTRIBUTING.md"));
        ASSERT_FALSE(boost::filesystem::exists(parentRepoRoot / "CONTRIBUTING.md.copy"));
    }
};

TEST_F(IsFileDifferent, InvalidRepository) {
    EXPECT_THROW(loot::IsFileDifferent(boost::filesystem::current_path(), "README.md"), loot::error);
}

TEST_F(IsFileDifferent, SameFile) {
    EXPECT_FALSE(loot::IsFileDifferent(parentRepoRoot, "README.md"));
}

TEST_F(IsFileDifferent, NewFile) {
    // New files not in the index are not tracked by Git, so aren't considered
    // different.
    EXPECT_FALSE(loot::IsFileDifferent(parentRepoRoot, "CONTRIBUTING.md.copy"));
}

TEST_F(IsFileDifferent, DifferentFile) {
    EXPECT_TRUE(loot::IsFileDifferent(parentRepoRoot, "CONTRIBUTING.md"));
}

#endif
