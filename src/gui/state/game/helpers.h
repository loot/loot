/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2018    Oliver Hamlet

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

#ifndef LOOT_GUI_STATE_GAME_HELPERS
#define LOOT_GUI_STATE_GAME_HELPERS

#include <filesystem>
#include <vector>

#include <loot/enum/game_type.h>
#include <loot/metadata/message.h>
#include <loot/metadata/plugin_cleaning_data.h>
#include <loot/vertex.h>

namespace loot {
bool ExecutableExists(const GameType& gameType,
                      const std::filesystem::path& gamePath);

void BackupLoadOrder(const std::vector<std::string>& loadOrder,
                     const std::filesystem::path& backupDirectory);

Message ToMessage(const PluginCleaningData& cleaningData);

std::string DescribeCycle(const std::vector<Vertex>& cycle);

std::vector<Message> CheckForRemovedPlugins(
    const std::vector<std::string> pluginsBefore,
    const std::vector<std::string> pluginsAfter);
}

#endif
