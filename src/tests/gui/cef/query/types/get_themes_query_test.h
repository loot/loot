/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014 WrinklyNinja

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

#ifndef LOOT_TESTS_GUI_CEF_QUERY_TYPES_GET_THEMES_QUERY_TEST
#define LOOT_TESTS_GUI_CEF_QUERY_TYPES_GET_THEMES_QUERY_TEST

#include "gui/cef/query/types/get_themes_query.h"

#include <fstream>

#include <gtest/gtest.h>

#include "tests/gui/test_helpers.h"

namespace loot {
namespace test {
class GetThemesQueryTest : public ::testing::Test {
public:
  GetThemesQueryTest() :
      resourcesPath(getTempPath()),
      cssPath(resourcesPath / "ui" / "css") {}

protected:
  void SetUp() override { std::filesystem::create_directories(cssPath); }

  void TearDown() override { std::filesystem::remove_all(resourcesPath); }

  const std::filesystem::path resourcesPath;
  const std::filesystem::path cssPath;
};

TEST_F(GetThemesQueryTest, executeLogicShouldFindFilesEndingInDotThemeDotCss) {
  touch(cssPath / "1.theme.css");
  touch(cssPath / "2.theme.css");

  GetThemesQuery query(resourcesPath);

  EXPECT_EQ("{\"themes\":[\"1\",\"2\"]}", query.executeLogic());
}

TEST_F(GetThemesQueryTest, executeLogicShouldNotFindNonThemeCssFiles) {
  touch(cssPath / "style.css");

  GetThemesQuery query(resourcesPath);

  EXPECT_EQ("{\"themes\":[]}", query.executeLogic());
}

TEST_F(GetThemesQueryTest,
       executeLogicShouldNotFindNonFilesEndingInDotThemeDotCss) {
  std::filesystem::create_directories(cssPath / "directory.theme.css");

  GetThemesQuery query(resourcesPath);

  EXPECT_EQ("{\"themes\":[]}", query.executeLogic());
}
}
}

#endif
