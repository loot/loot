/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2015    WrinklyNinja

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

#include "game/game.h"
#include "error.h"
#include "plugin_sorter.h"
#include "helpers/streams.h"
#include "helpers/helpers.h"

#include <cstdlib>

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/locale.hpp>
#include <boost/format.hpp>

using namespace std;

namespace loot {
    typedef boost::graph_traits<PluginGraph>::vertex_iterator vertex_it;
    typedef boost::graph_traits<PluginGraph>::edge_descriptor edge_t;
    typedef boost::graph_traits<PluginGraph>::edge_iterator edge_it;

    struct cycle_detector : public boost::dfs_visitor < > {
        cycle_detector() {}

        std::list<std::string> trail;

        inline void tree_edge(edge_t e, const PluginGraph& g) {
            vertex_t source = boost::source(e, g);

            string name = g[source].Name();

            // Check if the plugin already exists in the recorded trail.
            auto it = find(trail.begin(), trail.end(), name);

            if (it != trail.end()) {
                // Erase everything from this position onwards, as it doesn't
                // contribute to a forward-cycle.
                trail.erase(it, trail.end());
            }

            trail.push_back(name);
        }

        inline void back_edge(edge_t e, const PluginGraph& g) {
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

            throw loot::error(loot::error::sorting_error, (boost::format(boost::locale::translate("Cyclic interaction detected between plugins \"%1%\" and \"%2%\". Back cycle: %3%")) % g[vSource].Name() % g[vTarget].Name() % backCycle).str());
        }
    };

    struct path_detector : public boost::bfs_visitor < > {
        vertex_t target;

        inline void discover_vertex(vertex_t u, const PluginGraph& g) {
            if (u == target)
                throw error(error::ok, "Found a path.");
        }
    };

    std::list<Plugin> PluginSorter::Sort(Game& game,
                                         const unsigned int language,
                                         std::function<void(const std::string&)> progressCallback) {
        progressCallback(boost::locale::translate("Building plugin graph..."));
        BuildPluginGraph(game, language);

        // Get the existing load order.
        oldLoadOrder = game.GetLoadOrder();
        BOOST_LOG_TRIVIAL(info) << "Fetched existing load order: ";
        for (const auto &plugin : oldLoadOrder)
            BOOST_LOG_TRIVIAL(info) << plugin;

        // Now add edges and sort.
        progressCallback(boost::locale::translate("Adding edges to plugin graph and performing topological sort..."));

        //Now add the interactions between plugins to the graph as edges.
        BOOST_LOG_TRIVIAL(info) << "Adding edges to plugin graph.";
        BOOST_LOG_TRIVIAL(debug) << "Adding non-overlap edges.";
        AddSpecificEdges();

        BOOST_LOG_TRIVIAL(debug) << "Adding priority edges.";
        AddPriorityEdges();

        BOOST_LOG_TRIVIAL(debug) << "Adding overlap edges.";
        AddOverlapEdges();

        BOOST_LOG_TRIVIAL(debug) << "Adding tie-break edges.";
        AddTieBreakEdges();

        BOOST_LOG_TRIVIAL(info) << "Checking to see if the graph is cyclic.";
        CheckForCycles();

        //Now we can sort.
        BOOST_LOG_TRIVIAL(info) << "Performing a topological sort.";
        list<vertex_t> sortedVertices;
        boost::topological_sort(graph, std::front_inserter(sortedVertices), boost::vertex_index_map(vertexIndexMap));

        // Check that the sorted path is Hamiltonian (ie. unique).
        for (auto it = sortedVertices.begin(); it != sortedVertices.end(); ++it) {
            if (next(it) != sortedVertices.end() && !boost::edge(*it, *next(it), graph).second) {
                BOOST_LOG_TRIVIAL(error) << "The calculated load order is not unique. No edge exists between"
                    << graph[*it].Name() << " and " << graph[*next(it)].Name() << ".";
            }
        }

        // Output a plugin list using the sorted vertices.
        BOOST_LOG_TRIVIAL(info) << "Calculated order: ";
        list<Plugin> plugins;
        for (const auto &vertex : sortedVertices) {
            BOOST_LOG_TRIVIAL(info) << '\t' << graph[vertex].Name();
            plugins.push_back(graph[vertex]);
        }
        return plugins;
    }

    void PluginSorter::BuildPluginGraph(Game& game, const unsigned int language) {
        BOOST_LOG_TRIVIAL(info) << "Merging masterlist, userlist into plugin list, evaluating conditions and checking for install validity.";
        for (const auto &plugin : game.plugins) {
            vertex_t v = boost::add_vertex(plugin.second, graph);
            BOOST_LOG_TRIVIAL(trace) << "Merging for plugin \"" << graph[v].Name() << "\"";

            //Check if there is a plugin entry in the masterlist. This will also find matching regex entries.
            BOOST_LOG_TRIVIAL(trace) << "Merging masterlist data down to plugin list data.";
            graph[v].MergeMetadata(game.masterlist.FindPlugin(graph[v]));

            //Check if there is a plugin entry in the userlist. This will also find matching regex entries.
            PluginMetadata ulistPlugin = game.userlist.FindPlugin(graph[v]);

            if (!ulistPlugin.HasNameOnly() && ulistPlugin.Enabled()) {
                BOOST_LOG_TRIVIAL(trace) << "Merging userlist data down to plugin list data.";
                graph[v].MergeMetadata(ulistPlugin);
            }

            //Now that items are merged, evaluate any conditions they have.
            BOOST_LOG_TRIVIAL(trace) << "Evaluate conditions for merged plugin data.";
            try {
                graph[v].EvalAllConditions(game, language);
            }
            catch (std::exception& e) {
                BOOST_LOG_TRIVIAL(error) << "\"" << graph[v].Name() << "\" contains a condition that could not be evaluated. Details: " << e.what();
                list<Message> messages(graph[v].Messages());
                messages.push_back(loot::Message(loot::Message::error, (boost::format(boost::locale::translate("\"%1%\" contains a condition that could not be evaluated. Details: %2%")) % graph[v].Name() % e.what()).str()));
                graph[v].Messages(messages);
            }

            //Also check install validity.
            graph[v].CheckInstallValidity(game);
        }

        // Prebuild an index map, which std::list-based VertexList graphs don't have.
        vertexIndexMap = vertex_map_t(indexMap);
        size_t i = 0;
        BGL_FORALL_VERTICES(v, graph, PluginGraph)
            put(vertexIndexMap, v, i++);
    }

    bool PluginSorter::GetVertexByName(const std::string& name, vertex_t& vertex) const {
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

    void PluginSorter::CheckForCycles() const {
        cycle_detector vis;
        boost::depth_first_search(graph, visitor(vis).vertex_index_map(vertexIndexMap));
    }

    bool PluginSorter::EdgeCreatesCycle(const vertex_t& u, const vertex_t& v) const {
        //A cycle is created when adding the edge (u,v) if there already exists a path from v to u, so check for that using a breadth-first search.
        path_detector vis;
        vis.target = u;
        try {
            boost::breadth_first_search(graph, v, visitor(vis).vertex_index_map(vertexIndexMap));
        }
        catch (error& e) {
            if (e.code() == error::ok)
                return true;
        }
        return false;
    }

    void PluginSorter::AddSpecificEdges() {
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
                }
                else {
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
            for (const auto &master : strVec) {
                if (GetVertexByName(master, parentVertex) &&
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
            for (const auto &file : fileset) {
                if (GetVertexByName(file.Name(), parentVertex) &&
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
                if (GetVertexByName(file.Name(), parentVertex) &&
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
                graph[*vit].Priority(parentPriority);
            }
        }
    }

    void PluginSorter::AddPriorityEdges() {
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

                //BOOST_LOG_TRIVIAL(trace) << "Checking priority difference between \"" << graph[*vit].Name() << "\" and \"" << graph[*vit2].Name() << "\".";

                vertex_t vertex, parentVertex;
                //Modulo operator is not consistently defined for negative numbers except in C++11, so use function.
                int p1 = graph[*vit].Priority() % max_priority;
                int p2 = graph[*vit2].Priority() % max_priority;
                if (p1 < p2) {
                    parentVertex = *vit;
                    vertex = *vit2;
                }
                else {
                    parentVertex = *vit2;
                    vertex = *vit;
                }

                if (!boost::edge(parentVertex, vertex, graph).second &&
                    !EdgeCreatesCycle(parentVertex, vertex)) {  //No edge going the other way, OK to add this edge.
                    BOOST_LOG_TRIVIAL(trace) << "Adding edge from \"" << graph[parentVertex].Name() << "\" to \"" << graph[vertex].Name() << "\".";

                    boost::add_edge(parentVertex, vertex, graph);
                }
            }
        }
    }

    void PluginSorter::AddOverlapEdges() {
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
                    else {
                        // There's no way to determine the order between the two, so just leave them to be treated like
                        // any two unlinked, non-conflicting plugins in the next pass.
                        continue;
                    }

                    //BOOST_LOG_TRIVIAL(trace) << "Checking edge validity between \"" << graph[*vit].Name() << "\" and \"" << graph[*vit2].Name() << "\".";
                    if (!EdgeCreatesCycle(parentVertex, vertex)) {  //No edge going the other way, OK to add this edge.
                        BOOST_LOG_TRIVIAL(trace) << "Adding edge from \"" << graph[parentVertex].Name() << "\" to \"" << graph[vertex].Name() << "\".";

                        boost::add_edge(parentVertex, vertex, graph);
                    }
                }
            }
        }
    }

    int PluginSorter::plugincmp(const std::string& plugin1, const std::string& plugin2) const {
        auto it1 = find(oldLoadOrder.begin(), oldLoadOrder.end(), plugin1);
        auto it2 = find(oldLoadOrder.begin(), oldLoadOrder.end(), plugin2);

        if (it1 != oldLoadOrder.end() && it2 == oldLoadOrder.end())
            return -1;
        else if (it1 == oldLoadOrder.end() && it2 != oldLoadOrder.end())
            return 1;
        else if (it1 != oldLoadOrder.end() && it2 != oldLoadOrder.end()) {
            if (distance(oldLoadOrder.begin(), it1) < distance(oldLoadOrder.begin(), it2))
                return -1;
            else
                return 1;
        }
        else {
            // Neither plugin has a load order position. Need to use another
            // comparison to get an ordering.

            // Compare plugin basenames.
            string name1 = boost::locale::to_lower(plugin1);
            name1 = name1.substr(0, name1.length() - 4);
            string name2 = boost::locale::to_lower(plugin2);
            name2 = name2.substr(0, name2.length() - 4);

            if (name1 < name2)
                return -1;
            else if (name2 < name1)
                return 1;
            else {
                // Could be a .esp and .esm plugin with the same basename,
                // compare whole filenames.
                if (plugin1 < plugin2)
                    return -1;
                else
                    return 1;
            }
        }
        return 0;
    }

    void PluginSorter::AddTieBreakEdges() {
        // In order for the sort to be performed stably, there must be only one possible result.
        // This can be enforced by adding edges between all vertices that aren't already linked.
        // Use existing load order to decide the direction of these edges.
        loot::vertex_it vit, vitend;
        for (boost::tie(vit, vitend) = boost::vertices(graph); vit != vitend; ++vit) {
            BOOST_LOG_TRIVIAL(trace) << "Adding tie-break edges to vertex for \"" << graph[*vit].Name() << "\".";

            loot::vertex_it vit2, vitend2;
            for (boost::tie(vit2, vitend2) = boost::vertices(graph); vit2 != vitend2; ++vit2) {
                if (vit == vit2 || boost::edge(*vit, *vit2, graph).second || boost::edge(*vit2, *vit, graph).second)
                    //Vertices are the same or are already linked.
                    continue;

                vertex_t vertex, parentVertex;
                if (plugincmp(graph[*vit].Name(), graph[*vit2].Name()) < 0) {
                    parentVertex = *vit;
                    vertex = *vit2;
                }
                else {
                    parentVertex = *vit2;
                    vertex = *vit;
                }

                //BOOST_LOG_TRIVIAL(trace) << "Checking edge validity between \"" << graph[*vit].Name() << "\" and \"" << graph[*vit2].Name() << "\".";
                if (!EdgeCreatesCycle(parentVertex, vertex)) {  //No edge going the other way, OK to add this edge.
                    BOOST_LOG_TRIVIAL(trace) << "Adding edge from \"" << graph[parentVertex].Name() << "\" to \"" << graph[vertex].Name() << "\".";

                    boost::add_edge(parentVertex, vertex, graph);
                }
            }
        }
    }
}
