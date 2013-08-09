/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2012-2013    WrinklyNinja

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

    typedef boost::adjacency_list<boost::listS, boost::listS, boost::bidirectionalS, std::list<boss::Plugin>::iterator> PluginGraph;
    typedef boost::graph_traits<PluginGraph>::vertex_descriptor vertex_t;
    typedef boost::graph_traits<PluginGraph>::vertex_iterator vertex_it;
    typedef boost::graph_traits<PluginGraph>::edge_descriptor edge_t;

    struct cycle_detector : public boost::dfs_visitor<> {
        inline cycle_detector() { }

        inline void back_edge(edge_t e, const PluginGraph& g) {
            vertex_t vSource = boost::source(e, g);
            vertex_t vTarget = boost::target(e, g);

            throw boss::error(boss::error::invalid_args, "Back edge detected between plugins \"" + g[vSource]->Name() + "\" and \"" + g[vTarget]->Name() + "\".");
        }
    };

    //Gets the vertex for the plugin if it exists, or creates one if it doesn't.
    vertex_t GetPluginVertex(PluginGraph& graph, const Plugin& plugin, boost::unordered_map<std::string, vertex_t>& pluginVertexMap);

    //The map maps each plugin name to a vector of names of plugins that overlap with it and should load before it.
    void CalcPluginOverlaps(const std::list<Plugin>& plugins, boost::unordered_map< std::string, std::vector<std::string> >& overlapMap);

    bool GetVertexByName(const PluginGraph& graph, const std::string& name, vertex_t& vertex);

    void SaveGraph(const PluginGraph& graph, const boost::filesystem::path outpath);

    void Sort(const PluginGraph& graph, std::list<Plugin>& plugins);

  void CheckForCycles(const PluginGraph& graph);
}

#endif
