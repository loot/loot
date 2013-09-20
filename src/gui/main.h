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
#include "../backend/game.h"

#include <vector>
#include <yaml-cpp/yaml.h>
#include <wx/listctrl.h>

//Program class.
class BossGUI : public wxApp {
public:
	bool OnInit();  //Load settings, apply logging and language settings, check if BOSS is already running, detect games, set game to last game or to first detected game if auto, create launcher window.
private:
    wxLocale * wxLoc;

    YAML::Node _settings;
    std::vector<boss::Game> _games;
    boss::Game _game;
};

class Launcher : public wxFrame {
public:
    Launcher(const wxChar *title, YAML::Node& settings, boss::Game& inGame, std::vector<boss::Game>& games);

	void OnSortPlugins(wxCommandEvent& event);
    void OnEditMetadata(wxCommandEvent& event);
    void OnViewLastReport(wxCommandEvent& event);

    void OnOpenSettings(wxCommandEvent& event);
    void OnOpenDebugLog(wxCommandEvent& event);
    void OnGameChange(wxCommandEvent& event);
	void OnHelp(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);

    void OnClose(wxCloseEvent& event);
private:
    wxMenu * GameMenu;
	wxButton * ViewButton;

	boss::Game& _game;
    YAML::Node& _settings;  //BOSS Settings.
    std::vector<boss::Game>& _games;
};

class LoadOrderPreview : public wxDialog {
public:
    LoadOrderPreview(wxWindow *parent, const wxString title, const std::list<boss::Plugin>& plugins);

    void OnPluginSelect(wxListEvent& event);
    void OnMoveUp(wxCommandEvent& event);
    void OnMoveDown(wxCommandEvent& event);

    std::list<boss::Plugin> GetLoadOrder() const;
private:
    wxListView * _loadOrder;
    wxButton * _moveUp;
    wxButton * _moveDown;

    const std::list<boss::Plugin> _plugins;
    std::set<std::string> _movedPlugins;
};

#endif
