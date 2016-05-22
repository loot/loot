/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

    Copyright(C) 2012 - 2016    WrinklyNinja

    This file is part of LOOT.

    LOOT is free software : you can redistribute
    it and / or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    LOOT is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with LOOT.If not, see
    <http://www.gnu.org/licenses/>.
*/

#include "loot_paths.h"
#include "../helpers/helpers.h"
#include "../error.h"

#include <boost/locale.hpp>

#include <locale>

#ifdef _WIN32
#   ifndef UNICODE
#       define UNICODE
#   endif
#   ifndef _UNICODE
#      define _UNICODE
#   endif
#   include "windows.h"
#   include "shlobj.h"
#endif

namespace loot {
    boost::filesystem::path LootPaths::getReadmePath() {
        return lootAppPath / "docs" / "LOOT Readme.html";
    }

    boost::filesystem::path LootPaths::getUIIndexPath() {
        return lootAppPath / "resources" / "ui" / "index.html";
    }

    boost::filesystem::path LootPaths::getL10nPath() {
        return lootAppPath / "resources" / "l10n";
    }

    boost::filesystem::path LootPaths::getLootDataPath() {
        return lootDataPath;
    }

    boost::filesystem::path LootPaths::getSettingsPath() {
        return lootDataPath / "settings.yaml";
    }

    boost::filesystem::path LootPaths::getLogPath() {
        return lootDataPath / "LOOTDebugLog.txt";
    }

    void LootPaths::initialise() {
        // Set the locale to get UTF-8 conversions working correctly.
        std::locale::global(boost::locale::generator().generate(""));
        boost::filesystem::path::imbue(std::locale());

        lootAppPath = boost::filesystem::current_path();
        lootDataPath = getLocalAppDataPath() / "LOOT";
    }

    void LootPaths::setLootAppPath(const boost::filesystem::path& path) {
        lootAppPath = path;
    }

    void LootPaths::setLootDataPath(const boost::filesystem::path& path) {
        lootDataPath = path;
    }

    boost::filesystem::path LootPaths::getLocalAppDataPath() {
#ifdef _WIN32
        HWND owner = 0;
        PWSTR path;

        if (SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path) != S_OK)
            throw error(error::windows_error, boost::locale::translate("Failed to get %LOCALAPPDATA% path."));

        boost::filesystem::path localAppDataPath(FromWinWide(path));
        CoTaskMemFree(path);

        return localAppDataPath;
#else
        // Use XDG_CONFIG_HOME environmental variable if it's available.
        const char * xdgConfigHome = getenv("XDG_CONFIG_HOME");

        if (xdgConfigHome != nullptr)
            return boost::filesystem::path(xdgConfigHome);

        // Otherwise, use the HOME env. var. if it's available.
        xdgConfigHome = getenv("HOME");

        if (xdgConfigHome != nullptr)
            return boost::filesystem::path(xdgConfigHome) / ".config";

        // If somehow both are missing, use the current path.
        return boost::filesystem::current_path();
#endif
    }

    boost::filesystem::path LootPaths::lootAppPath = boost::filesystem::path();
    boost::filesystem::path LootPaths::lootDataPath = boost::filesystem::path();
}
