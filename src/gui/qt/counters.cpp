/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2021    Oliver Hamlet

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

#include "gui/qt/counters.h"

#include "gui/state/game/helpers.h"

namespace loot {
GeneralInformationCounters::GeneralInformationCounters(
    const std::vector<SourcedMessage>& generalMessages,
    const std::vector<PluginItem>& plugins) {
  countMessages(generalMessages);

  totalPlugins = plugins.size();

  for (const auto& plugin : plugins) {
    if (plugin.isActive) {
      if (plugin.isLightPlugin) {
        activeLight += 1;
      } else if (plugin.isMediumPlugin) {
        activeMedium += 1;
      } else {
        activeFull += 1;
      }
    }
    if (plugin.isDirty) {
      dirty += 1;
    }

    countMessages(plugin.messages);
  }
}

void GeneralInformationCounters::countMessages(
    const std::vector<SourcedMessage>& messages) {
  for (const auto& message : messages) {
    if (message.type == MessageType::warn) {
      warnings += 1;
    } else if (message.type == MessageType::error) {
      errors += 1;
    }
  }

  totalMessages += messages.size();
}

bool shouldFilterMessage(const std::string& pluginName,
                         const SourcedMessage& message,
                         const CardContentFiltersState& filters) {
  if (message.type == MessageType::say && filters.hideNotes) {
    return true;
  }

  if (filters.hideOfficialPluginsCleaningMessages &&
      message.source == MessageSource::cleaningMetadata &&
      IsOfficialPlugin(filters.gameId, pluginName)) {
    return true;
  }

  return false;
}

size_t countHiddenMessages(const std::vector<PluginItem>& plugins,
                           const CardContentFiltersState& filters) {
  size_t hidden = 0;

  for (const auto& plugin : plugins) {
    if (filters.hideAllPluginMessages) {
      hidden += plugin.messages.size();
      continue;
    }

    hidden += std::count_if(plugin.messages.begin(),
                            plugin.messages.end(),
                            [&](const SourcedMessage& message) {
                              return shouldFilterMessage(
                                  plugin.name, message, filters);
                            });
  }

  return hidden;
}
}
