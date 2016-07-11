/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2016    WrinklyNinja

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

#ifndef LOOT_BACKEND_LOOT_PATHS
#define LOOT_BACKEND_LOOT_PATHS

#include <boost/filesystem.hpp>

namespace loot {
    class LootPaths {
    public:
        static boost::filesystem::path getReadmePath();
        static boost::filesystem::path getUIIndexPath();
        static boost::filesystem::path getL10nPath();
        static boost::filesystem::path getLootDataPath();
        static boost::filesystem::path getSettingsPath();
        static boost::filesystem::path getLogPath();

        // Sets the app path to the current path, and the data path to the user
        // local app data path / "LOOT".
        static void initialise();
    private:
        static boost::filesystem::path lootAppPath;
        static boost::filesystem::path lootDataPath;

        //Get the local application data path.
        static boost::filesystem::path getLocalAppDataPath();
    };
}

#endif
