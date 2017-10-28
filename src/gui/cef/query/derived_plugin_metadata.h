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

#undef ERROR

#include "gui/cef/query/protobuf.h"
#include "schema/query.pb.h"

namespace loot {
class DerivedPluginMetadata {
public:
  DerivedPluginMetadata(LootState& state,
                        const std::shared_ptr<const PluginInterface>& file,
                        const PluginMetadata& evaluatedMetadata) {
    plugin.set_name(file->GetName());
    plugin.set_version(file->GetVersion());
    plugin.set_is_active(state.getCurrentGame().IsPluginActive(file->GetName()));
    plugin.set_is_dirty(!evaluatedMetadata.GetDirtyInfo().empty());
    plugin.set_is_empty(file->IsEmpty());
    plugin.set_is_master(file->IsMaster());
    plugin.set_loads_archive(file->LoadsArchive());

    plugin.set_crc(file->GetCRC());
    auto loadOrderIndex = state.getCurrentGame().GetActiveLoadOrderIndex(file,
      state.getCurrentGame().GetLoadOrder());
    plugin.set_load_order_index(loadOrderIndex);

    plugin.set_priority(evaluatedMetadata.GetLocalPriority().GetValue());
    plugin.set_global_priority(evaluatedMetadata.GetGlobalPriority().GetValue());

    if (!evaluatedMetadata.GetCleanInfo().empty()) {
      auto utility = evaluatedMetadata.GetCleanInfo().begin()->GetCleaningUtility();
      plugin.set_cleaned_with(utility);
    }

    set(plugin, evaluatedMetadata.GetSimpleMessages(state.getLanguage()));
    set(plugin, evaluatedMetadata.GetTags());
  }

  void storeUnevaluatedMetadata(PluginMetadata masterlistEntry,
                                PluginMetadata userlistEntry,
                                const std::string& language) {
    if (!masterlistEntry.HasNameOnly()) {
      *plugin.mutable_masterlist() = convert(masterlistEntry, language);
    }

    if (!userlistEntry.HasNameOnly()) {
      *plugin.mutable_userlist() = convert(userlistEntry, language);
    }
  }

  static DerivedPluginMetadata none() {
    return DerivedPluginMetadata();
  }

  protobuf::Plugin toProtobuf() const {
    return plugin;
  }

private:
  protobuf::Plugin plugin;

  DerivedPluginMetadata() {}

  static void set(protobuf::Plugin& plugin, const std::vector<SimpleMessage>& messages) {
    for (const auto& message : messages) {
      auto pbMessage = plugin.add_messages();
      convert(message, *pbMessage);
    }
  }

  static void set(protobuf::Plugin& plugin, const std::set<Tag>& tags) {
    for (const auto& tag : tags) {
      auto pbTag = plugin.add_tags();
      convert(tag, *pbTag);
    }
  }
};
}

#endif
