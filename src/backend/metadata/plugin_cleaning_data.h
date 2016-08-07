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

  MessageContent ChooseInfo(const Language::Code language) const;
  Message AsMessage() const;

  bool EvalCondition(Game& game, const std::string& pluginName) const;
private:
  uint32_t crc_;
  unsigned int itm_;
  unsigned int ref_;
  unsigned int nav_;
  std::string utility_;
  std::vector<MessageContent> info_;
};
}

namespace YAML {
template<>
struct convert<loot::PluginCleaningData> {
  static Node encode(const loot::PluginCleaningData& rhs) {
    Node node;
    node["crc"] = rhs.CRC();
    node["utility"] = rhs.CleaningUtility();
    node["info"] = rhs.Info();

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
    if (!node["utility"])
      throw RepresentationException(node.Mark(), "bad conversion: 'utility' key missing from 'cleaning data' object");

    uint32_t crc = node["crc"].as<uint32_t>();
    int itm = 0, ref = 0, nav = 0;

    if (node["itm"])
      itm = node["itm"].as<unsigned int>();
    if (node["udr"])
      ref = node["udr"].as<unsigned int>();
    if (node["nav"])
      nav = node["nav"].as<unsigned int>();

    std::string utility = node["utility"].as<std::string>();

    std::vector<loot::MessageContent> info;
    if (node["info"]) {
      if (node["info"].IsSequence())
        info = node["info"].as<std::vector<loot::MessageContent>>();
      else {
        info.push_back(loot::MessageContent(node["info"].as<std::string>(), loot::Language::Code::english));
      }
    }

    //Check now that at least one item in info is English if there are multiple items.
    if (info.size() > 1) {
      bool found = false;
      for (const auto &mc : info) {
        if (mc.GetLanguage() == loot::Language::Code::english)
          found = true;
      }
      if (!found)
        throw RepresentationException(node.Mark(), "bad conversion: multilingual messages must contain an English info string");
    }

    rhs = loot::PluginCleaningData(crc, utility, info, itm, ref, nav);

    return true;
  }
};

Emitter& operator << (Emitter& out, const loot::PluginCleaningData& rhs);
}

#endif
