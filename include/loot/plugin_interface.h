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
#ifndef LOOT_PLUGIN_INTERFACE
#define LOOT_PLUGIN_INTERFACE

#include <cstdint>
#include <string>
#include <vector>

#include "loot/metadata/message.h"
#include "loot/metadata/tag.h"

namespace loot {
/**
 * Represents a plugin file that has been parsed by LOOT.
 */
class PluginInterface {
public:
  /**
   * Get the plugin's filename.
   * @return The plugin filename.
   */
  virtual std::string GetName() const = 0;

  /**
   * Get the plugin's filename in lowercase characters.
   * @return The lowercased plugin filename.
   */
  virtual std::string GetLowercasedName() const = 0;

  /**
   * Get the plugin's version number from its description field.
   *
   * If no version number is found in the description field, an empty string is
   * returned. The description field parsing may fail to extract the version
   * number correctly, though it functions correctly in all known cases.
   * @return A string containing a version number, or an empty string.
   */
  virtual std::string GetVersion() const = 0;

  /**
   * Get the plugin's masters.
   * @return The plugin's masters in the same order they are listed in the file.
   */
  virtual std::vector<std::string> GetMasters() const = 0;

  /**
   * Get any status messages associated with the plugin.
   *
   * For example, if parsing failed, it could be recorded in a status message.
   * @return A vector of status messages.
   */
  virtual std::vector<Message> GetStatusMessages() const = 0;

  /**
   * Get any Bash Tags found in the plugin's description field.
   * @return A set of Bash Tags. The order of elements in the set holds no
   *         semantics.
   */
  virtual std::set<Tag> GetBashTags() const = 0;

  /**
   * Get the plugin's CRC-32 checksum.
   * @return The plugin's CRC-32 checksum if it has been fully read. If only the
   *         plugin's header has been read, ``0`` will be returned.
   */
  virtual uint32_t GetCRC() const = 0;

  /**
   * Check if the plugin's master flag is set.
   * @return True if the master flag is set, false otherwise.
   */
  virtual bool IsMaster() const = 0;

  /**
   * Check if the plugin contains any records other than its TES4 header.
   * @return True if the plugin only contains a TES4 header, false otherwise.
   */
  virtual bool IsEmpty() const = 0;

  /**
   * Check if the plugin loads an archive (BSA/BA2 depending on the game).
   * @return True if the plugin loads an archive, false otherwise.
   */
  virtual bool LoadsArchive() const = 0;

  /**
   * Check if two plugins contain records for the same FormIDs.
   * @param  plugin
   *         The other plugin to check for FormID overlap with.
   * @return True if the plugins both contain at least one record with the same
   *         FormID, false otherwise.
   */
  virtual bool DoFormIDsOverlap(const PluginInterface& plugin) const = 0;
};
}

namespace std {
/**
 * A specialisation of std::hash for loot::PluginInterface.
 */
template<>
struct hash<loot::PluginInterface> {
  /**
   * Calculate a hash value for an object of a class that implements
   * loot::PluginInterface.
   * @return The hash generated from the plugin's lowercased filename.
   */
  size_t operator() (const loot::PluginInterface& plugin) const {
    return hash<string>()(plugin.GetLowercasedName());
  }
};
}

#endif
