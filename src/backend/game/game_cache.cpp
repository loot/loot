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

#include "game_cache.h"
#include "../globals.h"
#include "../helpers/helpers.h"
#include "../error.h"
#include "../helpers/streams.h"

#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

using namespace std;

namespace fs = boost::filesystem;
namespace lc = boost::locale;

namespace loot {
    GameCache::GameCache() {}
    GameCache::GameCache(const GameCache& cache)
        : conditionCache(cache.conditionCache),
        crcCache(cache.crcCache),
        activePlugins(cache.activePlugins) {}

    GameCache& GameCache::operator=(const GameCache& cache) {
        conditionCache = cache.conditionCache;
        crcCache = cache.crcCache;
        activePlugins = cache.activePlugins;

        return *this;
    }

    void GameCache::CacheCrc(const std::string& plugin, uint32_t crc) {
        std::lock_guard<std::mutex> guard(mutex);
        crcCache.insert(pair<string, uint32_t>(boost::locale::to_lower(plugin), crc));
    }

    void GameCache::CacheCondition(const std::string& condition, bool result) {
        std::lock_guard<std::mutex> guard(mutex);
        conditionCache.insert(pair<string, bool>(boost::locale::to_lower(condition), result));
    }

    void GameCache::CacheActivePlugins(const std::unordered_set<std::string>& plugins) {
        std::lock_guard<std::mutex> guard(mutex);
        activePlugins = plugins;
    }

    uint32_t GameCache::GetCachedCrc(const std::string& plugin) const {
        std::lock_guard<std::mutex> guard(mutex);

        auto it = crcCache.find(boost::locale::to_lower(plugin));

        if (it != crcCache.end())
            return it->second;
        else
            return 0;
    }

    std::pair<bool, bool> GameCache::GetCachedCondition(const std::string& condition) const {
        std::lock_guard<std::mutex> guard(mutex);

        auto it = conditionCache.find(boost::locale::to_lower(condition));

        if (it != conditionCache.end())
            return std::pair<bool, bool>(it->second, true);
        else
            return std::pair<bool, bool>(false, false);
    }

    bool GameCache::IsPluginActive(const std::string& plugin) const {
        std::lock_guard<std::mutex> guard(mutex);

        return activePlugins.find(boost::locale::to_lower(plugin)) != activePlugins.end();
    }

    void GameCache::ClearCache() {
        std::lock_guard<std::mutex> guard(mutex);

        conditionCache.clear();
        crcCache.clear();
        activePlugins.clear();
    }
}
