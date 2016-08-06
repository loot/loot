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

#ifndef LOOT_BACKEND_METADATA_PLUGIN_CLEANING_DATA
#define LOOT_BACKEND_METADATA_PLUGIN_CLEANING_DATA

#include <cstdint>
#include <string>

#include <yaml-cpp/yaml.h>

#include "backend/metadata/message.h"

namespace loot {
class Game;

class PluginCleaningData {
public:
  PluginCleaningData();
  PluginCleaningData(uint32_t crc, const std::string& utility);
  PluginCleaningData(uint32_t crc, unsigned int itm, unsigned int ref, unsigned int nav, const std::string& utility);

  bool operator < (const PluginCleaningData& rhs) const;
  bool operator == (const PluginCleaningData& rhs) const;

  uint32_t CRC() const;
  unsigned int ITMs() const;
  unsigned int DeletedRefs() const;
  unsigned int DeletedNavmeshes() const;
  std::string CleaningUtility() const;

  Message AsMessage() const;

  bool EvalCondition(Game& game, const std::string& pluginName) const;
private:
  uint32_t crc_;
  unsigned int itm_;
  unsigned int ref_;
  unsigned int nav_;
  std::string utility_;
};
}

namespace YAML {
template<>
struct convert<loot::PluginCleaningData> {
  static Node encode(const loot::PluginCleaningData& rhs) {
    Node node;
    node["crc"] = rhs.CRC();
    node["util"] = rhs.CleaningUtility();

    if (rhs.ITMs() > 0)
      node["itm"] = rhs.ITMs();
    if (rhs.DeletedRefs() > 0)
      node["udr"] = rhs.DeletedRefs();
    if (rhs.DeletedNavmeshes() > 0)
      node["nav"] = rhs.DeletedNavmeshes();

    return node;
  }

  static bool decode(const Node& node, loot::PluginCleaningData& rhs) {
    if (!node.IsMap())
      throw RepresentationException(node.Mark(), "bad conversion: 'cleaning data' object must be a map");
    if (!node["crc"])
      throw RepresentationException(node.Mark(), "bad conversion: 'crc' key missing from 'cleaning data' object");
    if (!node["util"])
      throw RepresentationException(node.Mark(), "bad conversion: 'util' key missing from 'cleaning data' object");

    uint32_t crc = node["crc"].as<uint32_t>();
    int itm = 0, ref = 0, nav = 0;

    if (node["itm"])
      itm = node["itm"].as<unsigned int>();
    if (node["udr"])
      ref = node["udr"].as<unsigned int>();
    if (node["nav"])
      nav = node["nav"].as<unsigned int>();

    std::string utility = node["util"].as<std::string>();

    rhs = loot::PluginCleaningData(crc, itm, ref, nav, utility);

    return true;
  }
};

Emitter& operator << (Emitter& out, const loot::PluginCleaningData& rhs);
}

#endif
