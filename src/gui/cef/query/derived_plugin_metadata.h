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

#ifndef LOOT_GUI_QUERY_DERIVED_PLUGIN_METADATA
#define LOOT_GUI_QUERY_DERIVED_PLUGIN_METADATA

#include <optional>

#include <loot/api.h>
#include <json.hpp>

#include "gui/state/game/game.h"

namespace loot {
template<typename G>
class DerivedPluginMetadata {
public:
  DerivedPluginMetadata(const std::shared_ptr<const PluginInterface>& file,
                        const G& game,
                        std::string language) :
      name(file->GetName()),
      version(file->GetVersion()),
      isActive(game.IsPluginActive(file->GetName())),
      isDirty(false),
      isEmpty(file->IsEmpty()),
      isMaster(file->IsMaster()),
      isLightPlugin(file->IsLightPlugin()),
      loadsArchive(file->LoadsArchive()),
      crc(file->GetCRC()),
      loadOrderIndex(game.GetActiveLoadOrderIndex(file, game.GetLoadOrder())),
      currentTags(file->GetBashTags()),
      language(language) {}

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
  bool isLightPlugin;
  bool loadsArchive;

  std::optional<uint32_t> crc;
  std::optional<short> loadOrderIndex;

  std::optional<std::string> group;
  std::string cleanedWith;
  std::vector<SimpleMessage> messages;
  std::vector<Tag> currentTags;
  std::vector<Tag> suggestedTags;

  std::optional<PluginMetadata> masterlistMetadata;
  std::optional<PluginMetadata> userMetadata;

  std::string language;

  template<typename T>
  friend void to_json(nlohmann::json& json,
                      const DerivedPluginMetadata<T>& plugin);
};
}

#endif
