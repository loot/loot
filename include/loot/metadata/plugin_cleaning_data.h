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

#ifndef LOOT_METADATA_PLUGIN_CLEANING_DATA
#define LOOT_METADATA_PLUGIN_CLEANING_DATA

#include <cstdint>
#include <string>

#include "loot/api_decorator.h"
#include "loot/metadata/message.h"

namespace loot {
/**
 * Represents data identifying the plugin under which it is stored as dirty or
 * clean.
 */
class PluginCleaningData {
public:
  /**
   * Construct a PluginCleaningData object with zero CRC, ITM count, deleted
   * reference count and deleted navmesh count values, an empty utility string
   * and no info.
   * @return A PluginCleaningData object.
   */
  LOOT_API PluginCleaningData();

  /**
   * Construct a PluginCleaningData object with the given CRC and utility,
   * zero ITM count, deleted reference count and deleted navmesh count
   * values and no info.
   * @param  crc
   *         The CRC of a plugin.
   * @param  utility
   *         The utility that the plugin cleanliness was checked with.
   * @return A PluginCleaningData object.
   */
  LOOT_API PluginCleaningData(uint32_t crc, const std::string& utility);

  /**
   * Construct a PluginCleaningData object with the given values.
   * @param  crc
   *         A clean or dirty plugin's CRC.
   * @param  utility
   *         The utility that the plugin cleanliness was checked with.
   * @param  info
   *         A vector of localised information message strings about the plugin
   *         cleanliness.
   * @param  itm
   *         The number of Identical To Master records found in the plugin.
   * @param  ref
   *         The number of deleted references found in the plugin.
   * @param  nav
   *         The number of deleted navmeshes found in the plugin.
   * @return A PluginCleaningData object.
   */
  LOOT_API PluginCleaningData(uint32_t crc,
                              const std::string& utility,
                              const std::vector<MessageContent>& info,
                              unsigned int itm,
                              unsigned int ref,
                              unsigned int nav);
  /**
   * A less-than operator implemented with no semantics so that
   * PluginCleaningData objects can be stored in sets.
   * @returns True if this PluginCleaningData's CRC is less than the given
   *          PluginCleaningData's CRC, false otherwise.
   */
  LOOT_API bool operator < (const PluginCleaningData& rhs) const;

  /**
   * Check if two PluginCleaningData objects are equal by comparing their CRCs.
   * @returns True if the CRCs are equal, false otherwise.
   */
  LOOT_API bool operator == (const PluginCleaningData& rhs) const;

  /**
   * Get the CRC that identifies the plugin that the cleaning data is for.
   * @return A CRC-32 checksum.
   */
  LOOT_API uint32_t GetCRC() const;

  /**
   * Get the number of Identical To Master records in the plugin.
   * @return The number of Identical To Master records in the plugin.
   */
  LOOT_API unsigned int GetITMCount() const;

  /**
   * Get the number of deleted references in the plugin.
   * @return The number of deleted references in the plugin.
   */
  LOOT_API unsigned int GetDeletedReferenceCount() const;

  /**
   * Get the number of deleted navmeshes in the plugin.
   * @return The number of deleted navmeshes in the plugin.
   */
  LOOT_API unsigned int GetDeletedNavmeshCount() const;

  /**
   * Get the name of the cleaning utility that was used to check the plugin.
   * @return A cleaning utility name, possibly related information such as
   *         a version number and/or a Markdown-formatted URL to the utility's
   *         download location.
   */
  LOOT_API std::string GetCleaningUtility() const;

  /**
   * Get any additional informative message content supplied with the cleaning
   * data, eg. a link to a cleaning guide or information on wild edits or manual
   * cleaning steps.
   * @return A vector of localised MessageContent objects.
   */
  LOOT_API std::vector<MessageContent> GetInfo() const;

  /**
   * Choose an info MessageContent object given a preferred language.
   * @param  language
   *         The preferred language's LanguageCode.
   * @return The MessageContent object for the preferred language, or if one
   *         does not exist, the English-language MessageContent object.
   */
  LOOT_API MessageContent ChooseInfo(const LanguageCode language) const;

  /**
   * Get a warning message describing the cleaning data.
   * @return A Message object detailing the number and types of dirty edits
   *         found, the cleaning utility used, plus any additional information.
   */
  LOOT_API Message AsMessage() const;
private:
  uint32_t crc_;
  unsigned int itm_;
  unsigned int ref_;
  unsigned int nav_;
  std::string utility_;
  std::vector<MessageContent> info_;
};
}

#endif
