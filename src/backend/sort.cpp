/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014    WrinklyNinja

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

#include "game.h"
#include "helpers.h"
#include "graph.h"

#include <boost/log/trivial.hpp>
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>

using namespace std;

using boost::format;

namespace loc = boost::locale;
namespace fs = boost::filesystem;

namespace loot {

    std::list<Plugin> Game::Sort(const unsigned int language, std::function<void(const std::string&)> progressCallback) {
        //Create a plugin graph containing the plugin and masterlist data.
        loot::PluginGraph graph;

        progressCallback("Building plugin graph...");
        BOOST_LOG_TRIVIAL(info) << "Merging masterlist, userlist into plugin list, evaluating conditions and checking for install validity.";
        for (const auto &plugin : this->plugins) {
            vertex_t v = boost::add_vertex(plugin.second, graph);
            list<loot::Plugin>::iterator pos;
            BOOST_LOG_TRIVIAL(trace) << "Merging for plugin \"" << graph[v].Name() << "\"";

            //Check if there is a plugin entry in the masterlist. This will also find matching regex entries.
            pos = std::find(this->masterlist.plugins.begin(), this->masterlist.plugins.end(), graph[v]);

            if (pos != this->masterlist.plugins.end()) {
                BOOST_LOG_TRIVIAL(trace) << "Merging masterlist data down to plugin list data.";
                graph[v].MergeMetadata(*pos);
            }

            //Check if there is a plugin entry in the userlist. This will also find matching regex entries.
            pos = std::find(this->userlist.plugins.begin(), this->userlist.plugins.end(), graph[v]);

            if (pos != this->userlist.plugins.end() && pos->Enabled()) {
                BOOST_LOG_TRIVIAL(trace) << "Merging userlist data down to plugin list data.";
                graph[v].MergeMetadata(*pos);
            }

            //Now that items are merged, evaluate any conditions they have.
            BOOST_LOG_TRIVIAL(trace) << "Evaluate conditions for merged plugin data.";
            try {
                graph[v].EvalAllConditions(*this, language);
            }
            catch (std::exception& e) {
                BOOST_LOG_TRIVIAL(error) << "\"" << graph[v].Name() << "\" contains a condition that could not be evaluated. Details: " << e.what();
                list<Message> messages(graph[v].Messages());
                messages.push_back(loot::Message(loot::Message::error, (format(loc::translate("\"%1%\" contains a condition that could not be evaluated. Details: %2%")) % graph[v].Name() % e.what()).str()));
                graph[v].Messages(messages);
            }

            //Also check install validity.
            BOOST_LOG_TRIVIAL(trace) << "Checking that the current install is valid according to this plugin's data.";
            graph[v].CheckInstallValidity(*this);
        }

        BOOST_LOG_TRIVIAL(info) << "Building the plugin dependency graph...";

        //Now add the interactions between plugins to the graph as edges.
        std::map<std::string, int> overriddenPriorities;
        BOOST_LOG_TRIVIAL(debug) << "Adding non-overlap edges.";
        loot::AddSpecificEdges(graph, overriddenPriorities);

        BOOST_LOG_TRIVIAL(debug) << "Adding priority edges.";
        loot::AddPriorityEdges(graph);

        BOOST_LOG_TRIVIAL(debug) << "Adding overlap edges.";
        loot::AddOverlapEdges(graph);

        progressCallback("Checking for graph cycles...");

        BOOST_LOG_TRIVIAL(info) << "Checking to see if the graph is cyclic.";
        loot::CheckForCycles(graph);

        for (const auto &overriddenPriority : overriddenPriorities) {
            vertex_t vertex;
            if (loot::GetVertexByName(graph, overriddenPriority.first, vertex)) {
                graph[vertex].Priority(overriddenPriority.second);
            }
        }

        BOOST_LOG_TRIVIAL(info) << "Performing a topological sort.";
        progressCallback("Performing topological sort...");
        return loot::Sort(graph);
    }
}