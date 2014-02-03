/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2012-2014    WrinklyNinja

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
#ifndef __BOSS_GLOBALS__
#define __BOSS_GLOBALS__

#include <boost/filesystem.hpp>

namespace boss {

    //Game values.
    extern const unsigned int g_game_autodetect;
    extern const unsigned int g_game_tes4;
    extern const unsigned int g_game_tes5;
    extern const unsigned int g_game_fo3;
    extern const unsigned int g_game_fonv;

    //Message types.
    extern const unsigned int g_message_say;
    extern const unsigned int g_message_warn;
    extern const unsigned int g_message_error;
    extern const unsigned int g_message_tag;

    //Languages
    extern const unsigned int g_lang_any;
    extern const unsigned int g_lang_english;
    extern const unsigned int g_lang_spanish;
    extern const unsigned int g_lang_russian;
    extern const unsigned int g_lang_french;

    //Version numbers.
    extern const unsigned int g_version_major;
    extern const unsigned int g_version_minor;
    extern const unsigned int g_version_patch;

    //Common paths.
    extern const boost::filesystem::path g_path_local;
    extern const boost::filesystem::path g_path_readme;
    extern const boost::filesystem::path g_path_settings;
    extern const boost::filesystem::path g_path_css;
    extern const boost::filesystem::path g_path_js;
    extern const boost::filesystem::path g_path_polyfill;
    extern const boost::filesystem::path g_path_log;
    extern const boost::filesystem::path g_path_l10n;
}

#endif
