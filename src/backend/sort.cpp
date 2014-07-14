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

    void Game::SortPrep(const unsigned int language, std::list<Message>& messages, std::function<void(const std::string&)> progressCallback) {
        boost::thread_group group;

        BOOST_LOG_TRIVIAL(info) << "Using message language: " << Language(language).Name();

        ///////////////////////////////////////////////////////
        // Load Plugins & Lists
        ///////////////////////////////////////////////////////

        progressCallback("Reading installed plugins...");

        group.create_thread([this, language, &messages]() {
            try {
                this->masterlist.Load(*this, language);
            }
            catch (exception &e) {
                messages.push_back(loot::Message(loot::Message::error, (format(loc::translate("Masterlist parsing failed. Details: %1%")) % e.what()).str()));
            }
        });
        group.create_thread([this]() {
            this->LoadPlugins(false);
        });
        group.join_all();

        //Now load userlist.
        if (fs::exists(this->UserlistPath())) {
            BOOST_LOG_TRIVIAL(debug) << "Parsing userlist at: " << this->UserlistPath();

            try {
                this->userlist.Load(this->UserlistPath());
            }
            catch (exception& e) {
                BOOST_LOG_TRIVIAL(error) << "Userlist parsing failed. Details: " << e.what();
                messages.push_back(loot::Message(loot::Message::error, (format(loc::translate("Userlist parsing failed. Details: %1%")) % e.what()).str()));
            }
        }

        ///////////////////////////////////////////////////////
        // Evaluate Global Messages
        ///////////////////////////////////////////////////////

        progressCallback("Evaluating global messages...");

        //Merge all global message lists.
        BOOST_LOG_TRIVIAL(debug) << "Merging all global message lists.";
        if (!this->masterlist.messages.empty())
            messages.insert(messages.end(), this->masterlist.messages.begin(), this->masterlist.messages.end());
        if (!this->userlist.messages.empty())
            messages.insert(messages.end(), this->userlist.messages.begin(), this->userlist.messages.end());

        //Evaluate any conditions in the global messages.
        BOOST_LOG_TRIVIAL(debug) << "Evaluating global message conditions.";
        try {
            list<loot::Message>::iterator it = messages.begin();
            while (it != messages.end()) {
                if (!it->EvalCondition(*this, language))
                    it = messages.erase(it);
                else
                    ++it;
            }
        }
        catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << "A global message contains a condition that could not be evaluated. Details: " << e.what();
            messages.push_back(loot::Message(loot::Message::error, (format(loc::translate("A global message contains a condition that could not be evaluated. Details: %1%")) % e.what()).str()));
        }

        ////////////////////////////////////////////////////////
        // Slim down masterlist
        ////////////////////////////////////////////////////////
        //
        // Userlist data gets replaced every time sorting is looped, so there's no point evaluating it
        // outside the loop, but the masterlist can be slimmed down now.

        progressCallback("Filtering masterlist...");
       
        std::list<Plugin> tempMasterlistPlugins;
        for (const auto &plugin : this->plugins) {
            list<loot::Plugin>::iterator pos = std::find(this->masterlist.plugins.begin(), this->masterlist.plugins.end(), plugin.second);

            if (pos != this->masterlist.plugins.end()) {
                // The plugin exists in the masterlist, store a copy of its metadata.
                tempMasterlistPlugins.push_back(*pos);
            }
        }
        // Now replace the current full masterlist plugin metadata list with the install-specific one.
        this->masterlist.plugins = tempMasterlistPlugins;
    }


    std::list<Plugin> Game::Sort(const unsigned int language, std::list<Message>& messages, std::function<void(const std::string&)> progressCallback) {
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
                messages.push_back(loot::Message(loot::Message::error, (format(loc::translate("\"%1%\" contains a condition that could not be evaluated. Details: %2%")) % graph[v].Name() % e.what()).str()));
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