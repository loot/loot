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
#ifndef __BOSS_GLOBALS__
#define __BOSS_GLOBALS__

#include <boost/filesystem.hpp>

namespace boss {

    //Game values.
    extern const unsigned int GAME_AUTODETECT;
    extern const unsigned int GAME_TES4;
    extern const unsigned int GAME_TES5;
    extern const unsigned int GAME_FO3;
    extern const unsigned int GAME_FONV;

    //Message types.
    extern const unsigned int MESSAGE_SAY;
    extern const unsigned int MESSAGE_WARN;
    extern const unsigned int MESSAGE_ERROR;
    extern const unsigned int MESSAGE_TAG;

    //Languages
    extern const unsigned int LANG_AUTO;
    extern const unsigned int LANG_ENG;

    //Version numbers.
    extern const unsigned int VERSION_MAJOR;
    extern const unsigned int VERSION_MINOR;
    extern const unsigned int VERSION_PATCH;

    //Common paths.
    extern const boost::filesystem::path local_path;
    extern const boost::filesystem::path readme_path;
    extern const boost::filesystem::path settings_path;
    extern const boost::filesystem::path libespm_options_path;
    extern const boost::filesystem::path svn_path;
    extern const boost::filesystem::path log_path;
}

#endif
