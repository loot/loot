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

#include "backend/game/load_order_handler.h"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>

#include "loot/exception/error_categories.h"

using boost::format;
using std::string;

namespace loot {
LoadOrderHandler::LoadOrderHandler() : gh_(nullptr) {}

LoadOrderHandler::~LoadOrderHandler() {
  lo_destroy_handle(gh_);
}

void LoadOrderHandler::Init(const GameSettings& game, const boost::filesystem::path& gameLocalAppData) {
  if (game.GamePath().empty()) {
    throw std::invalid_argument("Game path is not initialised.");
  }

  const char * gameLocalDataPath = nullptr;
  string tempPathString = gameLocalAppData.string();
  if (!tempPathString.empty())
    gameLocalDataPath = tempPathString.c_str();

// If the handle has already been initialised, close it and open another.
  if (gh_ != nullptr) {
    lo_destroy_handle(gh_);
    gh_ = nullptr;
  }

  int ret;
  if (game.Type() == GameType::tes4)
    ret = lo_create_handle(&gh_, LIBLO_GAME_TES4, game.GamePath().string().c_str(), gameLocalDataPath);
  else if (game.Type() == GameType::tes5)
    ret = lo_create_handle(&gh_, LIBLO_GAME_TES5, game.GamePath().string().c_str(), gameLocalDataPath);
  else if (game.Type() == GameType::tes5se)
    ret = lo_create_handle(&gh_, LIBLO_GAME_TES5SE, game.GamePath().string().c_str(), gameLocalDataPath);
  else if (game.Type() == GameType::fo3)
    ret = lo_create_handle(&gh_, LIBLO_GAME_FO3, game.GamePath().string().c_str(), gameLocalDataPath);
  else if (game.Type() == GameType::fonv)
    ret = lo_create_handle(&gh_, LIBLO_GAME_FNV, game.GamePath().string().c_str(), gameLocalDataPath);
  else if (game.Type() == GameType::fo4)
    ret = lo_create_handle(&gh_, LIBLO_GAME_FO4, game.GamePath().string().c_str(), gameLocalDataPath);
  else
    ret = LIBLO_ERROR_INVALID_ARGS;

  if (ret != LIBLO_OK && ret != LIBLO_WARN_BAD_FILENAME && ret != LIBLO_WARN_INVALID_LIST && ret != LIBLO_WARN_LO_MISMATCH) {
    const char * e = nullptr;
    string err;
    lo_get_error_message(&e);
    if (e == nullptr) {
      err = "libloadorder failed to create a game handle. Details could not be fetched.";
    } else {
      err = (format("libloadorder failed to create a game handle. Details: %1%") % e).str();
    }
    lo_cleanup();
    throw std::system_error(ret, libloadorder_category(), err);
  }
}

bool LoadOrderHandler::IsPluginActive(const std::string& pluginName) const {
  BOOST_LOG_TRIVIAL(debug) << "Checking if plugin \"" << pluginName << "\" is active.";

  bool result = false;
  unsigned int ret = lo_get_plugin_active(gh_, pluginName.c_str(), &result);
  if (ret != LIBLO_OK && ret != LIBLO_WARN_BAD_FILENAME) {
    const char * e = nullptr;
    string err;
    lo_get_error_message(&e);
    if (e == nullptr) {
      err = "libloadorder failed to check if a plugin is active. Details could not be fetched.";
    } else {
      err = (format("libloadorder failed to check if a plugin is active. Details: %1%") % e).str();
    }
    lo_cleanup();
    throw std::system_error(ret, libloadorder_category(), err);
  }

  return result;
}

std::vector<std::string> LoadOrderHandler::GetLoadOrder() const {
  BOOST_LOG_TRIVIAL(debug) << "Getting load order.";

  char ** pluginArr;
  size_t pluginArrSize;

  unsigned int ret = lo_get_load_order(gh_, &pluginArr, &pluginArrSize);
  if (ret != LIBLO_OK && ret != LIBLO_WARN_BAD_FILENAME && ret != LIBLO_WARN_INVALID_LIST && ret != LIBLO_WARN_LO_MISMATCH) {
    const char * e = nullptr;
    string err;
    lo_get_error_message(&e);
    if (e == nullptr) {
      err = "libloadorder failed to get the load order. Details could not be fetched.";
    } else {
      err = (format("libloadorder failed to get the load order. Details: %1%") % e).str();
    }
    lo_cleanup();
    throw std::system_error(ret, libloadorder_category(), err);
  }

  std::vector<string> loadOrder;
  for (size_t i = 0; i < pluginArrSize; ++i) {
    loadOrder.push_back(string(pluginArr[i]));
  }
  return loadOrder;
}

void LoadOrderHandler::SetLoadOrder(const std::vector<std::string>& loadOrder) const {
  BOOST_LOG_TRIVIAL(info) << "Setting load order.";
  size_t pluginArrSize = loadOrder.size();
  char ** pluginArr = new char*[pluginArrSize];
  int i = 0;
  for (const auto &plugin : loadOrder) {
    BOOST_LOG_TRIVIAL(info) << '\t' << '\t' << plugin;
    pluginArr[i] = new char[plugin.length() + 1];
    strcpy(pluginArr[i], plugin.c_str());
    ++i;
  }

  unsigned int ret = lo_set_load_order(gh_, pluginArr, pluginArrSize);

  for (size_t i = 0; i < pluginArrSize; i++)
    delete[] pluginArr[i];
  delete[] pluginArr;

  if (ret != LIBLO_OK && ret != LIBLO_WARN_BAD_FILENAME && ret != LIBLO_WARN_INVALID_LIST && ret != LIBLO_WARN_LO_MISMATCH) {
    const char * e = nullptr;
    string err;
    lo_get_error_message(&e);
    if (e == nullptr) {
      err = "libloadorder failed to set the load order. Details could not be fetched.";
    } else {
      err = (format("libloadorder failed to set the load order. Details: %1%") % e).str();
    }
    lo_cleanup();

    throw std::system_error(ret, libloadorder_category(), err);
  }
}
}
