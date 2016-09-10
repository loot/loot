/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2016    WrinklyNinja

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

#include "backend/app/loot_state.h"
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
    } catch (Error&) {
      throw;
    } catch (std::exception& e) {
      // If this was a YAML conversion error, cut off the line and column numbers,
      // since the YAML wasn't written to a file.
      std::string error = e.what();
      size_t pos = std::string::npos;
      if ((pos = error.find("bad conversion")) != std::string::npos) {
        error = error.substr(pos);
      }

      throw std::runtime_error(error);
    }
  }

private:
  static PluginMetadata convertMetadata(YAML::Node& newMetadata, PluginMetadata& existingUserMetadata) {
    PluginMetadata newUserlistEntry(existingUserMetadata.Name());

    // First sort out the priority value. This is only given if it was changed.
    BOOST_LOG_TRIVIAL(trace) << "Calculating userlist metadata local priority value from Javascript variables.";
    if (newMetadata["priority"]) {
      BOOST_LOG_TRIVIAL(trace) << "Local priority value was changed, recalculating...";
      // Priority value was changed, so add it to the userlist data.
      newUserlistEntry.LocalPriority(Priority(newMetadata["priority"].as<int>()));
    } else {
      // Priority value wasn't changed, use the existing userlist value.
      BOOST_LOG_TRIVIAL(trace) << "Local priority value is unchanged, using existing userlist value (if it exists).";
      newUserlistEntry.LocalPriority(existingUserMetadata.LocalPriority());
    }

    if (newMetadata["globalPriority"]) {
      BOOST_LOG_TRIVIAL(trace) << "Global priority value was changed, recalculating...";
      // Priority value was changed, so add it to the userlist data.
      newUserlistEntry.GlobalPriority(Priority(newMetadata["globalPriority"].as<int>()));
    } else {
      BOOST_LOG_TRIVIAL(trace) << "Global priority value is unchanged, using existing userlist value (if it exists).";
      newUserlistEntry.GlobalPriority(existingUserMetadata.GlobalPriority());
    }

    // Now the enabled flag.
    newUserlistEntry.Enabled(newMetadata["userlist"]["enabled"].as<bool>());

    // Now metadata lists. These are given in their entirety, so replace anything that
    // currently exists.
    BOOST_LOG_TRIVIAL(trace) << "Recording metadata lists from Javascript variables.";
    if (newMetadata["userlist"]["after"])
      newUserlistEntry.LoadAfter(newMetadata["userlist"]["after"].as<std::set<File>>());
    if (newMetadata["userlist"]["req"])
      newUserlistEntry.Reqs(newMetadata["userlist"]["req"].as<std::set<File>>());
    if (newMetadata["userlist"]["inc"])
      newUserlistEntry.Incs(newMetadata["userlist"]["inc"].as<std::set<File>>());

    if (newMetadata["userlist"]["msg"])
      newUserlistEntry.Messages(toMessages(newMetadata["userlist"]["msg"].as<std::vector<EditorMessage>>()));
    if (newMetadata["userlist"]["tag"])
      newUserlistEntry.Tags(newMetadata["userlist"]["tag"].as<std::set<Tag>>());
    if (newMetadata["userlist"]["dirty"])
      newUserlistEntry.DirtyInfo(newMetadata["userlist"]["dirty"].as<std::set<PluginCleaningData>>());
    if (newMetadata["userlist"]["clean"])
      newUserlistEntry.CleanInfo(newMetadata["userlist"]["clean"].as<std::set<PluginCleaningData>>());
    if (newMetadata["userlist"]["url"])
      newUserlistEntry.Locations(newMetadata["userlist"]["url"].as<std::set<Location>>());

    return newUserlistEntry;
  }

  PluginMetadata getUniqueMetadata(const PluginMetadata& metadata) {
    BOOST_LOG_TRIVIAL(trace) << "Removing any user metadata that duplicates masterlist metadata.";
    try {
      Plugin tempPlugin(state_.getCurrentGame().GetPlugin(metadata.Name()));
      tempPlugin.MergeMetadata(state_.getCurrentGame().GetMasterlist().FindPlugin(metadata));
      return metadata.NewMetadata(tempPlugin);
    } catch (...) {
      return metadata.NewMetadata(state_.getCurrentGame().GetMasterlist().FindPlugin(metadata));
    }
  }

  std::string applyUserEdits() {
    if (!metadata_.IsMap())  // No edits to apply.
      return "null";

    const std::string pluginName = metadata_["name"].as<std::string>();

    BOOST_LOG_TRIVIAL(trace) << "Applying user edits for: " << pluginName;

    // Find existing userlist entry.
    PluginMetadata ulistPlugin = state_.getCurrentGame().GetUserlist().FindPlugin(pluginName);

    // Create new object for userlist entry.
    PluginMetadata newMetadata = getUniqueMetadata(convertMetadata(metadata_, ulistPlugin));

    // Now erase any existing userlist entry.
    if (!ulistPlugin.HasNameOnly()) {
      BOOST_LOG_TRIVIAL(trace) << "Erasing the existing userlist entry.";
      state_.getCurrentGame().GetUserlist().ErasePlugin(ulistPlugin);
    }

    // Add a new userlist entry if necessary.
    if (!newMetadata.HasNameOnly()) {
      BOOST_LOG_TRIVIAL(trace) << "Adding new metadata to new userlist entry.";
      state_.getCurrentGame().GetUserlist().AddPlugin(newMetadata);
    }

    // Save edited userlist.
    state_.getCurrentGame().GetUserlist().Save(state_.getCurrentGame().UserlistPath());

    // Now rederive the derived metadata.
    BOOST_LOG_TRIVIAL(trace) << "Returning newly derived display metadata.";
    YAML::Node derivedMetadata = generateDerivedMetadata(newMetadata.Name());
    if (derivedMetadata.size() > 0)
      return JSON::stringify(derivedMetadata);
    else
      return "null";
  }

  static std::vector<Message> toMessages(std::vector<EditorMessage> messages) {
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
