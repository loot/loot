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

#include "game_cache.h"
#include "../helpers/helpers.h"
#include "../error.h"

#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

using namespace std;

namespace fs = boost::filesystem;
namespace lc = boost::locale;

namespace loot {
    GameCache::GameCache() : isLoadOrderSorted(false) {}

    GameCache::GameCache(const GameCache& cache) :
        masterlist(cache.masterlist),
        userlist(cache.userlist),
        conditionCache(cache.conditionCache),
        plugins(cache.plugins),
        messages(cache.messages),
        isLoadOrderSorted(cache.isLoadOrderSorted) {}

    GameCache& GameCache::operator=(const GameCache& cache) {
        if (&cache != this) {
            masterlist = cache.masterlist;
            userlist = cache.userlist;
            conditionCache = cache.conditionCache;
            plugins = cache.plugins;
            messages = cache.messages;
            isLoadOrderSorted = cache.isLoadOrderSorted;
        }

        return *this;
    }

    Masterlist & GameCache::GetMasterlist() {
        return masterlist;
    }

    MetadataList & GameCache::GetUserlist() {
        return userlist;
    }

    void GameCache::CacheCondition(const std::string& condition, bool result) {
        std::lock_guard<std::mutex> guard(mutex);
        conditionCache.insert(pair<string, bool>(boost::locale::to_lower(condition), result));
    }

    std::pair<bool, bool> GameCache::GetCachedCondition(const std::string& condition) const {
        std::lock_guard<std::mutex> guard(mutex);

        auto it = conditionCache.find(boost::locale::to_lower(condition));

        if (it != conditionCache.end())
            return std::pair<bool, bool>(it->second, true);
        else
            return std::pair<bool, bool>(false, false);
    }

    std::set<Plugin> GameCache::GetPlugins() const {
        std::set<Plugin> output;
        std::transform(begin(plugins),
                       end(plugins),
                       inserter<set<Plugin>>(output, begin(output)),
                       [](const pair<std::string, Plugin>& pluginPair) {
            return pluginPair.second;
        });
        return output;
    }

    const Plugin& GameCache::GetPlugin(const std::string & pluginName) const {
        auto it = plugins.find(boost::locale::to_lower(pluginName));
        if (it != end(plugins))
            return it->second;

        throw Error(Error::invalid_args, "No plugin \"" + pluginName + "\" exists.");
    }

    void GameCache::AddPlugin(const Plugin&& plugin) {
        std::lock_guard<std::mutex> lock(mutex);

        auto pair = plugins.emplace(boost::locale::to_lower(plugin.Name()), plugin);
        if (!pair.second)
            pair.first->second = plugin;
    }

    std::vector<Message> GameCache::GetMessages() const {
        vector<Message> output(messages);
        if (!isLoadOrderSorted)
            output.push_back(Message(Message::warn, "You have not sorted your load order this session."));

        return output;
    }

    void GameCache::AppendMessage(const Message& message) {
        std::lock_guard<std::mutex> guard(mutex);

        messages.push_back(message);
    }

    void GameCache::SetLoadOrderSorted(bool isLoadOrderSorted) {
        this->isLoadOrderSorted = isLoadOrderSorted;
    }

    void GameCache::ClearCachedConditions() {
        std::lock_guard<std::mutex> guard(mutex);

        conditionCache.clear();
    }

    void GameCache::ClearCachedPlugins() {
        std::lock_guard<std::mutex> guard(mutex);

        plugins.clear();
    }
    void GameCache::ClearMessages() {
        std::lock_guard<std::mutex> guard(mutex);

        messages.clear();
    }
}
