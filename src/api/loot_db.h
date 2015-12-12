/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013-2015    WrinklyNinja

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

#ifndef LOOT_API_LOOT_DB_INT_H
#define LOOT_API_LOOT_DB_INT_H

#include "../backend/game/game.h"
#include "../backend/error.h"

#include <vector>
#include <unordered_map>

struct loot_db : public loot::Game {
    loot_db(const unsigned int clientGame, const std::string& gamePath, const boost::filesystem::path& gameLocalDataPath)
        : Game(clientGame) {
        this->SetGamePath(gamePath);
        this->Init(false, gameLocalDataPath);
    }

    loot::MetadataList rawUserMetadata;
    loot::Masterlist rawMetadata;

    const char * getRevisionIdString() const {
        return revisionId.c_str();
    }

    const char * getRevisionDateString() const {
        return revisionDate.c_str();
    }

    const std::vector<const char *>& getPluginNames() const {
        return cPluginNames;
    }

    const std::vector<const char *>& getBashTagMap() const {
        return cBashTagMap;
    }

    unsigned int getBashTagUid(const std::string& name) const {
        auto it = bashTagMap.find(name);
        if (it != end(bashTagMap))
            return it->second;

        throw loot::error(loot::error::no_tag_map, "The Bash Tag \"" + name + "\" does not exist in the Bash Tag map.");
    }

    const std::vector<unsigned int>& getAddedTagIds() const {
        return addedTagIds;
    }

    const std::vector<unsigned int>& getRemovedTagIds() const {
        return removedTagIds;
    }

    const std::vector<loot_message>& getPluginMessages() const {
        return cPluginMessages;
    }

    void setRevisionIdString(const std::string& str) {
        revisionId = str;
    }

    void setRevisionDateString(const std::string& str) {
        revisionDate = str;
    }

    template<class T>
    void setPluginNames(const T& plugins) {
        // First take copies of the C++ strings to store.
        pluginNames.resize(plugins.size());
        std::transform(begin(plugins),
                       end(plugins),
                       begin(pluginNames),
                       [](const loot::Plugin& plugin) {
            return plugin.Name();
        });

        // Now store their C strings.
        cPluginNames.resize(pluginNames.size());
        std::transform(begin(pluginNames),
                       end(pluginNames),
                       begin(cPluginNames),
                       [](const std::string& pluginName) {
            return pluginName.c_str();
        });
    }

    template<class T>
    void setAddedTags(const T& names) {
        for (const auto& name : names)
            addedTagIds.push_back(getBashTagUid(name));
    }

    template<class T>
    void setRemovedTags(const T& names) {
        for (const auto& name : names)
            removedTagIds.push_back(getBashTagUid(name));
    }

    template<class T>
    void setPluginMessages(const T& pluginMessages) {
        cPluginMessages.resize(pluginMessages.size());
        pluginMessageStrings.resize(pluginMessages.size());

        size_t i = 0;
        for (const auto& message : pluginMessages) {
            pluginMessageStrings[i] = message.ChooseContent(loot::Language::any).Str();

            cPluginMessages[i].type = message.Type();
            cPluginMessages[i].message = pluginMessageStrings[i].c_str();

            ++i;
        }
    }

    void addBashTagsToMap(std::set<std::string> names) {
        for (const auto& name : names) {
            // Try adding the Bash Tag to the map assuming it's not already in
            // there, then use the UID in the returned value, as that will be
            // equal to the value in the map, even if the Bash Tag was already
            // present.
            unsigned int uid = bashTagMap.size();
            // If the tag already exists in the map, do
            auto it = bashTagMap.emplace(name, uid).first;
            if (it->second == cBashTagMap.size())
                cBashTagMap.push_back(it->first.c_str());
            else
                cBashTagMap.at(it->second) = it->first.c_str();
        }
    }

    void clearBashTagMap() {
        bashTagMap.clear();
        cBashTagMap.clear();
    }

    void clearArrays() {
        pluginNames.clear();
        cPluginNames.clear();

        addedTagIds.clear();
        removedTagIds.clear();

        cPluginMessages.clear();
        pluginMessageStrings.clear();
    }
private:
    std::string revisionId;
    std::string revisionDate;

    std::vector<std::string> pluginNames;
    std::vector<const char *> cPluginNames;

    // For the Bash Tag map, a string is mapped to a UID that is also the
    // index of the vector where the C string can be found.
    std::unordered_map<std::string, unsigned int> bashTagMap;
    std::vector<const char *> cBashTagMap;

    std::vector<unsigned int> addedTagIds;
    std::vector<unsigned int> removedTagIds;

    std::vector<loot_message> cPluginMessages;
    std::vector<std::string> pluginMessageStrings;
};

#endif
