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

BEGIN_EVENT_TABLE ( SettingsFrame, wxFrame )
	EVT_BUTTON ( OPTION_OKExitSettings, SettingsFrame::OnQuit)
	EVT_BUTTON ( OPTION_CancelExitSettings, SettingsFrame::OnQuit)
END_EVENT_TABLE()

using namespace std;

SettingsFrame::SettingsFrame(const wxString title, wxFrame *parent, YAML::Node& settings) : wxFrame(parent, wxID_ANY, title), _settings(settings) {

    //Initialise drop-down list contents.
	wxString DebugVerbosity[] = {
        translate("None"),
        translate("Low"),
		translate("Medium"),
		translate("High")
    };

	wxString Game[] = {
		translate("Autodetect"),
		wxT("Oblivion"),
		wxT("Nehrim"),
		wxT("Skyrim"),
		wxT("Fallout 3"),
		wxT("Fallout: New Vegas"),
	};

	wxString Language[] = {
		wxT("English"),
	/*	wxString::FromUTF8("Español"),
		wxT("Deutsch"),
		wxString::FromUTF8("Русский"),
		wxString::FromUTF8("简体中文")*/
	};

    //Initialise controls.
    GameChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 6, Game);
    LanguageChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 1, Language);
    DebugVerbosityChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, DebugVerbosity);

    OblivionURL = new wxTextCtrl(this, wxID_ANY);
    NehrimURL = new wxTextCtrl(this, wxID_ANY);
    SkyrimURL = new wxTextCtrl(this, wxID_ANY);
    FO3URL = new wxTextCtrl(this, wxID_ANY);
    FONVURL = new wxTextCtrl(this, wxID_ANY);

    UpdateMasterlistBox = new wxCheckBox(this, wxID_ANY, translate("Update masterlist before sorting."));

    wxButton * okBtn = new wxButton(this, OPTION_OKExitSettings, translate("OK"), wxDefaultPosition, wxSize(70, 30));
    wxButton * cancelBtn = new wxButton(this, OPTION_CancelExitSettings, translate("Cancel"), wxDefaultPosition, wxSize(70, 30));

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
	
    GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Nehrim Masterlist URL:")), leftItem);
	GridSizer->Add(NehrimURL, rightItem);
	
    GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Skyrim Masterlist URL:")), leftItem);
	GridSizer->Add(SkyrimURL, rightItem);
	
    GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Fallout 3 Masterlist URL:")), leftItem);
	GridSizer->Add(FO3URL, rightItem);
	
    GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Fallout: New Vegas Masterlist URL:")), leftItem);
	GridSizer->Add(FONVURL, rightItem);
    
	bigBox->Add(GridSizer, 0, wxEXPAND|wxALL, 10);
   
    bigBox->Add(UpdateMasterlistBox, wholeItem);

    bigBox->AddStretchSpacer(1);
	
	bigBox->Add(new wxStaticText(this, wxID_ANY, translate("Settings will be applied after BOSS is restarted.")), wholeItem);
	
	//Need to add 'OK' and 'Cancel' buttons.
	wxBoxSizer * hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(okBtn);
	hbox->Add(cancelBtn, 0, wxLEFT, 10);

    wholeItem.Centre();

	//Now add TabHolder and OK button to window sizer.
	bigBox->Add(hbox, wholeItem);

	//Initialise options with values. For checkboxes, they are off by default.
	SetDefaultValues();

	//Tooltips.
	DebugVerbosityChoice->SetToolTip(translate("The output is logged to the BOSSDebugLog.txt file"));

	//Now set the layout and sizes.
	SetBackgroundColour(wxColour(255,255,255));
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
        if (game == "auto")
            GameChoice->SetSelection(0);
        else if (game == "oblivion")
            GameChoice->SetSelection(1);
        else if (game == "nehrim")
            GameChoice->SetSelection(2);
        else if (game == "skyrim")
            GameChoice->SetSelection(3);
        else if (game == "fallout3")
            GameChoice->SetSelection(4);
        else if (game == "falloutnv")
            GameChoice->SetSelection(5);
    }

    if (_settings["Debug Verbosity"]) {
        unsigned int verbosity = _settings["Debug Verbosity"].as<unsigned int>();
        DebugVerbosityChoice->SetSelection(verbosity);
    }

    if (_settings["Update Masterlist"]) {
        bool update = _settings["Update Masterlist"].as<bool>();
        UpdateMasterlistBox->SetValue(update);
    }

    if (_settings["Masterlist URLs"]) {
        YAML::Node urls = _settings["Masterlist URLs"];

        if (urls["Oblivion"])
            OblivionURL->SetValue(FromUTF8(urls["Oblivion"].as<string>()));

        if (urls["Nehrim"])
            NehrimURL->SetValue(FromUTF8(urls["Nehrim"].as<string>()));

        if (urls["Skyrim"])
            SkyrimURL->SetValue(FromUTF8(urls["Skyrim"].as<string>()));

        if (urls["Fallout 3"])
            FO3URL->SetValue(FromUTF8(urls["Fallout 3"].as<string>()));

        if (urls["Fallout New Vegas"])
            FONVURL->SetValue(FromUTF8(urls["Fallout New Vegas"].as<string>()));
    }
}

void SettingsFrame::OnQuit(wxCommandEvent& event) {
    if (event.GetId() == OPTION_OKExitSettings) {

        switch (GameChoice->GetSelection()) {
        case 0:
            _settings["Game"] = "auto";
            break;
        case 1:
            _settings["Game"] = "oblivion";
            break;
        case 2:
            _settings["Game"] = "nehrim";
            break;
        case 3:
            _settings["Game"] = "skyrim";
            break;
        case 4:
            _settings["Game"] = "fallout3";
            break;
        case 5:
            _settings["Game"] = "falloutnv";
            break;
        }
        
        switch (LanguageChoice->GetSelection()) {
        case 0:
            _settings["Language"] = "eng";
            break;
        }

        _settings["Debug Verbosity"] = DebugVerbosityChoice->GetSelection();

        _settings["Update Masterlist"] = UpdateMasterlistBox->IsChecked();

        _settings["Masterlist URLs"]["Oblivion"] = string(OblivionURL->GetValue().ToUTF8());

        _settings["Masterlist URLs"]["Nehrim"] = string(NehrimURL->GetValue().ToUTF8());

        _settings["Masterlist URLs"]["Skyrim"] = string(SkyrimURL->GetValue().ToUTF8());

        _settings["Masterlist URLs"]["Fallout 3"] = string(FO3URL->GetValue().ToUTF8());

        _settings["Masterlist URLs"]["Fallout New Vegas"] = string(FONVURL->GetValue().ToUTF8());

        YAML::Emitter yout;
        yout.SetIndent(2);
        yout << _settings;
        
        ofstream out(boss::settings_path.string().c_str());
        out << yout.c_str();
        out.close();
    }

	this->Close();
}
