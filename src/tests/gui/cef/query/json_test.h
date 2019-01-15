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

#ifndef LOOT_TESTS_GUI_CEF_QUERY_JSON_TEST
#define LOOT_TESTS_GUI_CEF_QUERY_JSON_TEST

#include "gui/cef/query/json.h"

#include <gtest/gtest.h>

namespace loot {
namespace test {
TEST(to_json, shouldEncodeGameSettingsPathsAsUtf8) {
  using std::filesystem::u8path;

  auto gameSettings = GameSettings(GameType::tes4)
    .SetGamePath(u8path(u8"non\u00C1sciiGamePath"))
    .SetGameLocalPath(u8path(u8"non\u00C1sciiGameLocalPath"));

  nlohmann::json json;
  to_json(json, gameSettings);

  EXPECT_EQ(u8"non\u00C1sciiGamePath", json.at("path"));
  EXPECT_EQ(u8"non\u00C1sciiGameLocalPath", json.at("localPath"));
}

TEST(from_json, shouldEncodeGameSettingsPathsAsUtf8) {
  using std::filesystem::u8path;

  nlohmann::json json;
  json["type"] = GameSettings(GameType::tes4).FolderName();
  json["folder"] = json["type"];
  json["path"] = u8"non\u00C1sciiGamePath";
  json["localPath"] = u8"non\u00C1sciiGameLocalPath";

  GameSettings gameSettings;

  from_json(json, gameSettings);

  EXPECT_EQ(u8path(u8"non\u00C1sciiGamePath"), gameSettings.GamePath());
  EXPECT_EQ(u8path(u8"non\u00C1sciiGameLocalPath"), gameSettings.GameLocalPath());
}

TEST(from_json, shouldNotSetPluginMetadataGroupIfUnspecified) {
  nlohmann::json json;
  json["name"] = "test.esp";

  PluginMetadata pluginMetadata;

  from_json(json, pluginMetadata);

  EXPECT_FALSE(pluginMetadata.GetGroup().has_value());
}

TEST(from_json, shouldSetPluginMetadataDefaultGroupIfSpecified) {
  nlohmann::json json;
  json["name"] = "test.esp";
  json["group"] = Group().GetName();

  PluginMetadata pluginMetadata;

  from_json(json, pluginMetadata);

  EXPECT_EQ(Group().GetName(), pluginMetadata.GetGroup().value());
}

TEST(from_json, shouldSetPluginMetadataNonDefaultGroupIfSpecified) {
  nlohmann::json json;
  json["name"] = "test.esp";
  json["group"] = "not default";

  PluginMetadata pluginMetadata;

  from_json(json, pluginMetadata);

  ASSERT_NE("not default", Group().GetName());
  EXPECT_EQ("not default", pluginMetadata.GetGroup().value());
}
}
}
#endif
