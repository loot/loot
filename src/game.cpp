/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2012    WrinklyNinja

    This file is part of BOSS.

    BOSS is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    BOSS is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BOSS.  If not, see
    <http://www.gnu.org/licenses/>.
*/

#include "game.h"
#include "globals.h"
#include "helpers.h"

#include <stdexcept>

#if _WIN32 || _WIN64
#   include <Windows.h>
#   include <Shlobj.h>
#endif

using namespace std;

namespace fs = boost::filesystem;

namespace boss {

    Game::Game()
        : id(BOSS_GAME_AUTODETECT) {}

    Game::Game(const uint32_t gameCode, const string path, const bool noPathInit)
        : id(gameCode) {
        if (Id() == BOSS_GAME_TES4) {
            name = "TES IV: Oblivion";

            registryKey = "Software\\Bethesda Softworks\\Oblivion";
            registrySubKey = "Installed Path";

            bossFolderName = "Oblivion";
            pluginsFolderName = "Data";
        } else if (Id() == BOSS_GAME_TES5) {
            name = "TES V: Skyrim";

            registryKey = "Software\\Bethesda Softworks\\Skyrim";
            registrySubKey = "Installed Path";

            bossFolderName = "Skyrim";
            pluginsFolderName = "Data";
        } else if (Id() == BOSS_GAME_FO3) {
            name = "Fallout 3";

            registryKey = "Software\\Bethesda Softworks\\Fallout3";
            registrySubKey = "Installed Path";

            bossFolderName = "Fallout 3";
            pluginsFolderName = "Data";
        } else if (Id() == BOSS_GAME_FONV) {
            name = "Fallout: New Vegas";

            registryKey = "Software\\Bethesda Softworks\\FalloutNV";
            registrySubKey = "Installed Path";

            bossFolderName = "Fallout New Vegas";
            pluginsFolderName = "Data";
        } else
            throw runtime_error("Invalid game ID supplied.");

        if (!noPathInit) {
            if (path.empty()) {
                //First look for local install, then look for Registry.
                if (IsInstalledLocally())
                    gamePath = "..";
                else if (RegKeyExists("HKEY_LOCAL_MACHINE", registryKey, registrySubKey))
                    gamePath = fs::path(RegKeyStringValue("HKEY_LOCAL_MACHINE", registryKey, registrySubKey));
                else
                    throw runtime_error("Game path could not be detected.");
            } else
                gamePath = fs::path(path);
        }
    }

    bool Game::IsInstalled() const {
        return (IsInstalledLocally() || RegKeyExists("HKEY_LOCAL_MACHINE", registryKey, registrySubKey));
    }

    bool Game::IsInstalledLocally() const {
        return fs::exists(fs::path("..") / pluginsFolderName);
    }

    uint32_t Game::Id() const {
        return id;
    }

    string Game::Name() const {
        return name;
    }

    fs::path Game::GamePath() const {
        return gamePath;
    }

    fs::path Game::DataPath() const {
        return GamePath() / pluginsFolderName;
    }

    void Game::CreateBOSSGameFolder() {
        //Make sure that the BOSS game path exists.
        try {
            if (!fs::exists(bossFolderName))
                fs::create_directory(bossFolderName);
        } catch (fs::filesystem_error e) {
            throw runtime_error("Could not create BOSS folder for game.");
        }
    }

    //Can be used to get the location of the LOCALAPPDATA folder (and its Windows XP equivalent).
    fs::path Game::GetLocalAppDataPath() {
#if _WIN32 || _WIN64
        HWND owner;
        TCHAR path[MAX_PATH];

        HRESULT res = SHGetFolderPath(owner, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);

        if (res == S_OK)
            return fs::path(path);
        else
            return fs::path("");
#else
        return fs::path("");
#endif
    }
}
