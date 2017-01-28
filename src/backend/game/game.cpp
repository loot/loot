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

#include <algorithm>
#include <thread>
#include <cmath>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

#include "loot/exception/file_access_error.h"
#include "loot/exception/game_detection_error.h"
#include "backend/helpers/helpers.h"

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
Game::Game(const GameSettings& gameSettings, 
           const boost::filesystem::path& lootDataPath, 
           const boost::filesystem::path& localDataPath) :
  GameSettings(gameSettings), 
  lootDataPath_(lootDataPath),
  localDataPath_(localDataPath),
  pluginsFullyLoaded_(false) {
  this->SetName(gameSettings.Name())
    .SetMaster(gameSettings.Master())
    .SetRepoURL(gameSettings.RepoURL())
    .SetRepoBranch(gameSettings.RepoBranch())
    .SetGamePath(gameSettings.GamePath())
    .SetRegistryKey(gameSettings.RegistryKey());
}

bool Game::IsInstalled() {
  try {
    BOOST_LOG_TRIVIAL(trace) << "Checking if game \"" << Name() << "\" is installed.";
    if (!GamePath().empty() && fs::exists(GamePath() / "Data" / Master()))
      return true;

    if (fs::exists(fs::path("..") / "Data" / Master())) {
      SetGamePath("..");
      return true;
    }

#ifdef _WIN32
    std::string path;
    std::string key_parent = fs::path(RegistryKey()).parent_path().string();
    std::string key_name = fs::path(RegistryKey()).filename().string();
    path = RegKeyStringValue("HKEY_LOCAL_MACHINE", key_parent, key_name);
    if (!path.empty() && fs::exists(fs::path(path) / "Data" / Master())) {
      SetGamePath(path);
      return true;
    }
#endif
  } catch (std::exception &e) {
    BOOST_LOG_TRIVIAL(error) << "Error while checking if game \"" << Name() << "\" is installed: " << e.what();
  }

  return false;
}

void Game::Init() {
  BOOST_LOG_TRIVIAL(info) << "Initialising filesystem-related data for game: " << Name();

  if (!this->IsInstalled()) {
    throw GameDetectionError("Game path could not be detected.");
  }

  if (!lootDataPath_.empty()) {
      //Make sure that the LOOT game path exists.
    try {
      if (!fs::exists(lootDataPath_ / FolderName()))
        fs::create_directories(lootDataPath_ / FolderName());
    } catch (fs::filesystem_error& e) {
      throw FileAccessError((boost::format("Could not create LOOT folder for game. Details: %1%") % e.what()).str());
    }
  }

  loadOrderHandler_.Init(*this, localDataPath_);
}

void Game::RedatePlugins() {
  if (Type() != GameType::tes5 && Type() != GameType::tes5se) {
    BOOST_LOG_TRIVIAL(warning) << "Cannot redate plugins for game " << Name();
    return;
  }

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
    return loadOrderHandler_.IsPluginActive(pluginName);
  }
}

short Game::GetActiveLoadOrderIndex(const std::string & pluginName) const {
  return GetActiveLoadOrderIndex(pluginName, GetLoadOrder());
}

short Game::GetActiveLoadOrderIndex(const std::string & pluginName, const std::vector<std::string>& loadOrder) const {
  // Get the full load order, then count the number of active plugins until the
  // given plugin is encountered. If the plugin isn't active or in the load
  // order, return -1.

  if (!IsPluginActive(pluginName))
    return -1;

  short numberOfActivePlugins = 0;
  for (const std::string& plugin : loadOrder) {
    if (boost::iequals(plugin, pluginName))
      return numberOfActivePlugins;

    if (IsPluginActive(plugin))
      ++numberOfActivePlugins;
  }

  return -1;
}

std::vector<std::string> Game::GetLoadOrder() const {
  if (loadOrder_.empty())
    loadOrder_ = loadOrderHandler_.GetLoadOrder();

  return loadOrder_;
}

void Game::SetLoadOrder(const std::vector<std::string>& loadOrder) const {
  loadOrderHandler_.BackupLoadOrder(loadOrder_, lootDataPath_ / FolderName());
  loadOrderHandler_.SetLoadOrder(loadOrder);
  loadOrder_ = loadOrder;
}

fs::path Game::MasterlistPath() const {
  if (lootDataPath_.empty() || FolderName().empty())
    return "";
  else
    return lootDataPath_ / FolderName() / "masterlist.yaml";
}

fs::path Game::UserlistPath() const {
  if (lootDataPath_.empty() || FolderName().empty())
    return "";
  else
    return lootDataPath_ / FolderName() / "userlist.yaml";
}

#ifdef _WIN32
std::string Game::RegKeyStringValue(const std::string& keyStr, const std::string& subkey, const std::string& value) {
  HKEY hKey = NULL;
  DWORD len = MAX_PATH;
  std::wstring wstr(MAX_PATH, 0);

  if (keyStr == "HKEY_CLASSES_ROOT")
    hKey = HKEY_CLASSES_ROOT;
  else if (keyStr == "HKEY_CURRENT_CONFIG")
    hKey = HKEY_CURRENT_CONFIG;
  else if (keyStr == "HKEY_CURRENT_USER")
    hKey = HKEY_CURRENT_USER;
  else if (keyStr == "HKEY_LOCAL_MACHINE")
    hKey = HKEY_LOCAL_MACHINE;
  else if (keyStr == "HKEY_USERS")
    hKey = HKEY_USERS;
  else
    throw std::invalid_argument("Invalid registry key given.");

  BOOST_LOG_TRIVIAL(trace) << "Getting string for registry key, subkey and value: " << keyStr << " + " << subkey << " + " << value;
  LONG ret = RegGetValue(hKey,
                         ToWinWide(subkey).c_str(),
                         ToWinWide(value).c_str(),
                         RRF_RT_REG_SZ | KEY_WOW64_32KEY,
                         NULL,
                         &wstr[0],
                         &len);

  if (ret == ERROR_SUCCESS) {
    BOOST_LOG_TRIVIAL(info) << "Found string: " << wstr.c_str();
    return FromWinWide(wstr.c_str());  // Passing c_str() cuts off any unused buffer.
  } else {
    BOOST_LOG_TRIVIAL(info) << "Failed to get string value.";
    return "";
  }
}
#endif
}
