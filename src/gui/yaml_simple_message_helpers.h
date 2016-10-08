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

#ifndef LOOT_GUI_YAML_SIMPLE_MESSAGE_HELPERS
#define LOOT_GUI_YAML_SIMPLE_MESSAGE_HELPERS

#include <yaml-cpp/yaml.h>

#include "loot/struct/simple_message.h"

namespace YAML {
template<>
struct convert<loot::SimpleMessage> {
  static Node encode(const loot::SimpleMessage& rhs) {
    Node node;

    if (rhs.type == loot::MessageType::say)
      node["type"] = "say";
    else if (rhs.type == loot::MessageType::warn)
      node["type"] = "warn";
    else
      node["type"] = "error";

    node["text"] = rhs.text;
    node["language"] = loot::Language(rhs.language).GetLocale();

    return node;
  }

  static bool decode(const Node& node, loot::SimpleMessage& rhs) {
    // Treat SimpleMessage as a MessageContent with a type.
    if (!node.IsMap())
      throw RepresentationException(node.Mark(), "bad conversion: 'simple message' object must be a map");
    if (!node["type"])
      throw RepresentationException(node.Mark(), "bad conversion: 'type' key missing from 'simple message' object");
    if (!node["text"])
      throw RepresentationException(node.Mark(), "bad conversion: 'text' key missing from 'simple message' object");
    if (!node["language"])
      throw RepresentationException(node.Mark(), "bad conversion: 'language' key missing from 'simple message' object");

    rhs.text = node["text"].as<std::string>();
    rhs.language = loot::Language(node["language"].as<std::string>()).GetCode();

    std::string type;
    type = node["type"].as<std::string>();

    loot::MessageType typeNo = loot::MessageType::say;
    if (boost::iequals(type, "warn"))
      typeNo = loot::MessageType::warn;
    else if (boost::iequals(type, "error"))
      typeNo = loot::MessageType::error;

    return true;
  }
};

Emitter& operator << (Emitter& out, const loot::SimpleMessage& rhs) {
  out << BeginMap;

  if (rhs.type == loot::MessageType::say)
    out << Key << "type" << Value << "say";
  else if (rhs.type == loot::MessageType::warn)
    out << Key << "type" << Value << "warn";
  else
    out << Key << "type" << Value << "error";

  out << Key << "language" << Value << loot::Language(rhs.language).GetLocale();

  out << Key << "text" << Value << YAML::SingleQuoted << rhs.text;

  return out;
}
}

#endif
