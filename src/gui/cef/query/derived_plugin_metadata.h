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
                        const std::shared_ptr<const PluginInterface>& file) {
    name = file->GetName();
    version = file->GetVersion();
    isActive = state.getCurrentGame().IsPluginActive(name);
    isEmpty = file->IsEmpty();
    isMaster = file->IsMaster();
    isLightMaster = file->IsLightMaster();
    loadsArchive = file->LoadsArchive();

    crc = file->GetCRC();
    loadOrderIndex = state.getCurrentGame().GetActiveLoadOrderIndex(file,
      state.getCurrentGame().GetLoadOrder());

    currentTags = file->GetBashTags();

    language = state.getLanguage();
  }

  void setEvaluatedMetadata(PluginMetadata metadata) {
    isDirty = !metadata.GetDirtyInfo().empty();
    group = metadata.GetGroup();
    if (!metadata.GetCleanInfo().empty()) {
      cleanedWith = metadata.GetCleanInfo().begin()->GetCleaningUtility();
    }
    messages = metadata.GetSimpleMessages(language);
    suggestedTags = metadata.GetTags();
  }

  void setMasterlistMetadata(PluginMetadata masterlistEntry) {
    this->masterlistMetadata = masterlistEntry;
  }

  void setUserMetadata(PluginMetadata userlistEntry) {
    this->userMetadata = userlistEntry;
  }

  void setLoadOrderIndex(short loadOrderIndex) {
    this->loadOrderIndex = loadOrderIndex;
  }

private:
  std::string name;
  std::optional<std::string> version;
  bool isActive;
  bool isDirty;
  bool isEmpty;
  bool isMaster;
  bool isLightMaster;
  bool loadsArchive;

  std::optional<uint32_t> crc;
  std::optional<short> loadOrderIndex;

  std::optional<std::string> group;
  std::string cleanedWith;
  std::vector<SimpleMessage> messages;
  std::set<Tag> currentTags;
  std::set<Tag> suggestedTags;

  std::optional<PluginMetadata> masterlistMetadata;
  std::optional<PluginMetadata> userMetadata;

  std::string language;

  friend void to_json(nlohmann::json& json, const DerivedPluginMetadata& plugin);
};
}

#endif
