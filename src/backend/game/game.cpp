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

#include "backend/game/game.h"

#include <thread>
#include <cmath>

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

#include "backend/app/loot_paths.h"
#include "loot/error.h"
#include "backend/helpers/helpers.h"

using boost::locale::translate;
using std::list;
using std::string;
using std::thread;
using std::vector;

namespace fs = boost::filesystem;

namespace loot {
Game::Game(const GameSettings& gameSettings) : GameSettings(gameSettings), pluginsFullyLoaded_(false) {
  this->SetName(gameSettings.Name())
    .SetMaster(gameSettings.Master())
    .SetRepoURL(gameSettings.RepoURL())
    .SetRepoBranch(gameSettings.RepoBranch())
    .SetGamePath(gameSettings.GamePath())
    .SetRegistryKey(gameSettings.RegistryKey());
}

Game::Game(const GameType gameType, const std::string& folder) : GameSettings(gameType, folder), pluginsFullyLoaded_(false) {}

void Game::Init(bool createFolder, const boost::filesystem::path& gameLocalAppData) {
  if (Type() != GameType::tes4 && Type() != GameType::tes5 && Type() != GameType::fo3 && Type() != GameType::fonv && Type() != GameType::fo4) {
    throw std::invalid_argument(translate("Invalid game ID supplied.").str());
  }

  BOOST_LOG_TRIVIAL(info) << "Initialising filesystem-related data for game: " << Name();

  if (!this->IsInstalled()) {
    BOOST_LOG_TRIVIAL(error) << "Game path could not be detected.";
    throw Error(Error::Code::path_not_found, translate("Game path could not be detected.").str());
  }

  if (createFolder) {
      //Make sure that the LOOT game path exists.
    try {
      if (!fs::exists(LootPaths::getLootDataPath() / FolderName()))
        fs::create_directories(LootPaths::getLootDataPath() / FolderName());
    } catch (fs::filesystem_error& e) {
      BOOST_LOG_TRIVIAL(error) << "Could not create LOOT folder for game. Details: " << e.what();
      throw Error(Error::Code::path_write_fail, translate("Could not create LOOT folder for game. Details:").str() + " " + e.what());
    }
  }

  LoadOrderHandler::Init(*this, gameLocalAppData);
}

void Game::RedatePlugins() {
  if (Type() != GameType::tes5)
    return;

  vector<string> loadorder = GetLoadOrder();
  if (!loadorder.empty()) {
    time_t lastTime = 0;
    for (const auto &pluginName : loadorder) {
      fs::path filepath = DataPath() / pluginName;
      if (!fs::exists(filepath)) {
        if (fs::exists(filepath.string() + ".ghost"))
          filepath += ".ghost";
        else
          continue;
      }

      time_t thisTime = fs::last_write_time(filepath);
      BOOST_LOG_TRIVIAL(info) << "Current timestamp for \"" << filepath.filename().string() << "\": " << thisTime;
      if (thisTime >= lastTime) {
        lastTime = thisTime;
        BOOST_LOG_TRIVIAL(trace) << "No need to redate \"" << filepath.filename().string() << "\".";
      } else {
        lastTime += 60;
        fs::last_write_time(filepath, lastTime);  //Space timestamps by a minute.
        BOOST_LOG_TRIVIAL(info) << "Redated \"" << filepath.filename().string() << "\" to: " << lastTime;
      }
    }
  }
}

void Game::LoadPlugins(const std::vector<std::string>& plugins, bool headersOnly) {
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
  size_t threadsToUse = std::min((size_t)thread::hardware_concurrency(), sizeMap.size());
  threadsToUse = std::max(threadsToUse, (size_t)1);

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
        if (boost::iequals(pluginName, Master()))
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

  pluginsFullyLoaded_ = !headersOnly;
}

void Game::LoadAllInstalledPlugins(bool headersOnly) {
  std::vector<std::string> plugins;

  BOOST_LOG_TRIVIAL(trace) << "Scanning for plugins in " << this->DataPath();
  for (fs::directory_iterator it(this->DataPath()); it != fs::directory_iterator(); ++it) {
    if (fs::is_regular_file(it->status()) && Plugin::IsValid(it->path().filename().string(), *this)) {
      string name = it->path().filename().string();
      BOOST_LOG_TRIVIAL(info) << "Found plugin: " << name;

      plugins.push_back(name);
    }
  }

  LoadPlugins(plugins, headersOnly);
}

bool Game::ArePluginsFullyLoaded() const {
  return pluginsFullyLoaded_;
}

bool Game::IsPluginActive(const std::string& pluginName) const {
  try {
    return GetPlugin(pluginName).IsActive();
  } catch (...) {
    return LoadOrderHandler::IsPluginActive(pluginName);
  }
}

short Game::GetActiveLoadOrderIndex(const std::string & pluginName) const {
  // Get the full load order, then count the number of active plugins until the
  // given plugin is encountered. If the plugin isn't active or in the load
  // order, return -1.

  if (!IsPluginActive(pluginName))
    return -1;

  short numberOfActivePlugins = 0;
  for (const std::string& plugin : GetLoadOrder()) {
    if (boost::iequals(plugin, pluginName))
      return numberOfActivePlugins;

    if (IsPluginActive(plugin))
      ++numberOfActivePlugins;
  }

  return -1;
}
std::vector<std::string> Game::GetLoadOrder() const {
  if (loadOrder_.empty())
    loadOrder_ = LoadOrderHandler::GetLoadOrder();

  return loadOrder_;
}
void Game::SetLoadOrder(const std::vector<std::string>& loadOrder) const {
  LoadOrderHandler::SetLoadOrder(loadOrder);
  loadOrder_ = loadOrder;
}
void Game::SetLoadOrder(const char * const * const loadOrder, const size_t numPlugins) const {
  LoadOrderHandler::SetLoadOrder(loadOrder, numPlugins);
}
}
