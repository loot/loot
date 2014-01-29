/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2012-2013    WrinklyNinja

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

#include "globals.h"
#include "helpers.h"

namespace boss {

    //Game values.
    const unsigned int g_game_autodetect  = 0;
    const unsigned int g_game_tes4        = 1;
    const unsigned int g_game_tes5        = 2;
    const unsigned int g_game_fo3         = 3;
    const unsigned int g_game_fonv        = 4;

    //Message types.
    const unsigned int g_message_say      = 0;
    const unsigned int g_message_warn     = 1;
    const unsigned int g_message_error    = 2;
    const unsigned int g_message_tag      = 3;

    //Languages
    const unsigned int g_lang_any      = 0;
    const unsigned int g_lang_english  = 1;
    const unsigned int g_lang_spanish  = 2;
    const unsigned int g_lang_russian  = 3;

    //Version numbers.
    const unsigned int g_version_major = 3;
    const unsigned int g_version_minor = 0;
    const unsigned int g_version_patch = 0;

    //Common paths.
    const boost::filesystem::path g_path_readme           = boost::filesystem::current_path() / "docs" / "BOSS Readme.html";
    const boost::filesystem::path g_path_css              = boost::filesystem::current_path() / "resources" / "style.css";
    const boost::filesystem::path g_path_js               = boost::filesystem::current_path() / "resources" / "script.js";
    const boost::filesystem::path g_path_polyfill         = boost::filesystem::current_path() / "resources" / "polyfill.js";
    const boost::filesystem::path g_path_l10n             = boost::filesystem::current_path() / "resources" / "l10n";
    const boost::filesystem::path g_path_local            = GetLocalAppDataPath() / "BOSS";
    const boost::filesystem::path g_path_settings         = g_path_local / "settings.yaml";
    const boost::filesystem::path g_path_log              = g_path_local / "BOSSDebugLog.txt";
}
