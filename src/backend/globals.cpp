/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2014    WrinklyNinja

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

#include "globals.h"
#include "helpers.h"

namespace loot {
    //Version numbers.
    const unsigned int g_version_major = 0;
    const unsigned int g_version_minor = 7;
    const unsigned int g_version_patch = 0;

    //Common paths.
    const boost::filesystem::path g_path_readme         = boost::filesystem::current_path() / "docs" / "LOOT Readme.html";
    const boost::filesystem::path g_path_report         = boost::filesystem::current_path() / "resources" / "report" / "report.html";
    const boost::filesystem::path g_path_l10n           = boost::filesystem::current_path() / "resources" / "l10n";
#ifdef _WIN32
    const boost::filesystem::path g_path_local          = GetLocalAppDataPath() / "LOOT";
#else
    const boost::filesystem::path g_path_local          = boost::filesystem::current_path();
#endif
    const boost::filesystem::path g_path_settings       = g_path_local / "settings.yaml";
    const boost::filesystem::path g_path_log            = g_path_local / "LOOTDebugLog.txt";
}
