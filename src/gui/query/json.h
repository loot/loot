/*  LOOT

A load order optimisation tool for
Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

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

#ifndef LOOT_GUI_QUERY_JSON
#define LOOT_GUI_QUERY_JSON

#undef min
#undef max

#include <loot/api.h>

#include <json.hpp>

#include "gui/helpers.h"
#include "gui/query/derived_plugin_metadata.h"
#include "gui/state/game/helpers.h"
#include "gui/state/loot_settings.h"

namespace loot {
void testConditionSyntax(const std::string& objectType,
                         const std::string& condition) {
  try {
    ConditionalMetadata(condition).ParseCondition();
  } catch (std::exception& e) {
    throw std::runtime_error(objectType +
                             " object has an invalid condition: " + e.what());
  }
}

void validateMessageContents(const std::vector<MessageContent>& contents) {
  if (contents.size() > 1) {
    bool found = false;
    for (const auto& mc : contents) {
      if (mc.GetLanguage() == loot::MessageContent::defaultLanguage)
        found = true;
    }
    if (!found)
      throw std::runtime_error(
          "MessageContent array does not contain an English MessageContent "
          "object");
  }
}

GameType mapGameType(const std::string& gameType) {
  if (gameType == GameSettings(GameType::tes3).FolderName())
    return GameType::tes3;
  if (gameType == GameSettings(GameType::tes4).FolderName())
    return GameType::tes4;
  else if (gameType == GameSettings(GameType::tes5).FolderName())
    return GameType::tes5;
  else if (gameType == GameSettings(GameType::tes5se).FolderName())
    return GameType::tes5se;
  else if (gameType == GameSettings(GameType::tes5vr).FolderName())
    return GameType::tes5vr;
  else if (gameType == GameSettings(GameType::fo3).FolderName())
    return GameType::fo3;
  else if (gameType == GameSettings(GameType::fonv).FolderName())
    return GameType::fonv;
  else if (gameType == GameSettings(GameType::fo4).FolderName())
    return GameType::fo4;
  else if (gameType == GameSettings(GameType::fo4vr).FolderName())
    return GameType::fo4vr;
  else
    throw std::runtime_error("Invalid game type value: " + gameType);
}

void to_json(nlohmann::json& json, const MessageType& type) {
  if (type == MessageType::say) {
    json = "say";
  } else if (type == MessageType::warn) {
    json = "warn";
  } else {
    json = "error";
  }
}

void to_json(nlohmann::json& json, const SimpleMessage& message) {
  json = {
      {"type", message.type},
      {"text", message.text},
      {"language", message.language},
      {"condition", message.condition},
  };
}

void from_json(const nlohmann::json& json, SimpleMessage& message) {
  message.type = mapMessageType(json.at("type"));
  message.text = json.at("text").get<std::string>();
  message.language = json.at("language").get<std::string>();
  message.condition = json.at("condition").get<std::string>();
}

void from_json(const nlohmann::json& json, Message& message) {
  if (json.count("text") == 0) {
    throw std::runtime_error("SimpleMessage object has an empty 'text' value");
  }
  if (json.count("language") == 0) {
    throw std::runtime_error(
        "SimpleMessage object has an empty 'language' value");
  }
  if (json.count("type") == 0) {
    throw std::runtime_error("SimpleMessage object has an empty 'type' value");
  }

  auto condition = json.value("condition", "");
  testConditionSyntax("SimpleMessage", condition);

  MessageContent content(json.at("text"), json.at("language"));

  message = Message(mapMessageType(json.at("type")),
                    std::vector<MessageContent>({content}),
                    condition);
}

void to_json(nlohmann::json& json, const Tag& tag) {
  json = {
      {"name", tag.GetName()},
      {"condition", tag.GetCondition()},
      {"isAddition", tag.IsAddition()},
  };
}

void from_json(const nlohmann::json& json, Tag& tag) {
  if (json.count("name") == 0) {
    throw std::runtime_error("Tag object has an empty 'name' value");
  }

  auto condition = json.value("condition", "");

  testConditionSyntax("Tag", condition);

  tag = Tag(json.at("name"), json.value("isAddition", false), condition);
}

void to_json(nlohmann::json& json, const MessageContent& content) {
  json = {
      {"text", content.GetText()},
      {"language", content.GetLanguage()},
  };
}

void from_json(const nlohmann::json& json, MessageContent& content) {
  if (json.count("text") == 0) {
    throw std::runtime_error("MessageContent object has an empty 'text' value");
  }
  if (json.count("language") == 0) {
    throw std::runtime_error(
        "MessageContent object has an empty 'language' value");
  }

  content = MessageContent(json.at("text"), json.at("language"));
}

void to_json(nlohmann::json& json, const PluginCleaningData& data) {
  json = {
      {"crc", data.GetCRC()},
      {"util", data.GetCleaningUtility()},
      {"itm", data.GetITMCount()},
      {"udr", data.GetDeletedReferenceCount()},
      {"nav", data.GetDeletedNavmeshCount()},
      {"detail", data.GetDetail()},
  };
}

void from_json(const nlohmann::json& json, PluginCleaningData& data) {
  if (json.value("crc", 0) == 0) {
    throw std::runtime_error("CleaningData object has a 'crc' value of zero");
  }
  if (json.count("util") == 0) {
    throw std::runtime_error("CleaningData object has an empty 'util' value");
  }

  auto detail = json.value("detail", std::vector<MessageContent>());

  validateMessageContents(detail);

  data = PluginCleaningData(json.at("crc"),
                            json.at("util"),
                            detail,
                            json.value("itm", 0),
                            json.value("udr", 0),
                            json.value("nav", 0));
}

void to_json(nlohmann::json& json, const File& file) {
  json = {
      {"name", file.GetName()},
      {"display", file.GetDisplayName()},
      {"condition", file.GetCondition()},
  };
}

void from_json(const nlohmann::json& json, File& file) {
  if (json.count("name") == 0) {
    throw std::runtime_error("File object has an empty 'name' value");
  }

  auto condition = json.value("condition", "");

  testConditionSyntax("File", condition);

  file = File(json.at("name"), json.value("display", ""), condition);
}

void to_json(nlohmann::json& json, const Group& group) {
  json = {
      {"name", group.GetName()},
      {"after", group.GetAfterGroups()},
  };
}

void from_json(const nlohmann::json& json, Group& group) {
  if (json.count("name") == 0) {
    throw std::runtime_error("Group object has an empty 'name' value");
  }

  group =
      Group(json.at("name"), json.value("after", std::vector<std::string>()));
}

void to_json(nlohmann::json& json, const Location& location) {
  json = {
      {"link", location.GetURL()},
      {"name", location.GetName()},
  };
}

void from_json(const nlohmann::json& json, Location& location) {
  if (json.count("link") == 0) {
    throw std::runtime_error("Location object has an empty 'link' value");
  }

  location = Location(json.at("link"), json.value("name", ""));
}

void to_json(nlohmann::json& json, const FileRevisionSummary& revision) {
  json = {{"id", revision.id}, {"date", revision.date}};
}

void from_json(const nlohmann::json& json, FileRevisionSummary& revision) {
  revision.date = json.at("date");
  revision.id = json.at("id");
}

void to_json(nlohmann::json& json, const GameSettings& game) {
  json = {
      {"type", GameSettings(game.Type()).FolderName()},
      {"name", game.Name()},
      {"master", game.Master()},
      {"registry", game.RegistryKeys()},
      {"folder", game.FolderName()},
      {"repo", game.RepoURL()},
      {"branch", game.RepoBranch()},
      {"path", game.GamePath().u8string()},
      {"localPath", game.GameLocalPath().u8string()},
  };
}

void from_json(const nlohmann::json& json, GameSettings& game) {
  using std::filesystem::u8path;

  auto gameType = mapGameType(json.at("type"));
  game = GameSettings(gameType, json.at("folder"));

  game.SetName(json.value("name", ""));
  game.SetMaster(json.value("master", ""));
  game.SetRegistryKeys(json.value("registry", std::vector<std::string>()));
  game.SetRepoURL(json.value("repo", ""));
  game.SetRepoBranch(json.value("branch", ""));
  game.SetGamePath(u8path(json.value("path", "")));
  game.SetGameLocalPath(u8path(json.value("localPath", "")));
}

void to_json(nlohmann::json& json, const LootSettings::Filters& filters) {
  json = {{"hideVersionNumbers", filters.hideVersionNumbers},
          {"hideCRCs", filters.hideCRCs},
          {"hideBashTags", filters.hideBashTags},
          {"hideNotes", filters.hideNotes},
          {"hideAllPluginMessages", filters.hideAllPluginMessages},
          {"hideInactivePlugins", filters.hideInactivePlugins},
          {"hideMessagelessPlugins", filters.hideMessagelessPlugins}};
}

void to_json(nlohmann::json& json, const LootSettings::Language& language) {
  json = {{"locale", language.locale}, {"name", language.name}};

  if (language.fontFamily.has_value()) {
    json["fontFamily"] = language.fontFamily.value();
  }
}

nlohmann::json to_json_with_language(const PluginMetadata& metadata,
                                     const std::string& language) {
  nlohmann::json json = {{"name", metadata.GetName()},
                         {"after", metadata.GetLoadAfterFiles()},
                         {"req", metadata.GetRequirements()},
                         {"inc", metadata.GetIncompatibilities()},
                         {"msg", metadata.GetSimpleMessages(language)},
                         {"tag", metadata.GetTags()},
                         {"dirty", metadata.GetDirtyInfo()},
                         {"clean", metadata.GetCleanInfo()},
                         {"url", metadata.GetLocations()}};

  if (metadata.GetGroup().has_value()) {
    json["group"] = metadata.GetGroup().value();
  }

  return json;
}

void from_json(const nlohmann::json& json, PluginMetadata& metadata) {
  if (json.count("name") == 0) {
    throw std::runtime_error("PluginMetadata object has an empty 'name' value");
  }

  metadata = PluginMetadata(json.at("name").get<std::string>());

  if (metadata.IsRegexPlugin()) {
    try {
      std::regex(metadata.GetName(),
                 std::regex::ECMAScript | std::regex::icase);
    } catch (std::regex_error& e) {
      throw std::runtime_error(
          std::string(
              "PluginMetadata object has an invalid regex 'name' value: ") +
          e.what());
    }

    if (json.count("dirty") != 0) {
      throw std::runtime_error(
          "PluginMetadata object cannot have a 'dirty' key with a regex "
          "'name' value");
    }
    if (json.count("clean") != 0) {
      throw std::runtime_error(
          "PluginMetadata object cannot have a 'clean' key with a regex "
          "'name' value");
    }
  }

  auto groupIt = json.find("group");
  if (groupIt != json.end()) {
    metadata.SetGroup(groupIt->get<std::string>());
  }

  metadata.SetLoadAfterFiles(json.value("after", std::vector<File>()));
  metadata.SetRequirements(json.value("req", std::vector<File>()));
  metadata.SetIncompatibilities(json.value("inc", std::vector<File>()));
  metadata.SetMessages(json.value("msg", std::vector<Message>()));
  metadata.SetTags(json.value("tag", std::vector<Tag>()));
  metadata.SetDirtyInfo(json.value("dirty", std::vector<PluginCleaningData>()));
  metadata.SetCleanInfo(json.value("clean", std::vector<PluginCleaningData>()));
  metadata.SetLocations(json.value("url", std::vector<Location>()));
}

void to_json(nlohmann::json& json, const DerivedPluginMetadata& plugin) {
  json = {
      {"name", plugin.name},
      {"isActive", plugin.isActive},
      {"isDirty", plugin.isDirty},
      {"isEmpty", plugin.isEmpty},
      {"isMaster", plugin.isMaster},
      {"isLightPlugin", plugin.isLightPlugin},
      {"loadsArchive", plugin.loadsArchive},
      {"messages", plugin.messages},
      {"suggestedTags", plugin.suggestedTags},
      {"currentTags", plugin.currentTags},
  };

  if (plugin.version.has_value()) {
    json["version"] = plugin.version.value();
  }

  if (plugin.crc.has_value()) {
    json["crc"] = plugin.crc.value();
  }

  if (plugin.group.has_value()) {
    json["group"] = plugin.group.value();
  }

  if (plugin.loadOrderIndex.has_value()) {
    json["loadOrderIndex"] = plugin.loadOrderIndex.value();
  }

  if (!plugin.cleanedWith.empty()) {
    json["cleanedWith"] = plugin.cleanedWith;
  }

  if (plugin.masterlistMetadata.has_value()) {
    json["masterlist"] = to_json_with_language(
        plugin.masterlistMetadata.value(), plugin.language);
  }

  if (plugin.userMetadata.has_value()) {
    json["userlist"] =
        to_json_with_language(plugin.userMetadata.value(), plugin.language);
  }
}

void from_json(const nlohmann::json& json, DerivedPluginMetadata& plugin) {
  plugin.name = json.at("name").get<std::string>();
  plugin.isActive = json.at("isActive").get<bool>();
  plugin.isDirty = json.at("isDirty").get<bool>();
  plugin.isEmpty = json.at("isEmpty").get<bool>();
  plugin.isMaster = json.at("isMaster").get<bool>();
  plugin.isLightPlugin = json.at("isLightPlugin").get<bool>();
  plugin.loadsArchive = json.at("loadsArchive").get<bool>();
  plugin.messages = json.at("messages").get<std::vector<SimpleMessage>>();
  plugin.suggestedTags = json.at("suggestedTags").get<std::vector<Tag>>();
  plugin.currentTags = json.at("currentTags").get<std::vector<Tag>>();

  auto versionIt = json.find("version");
  if (versionIt != json.end()) {
    plugin.version = versionIt->get<std::string>();
  }

  auto crcIt = json.find("crc");
  if (crcIt != json.end()) {
    plugin.crc = crcIt->get<uint32_t>();
  }

  auto groupIt = json.find("group");
  if (groupIt != json.end()) {
    plugin.group = groupIt->get<std::string>();
  }

  auto loadOrderIndexIt = json.find("loadOrderIndex");
  if (loadOrderIndexIt != json.end()) {
    plugin.loadOrderIndex = loadOrderIndexIt->get<short>();
  }

  auto cleanedWithIt = json.find("cleanedWith");
  if (cleanedWithIt != json.end()) {
    plugin.cleanedWith = cleanedWithIt->get<std::string>();
  }

  // userlist and masterlist are not deserialised because they cannot be
  // losslessly round-tripped, and having something is probably more confusing
  // than having nothing.
}
}

#endif
