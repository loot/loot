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

#include "loot/api_decorator.h"
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
  LOOT_API PluginMetadata();
  LOOT_API PluginMetadata(const std::string& name);

  //Merges from the given plugin into this one, unless there is already equal metadata present.
  //For 'enabled' and 'priority' metadata, use the given plugin's values, but if the 'priority' user value is zero, ignore it.
  LOOT_API void MergeMetadata(const PluginMetadata& plugin);

  // Returns metadata in this plugin not in the given plugin.
  //For 'enabled', use this plugin's value.
  //For 'priority', use 0 if the two plugin priorities are equal, and make it not explicit. Otherwise use this plugin's value.
  LOOT_API PluginMetadata NewMetadata(const PluginMetadata& plugin) const;

  LOOT_API std::string GetName() const;
  LOOT_API std::string GetLowercasedName() const;
  LOOT_API bool IsEnabled() const;
  LOOT_API Priority GetLocalPriority() const;
  LOOT_API Priority GetGlobalPriority() const;
  LOOT_API std::set<File> GetLoadAfterFiles() const;
  LOOT_API std::set<File> GetRequirements() const;
  LOOT_API std::set<File> GetIncompatibilities() const;
  LOOT_API std::vector<Message> GetMessages() const;
  LOOT_API std::set<Tag> GetTags() const;
  LOOT_API std::set<PluginCleaningData> GetDirtyInfo() const;
  LOOT_API std::set<PluginCleaningData> GetCleanInfo() const;
  LOOT_API std::set<Location> GetLocations() const;

  LOOT_API std::vector<SimpleMessage> GetSimpleMessages(const LanguageCode language) const;

  LOOT_API void SetEnabled(const bool enabled);
  LOOT_API void SetLocalPriority(const Priority& priority);
  LOOT_API void SetGlobalPriority(const Priority& priority);
  LOOT_API void SetLoadAfterFiles(const std::set<File>& after);
  LOOT_API void SetRequirements(const std::set<File>& reqs);
  LOOT_API void SetIncompatibilities(const std::set<File>& incs);
  LOOT_API void SetMessages(const std::vector<Message>& messages);
  LOOT_API void SetTags(const std::set<Tag>& tags);
  LOOT_API void SetDirtyInfo(const std::set<PluginCleaningData>& info);
  LOOT_API void SetCleanInfo(const std::set<PluginCleaningData>& info);
  LOOT_API void SetLocations(const std::set<Location>& locations);

  LOOT_API bool HasNameOnly() const;
  LOOT_API bool IsRegexPlugin() const;

  //Compare name strings.
  LOOT_API bool operator == (const PluginMetadata& rhs) const;
  LOOT_API bool operator != (const PluginMetadata& rhs) const;

  //Compare name string.
  LOOT_API bool operator == (const std::string& rhs) const;
  LOOT_API bool operator != (const std::string& rhs) const;
private:
  std::string name_;
  bool enabled_;
  Priority localPriority_;
  Priority globalPriority_;
  std::set<File> loadAfter_;
  std::set<File> requirements_;
  std::set<File> incompatibilities_;
  std::vector<Message> messages_;
  std::set<Tag> tags_;
  std::set<PluginCleaningData> dirtyInfo_;
  std::set<PluginCleaningData> cleanInfo_;
  std::set<Location> locations_;
};
}

namespace std {
template<>
struct hash<loot::PluginMetadata> {
  size_t operator() (const loot::PluginMetadata& plugin) const {
    return hash<string>()(plugin.GetLowercasedName());
  }
};
}

#endif
