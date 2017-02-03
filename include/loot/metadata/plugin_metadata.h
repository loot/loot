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
/**
 * Represents a plugin's metadata.
 */
class PluginMetadata {
public:
  /**
   * Construct a PluginMetadata object with a blank plugin name and no metadata.
   * @return A PluginMetadata object.
   */
  LOOT_API PluginMetadata();

  /**
   * Construct a PluginMetadata object with no metadata for a plugin with the
   * given filename.
   * @param  name
   *         The filename of the plugin that the object is constructed for.
   * @return A PluginMetadata object.
   */
  LOOT_API PluginMetadata(const std::string& name);

  //Merges from the given plugin into this one, unless there is already equal metadata present.
  //For 'enabled' and 'priority' metadata, use the given plugin's values, but if the 'priority' user value is zero, ignore it.
  /**
   * Merge metadata from the given PluginMetadata object into this object.
   *
   * If an equal metadata object already exists in this PluginMetadata object,
   * it is not duplicated. This object's priorities are replaced if the given
   * PluginMetadata object's priorities are explicit. This object's enabled
   * state is replaced by the given object's state.
   * @param plugin
   *        The plugin metadata to merge.
   */
  LOOT_API void MergeMetadata(const PluginMetadata& plugin);

  // Returns metadata in this plugin not in the given plugin.
  //For 'enabled', use this plugin's value.
  //For 'priority', use 0 if the two plugin priorities are equal, and make it not explicit. Otherwise use this plugin's value.

  /**
   * Get metadata in this object that isn't present in the given PluginMetadata
   * object.
   * @param  plugin
   *         The PluginMetadata object to compare against.
   * @return A PluginMetadata object containing the metadata in this object that
   *         is not in the given object. The returned object inherits this
   *         object's enabled state. The returned object also inherits this
   *         plugin's priorities, unless a priority is equal to the given
   *         object's priority, in which case the returned object is given
   *         an implicit zero priority instead.
   */
  LOOT_API PluginMetadata NewMetadata(const PluginMetadata& plugin) const;

  /**
   * Get the plugin name.
   * @return The plugin name.
   */
  LOOT_API std::string GetName() const;

  /**
   * Get the lowercased plugin name.
   * @return The lowercased plugin name.
   */
  LOOT_API std::string GetLowercasedName() const;

  /**
   * Check if the plugin metadata is enabled for use during sorting.
   * @return True if the metadata will be used during sorting, false otherwise.
   */
  LOOT_API bool IsEnabled() const;

  /**
   * Get the plugin's local priority metadata.
   * @return The plugin's local priority metadata.
   */
  LOOT_API Priority GetLocalPriority() const;

  /**
   * Get the plugin's global priority metadata.
   * @return The plugin's global priority metadata.
   */
  LOOT_API Priority GetGlobalPriority() const;

  /**
   * Get the plugins that the plugin must load after.
   * @return The plugins that the plugin must load after.
   */
  LOOT_API std::set<File> GetLoadAfterFiles() const;

  /**
   * Get the files that the plugin requires to be installed.
   * @return The files that the plugin requires to be installed.
   */
  LOOT_API std::set<File> GetRequirements() const;

  /**
   * Get the files that the plugin is incompatible with.
   * @return The files that the plugin is incompatible with.
   */
  LOOT_API std::set<File> GetIncompatibilities() const;

  /**
   * Get the plugin's messages.
   * @return The plugin's messages.
   */
  LOOT_API std::vector<Message> GetMessages() const;

  /**
   * Get the plugin's Bash Tag suggestions.
   * @return The plugin's Bash Tag suggestions.
   */
  LOOT_API std::set<Tag> GetTags() const;

  /**
   * Get the plugin's dirty plugin information.
   * @return The PluginCleaningData objects that identify the plugin as dirty.
   */
  LOOT_API std::set<PluginCleaningData> GetDirtyInfo() const;

  /**
   * Get the plugin's clean plugin information.
   * @return The PluginCleaningData objects that identify the plugin as clean.
   */
  LOOT_API std::set<PluginCleaningData> GetCleanInfo() const;

  /**
   * Get the locations at which this plugin can be found.
   * @return The locations at which this plugin can be found.
   */
  LOOT_API std::set<Location> GetLocations() const;

  /**
   * Get the plugin's messages as SimpleMessage objects for the given language.
   * @param language
   *        The language to create the SimpleMessage objects for.
   * @return The plugin's messages as SimpleMessage objects.
   */
  LOOT_API std::vector<SimpleMessage> GetSimpleMessages(const LanguageCode language) const;

  /**
   * Set whether the plugin metadata is enabled for use during sorting or not.
   * @param enabled
   *        The value to set.
   */
  LOOT_API void SetEnabled(const bool enabled);

  /**
   * Set the plugin's local priority.
   * @param priority
   *        The value to set.
   */
  LOOT_API void SetLocalPriority(const Priority& priority);

  /**
   * Set the plugin's local priority.
   * @param priority
   *        The value to set.
   */
  LOOT_API void SetGlobalPriority(const Priority& priority);

  /**
   * Set the files that the plugin must load after.
   * @param after
   *        The files to set.
   */
  LOOT_API void SetLoadAfterFiles(const std::set<File>& after);

  /**
   * Set the files that the plugin requires to be installed.
   * @param requirements
   *        The files to set.
   */
  LOOT_API void SetRequirements(const std::set<File>& requirements);

  /**
   * Set the files that the plugin must load after.
   * @param incompatibilities
   *        The files to set.
   */
  LOOT_API void SetIncompatibilities(const std::set<File>& incompatibilities);

  /**
   * Set the plugin's messages.
   * @param messages
   *        The messages to set.
   */
  LOOT_API void SetMessages(const std::vector<Message>& messages);

  /**
   * Set the plugin's Bash Tag suggestions.
   * @param tags
   *        The Bash Tag suggestions to set.
   */
  LOOT_API void SetTags(const std::set<Tag>& tags);

  /**
   * Set the plugin's dirty information.
   * @param info
   *        The dirty information to set.
   */
  LOOT_API void SetDirtyInfo(const std::set<PluginCleaningData>& info);

  /**
   * Set the plugin's clean information.
   * @param info
   *        The clean information to set.
   */
  LOOT_API void SetCleanInfo(const std::set<PluginCleaningData>& info);

  /**
   * Set the plugin's locations.
   * @param locations
   *        The locations to set.
   */
  LOOT_API void SetLocations(const std::set<Location>& locations);

  /**
   * Check if no plugin metadata is set.
   * @return True if the local and global priorities are implicit and the
   *         metadata containers are all empty, false otherwise.
   */
  LOOT_API bool HasNameOnly() const;

  /**
   * Check if the plugin name is a regular expression.
   * @return True if the plugin name contains any of the characters ``:\*?|``,
   *         false otherwise.
   */
  LOOT_API bool IsRegexPlugin() const;

  /**
   * Check if two PluginMetadata objects are equal by comparing their name
   * values.
   * @returns True if the plugin names are case-insensitively equal, false
   *          otherwise.
   */
  LOOT_API bool operator == (const PluginMetadata& rhs) const;

  /**
   * Check if two PluginMetadata objects are not equal by comparing their name
   * values.
   * @returns True if the plugin names are not case-insensitively equal, false
   *          otherwise.
   */
  LOOT_API bool operator != (const PluginMetadata& rhs) const;


  /**
   * Check if object's name value is equal to the given string.
   * @returns True if the plugin name is case-insensitively equal to the given
   *          string, false otherwise.
   */
  LOOT_API bool operator == (const std::string& rhs) const;

  /**
   * Check if object's name value is not equal to the given string.
   * @returns True if the plugin name is not case-insensitively equal to the
   *          given string, false otherwise.
   */
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
/**
 * A specialisation of std::hash for loot::PluginMetadata.
 */
template<>
struct hash<loot::PluginMetadata> {
  /**
   * Calculate a hash value for an object of a class that implements
   * loot::PluginMetadata.
   * @return The hash generated from the plugin's lowercased filename.
   */
  size_t operator() (const loot::PluginMetadata& plugin) const {
    return hash<string>()(plugin.GetLowercasedName());
  }
};
}

#endif
