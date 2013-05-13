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
#include <yaml-cpp/yaml.h>
#include <wx/listctrl.h>

class SettingsFrame : public wxDialog {
public:
	SettingsFrame(wxWindow *parent, const wxString& title, YAML::Node& settings);
	void OnQuit(wxCommandEvent& event);
	void SetDefaultValues();
	DECLARE_EVENT_TABLE()
private:
	wxChoice *DebugVerbosityChoice;
	wxChoice *GameChoice;
	wxChoice *LanguageChoice;
    wxCheckBox *UpdateMasterlistBox;
    wxCheckBox *reportViewBox;
    wxTextCtrl *OblivionURL;
    wxTextCtrl *NehrimURL;
    wxTextCtrl *SkyrimURL;
    wxTextCtrl *FO3URL;
    wxTextCtrl *FONVURL;

    YAML::Node& _settings;
};
#endif
