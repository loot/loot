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

#include "globals.h"
#include "helpers.h"

namespace boss {

    //Game values.
    const unsigned int GAME_AUTODETECT  = 0;
    const unsigned int GAME_TES4        = 1;
    const unsigned int GAME_TES5        = 2;
    const unsigned int GAME_FO3         = 3;
    const unsigned int GAME_FONV        = 4;

    //Message types.
    const unsigned int MESSAGE_SAY      = 0;
    const unsigned int MESSAGE_WARN     = 1;
    const unsigned int MESSAGE_ERROR    = 2;
    const unsigned int MESSAGE_TAG      = 3;

    //Languages
    const unsigned int LANG_AUTO    = 0;
    const unsigned int LANG_ENG     = 1;

    //Version numbers.
    const unsigned int VERSION_MAJOR = 3;
    const unsigned int VERSION_MINOR = 0;
    const unsigned int VERSION_PATCH = 0;

    //Common paths.
    const boost::filesystem::path readme_path           = "docs/BOSS Readme.html";
    const boost::filesystem::path libespm_options_path  = "resources/libespm.yaml";
    const boost::filesystem::path svn_path              = "resources/svn/svn.exe";
    const boost::filesystem::path local_path            = GetLocalAppDataPath() / "BOSS";
    const boost::filesystem::path css_path              = boost::filesystem::current_path() / "resources" / "style.css";
    const boost::filesystem::path js_path               = boost::filesystem::current_path() / "resources" / "script.js";
    const boost::filesystem::path polyfill_path         = boost::filesystem::current_path() / "resources" / "polyfill.js";
    const boost::filesystem::path settings_path         = local_path / "settings.yaml";
    const boost::filesystem::path log_path              = local_path / "BOSSDebugLog.txt";
}
