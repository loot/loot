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

#ifndef __LOOT_NETWORK__
#define __LOOT_NETWORK__

#include <vector>
#include <string>

#include "game.h"
#include "metadata.h"

namespace loot {

    std::pair<std::string, std::string> UpdateMasterlist(Game& game, std::list<Message>& parsingErrors, std::list<Plugin>& plugins, std::list<Message>& messages, const unsigned int language);

    std::pair<std::string, std::string> GetMasterlistRevision(const Game& game);
}
#endif
