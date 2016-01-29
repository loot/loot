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

#include "metadata_list.h"
#include "globals.h"
#include "error.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/log/trivial.hpp>

using namespace std;

namespace loot {
    void MetadataList::Load(const boost::filesystem::path& filepath) {
        plugins.clear();
        regexPlugins.clear();
        messages.clear();

        BOOST_LOG_TRIVIAL(debug) << "Loading file: " << filepath;

        boost::filesystem::ifstream in(filepath);
        if (!in.good())
            throw error(error::path_read_fail, "Cannot open " + filepath.string());

        YAML::Node metadataList = YAML::Load(in);
        in.close();

        if (metadataList["plugins"]) {
            for (const auto& node : metadataList["plugins"]) {
                PluginMetadata plugin(node.as<PluginMetadata>());
                if (plugin.IsRegexPlugin())
                    regexPlugins.push_back(plugin);
                else {
                    if (!plugins.insert(plugin).second)
                        throw error(error::path_read_fail, "More than one entry exists for \"" + plugin.Name() + "\"");
                }
            }
        }
        if (metadataList["globals"])
            messages = metadataList["globals"].as< list<Message> >();

        BOOST_LOG_TRIVIAL(debug) << "File loaded successfully.";
    }

    void MetadataList::Save(const boost::filesystem::path& filepath) {
        BOOST_LOG_TRIVIAL(trace) << "Saving metadata list to: " << filepath;
        YAML::Emitter yout;
        yout.SetIndent(2);
        yout << YAML::BeginMap
            << YAML::Key << "plugins" << YAML::Value << Plugins()
            << YAML::Key << "globals" << YAML::Value << messages
            << YAML::EndMap;

        boost::filesystem::ofstream uout(filepath);
        uout << yout.c_str();
        uout.close();
    }

    void MetadataList::clear() {
        plugins.clear();
        regexPlugins.clear();
        messages.clear();
    }

    std::list<PluginMetadata> MetadataList::Plugins() const {
        list<PluginMetadata> pluginList(plugins.begin(), plugins.end());

        pluginList.insert(pluginList.end(), regexPlugins.begin(), regexPlugins.end());

        return pluginList;
    }

    std::list<Message> MetadataList::Messages() const {
        return messages;
    }

    // Merges multiple matching regex entries if any are found.
    PluginMetadata MetadataList::FindPlugin(const PluginMetadata& plugin) const {
        PluginMetadata match(plugin.Name());

        auto it = plugins.find(plugin);

        if (it != plugins.end())
            match = *it;

        // Now we want to also match possibly multiple regex entries.
        auto regIt = find(regexPlugins.begin(), regexPlugins.end(), plugin);
        while (regIt != regexPlugins.end()) {
            match.MergeMetadata(*regIt);

            regIt = find(++regIt, regexPlugins.end(), plugin);
        }

        return match;
    }

    void MetadataList::AddPlugin(const PluginMetadata& plugin) {
        if (plugin.IsRegexPlugin())
            regexPlugins.push_back(plugin);
        else {
            if (!plugins.insert(plugin).second)
                throw error(error::invalid_args, "Cannot add \"" + plugin.Name() + "\" to the metadata list as another entry already exists.");
        }
    }

    // Doesn't erase matching regex entries, because they might also
    // be required for other plugins.
    void MetadataList::ErasePlugin(const PluginMetadata& plugin) {
        auto it = plugins.find(plugin);

        if (it != plugins.end()) {
            plugins.erase(it);
            return;
        }
    }

    void MetadataList::AppendMessage(const Message& message) {
        messages.push_back(message);
    }

    void MetadataList::EvalAllConditions(Game& game, const unsigned int language) {
        unordered_set<PluginMetadata> replacementSet;
        for (auto &plugin : plugins) {
            PluginMetadata p(plugin);
            p.EvalAllConditions(game, language);
            replacementSet.insert(p);
        }
        plugins = replacementSet;
        for (auto &plugin : regexPlugins) {
            plugin.EvalAllConditions(game, language);
        }
        for (auto &message : messages) {
            message.EvalCondition(game, language);
        }
    }
}
