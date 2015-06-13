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

#include "metadata_list.h"
#include "globals.h"
#include "error.h"
#include "helpers/streams.h"

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

using namespace std;

namespace loot {
    void MetadataList::Load(const boost::filesystem::path& filepath) {
        plugins.clear();
        messages.clear();

        BOOST_LOG_TRIVIAL(debug) << "Loading file: " << filepath;

        loot::ifstream in(filepath);
        YAML::Node metadataList = YAML::Load(in);
        in.close();

        if (metadataList["plugins"]) {
            for (const auto& node : metadataList["plugins"]) {
                PluginMetadata plugin(node.as<PluginMetadata>());
                if (plugin.IsRegexPlugin())
                    regexPlugins.push_back(plugin);
                else
                    plugins.insert(plugin);
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

        loot::ofstream uout(filepath);
        uout << yout.c_str();
        uout.close();
    }

    void MetadataList::clear() {
        plugins.clear();
        messages.clear();
    }

    bool MetadataList::operator == (const MetadataList& rhs) const {
        if (this->plugins.size() != rhs.plugins.size() || this->messages.size() != rhs.messages.size() || this->regexPlugins.size() != rhs.regexPlugins.size()) {
            BOOST_LOG_TRIVIAL(info) << "Metadata edited for some plugin, new and old userlists differ in size.";
            return false;
        }
        else {
            for (const auto& rhsPlugin : rhs.plugins) {
                const auto it = this->plugins.find(rhsPlugin);

                if (it == this->plugins.end()) {
                    BOOST_LOG_TRIVIAL(info) << "Metadata added for plugin: " << it->Name();
                    return false;
                }

                if (!it->DiffMetadata(rhsPlugin).HasNameOnly()) {
                    BOOST_LOG_TRIVIAL(info) << "Metadata edited for plugin: " << it->Name();
                    return false;
                }
            }
            for (const auto& rhsPlugin : rhs.regexPlugins) {
                const auto it = find(regexPlugins.begin(), regexPlugins.end(), rhsPlugin);

                if (it == this->regexPlugins.end()) {
                    BOOST_LOG_TRIVIAL(info) << "Metadata added for plugin: " << it->Name();
                    return false;
                }

                if (!it->DiffMetadata(rhsPlugin).HasNameOnly()) {
                    BOOST_LOG_TRIVIAL(info) << "Metadata edited for plugin: " << it->Name();
                    return false;
                }
            }
            // Messages are compared exactly by the '==' operator, so there's no need to do a more
            // fine-grained check.
            for (const auto& rhsMessage : rhs.messages) {
                const auto it = std::find(this->messages.begin(), this->messages.end(), rhsMessage);

                if (it == this->messages.end()) {
                    return  false;
                }
            }
        }
        return true;
    }

    std::list<PluginMetadata> MetadataList::Plugins() const {
        list<PluginMetadata> pluginList(plugins.begin(), plugins.end());

        pluginList.insert(pluginList.end(), regexPlugins.begin(), regexPlugins.end());

        return pluginList;
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
        else
            plugins.insert(plugin);
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
