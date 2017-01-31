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

#ifndef LOOT_YAML_SET
#define LOOT_YAML_SET

#include <set>
#include <unordered_set>

#include <yaml-cpp/yaml.h>

namespace YAML {
template<class T, class Compare>
struct convert<std::set<T, Compare>> {
  static Node encode(const std::set<T, Compare>& rhs) {
    Node node;
    for (const auto &element : rhs) {
      node.push_back(element);
    }
    return node;
  }

  static bool decode(const Node& node, std::set<T, Compare>& rhs) {
    if (!node.IsSequence())
      throw RepresentationException(node.Mark(), "bad conversion: set must be a sequence of elements");

    rhs.clear();
    for (const auto &element : node) {
      if (!rhs.insert(element.template as<T>()).second)
        throw RepresentationException(node.Mark(), "bad conversion: set elements must be unique");
    }
    return true;
  }
};

template<class T, class Compare>
Emitter& operator << (Emitter& out, const std::set<T, Compare>& rhs) {
  out << BeginSeq;
  for (const auto &element : rhs) {
    out << element;
  }
  out << EndSeq;

  return out;
}

template<class T, class Hash>
struct convert<std::unordered_set<T, Hash>> {
  static Node encode(const std::unordered_set<T, Hash>& rhs) {
    Node node;
    for (const auto &element : rhs) {
      node.push_back(element);
    }
    return node;
  }

  static bool decode(const Node& node, std::unordered_set<T, Hash>& rhs) {
    if (!node.IsSequence())
      throw RepresentationException(node.Mark(), "bad conversion: unordered set must be a sequence of elements");

    rhs.clear();
    for (const auto &element : node) {
      if (!rhs.insert(element.template as<T>()).second)
        throw RepresentationException(node.Mark(), "bad conversion: unordered set elements must be unique");
    }
    return true;
  }
};

template<class T, class Hash>
Emitter& operator << (Emitter& out, const std::unordered_set<T, Hash>& rhs) {
  out << BeginSeq;
  for (const auto &element : rhs) {
    out << element;
  }
  out << EndSeq;

  return out;
}
}

#endif
