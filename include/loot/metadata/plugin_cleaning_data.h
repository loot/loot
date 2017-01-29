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

#include "loot/metadata/message.h"

namespace loot {
class PluginCleaningData {
public:
  PluginCleaningData();
  PluginCleaningData(uint32_t crc, const std::string& utility);
  PluginCleaningData(uint32_t crc,
                     const std::string& utility,
                     const std::vector<MessageContent>& info,
                     unsigned int itm,
                     unsigned int ref,
                     unsigned int nav);

  bool operator < (const PluginCleaningData& rhs) const;
  bool operator == (const PluginCleaningData& rhs) const;

  uint32_t CRC() const;
  unsigned int ITMs() const;
  unsigned int DeletedRefs() const;
  unsigned int DeletedNavmeshes() const;
  std::string CleaningUtility() const;
  std::vector<MessageContent> Info() const;

  MessageContent ChooseInfo(const LanguageCode language) const;
  Message AsMessage() const;
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
