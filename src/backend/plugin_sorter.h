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

#ifndef __LOOT_GRAPH__
#define __LOOT_GRAPH__

#include "plugin/plugin.h"

#include <map>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

namespace loot {
    typedef boost::adjacency_list<boost::listS, boost::listS, boost::directedS, Plugin> PluginGraph;
    typedef boost::graph_traits<PluginGraph>::vertex_descriptor vertex_t;
    typedef boost::associative_property_map<std::map<vertex_t, size_t>> vertex_map_t;

    class Game;

    class PluginSorter {
    public:
        std::list<Plugin> Sort(Game& game, const unsigned int language);
    private:
        PluginGraph graph;
        std::map<vertex_t, size_t> indexMap;
        vertex_map_t vertexIndexMap;
        std::list<std::string> oldLoadOrder;

        bool GetVertexByName(const std::string& name, vertex_t& vertex) const;
        void CheckForCycles() const;
        bool EdgeCreatesCycle(const vertex_t& u, const vertex_t& v) const;

        int plugincmp(const std::string& plugin1, const std::string& plugin2) const;

        void PropagatePriorities();

        void addPluginVertices(Game& game, const unsigned int language);
        void addMasterFlagEdges();
        void AddSpecificEdges();
        void AddPriorityEdges();
        void AddOverlapEdges();
        void AddTieBreakEdges();

        void addEdge(const vertex_t& fromVertex, const vertex_t& toVertex);
    };
}

#endif
