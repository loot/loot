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

#ifndef __LOOT_GUI_SETTINGS__
#define __LOOT_GUI_SETTINGS__

#include "ids.h"
#include "../backend/game.h"

#include <yaml-cpp/yaml.h>
#include <wx/listctrl.h>

class SettingsFrame : public wxDialog {
public:
    SettingsFrame(wxWindow *parent, const wxString& title, YAML::Node& settings, std::vector<loot::Game>& games, loot::Game * currentGame, wxPoint pos, wxSize size);

	void OnQuit(wxCommandEvent& event);
    void OnGameSelect(wxListEvent& event);
    void OnAddGame(wxCommandEvent& event);
    void OnEditGame(wxCommandEvent& event);
    void OnRemoveGame(wxCommandEvent& event);

	void SetDefaultValues();
private:
	wxChoice *DebugVerbosityChoice;
	wxChoice *GameChoice;
	wxChoice *LanguageChoice;
    wxCheckBox *UpdateMasterlistBox;

    wxListView *gamesList;
    wxButton * addBtn;
    wxButton * editBtn;
    wxButton * removeBtn;

    YAML::Node& _settings;
    std::vector<loot::Game>& _games;
    loot::Game * _currentGame;
};

class GameEditDialog : public wxDialog {
public:
    GameEditDialog(wxWindow *parent, const wxString& title);

    void SetValues(unsigned int type, const wxString& name, const wxString& folderName, const wxString& master,
                    const wxString& repo, const wxString& branch, const wxString& path, const wxString& registry);
    wxString GetName() const;
    wxString GetType() const;
    wxString GetFolderName() const;
    wxString GetMaster() const;
    wxString GetRepoURL() const;
    wxString GetRepoBranch() const;
    wxString GetPath() const;
    wxString GetRegistryKey() const;
private:
    wxChoice * _type;
    wxTextCtrl * _name;
    wxTextCtrl * _folderName;
    wxTextCtrl * _master;
    wxTextCtrl * _repo;
    wxTextCtrl * _branch;
    wxTextCtrl * _path;
    wxTextCtrl * _registry;
};

#endif
