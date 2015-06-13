/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2015    WrinklyNinja

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

#include "masterlist.h"
#include "game/game.h"
#include "error.h"

using namespace std;

namespace fs = boost::filesystem;
namespace lc = boost::locale;

namespace loot {
    bool Masterlist::Load(Game& game, const unsigned int language) {
        try {
            return Update(game);
        }
        catch (error& e) {
            if (e.code() != error::ok) {
                // Error wasn't a parsing error. Need to try parsing masterlist if it exists.
                try {
                    MetadataList::Load(game.MasterlistPath());
                }
                catch (...) {}
            }
            throw;
        }
    }

    std::string Masterlist::GetRevision(const boost::filesystem::path& path, bool shortID) {
        if (revision.empty() || (shortID && revision.length() == 40) || (!shortID && revision.length() < 40))
            GetGitInfo(path, shortID);

        return revision;
    }

    std::string Masterlist::GetDate(const boost::filesystem::path& path) {
        if (date.empty())
            GetGitInfo(path, true);

        return date;
    }
}
