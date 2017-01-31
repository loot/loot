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

#ifndef LOOT_API_PLUGIN_PLUGIN_SORTER
#define LOOT_API_PLUGIN_PLUGIN_SORTER

#include <map>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

#include "api/game/game.h"
#include "api/plugin/plugin.h"

namespace loot {

class PluginSortingData : public Plugin, private PluginMetadata {
public:
  PluginSortingData(const Plugin& plugin, const PluginMetadata& metadata);

  using Plugin::GetName;
  using Plugin::IsMaster;
  using Plugin::LoadsArchive;
  using Plugin::GetMasters;
  using Plugin::NumOverrideFormIDs;
  using Plugin::DoFormIDsOverlap;

  using PluginMetadata::LocalPriority;
  using PluginMetadata::GlobalPriority;
  using PluginMetadata::Reqs;
  using PluginMetadata::LoadAfter;
};

typedef boost::adjacency_list<boost::listS, boost::listS, boost::directedS, PluginSortingData> PluginGraph;
typedef boost::graph_traits<PluginGraph>::vertex_descriptor vertex_t;
typedef boost::associative_property_map<std::map<vertex_t, size_t>> vertex_map_t;

class PluginSorter {
public:
  std::vector<std::string> Sort(Game& game);
private:
  bool GetVertexByName(const std::string& name, vertex_t& vertex) const;
  void CheckForCycles() const;
  bool EdgeCreatesCycle(const vertex_t& u, const vertex_t& v) const;

  int ComparePlugins(const std::string& plugin1, const std::string& plugin2) const;

  void PropagatePriorities();

  void AddPluginVertices(Game& game);
  void AddSpecificEdges();
  void AddPriorityEdges();
  void AddOverlapEdges();
  void AddTieBreakEdges();

  void AddEdge(const vertex_t& fromVertex, const vertex_t& toVertex);

  PluginGraph graph_;
  std::map<vertex_t, size_t> indexMap_;
  vertex_map_t vertexIndexMap_;
  std::vector<std::string> oldLoadOrder_;
};
}

#endif
