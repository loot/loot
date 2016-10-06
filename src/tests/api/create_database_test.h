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
<https://www.gnu.org/licenses/>.
*/

#ifndef LOOT_TESTS_API_CREATE_DATABASE_TEST
#define LOOT_TESTS_API_CREATE_DATABASE_TEST

#include "loot/api.h"

#include <climits>

#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class CreateDatabaseTest : public CommonGameTestFixture {
protected:
  CreateDatabaseTest() :
    db_(nullptr),
    gamePathSymlink(dataPath.parent_path().string() + "symlink"),
    localPathSymlink(localPath.string() + "symlink") {}

  void SetUp() {
    CommonGameTestFixture::SetUp();

    boost::filesystem::create_directory_symlink(dataPath.parent_path(), gamePathSymlink);
    boost::filesystem::create_directory_symlink(localPath, localPathSymlink);
  }

  void TearDown() {
    CommonGameTestFixture::TearDown();

    boost::filesystem::remove(gamePathSymlink);
    boost::filesystem::remove(localPathSymlink);
  }

  std::shared_ptr<DatabaseInterface> db_;

  const boost::filesystem::path gamePathSymlink;
  const boost::filesystem::path localPathSymlink;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        CreateDatabaseTest,
                        ::testing::Values(
                          GameType::tes4,
                          GameType::tes5,
                          GameType::fo3,
                          GameType::fonv,
                          GameType::fo4));

TEST_P(CreateDatabaseTest, shouldSucceedIfPassedValidParametersWithRelativePaths) {
  EXPECT_NO_THROW(db_ = CreateDatabase(GetParam(), dataPath.parent_path().string(), localPath.string()));
  EXPECT_NE(nullptr, db_);
}

TEST_P(CreateDatabaseTest, shouldSucceedIfPassedValidParametersWithAbsolutePaths) {
  boost::filesystem::path game = boost::filesystem::current_path() / dataPath.parent_path();
  boost::filesystem::path local = boost::filesystem::current_path() / localPath;

  EXPECT_NO_THROW(db_ = CreateDatabase(GetParam(), dataPath.parent_path().string(), localPath.string()));
  EXPECT_NE(nullptr, db_);
}

TEST_P(CreateDatabaseTest, shouldThrowIfPassedAGamePathThatDoesNotExist) {
  EXPECT_THROW(CreateDatabase(GetParam(), missingPath.string(), localPath.string()), std::invalid_argument);
}

TEST_P(CreateDatabaseTest, shouldThrowIfPassedALocalPathThatDoesNotExist) {
  EXPECT_THROW(CreateDatabase(GetParam(), dataPath.parent_path().string(), missingPath.string()), std::invalid_argument);
}

#ifdef _WIN32
TEST_P(CreateDatabaseTest, shouldReturnOkIfPassedAnEmptyLocalPathString) {
  EXPECT_NO_THROW(db_ = CreateDatabase(GetParam(), dataPath.parent_path().string(), ""));
  EXPECT_NE(nullptr, db_);
}
#endif

TEST_P(CreateDatabaseTest, shouldReturnOkIfPassedGameAndLocalPathSymlinks) {
  EXPECT_NO_THROW(db_ = CreateDatabase(GetParam(), gamePathSymlink.string(), localPathSymlink.string()));
  EXPECT_NE(nullptr, db_);
}
}
}

#endif
