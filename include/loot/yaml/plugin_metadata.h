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
#ifndef LOOT_BACKEND_METADATA_PLUGIN_METADATA
#define LOOT_BACKEND_METADATA_PLUGIN_METADATA

#include <cstdint>
#include <list>
#include <regex>
#include <set>
#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

#include "loot/metadata/plugin_metadata.h"

#include "loot/yaml/file.h"
#include "loot/yaml/location.h"
#include "loot/yaml/message_content.h"
#include "loot/yaml/message.h"
#include "loot/yaml/plugin_cleaning_data.h"
#include "loot/yaml/set.h"
#include "loot/yaml/tag.h"

namespace YAML {
template<>
struct convert<loot::PluginMetadata> {
  static Node encode(const loot::PluginMetadata& rhs) {
    Node node;
    node["name"] = rhs.Name();

    if (!rhs.Enabled())
      node["enabled"] = rhs.Enabled();

    if (rhs.LocalPriority().isExplicit())
      node["priority"] = rhs.LocalPriority().getValue();

    if (rhs.GlobalPriority().isExplicit())
      node["global_priority"] = rhs.GlobalPriority().getValue();

    if (!rhs.LoadAfter().empty())
      node["after"] = rhs.LoadAfter();
    if (!rhs.Reqs().empty())
      node["req"] = rhs.Reqs();
    if (!rhs.Incs().empty())
      node["inc"] = rhs.Incs();
    if (!rhs.Messages().empty())
      node["msg"] = rhs.Messages();
    if (!rhs.Tags().empty())
      node["tag"] = rhs.Tags();
    if (!rhs.DirtyInfo().empty())
      node["dirty"] = rhs.DirtyInfo();
    if (!rhs.CleanInfo().empty())
      node["clean"] = rhs.CleanInfo();
    if (!rhs.Locations().empty())
      node["url"] = rhs.Locations();

    return node;
  }

  static bool decode(const Node& node, loot::PluginMetadata& rhs) {
    if (!node.IsMap())
      throw RepresentationException(node.Mark(), "bad conversion: 'plugin metadata' object must be a map");
    if (!node["name"])
      throw RepresentationException(node.Mark(), "bad conversion: 'name' key missing from 'plugin metadata' object");

    rhs = loot::PluginMetadata(node["name"].as<std::string>());

    // Test for valid regex.
    if (rhs.IsRegexPlugin()) {
      try {
        std::regex(rhs.Name(), std::regex::ECMAScript | std::regex::icase);
      } catch (std::regex_error& e) {
        throw RepresentationException(node.Mark(), std::string("bad conversion: invalid regex in 'name' key: ") + e.what());
      }
    }

    if (node["enabled"])
      rhs.Enabled(node["enabled"].as<bool>());

    // Read priority values as int to prevent values that are too large from
    // being converted to -128.
    if (node["priority"]) {
      rhs.LocalPriority(loot::Priority(node["priority"].as<int>()));
    }

    if (node["global_priority"]) {
      rhs.GlobalPriority(loot::Priority(node["global_priority"].as<int>()));
    }

    if (node["after"])
      rhs.LoadAfter(node["after"].as<std::set<loot::File>>());
    if (node["req"])
      rhs.Reqs(node["req"].as<std::set<loot::File>>());
    if (node["inc"])
      rhs.Incs(node["inc"].as<std::set<loot::File>>());
    if (node["msg"])
      rhs.Messages(node["msg"].as<std::vector<loot::Message>>());
    if (node["tag"])
      rhs.Tags(node["tag"].as<std::set<loot::Tag>>());
    if (node["dirty"]) {
      if (rhs.IsRegexPlugin())
        throw RepresentationException(node.Mark(), "bad conversion: 'dirty' key must not be present in a regex 'plugin metadata' object");
      else
        rhs.DirtyInfo(node["dirty"].as<std::set<loot::PluginCleaningData>>());
    }
    if (node["clean"]) {
      if (rhs.IsRegexPlugin())
        throw RepresentationException(node.Mark(), "bad conversion: 'clean' key must not be present in a regex 'plugin metadata' object");
      else
        rhs.CleanInfo(node["clean"].as<std::set<loot::PluginCleaningData>>());
    }
    if (node["url"])
      rhs.Locations(node["url"].as<std::set<loot::Location>>());

    return true;
  }
};

inline Emitter& operator << (Emitter& out, const loot::PluginMetadata& rhs) {
  if (!rhs.HasNameOnly()) {
    out << BeginMap
      << Key << "name" << Value << YAML::SingleQuoted << rhs.Name();

    if (!rhs.Enabled())
      out << Key << "enabled" << Value << rhs.Enabled();

    if (rhs.LocalPriority().isExplicit()) {
      out << Key << "priority" << Value << rhs.LocalPriority().getValue();
    }

    if (rhs.GlobalPriority().isExplicit()) {
      out << Key << "global_priority" << Value << rhs.GlobalPriority().getValue();
    }

    if (!rhs.LoadAfter().empty())
      out << Key << "after" << Value << rhs.LoadAfter();

    if (!rhs.Reqs().empty())
      out << Key << "req" << Value << rhs.Reqs();

    if (!rhs.Incs().empty())
      out << Key << "inc" << Value << rhs.Incs();

    if (!rhs.Messages().empty())
      out << Key << "msg" << Value << rhs.Messages();

    if (!rhs.Tags().empty())
      out << Key << "tag" << Value << rhs.Tags();

    if (!rhs.DirtyInfo().empty())
      out << Key << "dirty" << Value << rhs.DirtyInfo();

    if (!rhs.CleanInfo().empty())
      out << Key << "clean" << Value << rhs.CleanInfo();

    if (!rhs.Locations().empty())
      out << Key << "url" << Value << rhs.Locations();

    out << EndMap;
  }

  return out;
}
}

#endif
