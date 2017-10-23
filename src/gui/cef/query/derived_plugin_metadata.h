/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2017    WrinklyNinja

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

#ifndef LOOT_GUI_QUERY_DERIVED_PLUGIN_METADATA
#define LOOT_GUI_QUERY_DERIVED_PLUGIN_METADATA

#include "gui/yaml_simple_message_helpers.h"

namespace loot {
class DerivedPluginMetadata {
public:
  DerivedPluginMetadata(LootState& state,
                        const std::shared_ptr<const PluginInterface>& file,
                        const PluginMetadata& evaluatedMetadata) {
    name = file->GetName();
    version = file->GetVersion();
    isActive = state.getCurrentGame().IsPluginActive(name);
    isDirty = !evaluatedMetadata.GetDirtyInfo().empty();
    isEmpty = file->IsEmpty();
    isMaster = file->IsMaster();
    loadsArchive = file->LoadsArchive();

    crc = file->GetCRC();
    loadOrderIndex = state.getCurrentGame().GetActiveLoadOrderIndex(file,
      state.getCurrentGame().GetLoadOrder());

    priority = evaluatedMetadata.GetLocalPriority().GetValue();
    globalPriority = evaluatedMetadata.GetGlobalPriority().GetValue();
    if (!evaluatedMetadata.GetCleanInfo().empty()) {
      cleanedWith = evaluatedMetadata.GetCleanInfo().begin()->GetCleaningUtility();
    }
    messages = evaluatedMetadata.GetSimpleMessages(state.getLanguage());
    tags = evaluatedMetadata.GetTags();

    language = state.getLanguage();
  }

  void storeUnevaluatedMetadata(PluginMetadata masterlistEntry, PluginMetadata userlistEntry) {
    masterlistMetadata = masterlistEntry;
    userMetadata = userlistEntry;
  }

  static DerivedPluginMetadata none() {
    return DerivedPluginMetadata();
  }

  bool isNone() {
    return name.empty();
  }

  YAML::Node toYaml() const {
    YAML::Node pluginNode;

    pluginNode["__type"] = "Plugin";  // For conversion back into a JS typed object.
    pluginNode["name"] = name;
    pluginNode["version"] = version;
    pluginNode["isActive"] = isActive;
    pluginNode["isDirty"] = isDirty;
    pluginNode["isEmpty"] = isEmpty;
    pluginNode["isMaster"] = isMaster;
    pluginNode["loadsArchive"] = loadsArchive;

    pluginNode["crc"] = crc;
    pluginNode["loadOrderIndex"] = loadOrderIndex;

    pluginNode["priority"] = priority;
    pluginNode["globalPriority"] = globalPriority;
    pluginNode["cleanedWith"] = cleanedWith;
    pluginNode["messages"] = messages;
    pluginNode["tags"] = tags;

    if (!masterlistMetadata.HasNameOnly()) {
      pluginNode["masterlist"] = toYaml(masterlistMetadata, language);
    }

    if (!userMetadata.HasNameOnly()) {
      pluginNode["userlist"] = toYaml(userMetadata, language);
    }

    return pluginNode;
  }

private:
  std::string name;
  std::string version;
  bool isActive;
  bool isDirty;
  bool isEmpty;
  bool isMaster;
  bool loadsArchive;

  uint32_t crc;
  short loadOrderIndex;

  short priority;
  short globalPriority;
  std::string cleanedWith;
  std::vector<SimpleMessage> messages;
  std::set<Tag> tags;

  PluginMetadata masterlistMetadata;
  PluginMetadata userMetadata;

  std::string language;

  DerivedPluginMetadata() {}

  static std::vector<EditorMessage> toEditorMessages(const std::vector<Message>& messages, const std::string& language) {
    std::vector<EditorMessage> list;

    for (const auto& message : messages) {
      list.push_back(EditorMessage(message, language));
    }

    return list;
  }

  static YAML::Node toYaml(const PluginMetadata& metadata, const std::string& language) {
    YAML::Node node;

    node["enabled"] = metadata.IsEnabled();
    node["priority"] = metadata.GetLocalPriority().GetValue();
    node["globalPriority"] = metadata.GetGlobalPriority().GetValue();
    node["after"] = metadata.GetLoadAfterFiles();
    node["req"] = metadata.GetRequirements();
    node["inc"] = metadata.GetIncompatibilities();
    node["msg"] = toEditorMessages(metadata.GetMessages(), language);
    node["tag"] = metadata.GetTags();
    node["dirty"] = metadata.GetDirtyInfo();
    node["clean"] = metadata.GetCleanInfo();
    node["url"] = metadata.GetLocations();

    return node;
  }
};
}

#endif
