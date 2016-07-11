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
    <http://www.gnu.org/licenses/>.
    */
#ifndef LOOT_BACKEND_METADATA_LOCATION
#define LOOT_BACKEND_METADATA_LOCATION

#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

namespace loot {
class Location {
public:
  Location();
  Location(const std::string& url, const std::string& name = "");

  bool operator < (const Location& rhs) const;
  bool operator == (const Location& rhs) const;

  std::string URL() const;
  std::string Name() const;
private:
  std::string url_;
  std::string name_;
};
}

namespace YAML {
template<>
struct convert<loot::Location> {
  static Node encode(const loot::Location& rhs) {
    Node node;

    node["link"] = rhs.URL();
    if (!rhs.Name().empty())
      node["name"] = rhs.Name();

    return node;
  }

  static bool decode(const Node& node, loot::Location& rhs) {
    if (!node.IsMap() && !node.IsScalar())
      throw RepresentationException(node.Mark(), "bad conversion: 'location' object must be a map or scalar");

    std::string url;
    std::string name;

    if (node.IsMap()) {
      if (!node["link"])
        throw RepresentationException(node.Mark(), "bad conversion: 'link' key missing from 'location' map object");

      url = node["link"].as<std::string>();
      if (node["name"])
        name = node["name"].as<std::string>();
    } else
      url = node.as<std::string>();

    rhs = loot::Location(url, name);

    return true;
  }
};

Emitter& operator << (Emitter& out, const loot::Location& rhs);
}

#endif
