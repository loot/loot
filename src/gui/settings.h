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

#ifndef __BOSS_GUI_SETTINGS__
#define __BOSS_GUI_SETTINGS__

#include "ids.h"
#include "../backend/game.h"

#include <yaml-cpp/yaml.h>
#include <wx/listctrl.h>

class SettingsFrame : public wxDialog {
public:
	SettingsFrame(wxWindow *parent, const wxString& title, YAML::Node& settings, std::vector<boss::Game>& games);

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
    wxCheckBox *reportViewBox;
    wxCheckBox *displayGraphImageBox;

    wxListView *gamesList;
    wxButton * addBtn;
    wxButton * editBtn;
    wxButton * removeBtn;

    YAML::Node& _settings;
    std::vector<boss::Game>& _games;
};

class GameEditDialog : public wxDialog {
public:
    GameEditDialog(wxWindow *parent, const wxString& title);

    void SetValues(unsigned int type, const wxString& name, const wxString& folderName, const wxString& master,
                    const wxString& url, const wxString& path, const wxString& registry);
    wxString GetName() const;
    wxString GetType() const;
    wxString GetFolderName() const;
    wxString GetMaster() const;
    wxString GetURL() const;
    wxString GetPath() const;
    wxString GetRegistryKey() const;
private:
    wxChoice * _type;
    wxTextCtrl * _name;
    wxTextCtrl * _folderName;
    wxTextCtrl * _master;
    wxTextCtrl * _url;
    wxTextCtrl * _path;
    wxTextCtrl * _registry;
};

#endif
