/*  BOSS

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2014    WrinklyNinja

    This file is part of BOSS.

    BOSS is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    BOSS is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BOSS.  If not, see
    <http://www.gnu.org/licenses/>.
*/

#ifndef __BOSS_GRAPH__
#define __BOSS_GRAPH__

#include "metadata.h"
#include "error.h"

#include <boost/unordered_map.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/graphviz.hpp>

namespace boss {

    typedef boost::adjacency_list<boost::listS, boost::listS, boost::bidirectionalS, boss::Plugin> PluginGraph;
    typedef boost::graph_traits<PluginGraph>::vertex_descriptor vertex_t;
    typedef boost::graph_traits<PluginGraph>::vertex_iterator vertex_it;
    typedef boost::graph_traits<PluginGraph>::edge_descriptor edge_t;
    typedef boost::graph_traits<PluginGraph>::edge_iterator edge_it;

    struct cycle_detector : public boost::dfs_visitor<> {
        cycle_detector();

        void back_edge(edge_t e, const PluginGraph& g);
    };

    bool GetVertexByName(const PluginGraph& graph, const std::string& name, vertex_t& vertex);

    void Sort(const PluginGraph& graph, std::list<Plugin>& plugins);

    void CheckForCycles(const PluginGraph& graph);

    void AddSpecificEdges(PluginGraph& graph);

    void AddPriorityEdges(PluginGraph& graph);

    void AddOverlapEdges(PluginGraph& graph);

    bool EdgeCreatesCycle(PluginGraph& graph, vertex_t u, vertex_t v);
}

#endif
