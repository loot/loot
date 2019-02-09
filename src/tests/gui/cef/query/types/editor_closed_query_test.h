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

#ifndef LOOT_TESTS_GUI_CEF_QUERY_TYPES_EDITOR_CLOSED_QUERY_TEST
#define LOOT_TESTS_GUI_CEF_QUERY_TYPES_EDITOR_CLOSED_QUERY_TEST

#include "gui/cef/query/types/editor_closed_query.h"

#include <gtest/gtest.h>

namespace loot {
namespace test {
class TestPlugin : public PluginInterface {
public:
  TestPlugin(std::string name) : name_(name) {}

  std::string GetName() const override { return name_; }

  float GetHeaderVersion() const { return 0.0f; }

  std::optional<std::string> GetVersion() const {
    return std::optional<std::string>();
  }

  std::vector<std::string> GetMasters() const {
    return std::vector<std::string>();
  }

  std::set<Tag> GetBashTags() const { return std::set<Tag>(); }

  std::optional<uint32_t> GetCRC() const { return std::optional<uint32_t>(); }

  bool IsMaster() const { return false; }

  bool IsLightMaster() const { return false; }

  bool IsValidAsLightMaster() const { return false; }

  bool IsEmpty() const { return false; }

  bool LoadsArchive() const { return false; }

  bool DoFormIDsOverlap(const PluginInterface& plugin) const { return false; }

private:
  const std::string name_;
};

class TestGame {
public:
  std::shared_ptr<const PluginInterface> GetPlugin(
      const std::string& name) const {
    return std::make_shared<TestPlugin>(name);
  }

  bool IsPluginActive(const std::string& pluginName) const { return false; }
  std::optional<short> GetActiveLoadOrderIndex(
      const std::shared_ptr<const PluginInterface>& plugin,
      const std::vector<std::string>& loadOrder) const {
    return std::nullopt;
  }

  std::vector<std::string> GetLoadOrder() const { return {}; }

  std::optional<PluginMetadata> GetUserMetadata(std::string name,
                                                bool eval = true) const {
    return userMetadata;
  }

  std::optional<PluginMetadata> GetMasterlistMetadata(std::string name,
                                                      bool eval = true) const {
    if (name == NO_MASTERLIST_METADATA_PLUGIN) {
      return std::nullopt;
    }

    PluginMetadata metadata(name);
    metadata.SetLoadAfterFiles({File("file")});

    if (name == MASTERLIST_DEFAULT_GROUP_PLUGIN) {
      metadata.SetGroup("default");
    } else if (name == MASTERLIST_DLC_GROUP_PLUGIN) {
      metadata.SetGroup("DLC");
    } else if (name == MASTERLIST_LATE_GROUP_PLUGIN) {
      metadata.SetGroup("Late");
    } else if (name != MASTERLIST_NO_GROUP_PLUGIN) {
      throw std::invalid_argument("unrecognised plugin name: " + name);
    }

    return metadata;
  }

  std::vector<Message> CheckInstallValidity(
      std::shared_ptr<const PluginInterface> file,
      PluginMetadata metadata) {
    return {};
  }

  void ClearUserMetadata(std::string name) { userMetadata = std::nullopt; }
  void AddUserMetadata(PluginMetadata metadata) { userMetadata = metadata; }
  void SaveUserMetadata() {}

  static constexpr auto NO_MASTERLIST_METADATA_PLUGIN = "no non-user metadata";
  static constexpr auto MASTERLIST_LATE_GROUP_PLUGIN =
      "masterlist metadata with Late group";
  static constexpr auto MASTERLIST_DEFAULT_GROUP_PLUGIN =
      "masterlist metadata with default group";
  static constexpr auto MASTERLIST_DLC_GROUP_PLUGIN =
      "masterlist metadata with DLC group";
  static constexpr auto MASTERLIST_NO_GROUP_PLUGIN =
      "masterlist metadata with no group";

private:
  std::optional<PluginMetadata> userMetadata;
};

TEST(EditorClosedQuery,
     shouldUnsetGroupIfItIsDefaultAndThereIsNoNonUserMetadata) {
  TestGame game;
  UnappliedChangeCounter counter;
  nlohmann::json json = {{"applyEdits", true},
                         {"metadata",
                          {{"name", TestGame::NO_MASTERLIST_METADATA_PLUGIN},
                           {"group", "default"}}}};
  EditorClosedQuery<TestGame> query(game, counter, "en", json);

  nlohmann::json responseJson = nlohmann::json::parse(query.executeLogic());

  EXPECT_EQ(TestGame::NO_MASTERLIST_METADATA_PLUGIN,
            responseJson.at("name").get<std::string>());
  EXPECT_EQ(0, responseJson.count("group"));
  EXPECT_EQ(0, responseJson.count("userlist"));
}

TEST(EditorClosedQuery, shouldLeaveGroupUnsetIfThereIsNoNonUserMetadata) {
  TestGame game;
  UnappliedChangeCounter counter;
  nlohmann::json json = {{"applyEdits", true},
                         {"metadata",
                          {
                              {"name", TestGame::NO_MASTERLIST_METADATA_PLUGIN},
                          }}};
  EditorClosedQuery<TestGame> query(game, counter, "en", json);

  nlohmann::json responseJson = nlohmann::json::parse(query.executeLogic());

  EXPECT_EQ(TestGame::NO_MASTERLIST_METADATA_PLUGIN,
            responseJson.at("name").get<std::string>());
  EXPECT_EQ(0, responseJson.count("group"));
  EXPECT_EQ(0, responseJson.count("userlist"));
}

TEST(EditorClosedQuery,
     shouldRetainGroupIfItIsNotDefaultAndThereIsNoNonUserMetadata) {
  TestGame game;
  UnappliedChangeCounter counter;
  nlohmann::json json = {
      {"applyEdits", true},
      {"metadata",
       {{"name", TestGame::NO_MASTERLIST_METADATA_PLUGIN}, {"group", "DLC"}}}};
  EditorClosedQuery<TestGame> query(game, counter, "en", json);

  nlohmann::json responseJson = nlohmann::json::parse(query.executeLogic());

  EXPECT_EQ(TestGame::NO_MASTERLIST_METADATA_PLUGIN,
            responseJson.at("name").get<std::string>());
  EXPECT_EQ("DLC", responseJson.at("group").get<std::string>());
  EXPECT_EQ("DLC", responseJson.at("userlist").at("group").get<std::string>());
}

TEST(EditorClosedQuery,
     shouldUnsetGroupIfItIsDefaultAndNonUserMetadataHasDefaultGroup) {
  TestGame game;
  UnappliedChangeCounter counter;
  nlohmann::json json = {{"applyEdits", true},
                         {"metadata",
                          {{"name", TestGame::MASTERLIST_DEFAULT_GROUP_PLUGIN},
                           {"group", "default"}}}};
  EditorClosedQuery<TestGame> query(game, counter, "en", json);

  nlohmann::json responseJson = nlohmann::json::parse(query.executeLogic());

  EXPECT_EQ(TestGame::MASTERLIST_DEFAULT_GROUP_PLUGIN,
            responseJson.at("name").get<std::string>());
  EXPECT_EQ("default", responseJson.at("group").get<std::string>());
  EXPECT_EQ(0, responseJson.count("userlist"));
}

TEST(EditorClosedQuery,
     shouldUnsetGroupIfItIsDefaultAndNonUserMetadataHasNoGroup) {
  TestGame game;
  UnappliedChangeCounter counter;
  nlohmann::json json = {
      {"applyEdits", true},
      {"metadata",
       {{"name", TestGame::MASTERLIST_NO_GROUP_PLUGIN}, {"group", "default"}}}};
  EditorClosedQuery<TestGame> query(game, counter, "en", json);

  nlohmann::json responseJson = nlohmann::json::parse(query.executeLogic());

  EXPECT_EQ(TestGame::MASTERLIST_NO_GROUP_PLUGIN,
            responseJson.at("name").get<std::string>());
  EXPECT_EQ(0, responseJson.count("group"));
  EXPECT_EQ(0, responseJson.count("userlist"));
}

TEST(EditorClosedQuery,
     shouldRetainGroupIfItIsDefaultAndNonUserMetadataHasNonDefaultGroup) {
  TestGame game;
  UnappliedChangeCounter counter;
  nlohmann::json json = {{"applyEdits", true},
                         {"metadata",
                          {{"name", TestGame::MASTERLIST_DLC_GROUP_PLUGIN},
                           {"group", "default"}}}};
  EditorClosedQuery<TestGame> query(game, counter, "en", json);

  nlohmann::json responseJson = nlohmann::json::parse(query.executeLogic());

  EXPECT_EQ(TestGame::MASTERLIST_DLC_GROUP_PLUGIN,
            responseJson.at("name").get<std::string>());
  EXPECT_EQ("default", responseJson.at("group").get<std::string>());
  EXPECT_EQ("default",
            responseJson.at("userlist").at("group").get<std::string>());
}

TEST(EditorClosedQuery,
     shouldRetainGroupIfItIsNotDefaultAndNonUserMetadataHasDefaultGroup) {
  TestGame game;
  UnappliedChangeCounter counter;
  nlohmann::json json = {{"applyEdits", true},
                         {"metadata",
                          {{"name", TestGame::MASTERLIST_DEFAULT_GROUP_PLUGIN},
                           {"group", "DLC"}}}};
  EditorClosedQuery<TestGame> query(game, counter, "en", json);

  nlohmann::json responseJson = nlohmann::json::parse(query.executeLogic());

  EXPECT_EQ(TestGame::MASTERLIST_DEFAULT_GROUP_PLUGIN,
            responseJson.at("name").get<std::string>());
  EXPECT_EQ("DLC", responseJson.at("group").get<std::string>());
  EXPECT_EQ("DLC", responseJson.at("userlist").at("group").get<std::string>());
}

TEST(EditorClosedQuery,
     shouldRetainGroupIfItIsNotDefaultAndNonUserMetadataHasNoGroup) {
  TestGame game;
  UnappliedChangeCounter counter;
  nlohmann::json json = {
      {"applyEdits", true},
      {"metadata",
       {{"name", TestGame::MASTERLIST_NO_GROUP_PLUGIN}, {"group", "DLC"}}}};
  EditorClosedQuery<TestGame> query(game, counter, "en", json);

  nlohmann::json responseJson = nlohmann::json::parse(query.executeLogic());

  EXPECT_EQ(TestGame::MASTERLIST_NO_GROUP_PLUGIN,
            responseJson.at("name").get<std::string>());
  EXPECT_EQ("DLC", responseJson.at("group").get<std::string>());
  EXPECT_EQ("DLC", responseJson.at("userlist").at("group").get<std::string>());
}

TEST(
    EditorClosedQuery,
    shouldRetainGroupIfItIsNotDefaultAndNonUserMetadataHasAnotherNonDefaultGroup) {
  TestGame game;
  UnappliedChangeCounter counter;
  nlohmann::json json = {
      {"applyEdits", true},
      {"metadata",
       {{"name", TestGame::MASTERLIST_LATE_GROUP_PLUGIN}, {"group", "DLC"}}}};
  EditorClosedQuery<TestGame> query(game, counter, "en", json);

  nlohmann::json responseJson = nlohmann::json::parse(query.executeLogic());

  EXPECT_EQ(TestGame::MASTERLIST_LATE_GROUP_PLUGIN,
            responseJson.at("name").get<std::string>());
  EXPECT_EQ("DLC", responseJson.at("group").get<std::string>());
  EXPECT_EQ("DLC", responseJson.at("userlist").at("group").get<std::string>());
}
}
}
#endif
