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

#ifndef LOOT_GUI_QUERY_EDITOR_CLOSED_QUERY
#define LOOT_GUI_QUERY_EDITOR_CLOSED_QUERY

#include "gui/state/loot_state.h"
#include "gui/query/metadata_query.h"

namespace loot {
class EditorClosedQuery : public MetadataQuery {
public:
  EditorClosedQuery(LootState& state, YAML::Node metadata) :
    MetadataQuery(state),
    state_(state), metadata_(metadata) {}

  std::string executeLogic() {
    try {
      const std::string json = applyUserEdits();
      state_.decrementUnappliedChangeCounter();

      return json;
    } catch (YAML::RepresentationException& e) {
      throw YAML::RepresentationException(YAML::Mark::null_mark(), e.msg);
    }
  }

private:
  static PluginMetadata convertMetadata(YAML::Node& newMetadata, PluginMetadata& existingUserMetadata) {
    PluginMetadata newUserlistEntry(existingUserMetadata.GetName());

    // First sort out the priority value. This is only given if it was changed.
    BOOST_LOG_TRIVIAL(trace) << "Calculating userlist metadata local priority value from Javascript variables.";
    if (newMetadata["priority"]) {
      BOOST_LOG_TRIVIAL(trace) << "Local priority value was changed, recalculating...";
      // Priority value was changed, so add it to the userlist data.
      newUserlistEntry.SetLocalPriority(Priority(newMetadata["priority"].as<int>()));
    } else {
      // Priority value wasn't changed, use the existing userlist value.
      BOOST_LOG_TRIVIAL(trace) << "Local priority value is unchanged, using existing userlist value (if it exists).";
      newUserlistEntry.SetLocalPriority(existingUserMetadata.GetLocalPriority());
    }

    if (newMetadata["globalPriority"]) {
      BOOST_LOG_TRIVIAL(trace) << "Global priority value was changed, recalculating...";
      // Priority value was changed, so add it to the userlist data.
      newUserlistEntry.SetGlobalPriority(Priority(newMetadata["globalPriority"].as<int>()));
    } else {
      BOOST_LOG_TRIVIAL(trace) << "Global priority value is unchanged, using existing userlist value (if it exists).";
      newUserlistEntry.SetGlobalPriority(existingUserMetadata.GetGlobalPriority());
    }

    // Now the enabled flag.
    newUserlistEntry.SetEnabled(newMetadata["userlist"]["enabled"].as<bool>());

    // Now metadata lists. These are given in their entirety, so replace anything that
    // currently exists.
    BOOST_LOG_TRIVIAL(trace) << "Recording metadata lists from Javascript variables.";
    if (newMetadata["userlist"]["after"])
      newUserlistEntry.SetLoadAfterFiles(newMetadata["userlist"]["after"].as<std::set<File>>());
    if (newMetadata["userlist"]["req"])
      newUserlistEntry.SetRequirements(newMetadata["userlist"]["req"].as<std::set<File>>());
    if (newMetadata["userlist"]["inc"])
      newUserlistEntry.SetIncompatibilities(newMetadata["userlist"]["inc"].as<std::set<File>>());

    if (newMetadata["userlist"]["msg"])
      newUserlistEntry.SetMessages(toMessages(newMetadata["userlist"]["msg"].as<std::vector<EditorMessage>>()));
    if (newMetadata["userlist"]["tag"])
      newUserlistEntry.SetTags(newMetadata["userlist"]["tag"].as<std::set<Tag>>());
    if (newMetadata["userlist"]["dirty"])
      newUserlistEntry.SetDirtyInfo(newMetadata["userlist"]["dirty"].as<std::set<PluginCleaningData>>());
    if (newMetadata["userlist"]["clean"])
      newUserlistEntry.SetCleanInfo(newMetadata["userlist"]["clean"].as<std::set<PluginCleaningData>>());
    if (newMetadata["userlist"]["url"])
      newUserlistEntry.SetLocations(newMetadata["userlist"]["url"].as<std::set<Location>>());

    return newUserlistEntry;
  }

  PluginMetadata getUniqueMetadata(const PluginMetadata& metadata) {
    BOOST_LOG_TRIVIAL(trace) << "Removing any user metadata that duplicates masterlist metadata.";
    try {
      auto plugin = state_.getCurrentGame().GetPlugin(metadata.GetName());
      auto masterlistMetadata = state_.getCurrentGame().GetMasterlistMetadata(metadata.GetName());
      auto nonUserMetadata = getNonUserMetadata(plugin, masterlistMetadata);
      return metadata.NewMetadata(nonUserMetadata);
    } catch (...) {
      return metadata.NewMetadata(state_.getCurrentGame().GetMasterlistMetadata(metadata.GetName()));
    }
  }

  std::string applyUserEdits() {
    if (!metadata_.IsMap())  // No edits to apply.
      return "null";

    const std::string pluginName = metadata_["name"].as<std::string>();

    BOOST_LOG_TRIVIAL(trace) << "Applying user edits for: " << pluginName;

    // Find existing userlist entry.
    PluginMetadata ulistPlugin = state_.getCurrentGame().GetUserMetadata(pluginName);

    // Create new object for userlist entry.
    PluginMetadata newMetadata = getUniqueMetadata(convertMetadata(metadata_, ulistPlugin));

    // Now erase any existing userlist entry.
    if (!ulistPlugin.HasNameOnly()) {
      BOOST_LOG_TRIVIAL(trace) << "Erasing the existing userlist entry.";
      state_.getCurrentGame().ClearUserMetadata(ulistPlugin.GetName());
    }

    // Add a new userlist entry if necessary.
    if (!newMetadata.HasNameOnly()) {
      BOOST_LOG_TRIVIAL(trace) << "Adding new metadata to new userlist entry.";
      state_.getCurrentGame().AddUserMetadata(newMetadata);
    }

    // Save edited userlist.
    state_.getCurrentGame().SaveUserMetadata();

    // Now rederive the derived metadata.
    BOOST_LOG_TRIVIAL(trace) << "Returning newly derived display metadata.";
    YAML::Node derivedMetadata = generateDerivedMetadata(newMetadata.GetName()).toYaml();
    if (derivedMetadata.size() > 0)
      return JSON::stringify(derivedMetadata);
    else
      return "null";
  }

  static std::vector<Message> toMessages(const std::vector<EditorMessage>& messages) {
    std::vector<Message> list;

    for (const auto& message : messages) {
      list.push_back(Message(
        message.type,
        {{message.text, message.language}},
        message.condition));
    }

    return list;
  }

  LootState& state_;
  YAML::Node metadata_;
};
}

#endif
