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

    /* Masterlist updating is carried out using subversion's executables via system() since it's API has too many dependencies to build myself.

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

    /* Also looking into adding Git support (issue #29). Git support would involve sparse checkouts of the masterlists into each game folder, with the masterlists sitting in the root of each game's repository online.

    Not sure how to handle the URL stuff with Git, because while Subversion allows you to pass it a URL and assumes that it goes to a location on a SVN server, with Git it needs to know the repository URL (with git extension), and then also the relative path of the file you want to checkout within the repository.

    Perhaps it's best to use a false URL that includes the ".git" extension at the location of the repository, eg.

    https://github.com/WrinklyNinja/BOSSv3.git/README.md

    That's not actually a valid URL, but it means that it can be split up in code into

    https://github.com/WrinklyNinja/BOSSv3.git

    and

    /README.md

    which is what is needed, AFAIK.
    */

    std::string UpdateMasterlist(const Game& game, std::vector<std::string>& parsingErrors);
}
#endif
