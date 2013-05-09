/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2013    WrinklyNinja

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

#ifndef __BOSS_GUI_MAIN__
#define __BOSS_GUI_MAIN__

#include "ids.h"
#include "../game.h"

#include <vector>
#include <yaml-cpp/yaml.h>

//Program class.
class BossGUI : public wxApp {
public:
	bool OnInit();  //Load settings, apply logging and language settings, check if BOSS is already running, detect games, set game to last game or to first detected game if auto, create launcher window.
private:
    wxLocale * wxLoc;
};

class Launcher : public wxFrame {
public:
    Launcher(const wxChar *title, const YAML::Node& settings, const boss::Game& inGame, const std::vector<unsigned int>& inGames);
    
	void OnSortPlugins(wxCommandEvent& event);
    void OnEditMetadata(wxCommandEvent& event);
    void OnViewLastReport(wxCommandEvent& event);
    
    void OnOpenSettings(wxCommandEvent& event);
    void OnGameChange(wxCommandEvent& event);
	void OnOpenFile(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);

    DECLARE_EVENT_TABLE()
private:
    wxMenu * GameMenu;
	wxButton * ViewButton;
    
	std::vector<unsigned int> _detectedGames;
	boss::Game _game;
    YAML::Node _settings;  //BOSS Settings.
    
	void DisableUndetectedGames();

    static bool AlphaSortPlugins(const boss::Plugin& lhs, const boss::Plugin& rhs);
};

#endif
