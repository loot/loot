/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2014    WrinklyNinja

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

#include "graph.h"
#include "streams.h"
#include "helpers.h"

#include <cstdlib>

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/locale.hpp>
#include <boost/format.hpp>

using namespace std;

namespace lc = boost::locale;

namespace loot {

    cycle_detector::cycle_detector() {}

    void cycle_detector::tree_edge(edge_t e, const PluginGraph& g) {
        vertex_t source = boost::source(e, g);

        trail.push_back(g[source].Name());
    }

    void cycle_detector::back_edge(edge_t e, const PluginGraph& g) {
        vertex_t vSource = boost::source(e, g);
        vertex_t vTarget = boost::target(e, g);

        trail.push_back(g[vSource].Name());
        list<string>::iterator it = find(trail.begin(), trail.end(), g[vTarget].Name());
        string backCycle;
        for (list<string>::iterator endIt = trail.end(); it != endIt; ++it) {
            backCycle += *it + ", ";
        }
        backCycle.erase(backCycle.length() - 2);

        BOOST_LOG_TRIVIAL(error) << "Cyclic interaction detected between plugins \"" << g[vSource].Name() << "\" and \"" << g[vTarget].Name() << "\". Back cycle: " << backCycle;

        throw loot::error(loot::error::sorting_error, (boost::format(lc::translate("Cyclic interaction detected between plugins \"%1%\" and \"%2%\". Back cycle: %3%")) % g[vSource].Name() % g[vTarget].Name() % backCycle).str());
    }

    bool GetVertexByName(const PluginGraph& graph, const std::string& name, vertex_t& vertex) {
        vertex_it vit, vit_end;
        boost::tie(vit, vit_end) = boost::vertices(graph);

        for (vit, vit_end; vit != vit_end; ++vit) {
            if (boost::iequals(graph[*vit].Name(), name)) {
                vertex = *vit;
                return true;
            }
        }

        return false;
    }

    void Sort(const PluginGraph& graph, std::list<Plugin>& plugins) {

        //Topological sort requires an index map, which std::list-based VertexList graphs don't have, so one needs to be built separately.

        map<vertex_t, size_t> index_map;
        boost::associative_property_map< map<vertex_t, size_t> > v_index_map(index_map);
        size_t i=0;
        BGL_FORALL_VERTICES(v, graph, PluginGraph)
            put(v_index_map, v, i++);

        //Now we can sort.

        std::list<vertex_t> sortedVertices;
        boost::topological_sort(graph, std::front_inserter(sortedVertices), boost::vertex_index_map(v_index_map));

        BOOST_LOG_TRIVIAL(info) << "Calculated order: ";
        list<loot::Plugin> tempPlugins;
        for (const auto &vertex: sortedVertices) {
            BOOST_LOG_TRIVIAL(info) << '\t' << graph[vertex].Name();
            tempPlugins.push_back(graph[vertex]);
        }
        plugins.swap(tempPlugins);
    }

    void CheckForCycles(const PluginGraph& graph) {

        //Depth-first search requires an index map, which std::list-based VertexList graphs don't have, so one needs to be built separately.

        map<vertex_t, size_t> index_map;
        boost::associative_property_map< map<vertex_t, size_t> > v_index_map(index_map);
        size_t i=0;
        BGL_FORALL_VERTICES(v, graph, PluginGraph)
            put(v_index_map, v, i++);

        loot::cycle_detector vis;
        boost::depth_first_search(graph, visitor(vis).vertex_index_map(v_index_map));
    }

    void AddSpecificEdges(PluginGraph& graph, std::map<std::string, int>& overriddenPriorities) {
        //Add edges for all relationships that aren't overlaps or priority differences.
        loot::vertex_it vit, vitend;
        for (boost::tie(vit, vitend) = boost::vertices(graph); vit != vitend; ++vit) {
            vertex_t parentVertex;
            int parentPriority = graph[*vit].Priority();

            BOOST_LOG_TRIVIAL(trace) << "Adding specific edges to vertex for \"" << graph[*vit].Name() << "\".";

            BOOST_LOG_TRIVIAL(trace) << "Adding edges for master flag differences.";

            loot::vertex_it vit2 = vit;
            ++vit2;
            while (vit2 != vitend) {

                if (graph[*vit].IsMaster() == graph[*vit2].IsMaster()) {
                    ++vit2;
                    continue;
                }

                vertex_t vertex, parentVertex;
                if (graph[*vit2].IsMaster()) {
                    parentVertex = *vit2;
                    vertex = *vit;
                } else {
                    parentVertex = *vit;
                    vertex = *vit2;
                }

                if (!boost::edge(parentVertex, vertex, graph).second) {

                    BOOST_LOG_TRIVIAL(trace) << "Adding edge from \"" << graph[parentVertex].Name() << "\" to \"" << graph[vertex].Name() << "\".";

                    boost::add_edge(parentVertex, vertex, graph);
                }
                ++vit2;
            }

            BOOST_LOG_TRIVIAL(trace) << "Adding in-edges for masters.";
            vector<string> strVec(graph[*vit].Masters());
            for (const auto &master: strVec) {
                if (loot::GetVertexByName(graph, master, parentVertex) &&
                    !boost::edge(parentVertex, *vit, graph).second) {

                    BOOST_LOG_TRIVIAL(trace) << "Adding edge from \"" << graph[parentVertex].Name() << "\" to \"" << graph[*vit].Name() << "\".";

                    boost::add_edge(parentVertex, *vit, graph);

                    int priority = graph[parentVertex].Priority();
                    if (priority > parentPriority) {
                        parentPriority = priority;
                    }
                }
            }
            BOOST_LOG_TRIVIAL(trace) << "Adding in-edges for requirements.";
            set<File> fileset(graph[*vit].Reqs());
            for (const auto &file: fileset) {
                if (loot::IsPlugin(file.Name()) &&
                    loot::GetVertexByName(graph, file.Name(), parentVertex) &&
                    !boost::edge(parentVertex, *vit, graph).second) {

                    BOOST_LOG_TRIVIAL(trace) << "Adding edge from \"" << graph[parentVertex].Name() << "\" to \"" << graph[*vit].Name() << "\".";

                    boost::add_edge(parentVertex, *vit, graph);

                    int priority = graph[parentVertex].Priority();
                    if (priority > parentPriority) {
                        parentPriority = priority;
                    }
                }
            }

            BOOST_LOG_TRIVIAL(trace) << "Adding in-edges for 'load after's.";
            fileset = graph[*vit].LoadAfter();
            for (const auto &file : fileset) {
                if (loot::IsPlugin(file.Name()) &&
                    loot::GetVertexByName(graph, file.Name(), parentVertex) &&
                    !boost::edge(parentVertex, *vit, graph).second) {

                    BOOST_LOG_TRIVIAL(trace) << "Adding edge from \"" << graph[parentVertex].Name() << "\" to \"" << graph[*vit].Name() << "\".";

                    boost::add_edge(parentVertex, *vit, graph);

                    int priority = graph[parentVertex].Priority();
                    if (priority > parentPriority) {
                        parentPriority = priority;
                    }
                }
            }

            //parentPriority is now the highest priority value of any plugin that the current plugin needs to load after.
            //Set the current plugin's priority to parentPlugin.
            if (parentPriority > 0 && graph[*vit].Priority() < parentPriority) {
                BOOST_LOG_TRIVIAL(trace) << "Overriding priority for " << graph[*vit].Name() << " from " << graph[*vit].Priority() << " to " << parentPriority;
                overriddenPriorities.insert(pair<string, int>(graph[*vit].Name(), graph[*vit].Priority()));
                graph[*vit].Priority(parentPriority);
            }

        }
    }

    void AddPriorityEdges(PluginGraph& graph) {
        loot::vertex_it vit, vitend;

        for (boost::tie(vit, vitend) = boost::vertices(graph); vit != vitend; ++vit) {
            BOOST_LOG_TRIVIAL(trace) << "Adding priority difference edges to vertex for \"" << graph[*vit].Name() << "\".";
            //Priority differences should only be taken account between plugins that conflict. 
            //However, an exception is made for plugins that contain only a header record,
            //as they are for loading BSAs, and in Skyrim that means the resources they load can
            //be affected by load order.

            loot::vertex_it vit2, vitend2;
            for (boost::tie(vit2, vitend2) = boost::vertices(graph); vit2 != vitend2; ++vit2) {

                if (graph[*vit].Priority() == graph[*vit2].Priority() 
                    || (abs(graph[*vit].Priority()) < max_priority && abs(graph[*vit2].Priority()) < max_priority
                        && !graph[*vit].FormIDs().empty() && !graph[*vit2].FormIDs().empty() && !graph[*vit].DoFormIDsOverlap(graph[*vit2])
                       )
                   ) {
                    continue;
                }

                BOOST_LOG_TRIVIAL(trace) << "Checking priority difference between \"" << graph[*vit].Name() << "\" and \"" << graph[*vit2].Name() << "\".";

                vertex_t vertex, parentVertex;
                //Modulo operator is not consistently defined for negative numbers except in C++11, so use function.
                int p1 = modulo(graph[*vit].Priority(), max_priority);
                int p2 = modulo(graph[*vit2].Priority(), max_priority);
                if (p1 < p2) {
                    parentVertex = *vit;
                    vertex = *vit2;
                } else {
                    parentVertex = *vit2;
                    vertex = *vit;
                }

                if (!boost::edge(parentVertex, vertex, graph).second &&
                    !EdgeCreatesCycle(graph, parentVertex, vertex)) {  //No edge going the other way, OK to add this edge.

                    BOOST_LOG_TRIVIAL(trace) << "Adding edge from \"" << graph[parentVertex].Name() << "\" to \"" << graph[vertex].Name() << "\".";

                    boost::add_edge(parentVertex, vertex, graph);
                }
            }
        }
    }

    void AddOverlapEdges(PluginGraph& graph) {
        loot::vertex_it vit, vitend;

        for (boost::tie(vit, vitend) = boost::vertices(graph); vit != vitend; ++vit) {

            BOOST_LOG_TRIVIAL(trace) << "Adding overlap edges to vertex for \"" << graph[*vit].Name() << "\".";

            if (graph[*vit].NumOverrideFormIDs() == 0) {
                BOOST_LOG_TRIVIAL(trace) << "Skipping vertex for \"" << graph[*vit].Name() << "\": the plugin contains no override records.";
                continue;
            }

            loot::vertex_it vit2, vitend2;
            for (boost::tie(vit2, vitend2) = boost::vertices(graph); vit2 != vitend2; ++vit2) {
                if (vit == vit2 || boost::edge(*vit, *vit2, graph).second || boost::edge(*vit2, *vit, graph).second)
                    //Vertices are the same or are already linked.
                    continue;

                BOOST_LOG_TRIVIAL(trace) << "Checking edge validity between \"" << graph[*vit].Name() << "\" and \"" << graph[*vit2].Name() << "\".";

                if (graph[*vit].DoFormIDsOverlap(graph[*vit2])) {
                    vertex_t vertex, parentVertex;
                    if (graph[*vit].NumOverrideFormIDs() > graph[*vit2].NumOverrideFormIDs()) {
                        parentVertex = *vit;
                        vertex = *vit2;
                    }
                    else if (graph[*vit].NumOverrideFormIDs() < graph[*vit2].NumOverrideFormIDs()) {
                        parentVertex = *vit2;
                        vertex = *vit;
                    }
                    else if (graph[*vit].Name() < graph[*vit2].Name()) {  //There needs to be an edge between the two, but direction cannot be decided using overlap size. Just use names.
                        parentVertex = *vit;
                        vertex = *vit2;
                    }
                    else {
                        parentVertex = *vit2;
                        vertex = *vit;
                    }

                    if (!EdgeCreatesCycle(graph, parentVertex, vertex)) {  //No edge going the other way, OK to add this edge.

                        BOOST_LOG_TRIVIAL(trace) << "Adding edge from \"" << graph[parentVertex].Name() << "\" to \"" << graph[vertex].Name() << "\".";

                        boost::add_edge(parentVertex, vertex, graph);
                    }
                }
            }
        }
    }

    bool EdgeCreatesCycle(PluginGraph& graph, vertex_t u, vertex_t v) {
        //A cycle is created when adding the edge (u,v) if there already exists a path from v to u, so check for that using a breadth-first search.

        //Breadth-first search requires an index map, which std::list-based VertexList graphs don't have, so one needs to be built separately.

        map<vertex_t, size_t> index_map;
        boost::associative_property_map< map<vertex_t, size_t> > v_index_map(index_map);
        size_t i=0;
        BGL_FORALL_VERTICES(v, graph, PluginGraph)
            put(v_index_map, v, i++);

        map<vertex_t, vertex_t> predecessor_map;
        boost::associative_property_map< map<vertex_t, vertex_t> > v_predecessor_map(predecessor_map);

        boost::breadth_first_search(graph, v, visitor(boost::make_bfs_visitor(boost::record_predecessors(v_predecessor_map, boost::on_tree_edge()))).vertex_index_map(v_index_map));

        return predecessor_map.find(u) != predecessor_map.end();
    }
}
