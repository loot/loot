/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013-2016    WrinklyNinja

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
#include "../include/loot/api.h"

#include <vector>
#include <unordered_map>

struct loot_db : public loot::Game {
    loot_db(const unsigned int clientGame,
            const std::string& gamePath,
            const boost::filesystem::path& gameLocalDataPath);

    loot::Masterlist& getUnevaluatedMasterlist();
    loot::MetadataList& getUnevaluatedUserlist();

    loot::MetadataList rawUserMetadata;
    loot::Masterlist rawMetadata;

    const char * getRevisionIdString() const;
    const char * getRevisionDateString() const;

    const std::vector<const char *>& getPluginNames() const;

    const std::vector<const char *>& getBashTagMap() const;
    unsigned int getBashTagUid(const std::string& name) const;

    const std::vector<unsigned int>& getAddedTagIds() const;
    const std::vector<unsigned int>& getRemovedTagIds() const;

    const std::vector<loot_message>& getPluginMessages() const;

    void setRevisionIdString(const std::string& str);
    void setRevisionDateString(const std::string& str);

    template<class T>
    void setPluginNames(const T& plugins) {
        // First take copies of the C++ strings to store.
        pluginNames.resize(plugins.size());
        std::transform(begin(plugins),
                       end(plugins),
                       begin(pluginNames),
                       [](const loot::PluginMetadata& plugin) {
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

    void setAddedTags(const std::set<std::string>& names);
    void setRemovedTags(const std::set<std::string>& names);

    void setPluginMessages(const std::list<loot::Message>& pluginMessages);

    void addBashTagsToMap(std::set<std::string> names);

    void clearBashTagMap();
    void clearArrays();
private:
    loot::Masterlist unevaluatedMasterlist_;
    loot::MetadataList unevaluatedUserlist_;

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
