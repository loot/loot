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

#ifndef LOOT_BACKEND_GAME_GAME_CACHE
#define LOOT_BACKEND_GAME_GAME_CACHE

#include <mutex>
#include <string>
#include <unordered_map>

#include "backend/masterlist.h"
#include "backend/metadata_list.h"
#include "backend/plugin/plugin.h"

namespace loot {
class GameCache {
public:
  GameCache();
  GameCache(const GameCache& cache);

  GameCache& operator=(const GameCache& cache);

  Masterlist& GetMasterlist();
  MetadataList& GetUserlist();

  // Returns false for second bool if no cached condition.
  std::pair<bool, bool> GetCachedCondition(const std::string& condition) const;
  void CacheCondition(const std::string& condition, bool result);

  std::set<std::shared_ptr<const Plugin>> GetPlugins() const;
  std::shared_ptr<const Plugin> GetPlugin(const std::string& pluginName) const;
  void AddPlugin(const Plugin&& plugin);

  std::vector<Message> GetMessages() const;
  void AppendMessage(const Message& message);

  std::vector<std::string> GetLoadOrder() const;
  void StoreLoadOrder(const std::vector<std::string>& loadOrder);

  void IncrementLoadOrderSortCount();
  void DecrementLoadOrderSortCount();

  void ClearCachedConditions();
  void ClearCachedPlugins();
  void ClearMessages();
private:
  Masterlist masterlist_;
  MetadataList userlist_;
  std::unordered_map<std::string, bool> conditions_;
  std::unordered_map<std::string, std::shared_ptr<const Plugin>> plugins_;
  std::vector<Message> messages_;
  std::vector<std::string> loadOrder_;
  unsigned short loadOrderSortCount_;

  mutable std::mutex mutex_;
};
}

#endif
