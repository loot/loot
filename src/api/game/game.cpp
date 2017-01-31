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

#include "api/game/game.h"

#include <algorithm>
#include <thread>
#include <cmath>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

#include "loot/exception/file_access_error.h"
#include "loot/exception/game_detection_error.h"

#ifdef _WIN32
#   ifndef UNICODE
#       define UNICODE
#   endif
#   ifndef _UNICODE
#      define _UNICODE
#   endif
#   define NOMINMAX
#   include "windows.h"
#   include "shlobj.h"
#   include "shlwapi.h"
#endif

using std::list;
using std::string;
using std::thread;
using std::vector;

namespace fs = boost::filesystem;

namespace loot {
Game::Game(const GameType gameType,
           const boost::filesystem::path& gamePath,
           const boost::filesystem::path& localDataPath) :
  type_(gameType),
  gamePath_(gamePath),
  localDataPath_(localDataPath) {}

GameType Game::Type() const {
  return type_;
}

boost::filesystem::path Game::DataPath() const {
  return gamePath_ / "Data";
}

std::string Game::GetArchiveFileExtension() const {
  if (type_ == GameType::fo4)
    return ".ba2";
  else
    return ".bsa";
}

void Game::Init() {
  BOOST_LOG_TRIVIAL(info) << "Initialising filesystem-related data for game of type " << (int) type_ << " at: " << gamePath_;

  loadOrderHandler_.Init(type_, gamePath_, localDataPath_);
  StoreLoadOrder(loadOrderHandler_.GetLoadOrder());
}

void Game::LoadPlugins(const std::vector<std::string>& plugins, const std::string& masterFile, bool headersOnly) {
  uintmax_t meanFileSize = 0;
  std::multimap<uintmax_t, string> sizeMap;

  // First get the plugin sizes.
  for (const auto& plugin : plugins) {
    if (!Plugin::IsValid(plugin, *this))
      throw std::invalid_argument("\"" + plugin + "\" is not a valid plugin");

    uintmax_t fileSize = Plugin::GetFileSize(plugin, *this);
    meanFileSize += fileSize;

    // Trim .ghost extension if present.
    if (boost::iends_with(plugin, ".ghost"))
      sizeMap.emplace(fileSize, plugin.substr(0, plugin.length() - 6));
    else
      sizeMap.emplace(fileSize, plugin);
  }
  meanFileSize /= sizeMap.size();  //Rounding error, but not important.

                                   // Get the number of threads to use.
                                   // hardware_concurrency() may be zero, if so then use only one thread.
  size_t threadsToUse = ::std::min((size_t)thread::hardware_concurrency(), sizeMap.size());
  threadsToUse = ::std::max(threadsToUse, (size_t)1);

  // Divide the plugins up by thread.
  unsigned int pluginsPerThread = ceil((double)sizeMap.size() / threadsToUse);
  vector<vector<string>> pluginGroups(threadsToUse);
  BOOST_LOG_TRIVIAL(info) << "Loading " << sizeMap.size() << " plugins using " << threadsToUse << " threads, with up to " << pluginsPerThread << " plugins per thread.";

  // The plugins should be split between the threads so that the data
  // load is as evenly spread as possible.
  size_t currentGroup = 0;
  for (const auto& plugin : sizeMap) {
    if (currentGroup == threadsToUse)
      currentGroup = 0;
    BOOST_LOG_TRIVIAL(trace) << "Adding plugin " << plugin.second << " to loading group " << currentGroup;
    pluginGroups[currentGroup].push_back(plugin.second);
    ++currentGroup;
  }

  // Clear the existing plugin cache.
  ClearCachedPlugins();

  // Load the plugins.
  BOOST_LOG_TRIVIAL(trace) << "Starting plugin loading.";
  vector<thread> threads;
  while (threads.size() < threadsToUse) {
    vector<string>& pluginGroup = pluginGroups[threads.size()];
    threads.push_back(thread([&]() {
      for (auto pluginName : pluginGroup) {
        BOOST_LOG_TRIVIAL(trace) << "Loading " << pluginName;
        if (boost::iequals(pluginName, masterFile))
          AddPlugin(Plugin(*this, pluginName, true));
        else
          AddPlugin(Plugin(*this, pluginName, headersOnly));
      }
    }));
  }

  // Join all threads.
  for (auto& thread : threads) {
    if (thread.joinable())
      thread.join();
  }
}

bool Game::IsPluginActive(const std::string& pluginName) const {
  try {
    return GetPlugin(pluginName)->IsActive();
  } catch (...) {
    return loadOrderHandler_.IsPluginActive(pluginName);
  }
}

std::vector<std::string> Game::GetLoadOrder() const {
  auto loadOrder = GameCache::GetLoadOrder();
  if (loadOrder.empty())
    return loadOrderHandler_.GetLoadOrder();

  return loadOrder;
}

void Game::SetLoadOrder(const std::vector<std::string>& loadOrder) {
  loadOrderHandler_.SetLoadOrder(loadOrder);
  StoreLoadOrder(loadOrder);
}
}
