/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2018    WrinklyNinja

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

#include <optional>

#include <json.hpp>
#include <loot/api.h>

#include "gui/state/loot_state.h"

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
    isLightMaster = file->IsLightMaster();
    loadsArchive = file->LoadsArchive();

    crc = file->GetCRC();
    loadOrderIndex = state.getCurrentGame().GetActiveLoadOrderIndex(file,
      state.getCurrentGame().GetLoadOrder());

    group = evaluatedMetadata.GetGroup();
    if (!evaluatedMetadata.GetCleanInfo().empty()) {
      cleanedWith = evaluatedMetadata.GetCleanInfo().begin()->GetCleaningUtility();
    }
    messages = evaluatedMetadata.GetSimpleMessages(state.getLanguage());
    suggestedTags = evaluatedMetadata.GetTags();
    currentTags = file->GetBashTags();

    language = state.getLanguage();
  }

  void storeUnevaluatedMetadata(PluginMetadata masterlistEntry, PluginMetadata userlistEntry) {
    masterlistMetadata = masterlistEntry;
    userMetadata = userlistEntry;
  }

  void setLoadOrderIndex(short loadOrderIndex) {
    this->loadOrderIndex = loadOrderIndex;
  }

  static DerivedPluginMetadata none() {
    return DerivedPluginMetadata();
  }
private:
  std::string name;
  std::string version;
  bool isActive;
  bool isDirty;
  bool isEmpty;
  bool isMaster;
  bool isLightMaster;
  bool loadsArchive;

  uint32_t crc;
  std::optional<short> loadOrderIndex;

  std::string group;
  std::string cleanedWith;
  std::vector<SimpleMessage> messages;
  std::set<Tag> currentTags;
  std::set<Tag> suggestedTags;

  PluginMetadata masterlistMetadata;
  PluginMetadata userMetadata;

  std::string language;

  DerivedPluginMetadata() {}

  friend void to_json(nlohmann::json& json, const DerivedPluginMetadata& plugin);
};
}

#endif
