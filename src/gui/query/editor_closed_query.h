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
  EditorClosedQuery(LootState& state, bool applyEdits, PluginMetadata metadata) :
    MetadataQuery(state),
    state_(state), applyEdits_(applyEdits), metadata_(metadata) {}

  std::string executeLogic() {
    if (applyEdits_) {
      applyUserEdits();
    }
    state_.decrementUnappliedChangeCounter();

    auto response = generateDerivedMetadata(metadata_.GetName()).toYaml();
    return JSON::stringify(response);
  }

private:
  PluginMetadata getNonUserMetadata() {
    BOOST_LOG_TRIVIAL(trace) << "Getting non-user metadata for: " << metadata_.GetName();
    auto masterlistMetadata = state_.getCurrentGame().GetMasterlistMetadata(metadata_.GetName());

    try {
      auto plugin = state_.getCurrentGame().GetPlugin(metadata_.GetName());
      return MetadataQuery::getNonUserMetadata(plugin, masterlistMetadata);
    } catch (...) {}

    return masterlistMetadata;
  }

  PluginMetadata getUserMetadata() {
    auto nonUserMetadata = getNonUserMetadata();
    auto userMetadata = metadata_.NewMetadata(nonUserMetadata);

    if (metadata_.GetLocalPriority().GetValue() != nonUserMetadata.GetLocalPriority().GetValue()) {
      userMetadata.SetLocalPriority(metadata_.GetLocalPriority());
    } else {
      userMetadata.SetLocalPriority(Priority());
    }

    if (metadata_.GetGlobalPriority().GetValue() != nonUserMetadata.GetGlobalPriority().GetValue()) {
      userMetadata.SetGlobalPriority(metadata_.GetGlobalPriority());
    } else {
      userMetadata.SetGlobalPriority(Priority());
    }

    return userMetadata;
  }

  void applyUserEdits() {
    BOOST_LOG_TRIVIAL(trace) << "Applying user edits for: " << metadata_.GetName();

    // Determine what metadata in the response is user-added.
    auto userMetadata = getUserMetadata();

    // Now erase any existing userlist entry.
    BOOST_LOG_TRIVIAL(trace) << "Erasing the existing userlist entry.";
    state_.getCurrentGame().ClearUserMetadata(userMetadata.GetName());

    // Add a new userlist entry if necessary.
    if (!userMetadata.HasNameOnly()) {
      BOOST_LOG_TRIVIAL(trace) << "Adding new metadata to new userlist entry.";
      state_.getCurrentGame().AddUserMetadata(userMetadata);
    }

    // Save edited userlist.
    state_.getCurrentGame().SaveUserMetadata();
  }

  LootState& state_;
  const bool applyEdits_;
  const PluginMetadata metadata_;
};
}

#endif
