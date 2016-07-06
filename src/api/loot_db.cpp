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

#include "loot_db.h"

#include "../backend/error.h"

loot_db::loot_db(const unsigned int clientGame, const std::string& gamePath, const boost::filesystem::path& gameLocalDataPath)
    : Game(clientGame) {
    this->SetGamePath(gamePath);
    this->Init(false, gameLocalDataPath);
}

const char * loot_db::getRevisionIdString() const {
    return revisionId.c_str();
}

const char * loot_db::getRevisionDateString() const {
    return revisionDate.c_str();
}

const std::vector<const char *>& loot_db::getPluginNames() const {
    return cPluginNames;
}

const std::vector<const char *>& loot_db::getBashTagMap() const {
    return cBashTagMap;
}

unsigned int loot_db::getBashTagUid(const std::string& name) const {
    auto it = bashTagMap.find(name);
    if (it != end(bashTagMap))
        return it->second;

    throw loot::error(loot::error::no_tag_map, "The Bash Tag \"" + name + "\" does not exist in the Bash Tag map.");
}

const std::vector<unsigned int>& loot_db::getAddedTagIds() const {
    return addedTagIds;
}

const std::vector<unsigned int>& loot_db::getRemovedTagIds() const {
    return removedTagIds;
}

const std::vector<loot_message>& loot_db::getPluginMessages() const {
    return cPluginMessages;
}

void loot_db::setRevisionIdString(const std::string& str) {
    revisionId = str;
}

void loot_db::setRevisionDateString(const std::string& str) {
    revisionDate = str;
}

void loot_db::setAddedTags(const std::set<std::string>& names) {
    addedTagIds.clear();
    for (const auto& name : names)
        addedTagIds.push_back(getBashTagUid(name));
}

void loot_db::setRemovedTags(const std::set<std::string>& names) {
    removedTagIds.clear();
    for (const auto& name : names)
        removedTagIds.push_back(getBashTagUid(name));
}

void loot_db::setPluginMessages(const std::list<loot::Message>& pluginMessages) {
    cPluginMessages.resize(pluginMessages.size());
    pluginMessageStrings.resize(pluginMessages.size());

    size_t i = 0;
    for (const auto& message : pluginMessages) {
        pluginMessageStrings[i] = message.ChooseContent(loot::Language::english).Str();

        cPluginMessages[i].type = message.Type();
        cPluginMessages[i].message = pluginMessageStrings[i].c_str();

        ++i;
    }
}

void loot_db::addBashTagsToMap(std::set<std::string> names) {
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

void loot_db::clearBashTagMap() {
    bashTagMap.clear();
    cBashTagMap.clear();
}

void loot_db::clearArrays() {
    pluginNames.clear();
    cPluginNames.clear();

    addedTagIds.clear();
    removedTagIds.clear();

    cPluginMessages.clear();
    pluginMessageStrings.clear();
}
