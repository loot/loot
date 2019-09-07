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

#ifndef LOOT_TESTS_GUI_CEF_QUERY_TYPES_GET_SETTINGS_QUERY_TEST
#define LOOT_TESTS_GUI_CEF_QUERY_TYPES_GET_SETTINGS_QUERY_TEST

#include "gui/cef/query/types/get_settings_query.h"

#include <gtest/gtest.h>

namespace loot {
namespace test {
TEST(GetSettingsQuery, shouldIncludeThemeInOutput) {
  LootSettings settings;
  settings.setTheme("test");

  GetSettingsQuery query(settings);
  auto json = query.executeLogic();

  EXPECT_NE(std::string::npos, json.find("\"theme\":\"test\""));
}
}
}

#endif
