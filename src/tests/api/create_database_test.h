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
class CreateDatabaseTest :
  public ::testing::TestWithParam<GameType>,
  public CommonGameTestFixture {
protected:
  CreateDatabaseTest() :
    CommonGameTestFixture(static_cast<unsigned int>(GetParam())),
    db_(nullptr) {}

  void SetUp() {
    setUp();
  }

  void TearDown() {
    tearDown();
  }

  std::shared_ptr<DatabaseInterface> db_;
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
  EXPECT_ANY_THROW(CreateDatabase(GetParam(), missingPath.string(), localPath.string()));
}

TEST_P(CreateDatabaseTest, shouldThrowIfPassedALocalPathThatDoesNotExist) {
  EXPECT_ANY_THROW(CreateDatabase(GetParam(), dataPath.parent_path().string(), missingPath.string()));
}

#ifdef _WIN32
TEST_P(CreateDatabaseTest, shouldReturnOkIfPassedAnEmptyLocalPathString) {
  EXPECT_NO_THROW(db_ = CreateDatabase(GetParam(), dataPath.parent_path().string(), ""));
  EXPECT_NE(nullptr, db_);
}
#endif
}
}

#endif
