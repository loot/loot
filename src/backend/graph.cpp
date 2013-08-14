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
    vertex_t GetPluginVertex(PluginGraph& graph, const Plugin& plugin, boost::unordered_map<std::string, vertex_t>& pluginVertexMap) {

        vertex_t vertex;
        string name = boost::to_lower_copy(plugin.Name());

        boost::unordered_map<string, vertex_t>::iterator vertexMapIt = pluginVertexMap.find(name);
        if (vertexMapIt == pluginVertexMap.end()) {
            BOOST_LOG_TRIVIAL(trace) << "Vertex for \"" << name << "\" doesn't exist, creating one.";
            Plugin p;
           // vertex = boost::add_vertex(p, graph);
            BOOST_LOG_TRIVIAL(trace) << "Adding vertex to map.";
            pluginVertexMap.emplace(name, vertex);
        } else
            vertex = vertexMapIt->second;

        return vertex;
    }

    //The map maps each plugin name to a vector of names of plugins that overlap with it and should load before it.
    void CalcPluginOverlaps(const std::list<Plugin>& plugins, boost::unordered_map< std::string, std::vector<std::string> >& overlapMap) {
        for (list<Plugin>::const_iterator it=plugins.begin(),
                                          endit=plugins.end();
                                          it != endit;
                                          ++it) {
            list<Plugin>::const_iterator jt = it;
            ++jt;
            for (; jt != endit; ++jt) {
                BOOST_LOG_TRIVIAL(trace) << "Checking for FormID overlap between \"" << it->Name() << "\" and \"" << jt->Name() << "\".";
                if (it->DoFormIDsOverlap(*jt)) {
                    std::string key;
                    std::string value;
                    //Priority values should override the number of override records as the deciding factor if they differ.
                    if (it->MustLoadAfter(*jt) || jt->MustLoadAfter(*it) || it->Priority() != jt->Priority())
                        continue;
                    if (it->NumOverrideFormIDs() >= jt->NumOverrideFormIDs()) {
                        key = jt->Name();
                        value = it->Name();
                    } else {
                        key = it->Name();
                        value = jt->Name();
                    }
                    boost::unordered_map< string, vector<string> >::iterator mapIt = overlapMap.find(key);
                    if (mapIt == overlapMap.end()) {
                        overlapMap.insert(pair<string, vector<string> >(key, vector<string>(1, value)));
                    } else {
                        mapIt->second.push_back(value);
                    }
                }
            }
        }
    }

    void CalcPriorityMap(const std::list<Plugin>& plugins, boost::unordered_map< std::string, std::vector<std::string> >& priorityMap) {
        for (list<Plugin>::const_iterator it=plugins.begin(),
                                          endit=plugins.end();
                                          it != endit;
                                          ++it) {
            list<Plugin>::const_iterator jt = it;
            ++jt;
            for (; jt != endit; ++jt) {
                BOOST_LOG_TRIVIAL(trace) << "Checking for priority difference between \"" << it->Name() << "\" and \"" << jt->Name() << "\".";
                if (it->MustLoadAfter(*jt) || jt->MustLoadAfter(*it) || it->Priority() == jt->Priority())
                    continue;

                std::string key;
                std::string value;
                if (it->Priority() < jt->Priority()) {
                    key = jt->Name();
                    value = it->Name();
                } else if (jt->Priority() < it->Priority()) {
                    key = it->Name();
                    value = jt->Name();
                }
                boost::unordered_map< string, vector<string> >::iterator mapIt = priorityMap.find(key);
                if (mapIt == priorityMap.end()) {
                    priorityMap.insert(pair<string, vector<string> >(key, vector<string>(1, value)));
                } else {
                    mapIt->second.push_back(value);
                }
            }
        }
    }

    bool GetVertexByName(const PluginGraph& graph, const std::string& name, vertex_t& vertex) {
        vertex_it vit, vit_end;
        boost::tie(vit, vit_end) = boost::vertices(graph);

        for (vit, vit_end; vit != vit_end; ++vit) {
            if (boost::iequals(graph[*vit]->Name(), name)) {
      //      if (graph[*vit]->Name() == name) {
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
            names.push_back(graph[*vit]->Name());
        }

        //Also, writing the graph requires an index map, which std::list-based VertexList graphs don't have, so one needs to be built separately.

        map<vertex_t, string> index_map;
        boost::associative_property_map< map<vertex_t, string> > v_index_map(index_map);
        BGL_FORALL_VERTICES(v, graph, PluginGraph)
            put(v_index_map, v, graph[v]->Name());

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
            BOOST_LOG_TRIVIAL(info) << '\t' << graph[*it]->Name();
            tempPlugins.push_back(*graph[*it]);
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

    void AddNonOverlapEdges(PluginGraph& graph, const boost::unordered_map< std::string, std::vector<std::string> >& priorityMap) {
        //First find the position of the first non-master.
        boss::vertex_it vitFirstNonMaster, vit, vitend;
        for (boost::tie(vit, vitend) = boost::vertices(graph); vit != vitend; ++vit) {
            if (!graph[*vit]->IsMaster()) {
                vitFirstNonMaster = vit;
                break;
            }
        }

        //Now add edges for all relationships that aren't overlaps.
        for (boost::tie(vit, vitend) = boost::vertices(graph); vit != vitend; ++vit) {
            vertex_t parentVertex;

            BOOST_LOG_TRIVIAL(trace) << "Adding non-overlap in-edges to vertex for \"" << graph[*vit]->Name() << "\".";

            if (graph[*vit]->IsMaster()) {
                BOOST_LOG_TRIVIAL(trace) << "Adding out-edges for non-master plugins.";

                for (boss::vertex_it vit2 = vitFirstNonMaster; vit2 != vitend; ++vit2) {
                    BOOST_LOG_TRIVIAL(trace) << "Adding edge from \"" << graph[*vit]->Name() << "\" to \"" << graph[*vit2]->Name() << "\".";

                    if (!boost::edge(*vit, *vit2, graph).second)  //To avoid duplicates (helps visualisation).
                        boost::add_edge(*vit, *vit2, graph);
                }
            }

            BOOST_LOG_TRIVIAL(trace) << "Adding in-edges for masters.";
            vector<string> strVec(graph[*vit]->Masters());
            for (vector<string>::const_iterator it=strVec.begin(), itend=strVec.end(); it != itend; ++it) {
                if (boss::GetVertexByName(graph, *it, parentVertex) &&
                    !boost::edge(parentVertex, *vit, graph).second) {

                    BOOST_LOG_TRIVIAL(trace) << "Adding edge from \"" << graph[parentVertex]->Name() << "\" to \"" << graph[*vit]->Name() << "\".";

                    boost::add_edge(parentVertex, *vit, graph);
                }
            }
            BOOST_LOG_TRIVIAL(trace) << "Adding in-edges for requirements.";
            set<File> fileset(graph[*vit]->Reqs());
            for (set<File>::const_iterator it=fileset.begin(), itend=fileset.end(); it != itend; ++it) {
                if (boss::IsPlugin(it->Name()) &&
                    boss::GetVertexByName(graph, it->Name(), parentVertex) &&
                    !boost::edge(parentVertex, *vit, graph).second) {

                    BOOST_LOG_TRIVIAL(trace) << "Adding edge from \"" << graph[parentVertex]->Name() << "\" to \"" << graph[*vit]->Name() << "\".";

                    boost::add_edge(parentVertex, *vit, graph);
                }
            }

            BOOST_LOG_TRIVIAL(trace) << "Adding in-edges for 'load after's.";
            fileset = graph[*vit]->LoadAfter();
            for (set<File>::const_iterator it=fileset.begin(), itend=fileset.end(); it != itend; ++it) {
                if (boss::IsPlugin(it->Name()) &&
                    boss::GetVertexByName(graph, it->Name(), parentVertex) &&
                    !boost::edge(parentVertex, *vit, graph).second) {

                    BOOST_LOG_TRIVIAL(trace) << "Adding edge from \"" << graph[parentVertex]->Name() << "\" to \"" << graph[*vit]->Name() << "\".";

                    boost::add_edge(parentVertex, *vit, graph);
                }
            }

            BOOST_LOG_TRIVIAL(trace) << "Adding in-edges for priority differences.";
            boost::unordered_map< string, vector<string> >::const_iterator priorityIt = priorityMap.find(graph[*vit]->Name());
            if (priorityIt != priorityMap.end()) {
                for (vector<string>::const_iterator it=priorityIt->second.begin(), itend=priorityIt->second.end(); it != itend; ++it) {
                    if (boss::GetVertexByName(graph, *it, parentVertex) &&
                        !boost::edge(parentVertex, *vit, graph).second &&
                        !EdgeCreatesCycle(graph, parentVertex, *vit)) {  //No edge going the other way, OK to add this edge.

                        BOOST_LOG_TRIVIAL(trace) << "Adding edge from \"" << graph[parentVertex]->Name() << "\" to \"" << graph[*vit]->Name() << "\".";

                        boost::add_edge(parentVertex, *vit, graph);
                    }
                }
            }
        }
    }

    void AddOverlapEdges(PluginGraph& graph, const boost::unordered_map< std::string, std::vector<std::string> >& overlapMap) {
        boss::vertex_it vit, vitend;

        for (boost::tie(vit, vitend) = boost::vertices(graph); vit != vitend; ++vit) {
            vertex_t parentVertex;

            BOOST_LOG_TRIVIAL(trace) << "Adding overlap in-edges to vertex for \"" << graph[*vit]->Name() << "\".";


            //Now add any overlaps, except where an edge already exists between the two plugins, going the other way, since overlap-based edges have the lowest priority and are not a candidate for causing cyclic loop errors.
            boost::unordered_map< string, vector<string> >::const_iterator overlapIt = overlapMap.find(graph[*vit]->Name());
            if (overlapIt != overlapMap.end()) {
                for (vector<string>::const_iterator it=overlapIt->second.begin(), itend=overlapIt->second.end(); it != itend; ++it) {
                    if (boss::GetVertexByName(graph, *it, parentVertex) &&
                        !boost::edge(parentVertex, *vit, graph).second &&
                        !EdgeCreatesCycle(graph, parentVertex, *vit)) {  //No edge going the other way, OK to add this edge.

                        BOOST_LOG_TRIVIAL(trace) << "Adding edge from \"" << graph[parentVertex]->Name() << "\" to \"" << graph[*vit]->Name() << "\".";

                        boost::add_edge(parentVertex, *vit, graph);
                    }
                }
            }
        }
    }

    void ClearEdges(PluginGraph& graph) {

        edge_it it, itend;
        for (boost::tie(it, itend) = boost::edges(graph); it != itend; ++it) {
            boost::remove_edge(*it, graph);
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
