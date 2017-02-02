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
#ifndef LOOT_YAML_FILE
#define LOOT_YAML_FILE

#include <string>

#include <yaml-cpp/yaml.h>

#include "loot/metadata/file.h"

namespace YAML {
template<>
struct convert<loot::File> {
  static Node encode(const loot::File& rhs) {
    Node node;
    node["name"] = rhs.GetName();

    if (rhs.IsConditional())
      node["condition"] = rhs.GetCondition();

    if (rhs.GetDisplayName() != rhs.GetName())
      node["display"] = rhs.GetDisplayName();

    return node;
  }

  static bool decode(const Node& node, loot::File& rhs) {
    if (!node.IsMap() && !node.IsScalar())
      throw RepresentationException(node.Mark(), "bad conversion: 'file' object must be a map or scalar");

    if (node.IsMap()) {
      if (!node["name"])
        throw RepresentationException(node.Mark(), "bad conversion: 'name' key missing from 'file' map object");

      std::string name = node["name"].as<std::string>();
      std::string condition, display;
      if (node["condition"])
        condition = node["condition"].as<std::string>();
      if (node["display"])
        display = node["display"].as<std::string>();
      rhs = loot::File(name, display, condition);
    } else
      rhs = loot::File(node.as<std::string>());

  // Test condition syntax.
    try {
      rhs.ParseCondition();
    } catch (std::exception& e) {
      throw RepresentationException(node.Mark(), std::string("bad conversion: invalid condition syntax: ") + e.what());
    }

    return true;
  }
};

inline Emitter& operator << (Emitter& out, const loot::File& rhs) {
  if (!rhs.IsConditional() && (rhs.GetDisplayName().empty() || rhs.GetDisplayName() == rhs.GetName()))
    out << YAML::SingleQuoted << rhs.GetName();
  else {
    out << BeginMap
      << Key << "name" << Value << YAML::SingleQuoted << rhs.GetName();

    if (rhs.IsConditional())
      out << Key << "condition" << Value << YAML::SingleQuoted << rhs.GetCondition();

    if (rhs.GetDisplayName() != rhs.GetName())
      out << Key << "display" << Value << YAML::SingleQuoted << rhs.GetDisplayName();

    out << EndMap;
  }

  return out;
}
}

#endif
