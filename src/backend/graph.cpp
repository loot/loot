/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2012    WrinklyNinja

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

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

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
    void CalcPluginOverlaps(const std::list<Plugin>& plugins, boost::unordered_map< std::string, std::vector<list<Plugin>::const_iterator> >& overlapMap) {
        for (list<Plugin>::const_iterator it=plugins.begin(),
                                          endit=plugins.end();
                                          it != endit;
                                          ++it) {
            list<Plugin>::const_iterator jt = it;
            ++jt;
            for (jt, endit; jt != endit; ++jt) {
                    BOOST_LOG_TRIVIAL(trace) << "Checking for FormID overlap between \"" << it->Name() << "\" and \"" << jt->Name() << "\".";
                if (it->DoFormIDsOverlap(*jt)) {
                    std::string key;
                    list<Plugin>::const_iterator value;
                    //Priority values should override the number of override records as the deciding factor if they differ.
                    if (it->MustLoadAfter(*jt) || jt->MustLoadAfter(*it))
                        break;
                    if (it->Priority() < jt->Priority()) {
                        key = jt->Name();
                        value = it;
                    } else if (jt->Priority() < it->Priority()) {
                        key = it->Name();
                        value = jt;
                    } else if (it->NumOverrideFormIDs() >= jt->NumOverrideFormIDs()) {
                        key = jt->Name();
                        value = it;
                    } else {
                        key = it->Name();
                        value = jt;
                    }
                    boost::unordered_map< string, vector<list<Plugin>::const_iterator> >::iterator mapIt = overlapMap.find(key);
                    if (mapIt == overlapMap.end()) {
                        overlapMap.insert(pair<string, vector<list<Plugin>::const_iterator> >(key, vector<list<Plugin>::const_iterator>(1, value)));
                    } else {
                        mapIt->second.push_back(value);
                    }
                }
            }
        }
    }
}
