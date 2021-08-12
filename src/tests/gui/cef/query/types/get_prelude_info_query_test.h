/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2021 Oliver Hamlet

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

#ifndef LOOT_TESTS_GUI_CEF_QUERY_TYPES_GET_PRELUDE_INFO_QUERY_TEST
#define LOOT_TESTS_GUI_CEF_QUERY_TYPES_GET_PRELUDE_INFO_QUERY_TEST

#include <gtest/gtest.h>

#include <fstream>

#include "gui/cef/query/types/get_prelude_info_query.h"
#include "tests/gui/test_helpers.h"

namespace loot {
namespace test {
class GetPreludeInfoQueryTest : public ::testing::Test {
public:
  GetPreludeInfoQueryTest() :
      repoPath(getTempPath()), preludePath(repoPath / "prelude.yaml") {}

protected:
  void SetUp() override {
    std::filesystem::create_directories(repoPath);

    touch(preludePath);
  }

  void TearDown() override {
    try {
      std::filesystem::remove_all(repoPath);
    } catch (std::exception& e) {
      // This might fail for the tests that call system(), so just ignore
      // the exception (the OS will clear out the temp dir at some point).
    }
  }

  const std::filesystem::path repoPath;
  const std::filesystem::path preludePath;
};

TEST_F(GetPreludeInfoQueryTest,
       executeLogicShouldReturnInfoForUnchangedFileInGitRepo) {
  auto originalCwd = std::filesystem::current_path();
  std::filesystem::current_path(repoPath);
  system("git init");
  system("git add prelude.yaml");
  system("git commit -m \"Initial commit\" --no-gpg-sign");
  std::filesystem::current_path(originalCwd);

  GetPreludeInfoQuery query(preludePath);

  auto output = query.executeLogic();
  EXPECT_NE(std::string::npos, output.find("\"date\""));
  EXPECT_NE(std::string::npos, output.find("\"id\""));
}

TEST_F(GetPreludeInfoQueryTest,
       executeLogicShouldReturnInfoForChangedFileInGitRepo) {
  auto originalCwd = std::filesystem::current_path();
  std::filesystem::current_path(repoPath);
  system("git init");
  system("git add prelude.yaml");
  system("git commit -m \"Initial commit\" --no-gpg-sign");
  std::filesystem::current_path(originalCwd);

  std::ofstream out(preludePath);
  out << "Change" << std::endl;
  out.close();

  GetPreludeInfoQuery query(preludePath);

  auto output = query.executeLogic();
  EXPECT_NE(std::string::npos, output.find("\"date\""));
  EXPECT_NE(std::string::npos, output.find("\"id\""));
  EXPECT_NE(std::string::npos, output.find("(edited)"));
}

TEST_F(
    GetPreludeInfoQueryTest,
    executeLogicShouldReturnMissingRepositoryIfTheParentDirectoryIsNotAGitRepo) {
  GetPreludeInfoQuery query(preludePath);

  auto output = query.executeLogic();
  auto expectedOutput =
      "{\"date\":\"Unknown: Git repository missing\",\"id\":\"Unknown: "
      "Git repository missing\"}";
  EXPECT_EQ(expectedOutput, output);
}

TEST_F(GetPreludeInfoQueryTest,
       executeLogicShouldReturnMissingFileIfTheGivenPathDoesNotExist) {
  std::filesystem::remove(preludePath);

  GetPreludeInfoQuery query(preludePath);

  auto output = query.executeLogic();
  auto expectedOutput =
      "{\"date\":\"N/A: No masterlist prelude present\",\"id\":\"N/A: No "
      "masterlist prelude present\"}";
  EXPECT_EQ(expectedOutput, output);
}
}
}

#endif