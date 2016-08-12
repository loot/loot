/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2016    WrinklyNinja

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

#ifndef LOOT_BACKEND_METADATA_LIST
#define LOOT_BACKEND_METADATA_LIST

#include <string>
#include <unordered_set>
#include <vector>

#include <boost/filesystem.hpp>

#include "backend/metadata/plugin_metadata.h"

namespace loot {
class Game;

class MetadataList {
public:
  void Load(const boost::filesystem::path& filepath);
  void Save(const boost::filesystem::path& filepath);
  void Clear();

  std::list<PluginMetadata> Plugins() const;
  std::list<Message> Messages() const;
  std::set<std::string> BashTags() const;

  // Merges multiple matching regex entries if any are found.
  PluginMetadata FindPlugin(const PluginMetadata& plugin) const;
  void AddPlugin(const PluginMetadata& plugin);

  // Doesn't erase matching regex entries, because they might also
  // be required for other plugins.
  void ErasePlugin(const PluginMetadata& plugin);

  void AppendMessage(const Message& message);

  // Eval plugin conditions.
  void EvalAllConditions(Game& game, const LanguageCode language);

protected:
  std::set<std::string> bashTags_;
  std::unordered_set<PluginMetadata> plugins_;
  std::list<PluginMetadata> regexPlugins_;
  std::list<Message> messages_;
};
}

#endif
