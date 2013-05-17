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

#include "settings.h"
#include "../globals.h"
#include <fstream>
#include <boost/algorithm/string.hpp>

BEGIN_EVENT_TABLE ( SettingsFrame, wxDialog )
	EVT_BUTTON ( wxID_OK, SettingsFrame::OnQuit)
END_EVENT_TABLE()

using namespace std;

SettingsFrame::SettingsFrame(wxWindow *parent, const wxString& title, YAML::Node& settings, const std::vector<boss::Game>& games) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER), _settings(settings), _games(games) {

    //Initialise drop-down list contents.
	wxString DebugVerbosity[] = {
        translate("None"),
        translate("Low"),
		translate("Medium"),
		translate("High")
    };

    wxArrayString Games;
    Games.Add(translate("Autodetect"));
    for (size_t i=0,max=_games.size(); i < max; ++i) {
        Games.Add(FromUTF8(_games[i].Name()));
    }

	wxString Language[] = {
		wxT("English"),
	/*	wxString::FromUTF8("Español"),
		wxT("Deutsch"),
		wxString::FromUTF8("Русский"),
		wxString::FromUTF8("简体中文")*/
	};

    //Initialise controls.
    GameChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, Games);
    LanguageChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 1, Language);
    DebugVerbosityChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, DebugVerbosity);

    OblivionURL = new wxTextCtrl(this, wxID_ANY);
    SkyrimURL = new wxTextCtrl(this, wxID_ANY);
    FO3URL = new wxTextCtrl(this, wxID_ANY);
    FONVURL = new wxTextCtrl(this, wxID_ANY);

    UpdateMasterlistBox = new wxCheckBox(this, wxID_ANY, translate("Update masterlist before sorting."));
    reportViewBox = new wxCheckBox(this, wxID_ANY, translate("View reports externally in default browser."));

    //Set up layout.
	wxSizerFlags leftItem(0);
	leftItem.Left();

	wxSizerFlags rightItem(1);
	rightItem.Right();

	wxSizerFlags wholeItem(0);
	wholeItem.Border(wxLEFT|wxRIGHT|wxBOTTOM, 10);

    wxBoxSizer * bigBox = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer * GridSizer = new wxFlexGridSizer(2, 5, 5);
    GridSizer->AddGrowableCol(1,1);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Default Game:")), leftItem);
	GridSizer->Add(GameChoice, rightItem);
    
	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Language:")), leftItem);
	GridSizer->Add(LanguageChoice, rightItem);
	
    GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Debug Verbosity:")), leftItem);
	GridSizer->Add(DebugVerbosityChoice, rightItem);

    rightItem.Expand();

    GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Oblivion Masterlist URL:")), leftItem);
	GridSizer->Add(OblivionURL, rightItem);
	
    GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Skyrim Masterlist URL:")), leftItem);
	GridSizer->Add(SkyrimURL, rightItem);
	
    GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Fallout 3 Masterlist URL:")), leftItem);
	GridSizer->Add(FO3URL, rightItem);
	
    GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Fallout: New Vegas Masterlist URL:")), leftItem);
	GridSizer->Add(FONVURL, rightItem);
    
	bigBox->Add(GridSizer, 0, wxEXPAND|wxALL, 10);
   
    bigBox->Add(UpdateMasterlistBox, wholeItem);

    bigBox->Add(reportViewBox, wholeItem);
    
    bigBox->AddSpacer(10);
    bigBox->AddStretchSpacer(1);
	
	bigBox->Add(new wxStaticText(this, wxID_ANY, translate("Language changes will be applied after BOSS is restarted.")), wholeItem);
	
	//Need to add 'OK' and 'Cancel' buttons.
	wxSizer * sizer = CreateSeparatedButtonSizer(wxOK|wxCANCEL);

	//Now add TabHolder and OK button to window sizer.
    if (sizer != NULL)
        bigBox->Add(sizer, 0, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT, 15);

	//Initialise options with values. For checkboxes, they are off by default.
	SetDefaultValues();

	//Tooltips.
	DebugVerbosityChoice->SetToolTip(translate("The output is logged to the BOSSDebugLog.txt file"));

	//Now set the layout and sizes.
	SetBackgroundColour(wxColour(255,255,255));
    SetIcon(wxIconLocation("BOSS.exe"));
	SetSizerAndFit(bigBox);
}

void SettingsFrame::SetDefaultValues() {

    if (_settings["Language"]) {
        string lang = _settings["Language"].as<string>();
        if (lang == "eng")
            LanguageChoice->SetSelection(0);
    }

    if (_settings["Game"]) {
        string game = _settings["Game"].as<string>();
        if (boost::iequals(game, "auto"))
            GameChoice->SetSelection(0);
        else {
            for (size_t i=0,max=_games.size(); i < max; ++i) {
                if (boost::iequals(game, _games[i].FolderName()))
                    GameChoice->SetSelection(i+1);
            }
        }
    }

    if (_settings["Debug Verbosity"]) {
        unsigned int verbosity = _settings["Debug Verbosity"].as<unsigned int>();
        DebugVerbosityChoice->SetSelection(verbosity);
    }

    if (_settings["Update Masterlist"]) {
        bool update = _settings["Update Masterlist"].as<bool>();
        UpdateMasterlistBox->SetValue(update);
    }

    if (_settings["View Report Externally"]) {
        bool view = _settings["View Report Externally"].as<bool>();
        reportViewBox->SetValue(view);
    }

    if (_settings["Games"]) {

        if (_settings["Games"][boss::Game(boss::GAME_TES4).FolderName()])
            OblivionURL->SetValue(FromUTF8(_settings["Games"][boss::Game(boss::GAME_TES4).FolderName()]["url"].as<string>()));

        if (_settings["Games"][boss::Game(boss::GAME_TES5).FolderName()])
            SkyrimURL->SetValue(FromUTF8(_settings["Games"][boss::Game(boss::GAME_TES5).FolderName()]["url"].as<string>()));

        if (_settings["Games"][boss::Game(boss::GAME_FO3).FolderName()])
            FO3URL->SetValue(FromUTF8(_settings["Games"][boss::Game(boss::GAME_FO3).FolderName()]["url"].as<string>()));

        if (_settings["Games"][boss::Game(boss::GAME_FONV).FolderName()])
            FONVURL->SetValue(FromUTF8(_settings["Games"][boss::Game(boss::GAME_FONV).FolderName()]["url"].as<string>()));
    }
}

void SettingsFrame::OnQuit(wxCommandEvent& event) {
    if (event.GetId() == wxID_OK) {

        if (GameChoice->GetSelection() == 0)
            _settings["Game"] = "auto";
        else
            _settings["Game"] = _games[GameChoice->GetSelection() - 1].FolderName();
        
        switch (LanguageChoice->GetSelection()) {
        case 0:
            _settings["Language"] = "eng";
            break;
        }

        _settings["Debug Verbosity"] = DebugVerbosityChoice->GetSelection();

        _settings["Update Masterlist"] = UpdateMasterlistBox->IsChecked();

        _settings["View Report Externally"] = reportViewBox->IsChecked();

        _settings["Games"][boss::Game(boss::GAME_TES4).FolderName()]["url"] = string(OblivionURL->GetValue().ToUTF8());

        _settings["Games"][boss::Game(boss::GAME_TES5).FolderName()]["url"] = string(SkyrimURL->GetValue().ToUTF8());

        _settings["Games"][boss::Game(boss::GAME_FO3).FolderName()]["url"] = string(FO3URL->GetValue().ToUTF8());

        _settings["Games"][boss::Game(boss::GAME_FONV).FolderName()]["url"] = string(FONVURL->GetValue().ToUTF8());
    }

	EndModal(0);
}
