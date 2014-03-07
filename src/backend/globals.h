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
#ifndef __LOOT_GLOBALS__
#define __LOOT_GLOBALS__

#include <boost/filesystem.hpp>

namespace loot {

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
