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

#ifndef __BOSS_NETWORK__
#define __BOSS_NETWORK__

#include <vector>
#include <string>

#include "game.h"

namespace boss {

    /* Masterlist updating is carried out using subversion. Haven't decided whether to use its API (which is in C and a bit complex) or call its executables via system().

    Basic workflow is:

    1. Check if there's a working copy present in the masterlist's directory.
    2. If not, perform a sparse checkout of the remote folder containing the masterlist, at a folder-only depth.
    3. SVN update the masterlist to the latest available revision.
    4. Check whether the masterlist parses without errors.
    5. If not, revert the masterlist back one revision. Loop back to step 4 until it parses OK.
    6. Record the revision of the masterlist for display in the BOSS report.

    Also need to record error messages for display in the BOSS report, including any parsing errors encountered in step 4, even if a working masterlist was found.

    ```svn info``` on a non-working-copy returns "svn: '.' is not a working copy".

    ```svn co --depth empty http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-skyrim/ .```
    followed by
    ```svn update masterlist.txt```
    works as desired when in BOSS's Skyrim folder.

    ```svn update --revision PREV masterlist.txt```
    would then roll the masterlist back one revision.

    ```svn info masterlist.txt``` returns a bunch of info: the line "Revision: XXXX" contains the revision number.
    */

    unsigned int UpdateMasterlist(const Game& game, std::vector<std::string>& parsingErrors);
}
#endif
