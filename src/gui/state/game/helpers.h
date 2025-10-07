/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

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

#include <loot/enum/game_type.h>
#include <loot/metadata/message.h>
#include <loot/metadata/plugin_cleaning_data.h>
#include <loot/metadata/tag.h>
#include <loot/vertex.h>

#include <filesystem>
#include <tuple>
#include <vector>

#include "gui/sourced_message.h"
#include "gui/state/game/game_id.h"
#include "gui/state/game/load_order_backup.h"

namespace loot {
void backupLoadOrder(const std::vector<std::string>& loadOrder,
                     const std::filesystem::path& backupDirectory);

void backupLoadOrder(const std::vector<std::string>& loadOrder,
                     const std::filesystem::path& backupDirectory,
                     std::string_view name);

std::vector<LoadOrderBackup> findLoadOrderBackups(
    const std::filesystem::path& backupDirectory);

// Escape any Markdown special characters in the input text.
std::string escapeMarkdownASCIIPunctuation(const std::string& text);

std::string describeCycle(const std::vector<Vertex>& cycle);

std::vector<std::string> checkForRemovedPlugins(
    const std::vector<std::string>& pluginNamesBefore,
    const std::vector<std::string>& pluginNamesAfter);

std::vector<Tag> readBashTagsFile(std::istream& in);

std::vector<Tag> readBashTagsFile(const std::filesystem::path& dataPath,
                                  const std::string& pluginName);

// Return a list of tag names that are added by one source but removed by the
// other.
std::vector<std::string> getTagConflicts(const std::vector<Tag>& tags1,
                                         const std::vector<Tag>& tags2);

bool hasPluginFileExtension(const std::string& filename);

std::optional<std::filesystem::path> resolveGameFilePath(
    GameId gameId,
    const std::vector<std::filesystem::path>& externalDataPaths,
    const std::filesystem::path& dataPath,
    const std::string& filename);

bool isOfficialPlugin(const GameId gameId, const std::string& pluginName);
}

#endif
