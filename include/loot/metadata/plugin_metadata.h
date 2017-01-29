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
#ifndef LOOT_METADATA_PLUGIN_METADATA
#define LOOT_METADATA_PLUGIN_METADATA

#include <cstdint>
#include <list>
#include <regex>
#include <set>
#include <string>
#include <vector>

#include "loot/yaml/set.h"
#include "loot/metadata/file.h"
#include "loot/metadata/location.h"
#include "loot/metadata/message.h"
#include "loot/metadata/plugin_cleaning_data.h"
#include "loot/metadata/priority.h"
#include "loot/metadata/tag.h"

namespace loot {
class PluginMetadata {
public:
  PluginMetadata();
  PluginMetadata(const std::string& name);

  //Merges from the given plugin into this one, unless there is already equal metadata present.
  //For 'enabled' and 'priority' metadata, use the given plugin's values, but if the 'priority' user value is zero, ignore it.
  void MergeMetadata(const PluginMetadata& plugin);

  // Returns metadata in this plugin not in the given plugin.
  //For 'enabled', use this plugin's value.
  //For 'priority', use 0 if the two plugin priorities are equal, and make it not explicit. Otherwise use this plugin's value.
  PluginMetadata NewMetadata(const PluginMetadata& plugin) const;

  std::string Name() const;
  std::string LowercasedName() const;
  bool Enabled() const;
  Priority LocalPriority() const;
  Priority GlobalPriority() const;
  std::set<File> LoadAfter() const;
  std::set<File> Reqs() const;
  std::set<File> Incs() const;
  std::vector<Message> Messages() const;
  std::set<Tag> Tags() const;
  std::set<PluginCleaningData> DirtyInfo() const;
  std::set<PluginCleaningData> CleanInfo() const;
  std::set<Location> Locations() const;

  std::vector<SimpleMessage> SimpleMessages(const LanguageCode language) const;

  void Enabled(const bool enabled);
  void LocalPriority(const Priority& priority);
  void GlobalPriority(const Priority& priority);
  void LoadAfter(const std::set<File>& after);
  void Reqs(const std::set<File>& reqs);
  void Incs(const std::set<File>& incs);
  void Messages(const std::vector<Message>& messages);
  void Tags(const std::set<Tag>& tags);
  void DirtyInfo(const std::set<PluginCleaningData>& info);
  void CleanInfo(const std::set<PluginCleaningData>& info);
  void Locations(const std::set<Location>& locations);

  bool HasNameOnly() const;
  bool IsRegexPlugin() const;

  //Compare name strings.
  bool operator == (const PluginMetadata& rhs) const;
  bool operator != (const PluginMetadata& rhs) const;

  //Compare name string.
  bool operator == (const std::string& rhs) const;
  bool operator != (const std::string& rhs) const;
protected:
  std::vector<Message> messages_;
  std::set<Tag> tags_;
private:
  std::string name_;
  bool enabled_;  //Default to true.
  Priority localPriority_;
  Priority globalPriority_;
  std::set<File> loadAfter_;
  std::set<File> requirements_;
  std::set<File> incompatibilities_;
  std::set<PluginCleaningData> dirtyInfo_;
  std::set<PluginCleaningData> cleanInfo_;
  std::set<Location> locations_;
};
}

namespace std {
template<>
struct hash<loot::PluginMetadata> {
  size_t operator() (const loot::PluginMetadata& plugin) const {
    return hash<string>()(plugin.LowercasedName());
  }
};
}

#endif
