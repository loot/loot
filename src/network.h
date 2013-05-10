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

namespace boss {

    /* Need to be able to:

      - Check if there's an internet connection.
      - Get the revision number for the local masterlist, if present.
      - Get the revision number for the remote masterlist.
      - Compare the two revision numbers.
      - Download the remote masterlist if it has a higher revision number.
      - Test that the downloaded masterlist is valid by parsing it to see if any errors come up.
      - If the downloaded masterlist is parsed OK, replace the local masterlist with it.
    */
}
