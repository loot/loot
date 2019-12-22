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

#ifndef LOOT_TESTS_GUI_CEF_QUERY_TYPES_CLOSE_SETTINGS_QUERY_TEST
#define LOOT_TESTS_GUI_CEF_QUERY_TYPES_CLOSE_SETTINGS_QUERY_TEST

#include "gui/cef/query/types/close_settings_query.h"

#include <fstream>

#include <gtest/gtest.h>

#include "tests/gui/test_helpers.h"

namespace loot {
namespace test {
class CloseSettingsQueryTest : public ::testing::Test {
public:
  CloseSettingsQueryTest() :
      appPath(getTempPath()),
      dataPath(getTempPath()),
      state(appPath, dataPath) {}

protected:
  void SetUp() override {
    std::filesystem::create_directories(appPath);
    std::filesystem::create_directories(dataPath);

    auto cssPath = state.getResourcesPath() / "ui" / "css";
    std::filesystem::create_directories(cssPath);

    touch(cssPath / "1.theme.css");
    touch(cssPath / "2.theme.css");
    touch(cssPath / "style.css");

    std::filesystem::create_directories(cssPath / "directory.theme.css");
  }

  void TearDown() override {
    std::filesystem::remove_all(appPath);
    std::filesystem::remove_all(dataPath);
  }

  const std::filesystem::path appPath;
  const std::filesystem::path dataPath;
  LootState state;
};

TEST_F(CloseSettingsQueryTest,
       executeLogicShouldRemoveThemeDotCssIfTheOldThemeIsNotDefaultAndTheNewThemeIsDefault) {
  auto themePath = state.getLootDataPath() / "theme.css";
  touch(themePath);
  ASSERT_TRUE(std::filesystem::exists(themePath));

  state.setTheme("blue");
  CloseSettingsQuery query(state, {{"theme", "default"}});
  query.executeLogic();

  EXPECT_FALSE(std::filesystem::exists(themePath));
}

TEST_F(CloseSettingsQueryTest,
       executeLogicShouldCopyNewThemeCssFileAsThemeDotCss) {
  auto newThemePath = state.getResourcesPath() / "ui" / "css" / "blue.theme.css";
  touch(newThemePath);
  ASSERT_TRUE(std::filesystem::exists(newThemePath));

  CloseSettingsQuery query(state, {{"theme", "blue"}});
  query.executeLogic();

  auto themePath = state.getLootDataPath() / "theme.css";
  EXPECT_TRUE(std::filesystem::exists(themePath));
  EXPECT_TRUE(std::filesystem::exists(newThemePath));
}
}
}

#endif
