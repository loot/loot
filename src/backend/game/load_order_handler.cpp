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

#include "load_order_handler.h"
#include "../error.h"

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

using namespace std;

namespace fs = boost::filesystem;
namespace lc = boost::locale;

namespace loot {
    LoadOrderHandler::LoadOrderHandler() : _gh(nullptr) {}

    LoadOrderHandler::~LoadOrderHandler() {
        lo_destroy_handle(_gh);
    }

    void LoadOrderHandler::Init(const GameSettings& game, const boost::filesystem::path& gameLocalAppData) {
        if (game.Id() != GameSettings::tes4
            && game.Id() != GameSettings::tes5
            && game.Id() != GameSettings::fo3
            && game.Id() != GameSettings::fonv) {
            throw error(error::invalid_args, lc::translate("Unsupported game ID supplied.").str());
        }

        if (game.GamePath().empty()) {
            BOOST_LOG_TRIVIAL(error) << "Game path is not initialised.";
            throw error(error::invalid_args, lc::translate("Game path is not initialised.").str());
        }

        const char * gameLocalDataPath = nullptr;
        string tempPathString = gameLocalAppData.string();
        if (!tempPathString.empty())
            gameLocalDataPath = tempPathString.c_str();

        // If the handle has already been initialised, close it and open another.
        if (_gh != nullptr) {
            lo_destroy_handle(_gh);
            _gh = nullptr;
        }

        int ret;
        if (game.Id() == GameSettings::tes4)
            ret = lo_create_handle(&_gh, LIBLO_GAME_TES4, game.GamePath().string().c_str(), gameLocalDataPath);
        else if (game.Id() == GameSettings::tes5)
            ret = lo_create_handle(&_gh, LIBLO_GAME_TES5, game.GamePath().string().c_str(), gameLocalDataPath);
        else if (game.Id() == GameSettings::fo3)
            ret = lo_create_handle(&_gh, LIBLO_GAME_FO3, game.GamePath().string().c_str(), gameLocalDataPath);
        else if (game.Id() == GameSettings::fonv)
            ret = lo_create_handle(&_gh, LIBLO_GAME_FNV, game.GamePath().string().c_str(), gameLocalDataPath);
        else
            ret = LIBLO_ERROR_INVALID_ARGS;

        if (ret != LIBLO_OK && ret != LIBLO_WARN_BAD_FILENAME && ret != LIBLO_WARN_INVALID_LIST && ret != LIBLO_WARN_LO_MISMATCH) {
            const char * e = nullptr;
            string err;
            lo_get_error_message(&e);
            if (e == nullptr) {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to create a game handle. Details could not be fetched.";
                err = lc::translate("libloadorder failed to create a game handle. Details could not be fetched.").str();
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to create a game handle. Details: " << e;
                err = lc::translate("libloadorder failed to create a game handle. Details:").str() + " " + e;
            }
            lo_cleanup();
            throw error(error::liblo_error, err);
        }

        if (game.Id() != GameSettings::tes5) {
            ret = lo_set_game_master(_gh, game.Master().c_str());
            if (ret != LIBLO_OK && ret != LIBLO_WARN_BAD_FILENAME && ret != LIBLO_WARN_INVALID_LIST && ret != LIBLO_WARN_LO_MISMATCH) {
                const char * e = nullptr;
                string err;
                lo_get_error_message(&e);
                lo_destroy_handle(_gh);
                _gh = nullptr;

                if (e == nullptr) {
                    BOOST_LOG_TRIVIAL(error) << "libloadorder failed to initialise game master file support. Details could not be fetched.";
                    err = lc::translate("libloadorder failed to initialise game master file support. Details could not be fetched.").str();
                }
                else {
                    BOOST_LOG_TRIVIAL(error) << "libloadorder failed to initialise game master file support. Details: " << e;
                    err = lc::translate("libloadorder failed to initialise game master file support. Details:").str() + " " + e;
                }
                lo_cleanup();
                throw error(error::liblo_error, err);
            }
        }
    }

    std::unordered_set<std::string> LoadOrderHandler::GetActivePlugins() const {
        BOOST_LOG_TRIVIAL(debug) << "Getting active plugins.";

        char ** pluginArr;
        size_t pluginArrSize;
        unsigned int ret = lo_get_active_plugins(_gh, &pluginArr, &pluginArrSize);
        if (ret != LIBLO_OK && ret != LIBLO_WARN_BAD_FILENAME && ret != LIBLO_WARN_INVALID_LIST && ret != LIBLO_WARN_LO_MISMATCH) {
            const char * e = nullptr;
            string err;
            lo_get_error_message(&e);
            if (e == nullptr) {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to get the active plugins list. Details could not be fetched.";
                err = lc::translate("libloadorder failed to get the active plugins list. Details could not be fetched.").str();
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to get the active plugins list. Details: " << e;
                err = lc::translate("libloadorder failed to get the active plugins list. Details:").str() + " " + e;
            }
            lo_cleanup();
            throw error(error::liblo_error, err);
        }

        std::unordered_set<std::string> activePlugins;
        for (size_t i = 0; i < pluginArrSize; ++i) {
            activePlugins.insert(boost::locale::to_lower(string(pluginArr[i])));
        }
        return activePlugins;
    }

    std::list<std::string> LoadOrderHandler::GetLoadOrder() const {
        BOOST_LOG_TRIVIAL(debug) << "Getting load order.";

        char ** pluginArr;
        size_t pluginArrSize;

        unsigned int ret = lo_get_load_order(_gh, &pluginArr, &pluginArrSize);
        if (ret != LIBLO_OK && ret != LIBLO_WARN_BAD_FILENAME && ret != LIBLO_WARN_INVALID_LIST && ret != LIBLO_WARN_LO_MISMATCH) {
            const char * e = nullptr;
            string err;
            lo_get_error_message(&e);
            if (e == nullptr) {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to get the load order. Details could not be fetched.";
                err = lc::translate("libloadorder failed to get the load order. Details could not be fetched.").str();
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to get the load order. Details: " << e;
                err = lc::translate("libloadorder failed to get the load order. Details:").str() + " " + e;
            }
            lo_cleanup();
            throw error(error::liblo_error, err);
        }

        std::list<std::string> loadOrder;
        for (size_t i = 0; i < pluginArrSize; ++i) {
            loadOrder.push_back(string(pluginArr[i]));
        }
        return loadOrder;
    }

    void LoadOrderHandler::SetLoadOrder(const char * const * const loadOrder, const size_t numPlugins) const {
        BOOST_LOG_TRIVIAL(debug) << "Setting load order.";

        unsigned int ret = lo_set_load_order(_gh, loadOrder, numPlugins);
        if (ret != LIBLO_OK && ret != LIBLO_WARN_BAD_FILENAME && ret != LIBLO_WARN_INVALID_LIST && ret != LIBLO_WARN_LO_MISMATCH) {
            const char * e = nullptr;
            string err;
            lo_get_error_message(&e);
            if (e == nullptr) {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to set the load order. Details could not be fetched.";
                err = lc::translate("libloadorder failed to set the load order. Details could not be fetched.").str();
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "libloadorder failed to set the load order. Details: " << e;
                err = lc::translate("libloadorder failed to set the load order. Details:").str() + " " + e;
            }
            lo_cleanup();
            throw error(error::liblo_error, err);
        }
    }

    void LoadOrderHandler::SetLoadOrder(const std::list<std::string>& loadOrder) const {
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

        try {
            SetLoadOrder(pluginArr, pluginArrSize);
        }
        catch (error &/*e*/) {
            for (size_t i = 0; i < pluginArrSize; i++)
                delete[] pluginArr[i];
            delete[] pluginArr;
            throw;
        }

        for (size_t i = 0; i < pluginArrSize; i++)
            delete[] pluginArr[i];
        delete[] pluginArr;
    }
}
