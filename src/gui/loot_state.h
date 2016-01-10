/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2014-2015    WrinklyNinja

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

#ifndef __LOOT_GUI_LOOT_STATE__
#define __LOOT_GUI_LOOT_STATE__

#include "loot_settings.h"
#include "backend/game/game.h"

#include <include/cef_app.h>
#include <include/base/cef_lock.h>

namespace loot {
    class LootState : public CefBase, public LootSettings {
    public:
        LootState();

        void Init(const std::string& cmdLineGame);
        const std::vector<std::string>& InitErrors() const;

        void save(const boost::filesystem::path& file);

        Game& CurrentGame();
        void ChangeGame(const std::string& newGameFolder);
        void UpdateGamesFromSettings();

        // Get the folder names of the installed games.
        std::vector<std::string> InstalledGames();

        // Used to check if LOOT has unaccepted sorting or metadata changes on quit.
        int numUnappliedChanges;
    private:
        std::list<Game> _games;
        std::list<Game>::iterator _currentGame;
        std::vector<std::string> _initErrors;

        // Select initial game.
        void SelectGame(std::string cmdLineGame);

        static std::list<Game> ToGames(const std::vector<GameSettings>& settings);
        static std::vector<GameSettings> ToGameSettings(const std::list<Game>& games);

        // Lock used to protect access to member variables.
        base::Lock _lock;
        IMPLEMENT_REFCOUNTING(LootState);
    };
}

#endif
