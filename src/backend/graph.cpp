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

#include "graph.h"
#include "streams.h"

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <boost/graph/breadth_first_search.hpp>

using namespace std;

namespace boss {

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

    void SaveGraph(const PluginGraph& graph, const boost::filesystem::path outpath) {
        //First need to extract vertex names, since their stored as private members otherwise.
        vector<string> names;
        vertex_it vit, vit_end;
        boost::tie(vit, vit_end) = boost::vertices(graph);

        for (vit, vit_end; vit != vit_end; ++vit) {
            names.push_back(graph[*vit].Name());
        }

        //Also, writing the graph requires an index map, which std::list-based VertexList graphs don't have, so one needs to be built separately.

        map<vertex_t, string> index_map;
        boost::associative_property_map< map<vertex_t, string> > v_index_map(index_map);
        BGL_FORALL_VERTICES(v, graph, PluginGraph)
            put(v_index_map, v, graph[v].Name());

        //Now write graph to file.
        boss::ofstream out(outpath);
        boost::write_graphviz(out, graph, boost::default_writer(), boost::default_writer(), boost::default_writer(), v_index_map);
        out.close();
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
        list<boss::Plugin> tempPlugins;
        for (std::list<vertex_t>::iterator it = sortedVertices.begin(), endit = sortedVertices.end(); it != endit; ++it) {
            BOOST_LOG_TRIVIAL(info) << '\t' << graph[*it].Name();
            tempPlugins.push_back(graph[*it]);
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

        boss::cycle_detector vis;
        boost::depth_first_search(graph, visitor(vis).vertex_index_map(v_index_map));
    }

    void AddNonOverlapEdges(PluginGraph& graph) {
        //Add edges for all relationships that aren't overlaps.
        boss::vertex_it vit, vitend;
        for (boost::tie(vit, vitend) = boost::vertices(graph); vit != vitend; ++vit) {
            vertex_t parentVertex;

            BOOST_LOG_TRIVIAL(trace) << "Adding non-overlap edges to vertex for \"" << graph[*vit].Name() << "\".";

            BOOST_LOG_TRIVIAL(trace) << "Adding edges for master flag differences.";

            boss::vertex_it vit2 = vit;
            ++vit2;
            for (vit2,vitend; vit2 != vitend; ++vit2) {

                if (graph[*vit].IsMaster() == graph[*vit2].IsMaster())
                    continue;

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
            }

            BOOST_LOG_TRIVIAL(trace) << "Adding in-edges for masters.";
            vector<string> strVec(graph[*vit].Masters());
            for (vector<string>::const_iterator it=strVec.begin(), itend=strVec.end(); it != itend; ++it) {
                if (boss::GetVertexByName(graph, *it, parentVertex) &&
                    !boost::edge(parentVertex, *vit, graph).second) {

                    BOOST_LOG_TRIVIAL(trace) << "Adding edge from \"" << graph[parentVertex].Name() << "\" to \"" << graph[*vit].Name() << "\".";

                    boost::add_edge(parentVertex, *vit, graph);
                }
            }
            BOOST_LOG_TRIVIAL(trace) << "Adding in-edges for requirements.";
            set<File> fileset(graph[*vit].Reqs());
            for (set<File>::const_iterator it=fileset.begin(), itend=fileset.end(); it != itend; ++it) {
                if (boss::IsPlugin(it->Name()) &&
                    boss::GetVertexByName(graph, it->Name(), parentVertex) &&
                    !boost::edge(parentVertex, *vit, graph).second) {

                    BOOST_LOG_TRIVIAL(trace) << "Adding edge from \"" << graph[parentVertex].Name() << "\" to \"" << graph[*vit].Name() << "\".";

                    boost::add_edge(parentVertex, *vit, graph);
                }
            }

            BOOST_LOG_TRIVIAL(trace) << "Adding in-edges for 'load after's.";
            fileset = graph[*vit].LoadAfter();
            for (set<File>::const_iterator it=fileset.begin(), itend=fileset.end(); it != itend; ++it) {
                if (boss::IsPlugin(it->Name()) &&
                    boss::GetVertexByName(graph, it->Name(), parentVertex) &&
                    !boost::edge(parentVertex, *vit, graph).second) {

                    BOOST_LOG_TRIVIAL(trace) << "Adding edge from \"" << graph[parentVertex].Name() << "\" to \"" << graph[*vit].Name() << "\".";

                    boost::add_edge(parentVertex, *vit, graph);
                }
            }

            BOOST_LOG_TRIVIAL(trace) << "Adding edges for priority differences.";
            vit2 = vit;
            ++vit2;
            for (vit2,vitend; vit2 != vitend; ++vit2) {
                BOOST_LOG_TRIVIAL(trace) << "Checking for priority difference between \"" << graph[*vit].Name() << "\" and \"" << graph[*vit2].Name() << "\".";
                if (graph[*vit].MustLoadAfter(graph[*vit2]) || graph[*vit2].MustLoadAfter(graph[*vit]) || graph[*vit].Priority() == graph[*vit2].Priority())
                    continue;

                vertex_t vertex, parentVertex;
                if (graph[*vit].Priority() < graph[*vit2].Priority()) {
                    parentVertex = *vit2;
                    vertex = *vit;
                } else {
                    parentVertex = *vit;
                    vertex = *vit2;
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
        boss::vertex_it vit, vitend;

        for (boost::tie(vit, vitend) = boost::vertices(graph); vit != vitend; ++vit) {
            vertex_t parentVertex;

            BOOST_LOG_TRIVIAL(trace) << "Adding overlap edges to vertex for \"" << graph[*vit].Name() << "\".";

            boss::vertex_it vit2 = vit;
            ++vit2;
            for (vit2,vitend; vit2 != vitend; ++vit2) {
                BOOST_LOG_TRIVIAL(trace) << "Checking for FormID overlap between \"" << graph[*vit].Name() << "\" and \"" << graph[*vit2].Name() << "\".";

                if (graph[*vit].DoFormIDsOverlap(graph[*vit2])) {

                    if (graph[*vit].MustLoadAfter(graph[*vit2]) || graph[*vit2].MustLoadAfter(graph[*vit]) || graph[*vit].Priority() != graph[*vit2].Priority())
                        continue;

                    vertex_t vertex, parentVertex;
                    if (graph[*vit].NumOverrideFormIDs() >= graph[*vit2].NumOverrideFormIDs()) {
                        parentVertex = *vit2;
                        vertex = *vit;
                    } else {
                        parentVertex = *vit;
                        vertex = *vit2;
                    }

                    if (!boost::edge(parentVertex, vertex, graph).second &&
                        !EdgeCreatesCycle(graph, parentVertex, vertex)) {  //No edge going the other way, OK to add this edge.

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
