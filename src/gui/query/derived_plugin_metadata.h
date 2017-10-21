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

namespace loot {
class DerivedPluginMetadata {
public:
  DerivedPluginMetadata(LootState& state,
                        const std::shared_ptr<const PluginInterface>& file,
                        const PluginMetadata& metadata) {
    name = file->GetName();
    priority = metadata.GetLocalPriority().GetValue();
    globalPriority = metadata.GetGlobalPriority().GetValue();
    messages = metadata.GetSimpleMessages(state.getLanguage());
    tags = metadata.GetTags();
    isDirty = !metadata.GetDirtyInfo().empty();
    loadOrderIndex = state.getCurrentGame().GetActiveLoadOrderIndex(file, 
      state.getCurrentGame().GetLoadOrder());

    if (!metadata.GetCleanInfo().empty()) {
      cleanedWith = metadata.GetCleanInfo().begin()->GetCleaningUtility();
    }
  }
    
  static DerivedPluginMetadata none() {
    return DerivedPluginMetadata();
  }

  bool isNone() {
    return name.empty();
  }

  YAML::Node toYaml() const {
    YAML::Node pluginNode;

    pluginNode["name"] = name;
    pluginNode["priority"] = priority;
    pluginNode["globalPriority"] = globalPriority;
    pluginNode["messages"] = messages;
    pluginNode["tags"] = tags;
    pluginNode["isDirty"] = isDirty;
    pluginNode["loadOrderIndex"] = loadOrderIndex;
    pluginNode["cleanedWith"] = cleanedWith;
    
    return pluginNode;
  }

private:
  std::string name;
  short priority;
  short globalPriority;
  std::vector<SimpleMessage> messages;
  std::set<Tag> tags;
  bool isDirty;
  short loadOrderIndex;
  std::string cleanedWith;

  DerivedPluginMetadata() {}
};
}

#endif