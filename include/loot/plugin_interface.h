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
class PluginInterface {
public:
  virtual std::string GetName() const = 0;
  virtual std::string GetLowercasedName() const = 0;
  virtual std::string GetVersion() const = 0;
  virtual std::vector<std::string> GetMasters() const = 0;
  virtual std::vector<Message> GetStatusMessages() const = 0;
  virtual std::set<Tag> GetBashTags() const = 0;
  virtual uint32_t GetCRC() const = 0;

  virtual bool IsMaster() const = 0;
  virtual bool IsEmpty() const = 0;
  virtual bool LoadsArchive() const = 0;
  virtual bool DoFormIDsOverlap(const PluginInterface& plugin) const = 0;
};
}

namespace std {
template<>
struct hash<loot::PluginInterface> {
  size_t operator() (const loot::PluginInterface& plugin) const {
    return hash<string>()(plugin.GetLowercasedName());
  }
};
}

#endif
