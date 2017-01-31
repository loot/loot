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
    <https://www.gnu.org/licenses/>.
    */

#include "api/game/game_cache.h"

#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

using boost::locale::to_lower;
using std::lock_guard;
using std::mutex;
using std::pair;
using std::string;

namespace loot {
GameCache::GameCache() {}

GameCache::GameCache(const GameCache& cache) :
  masterlist_(cache.masterlist_),
  userlist_(cache.userlist_),
  conditions_(cache.conditions_),
  plugins_(cache.plugins_) {}

GameCache& GameCache::operator=(const GameCache& cache) {
  if (&cache != this) {
    masterlist_ = cache.masterlist_;
    userlist_ = cache.userlist_;
    conditions_ = cache.conditions_;
    plugins_ = cache.plugins_;
  }

  return *this;
}

Masterlist & GameCache::GetMasterlist() {
  return masterlist_;
}

MetadataList & GameCache::GetUserlist() {
  return userlist_;
}

void GameCache::CacheCondition(const std::string& condition, bool result) {
  lock_guard<mutex> guard(mutex_);
  conditions_.insert(pair<string, bool>(to_lower(condition), result));
}

std::pair<bool, bool> GameCache::GetCachedCondition(const std::string& condition) const {
  lock_guard<mutex> guard(mutex_);

  auto it = conditions_.find(to_lower(condition));

  if (it != conditions_.end())
    return pair<bool, bool>(it->second, true);
  else
    return pair<bool, bool>(false, false);
}

std::set<std::shared_ptr<const Plugin>> GameCache::GetPlugins() const {
  std::set<std::shared_ptr<const Plugin>> output;
  std::transform(begin(plugins_),
                 end(plugins_),
                 std::inserter<std::set<std::shared_ptr<const Plugin>>>(output, begin(output)),
                 [](const pair<string, std::shared_ptr<const Plugin>>& pluginPair) {
    return pluginPair.second;
  });
  return output;
}

std::shared_ptr<const Plugin> GameCache::GetPlugin(const std::string& pluginName) const {
  auto it = plugins_.find(to_lower(pluginName));
  if (it != end(plugins_))
    return it->second;

  throw std::invalid_argument("No plugin \"" + pluginName + "\" exists.");
}

void GameCache::AddPlugin(const Plugin&& plugin) {
  lock_guard<mutex> lock(mutex_);

  auto it = plugins_.find(plugin.GetLowercasedName());
  if (it != end(plugins_))
    plugins_.erase(it);

  
  plugins_.emplace(plugin.GetLowercasedName(), std::make_shared<Plugin>(std::move(plugin)));
}

std::vector<std::string> GameCache::GetLoadOrder() const {
  return loadOrder_;
}

void GameCache::StoreLoadOrder(const std::vector<std::string>& loadOrder) {
  loadOrder_ = loadOrder;
}

void GameCache::ClearCachedConditions() {
  lock_guard<mutex> guard(mutex_);

  conditions_.clear();
}

void GameCache::ClearCachedPlugins() {
  lock_guard<mutex> guard(mutex_);

  plugins_.clear();
}
}
