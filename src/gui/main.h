/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013-2014    WrinklyNinja

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

#ifndef __LOOT_GUI_MAIN__
#define __LOOT_GUI_MAIN__

#include "ids.h"
#include "../backend/game.h"

#include <vector>
#include <yaml-cpp/yaml.h>
#include <wx/listctrl.h>

//Program class.
class LOOT : public wxApp {
public:
	bool OnInit();  //Load settings, apply logging and language settings, check if LOOT is already running, detect games, set game to last game or to first detected game if auto, create launcher window.
private:
    wxLocale * wxLoc;

    YAML::Node _settings;
    std::vector<loot::Game> _games;
};

class Launcher : public wxFrame {
public:
    Launcher(const wxChar *title, YAML::Node& settings, loot::Game * inGame, std::vector<loot::Game>& games);

	void OnSortPlugins(wxCommandEvent& event);
    void OnEditMetadata(wxCommandEvent& event);
    void OnViewLastReport(wxCommandEvent& event);
    void OnRedatePlugins(wxCommandEvent& event);

    void OnOpenSettings(wxCommandEvent& event);
    void OnOpenDebugLog(wxCommandEvent& event);
    void OnGameChange(wxCommandEvent& event);
	void OnHelp(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);

    void OnClose(wxCloseEvent& event);
private:
    wxMenu * GameMenu;
    wxMenuItem * RedatePluginsItem;
	wxButton * ViewButton;

	loot::Game * _game;
    YAML::Node& _settings;  //LOOT Settings.
    std::vector<loot::Game>& _games;
};

#endif
