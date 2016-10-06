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

#include "backend/game/game_cache.h"

#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

#include "loot/error.h"
#include "backend/helpers/helpers.h"

using boost::locale::to_lower;
using std::lock_guard;
using std::mutex;
using std::pair;
using std::string;

namespace loot {
GameCache::GameCache() : loadOrderSortCount_(0) {}

GameCache::GameCache(const GameCache& cache) :
  masterlist_(cache.masterlist_),
  userlist_(cache.userlist_),
  conditions_(cache.conditions_),
  plugins_(cache.plugins_),
  messages_(cache.messages_),
  loadOrderSortCount_(cache.loadOrderSortCount_) {}

GameCache& GameCache::operator=(const GameCache& cache) {
  if (&cache != this) {
    masterlist_ = cache.masterlist_;
    userlist_ = cache.userlist_;
    conditions_ = cache.conditions_;
    plugins_ = cache.plugins_;
    messages_ = cache.messages_;
    loadOrderSortCount_ = cache.loadOrderSortCount_;
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

std::set<Plugin> GameCache::GetPlugins() const {
  std::set<Plugin> output;
  std::transform(begin(plugins_),
                 end(plugins_),
                 std::inserter<std::set<Plugin>>(output, begin(output)),
                 [](const pair<string, Plugin>& pluginPair) {
    return pluginPair.second;
  });
  return output;
}

const Plugin& GameCache::GetPlugin(const std::string & pluginName) const {
  auto it = plugins_.find(to_lower(pluginName));
  if (it != end(plugins_))
    return it->second;

  throw std::invalid_argument("No plugin \"" + pluginName + "\" exists.");
}

void GameCache::AddPlugin(const Plugin&& plugin) {
  lock_guard<mutex> lock(mutex_);

  auto pair = plugins_.emplace(to_lower(plugin.Name()), plugin);
  if (!pair.second)
    pair.first->second = plugin;
}

std::vector<Message> GameCache::GetMessages() const {
  std::vector<Message> output(messages_);
  if (loadOrderSortCount_ == 0)
    output.push_back(Message(MessageType::warn, "You have not sorted your load order this session."));

  return output;
}

void GameCache::AppendMessage(const Message& message) {
  lock_guard<mutex> guard(mutex_);

  messages_.push_back(message);
}

void GameCache::IncrementLoadOrderSortCount() {
  lock_guard<mutex> guard(mutex_);

  ++loadOrderSortCount_;
}

void GameCache::DecrementLoadOrderSortCount() {
  lock_guard<mutex> guard(mutex_);

  if (loadOrderSortCount_ > 0)
    --loadOrderSortCount_;
}

void GameCache::ClearCachedConditions() {
  lock_guard<mutex> guard(mutex_);

  conditions_.clear();
}

void GameCache::ClearCachedPlugins() {
  lock_guard<mutex> guard(mutex_);

  plugins_.clear();
}
void GameCache::ClearMessages() {
  lock_guard<mutex> guard(mutex_);

  messages_.clear();
}
}
