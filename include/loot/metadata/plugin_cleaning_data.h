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
class PluginCleaningData {
public:
  LOOT_API PluginCleaningData();
  LOOT_API PluginCleaningData(uint32_t crc, const std::string& utility);
  LOOT_API PluginCleaningData(uint32_t crc,
                              const std::string& utility,
                              const std::vector<MessageContent>& info,
                              unsigned int itm,
                              unsigned int ref,
                              unsigned int nav);

  LOOT_API bool operator < (const PluginCleaningData& rhs) const;
  LOOT_API bool operator == (const PluginCleaningData& rhs) const;

  LOOT_API uint32_t GetCRC() const;
  LOOT_API unsigned int GetITMCount() const;
  LOOT_API unsigned int GetDeletedReferenceCount() const;
  LOOT_API unsigned int GetDeletedNavmeshCount() const;
  LOOT_API std::string GetCleaningUtility() const;
  LOOT_API std::vector<MessageContent> GetInfo() const;

  LOOT_API MessageContent ChooseInfo(const LanguageCode language) const;
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
