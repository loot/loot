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
#ifndef LOOT_YAML_TAG
#define LOOT_YAML_TAG

#include <string>

#include <yaml-cpp/yaml.h>

#include "loot/metadata/tag.h"

namespace YAML {
template<>
struct convert<loot::Tag> {
  static Node encode(const loot::Tag& rhs) {
    Node node;
    if (rhs.IsConditional())
      node["condition"] = rhs.GetCondition();
    if (rhs.IsAddition())
      node["name"] = rhs.GetName();
    else
      node["name"] = "-" + rhs.GetName();
    return node;
  }

  static bool decode(const Node& node, loot::Tag& rhs) {
    if (!node.IsMap() && !node.IsScalar())
      throw RepresentationException(node.Mark(), "bad conversion: 'tag' object must be a map or scalar");

    std::string condition, tag;
    if (node.IsMap()) {
      if (!node["name"])
        throw RepresentationException(node.Mark(), "bad conversion: 'name' key missing from 'tag' map object");

      tag = node["name"].as<std::string>();
      if (node["condition"])
        condition = node["condition"].as<std::string>();
    } else
      tag = node.as<std::string>();

    if (tag[0] == '-')
      rhs = loot::Tag(tag.substr(1), false, condition);
    else
      rhs = loot::Tag(tag, true, condition);

  // Test condition syntax.
    try {
      rhs.ParseCondition();
    } catch (std::exception& e) {
      throw RepresentationException(node.Mark(), std::string("bad conversion: invalid condition syntax: ") + e.what());
    }

    return true;
  }
};

inline Emitter& operator << (Emitter& out, const loot::Tag& rhs) {
  if (!rhs.IsConditional()) {
    if (rhs.IsAddition())
      out << rhs.GetName();
    else
      out << ('-' + rhs.GetName());
  } else {
    out << BeginMap;
    if (rhs.IsAddition())
      out << Key << "name" << Value << rhs.GetName();
    else
      out << Key << "name" << Value << ('-' + rhs.GetName());

    out << Key << "condition" << Value << YAML::SingleQuoted << rhs.GetCondition()
      << EndMap;
  }

  return out;
}
}

#endif
