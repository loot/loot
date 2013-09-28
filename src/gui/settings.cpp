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
#include "../backend/globals.h"
#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>

using namespace std;

SettingsFrame::SettingsFrame(wxWindow *parent, const wxString& title, YAML::Node& settings, std::vector<boss::Game>& games) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER), _settings(settings), _games(games) {

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

    //Initialise controls.
    GameChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, Games);
    LanguageChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, Language);
    DebugVerbosityChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, DebugVerbosity);

    gamesList = new wxListView(this, LIST_Games, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);

    addBtn = new wxButton(this, BUTTON_AddGame, translate("Add Game"));
    editBtn = new wxButton(this, BUTTON_EditGame, translate("Edit Game"));
    removeBtn = new wxButton(this, BUTTON_RemoveGame, translate("Remove Game"));

    UpdateMasterlistBox = new wxCheckBox(this, wxID_ANY, translate("Update masterlist before sorting."));
    reportViewBox = new wxCheckBox(this, wxID_ANY, translate("View reports externally in default browser."));
    displayGraphImageBox = new wxCheckBox(this, wxID_ANY, translate("Generate and display plugin graph images."));

    //Set up list columns.
    gamesList->AppendColumn(translate("Name"));
    gamesList->AppendColumn(translate("Base Game Type"));
    gamesList->AppendColumn(translate("BOSS Folder Name"));
    gamesList->AppendColumn(translate("Master File"));
    gamesList->AppendColumn(translate("Online Masterlist URL"));
    gamesList->AppendColumn(translate("Install Path"));
    gamesList->AppendColumn(translate("Install Path Registry Key"));

    //Set up event handling.
    Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &SettingsFrame::OnGameSelect, this, LIST_Games);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SettingsFrame::OnQuit, this, wxID_OK);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SettingsFrame::OnAddGame, this, BUTTON_AddGame);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SettingsFrame::OnEditGame, this, BUTTON_EditGame);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SettingsFrame::OnRemoveGame, this, BUTTON_RemoveGame);

    //Set up layout.
	wxSizerFlags leftItem(0);
	leftItem.Left();

	wxSizerFlags rightItem(1);
	rightItem.Right();

	wxSizerFlags wholeItem(0);
	wholeItem.Border(wxLEFT|wxRIGHT|wxBOTTOM, 15);

    wxBoxSizer * bigBox = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer * GridSizer = new wxFlexGridSizer(2, 5, 5);
    GridSizer->AddGrowableCol(1,1);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Default Game:")), leftItem);
	GridSizer->Add(GameChoice, rightItem);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Language:")), leftItem);
	GridSizer->Add(LanguageChoice, rightItem);

    GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Debug Verbosity:")), leftItem);
	GridSizer->Add(DebugVerbosityChoice, rightItem);

	bigBox->Add(GridSizer, 0, wxEXPAND|wxALL, 15);

    bigBox->Add(gamesList, wholeItem);

    wxBoxSizer * hbox2 = new wxBoxSizer(wxHORIZONTAL);
    hbox2->Add(addBtn, 0, wxRIGHT, 5);
    hbox2->Add(editBtn, 0, wxLEFT|wxRIGHT, 5);
    hbox2->Add(removeBtn, 0, wxLEFT, 5);
    bigBox->Add(hbox2, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT, 15);

    bigBox->Add(UpdateMasterlistBox, wholeItem);

    bigBox->Add(reportViewBox, wholeItem);

    bigBox->Add(displayGraphImageBox, wholeItem);

    bigBox->AddSpacer(10);
    bigBox->AddStretchSpacer(1);

	bigBox->Add(new wxStaticText(this, wxID_ANY, translate("Language and game changes will be applied after BOSS is restarted.")), wholeItem);

	//Need to add 'OK' and 'Cancel' buttons.
	wxSizer * sizer = CreateSeparatedButtonSizer(wxOK|wxCANCEL);

	//Now add TabHolder and OK button to window sizer.
    if (sizer != NULL)
        bigBox->Add(sizer, 0, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT, 15);

	//Initialise options with values. For checkboxes, they are off by default.
	SetDefaultValues();

	//Tooltips.
	DebugVerbosityChoice->SetToolTip(translate("The output is logged to the BOSSDebugLog.txt file."));

	//Now set the layout and sizes.
	SetBackgroundColour(wxColour(255,255,255));
    SetIcon(wxIconLocation("BOSS.exe"));
	SetSizerAndFit(bigBox);
}

void SettingsFrame::SetDefaultValues() {

    BOOST_LOG_TRIVIAL(debug) << "Setting default values for BOSS's settings.";

    if (_settings["Language"]) {
        LanguageChoice->SetSelection(GetLangIndex(_settings["Language"].as<string>()));
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

    if (_settings["Generate Graph Image"]) {
        bool graph = _settings["Generate Graph Image"].as<bool>();
        displayGraphImageBox->SetValue(graph);
    }

    for (size_t i=0, max=_games.size(); i < max; ++i) {
        gamesList->InsertItem(i, FromUTF8(_games[i].Name()));
        gamesList->SetItem(i, 1, FromUTF8(boss::Game(_games[i].Id()).FolderName()));
        gamesList->SetItem(i, 2, FromUTF8(_games[i].FolderName()));
        gamesList->SetItem(i, 3, FromUTF8(_games[i].Master()));
        gamesList->SetItem(i, 4, FromUTF8(_games[i].URL()));
        gamesList->SetItem(i, 5, FromUTF8(_games[i].GamePath().string()));
        gamesList->SetItem(i, 6, FromUTF8(_games[i].RegistryKey()));
    }

    addBtn->Enable(true);
    editBtn->Enable(false);
    removeBtn->Enable(false);
}

void SettingsFrame::OnQuit(wxCommandEvent& event) {
    if (event.GetId() == wxID_OK) {

        BOOST_LOG_TRIVIAL(debug) << "Applying settings.";

        if (GameChoice->GetSelection() == 0)
            _settings["Game"] = "auto";
        else
            _settings["Game"] = _games[GameChoice->GetSelection() - 1].FolderName();

        _settings["Language"] = GetLangStringFromIndex(LanguageChoice->GetSelection());

        _settings["Debug Verbosity"] = DebugVerbosityChoice->GetSelection();

        unsigned int verbosity = _settings["Debug Verbosity"].as<unsigned int>();
        if (verbosity == 0)
            boost::log::core::get()->set_logging_enabled(false);
        else {
            boost::log::core::get()->set_logging_enabled(true);

            if (verbosity == 1)
                boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::warning);  //Log all warnings, errors and fatals.
            else if (verbosity == 2)
                boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);  //Log debugs, infos, warnings, errors and fatals.
            else
                boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);  //Log everything.
        }

        _settings["Update Masterlist"] = UpdateMasterlistBox->IsChecked();

        _settings["View Report Externally"] = reportViewBox->IsChecked();

        _settings["Generate Graph Image"] = displayGraphImageBox->IsChecked();

        for (size_t i=0,max=gamesList->GetItemCount(); i < max; ++i) {
            string name, folder, master, url, path, registry;
            unsigned int id;

            name = gamesList->GetItemText(i, 0).ToUTF8();
            folder = gamesList->GetItemText(i, 2).ToUTF8();
            master = gamesList->GetItemText(i, 3).ToUTF8();
            url = gamesList->GetItemText(i, 4).ToUTF8();
            path = gamesList->GetItemText(i, 5).ToUTF8();
            registry = gamesList->GetItemText(i, 6).ToUTF8();

            if (gamesList->GetItemText(i, 1).ToUTF8() == boss::Game(boss::g_game_tes4).FolderName())
                id = boss::g_game_tes4;
            else if (gamesList->GetItemText(i, 1).ToUTF8() == boss::Game(boss::g_game_tes5).FolderName())
                id = boss::g_game_tes5;
            else if (gamesList->GetItemText(i, 1).ToUTF8() == boss::Game(boss::g_game_fo3).FolderName())
                id = boss::g_game_fo3;
            else
                id = boss::g_game_fonv;

            _games[i] = boss::Game(id, folder).SetDetails(name, master, url, path, registry);
        }
    }

	EndModal(0);
}

void SettingsFrame::OnGameSelect(wxListEvent& event) {
    wxString name = gamesList->GetItemText(event.GetIndex());
    if (name == boss::Game(boss::g_game_tes4).Name()
     || name == boss::Game(boss::g_game_tes5).Name()
     || name == boss::Game(boss::g_game_fo3).Name()
     || name == boss::Game(boss::g_game_fonv).Name()) {
        removeBtn->Enable(false);
     } else {
        removeBtn->Enable(true);
    }
    editBtn->Enable(true);
}

void SettingsFrame::OnAddGame(wxCommandEvent& event) {
    BOOST_LOG_TRIVIAL(debug) << "Adding new game to settings.";

    GameEditDialog * rowDialog = new GameEditDialog(this, translate("BOSS: Add Game"));

    if (rowDialog->ShowModal() == wxID_OK) {

        if (rowDialog->GetName().empty()) {
            BOOST_LOG_TRIVIAL(error) << "Tried to add a new game with no name given.";
            wxMessageBox(
                translate("Error: Name is required. Row will not be added."),
                translate("BOSS: Error"),
                wxOK | wxICON_ERROR,
                this);
            return;
        } else if (rowDialog->GetFolderName().empty()) {
            BOOST_LOG_TRIVIAL(error) << "Tried to add a new game with no folder given.";
            wxMessageBox(
                translate("Error: Folder is required. Row will not be added."),
                translate("BOSS: Error"),
                wxOK | wxICON_ERROR,
                this);
            return;
        } else if (rowDialog->GetPath().empty() && rowDialog->GetRegistryKey().empty()) {
            BOOST_LOG_TRIVIAL(error) << "Tried to add a new game with no path or registry key given.";
            wxMessageBox(
                translate("Error: A path and/or registry key is required. Row will not be added."),
                translate("BOSS: Error"),
                wxOK | wxICON_ERROR,
                this);
            return;
        }

        //Also check that name and folder name don't already exist in the list.
        for (size_t i=0,max=gamesList->GetItemCount(); i < max; ++i) {
            if (rowDialog->GetName() == gamesList->GetItemText(i, 0)) {
                BOOST_LOG_TRIVIAL(error) << "Tried to add a new game with the same name as one that is already defined.";
                wxMessageBox(
                    translate("Error: A game with this name is already defined. Row will not be added."),
                    translate("BOSS: Error"),
                    wxOK | wxICON_ERROR,
                    this);
                return;
            } else if (rowDialog->GetFolderName() == gamesList->GetItemText(i, 2)) {
                BOOST_LOG_TRIVIAL(error) << "Tried to add a new game with the same folder as one that is already defined.";
                wxMessageBox(
                    translate("Error: A game with this folder name is already defined. Row will not be added."),
                    translate("BOSS: Error"),
                    wxOK | wxICON_ERROR,
                    this);
                return;
            }
        }

        long i = gamesList->GetItemCount();
        gamesList->InsertItem(i, rowDialog->GetName());
        gamesList->SetItem(i, 1, FromUTF8(boss::Game(string(rowDialog->GetType().ToUTF8())).FolderName()));
        gamesList->SetItem(i, 2, rowDialog->GetFolderName());
        gamesList->SetItem(i, 3, rowDialog->GetMaster());
        gamesList->SetItem(i, 4, rowDialog->GetURL());
        gamesList->SetItem(i, 5, rowDialog->GetPath());
        gamesList->SetItem(i, 6, rowDialog->GetRegistryKey());
    }
}

void SettingsFrame::OnEditGame(wxCommandEvent& event) {
    BOOST_LOG_TRIVIAL(debug) << "Editing game settings.";

    GameEditDialog * rowDialog = new GameEditDialog(this, translate("BOSS: Edit Game"));

    long i = gamesList->GetFirstSelected();

    int stateNo;
    if (gamesList->GetItemText(i, 1) == boss::Game(boss::g_game_tes4).FolderName())
        stateNo = boss::g_game_tes4;
    else if (gamesList->GetItemText(i, 1) == boss::Game(boss::g_game_tes5).FolderName())
        stateNo = boss::g_game_tes5;
    else if (gamesList->GetItemText(i, 1) == boss::Game(boss::g_game_fo3).FolderName())
        stateNo = boss::g_game_fo3;
    else
        stateNo = boss::g_game_fonv;

    rowDialog->SetValues(stateNo, gamesList->GetItemText(i, 0), gamesList->GetItemText(i, 2), gamesList->GetItemText(i, 3), gamesList->GetItemText(i, 4), gamesList->GetItemText(i, 5), gamesList->GetItemText(i, 6));

    if (rowDialog->ShowModal() == wxID_OK) {

        if (rowDialog->GetName().empty()) {
            BOOST_LOG_TRIVIAL(error) << "Tried to blank a game's name field.";
            wxMessageBox(
                translate("Error: Name is required. Row will not be added."),
                translate("BOSS: Error"),
                wxOK | wxICON_ERROR,
                this);
            return;
        } else if (rowDialog->GetFolderName().empty()) {
            BOOST_LOG_TRIVIAL(error) << "Tried to blank a game's folder field.";
            wxMessageBox(
                translate("Error: Folder is required. Row will not be added."),
                translate("BOSS: Error"),
                wxOK | wxICON_ERROR,
                this);
            return;
        } else if (rowDialog->GetPath().empty() && rowDialog->GetRegistryKey().empty()) {
            BOOST_LOG_TRIVIAL(error) << "Tried to edit a game with no path or registry key given.";
            wxMessageBox(
                translate("Error: A path and/or registry key is required. Row will not be added."),
                translate("BOSS: Error"),
                wxOK | wxICON_ERROR,
                this);
            return;
        }

        gamesList->SetItem(i, 0, rowDialog->GetName());
        gamesList->SetItem(i, 1, FromUTF8(boss::Game(string(rowDialog->GetType().ToUTF8())).FolderName()));
        gamesList->SetItem(i, 2, rowDialog->GetFolderName());
        gamesList->SetItem(i, 3, rowDialog->GetMaster());
        gamesList->SetItem(i, 4, rowDialog->GetURL());
        gamesList->SetItem(i, 5, rowDialog->GetPath());
        gamesList->SetItem(i, 6, rowDialog->GetRegistryKey());
    }
}

void SettingsFrame::OnRemoveGame(wxCommandEvent& event) {
    BOOST_LOG_TRIVIAL(debug) << "Removing game from settings.";

    gamesList->DeleteItem(gamesList->GetFirstSelected());

    editBtn->Enable(false);
    removeBtn->Enable(false);
}

GameEditDialog::GameEditDialog(wxWindow *parent, const wxString& title) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {

    wxString Types[] = {
        FromUTF8(boss::Game(boss::g_game_tes4).FolderName()),
        FromUTF8(boss::Game(boss::g_game_tes5).FolderName()),
        FromUTF8(boss::Game(boss::g_game_fo3).FolderName()),
        FromUTF8(boss::Game(boss::g_game_fonv).FolderName())
    };

    //Initialise controls.
    _type = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, Types);

    _name = new wxTextCtrl(this, wxID_ANY);
    _folderName = new wxTextCtrl(this, wxID_ANY);
    _master = new wxTextCtrl(this, wxID_ANY);
    _url = new wxTextCtrl(this, wxID_ANY);
    _path = new wxTextCtrl(this, wxID_ANY);
    _registry = new wxTextCtrl(this, wxID_ANY);

    //Sizers stuff.
    wxSizerFlags leftItem(0);
	leftItem.Left();

	wxSizerFlags rightItem(1);
	rightItem.Right().Expand();

    wxBoxSizer * bigBox = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer * GridSizer = new wxFlexGridSizer(2, 5, 5);
    GridSizer->AddGrowableCol(1,1);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Name:")), leftItem);
	GridSizer->Add(_name, rightItem);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Type:")), leftItem);
	GridSizer->Add(_type, rightItem);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("BOSS Folder Name:")), leftItem);
	GridSizer->Add(_folderName, rightItem);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Master File:")), leftItem);
	GridSizer->Add(_master, rightItem);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Masterlist URL:")), leftItem);
	GridSizer->Add(_url, rightItem);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Install Path:")), leftItem);
	GridSizer->Add(_path, rightItem);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Install Path Registry Key:")), leftItem);
	GridSizer->Add(_registry, rightItem);

    bigBox->Add(GridSizer, 0, wxEXPAND|wxALL, 15);

    bigBox->AddSpacer(10);
    bigBox->AddStretchSpacer(1);

    //Need to add 'OK' and 'Cancel' buttons.
	wxSizer * sizer = CreateSeparatedButtonSizer(wxOK|wxCANCEL);
    if (sizer != NULL)
        bigBox->Add(sizer, 0, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT, 15);

    //Set defaults.
    _type->SetSelection(0);

    SetBackgroundColour(wxColour(255,255,255));
    SetIcon(wxIconLocation("BOSS.exe"));
	SetSizerAndFit(bigBox);
}

void GameEditDialog::SetValues(unsigned int type, const wxString& name, const wxString& folderName, const wxString& master,
                const wxString& url, const wxString& path, const wxString& registry) {
    if (type == boss::g_game_tes4)
        _type->SetSelection(0);
    else if (type == boss::g_game_tes5)
        _type->SetSelection(1);
    else if (type  == boss::g_game_fo3)
        _type->SetSelection(2);
    else
        _type->SetSelection(3);

    _name->SetValue(name);
    _folderName->SetValue(folderName);
    _master->SetValue(master);
    _url->SetValue(url);
    _path->SetValue(path);
    _registry->SetValue(registry);

    //Also disable the name and folder name text controls to prevent them being changed for games that already exist.
    _name->Enable(false);
    _folderName->Enable(false);
}

wxString GameEditDialog::GetName() const {
    return _name->GetValue();
}

wxString GameEditDialog::GetType() const {
    return _type->GetString(_type->GetSelection());
}

wxString GameEditDialog::GetFolderName() const {
    return _folderName->GetValue();
}

wxString GameEditDialog::GetMaster() const {
    return _master->GetValue();
}

wxString GameEditDialog::GetURL() const {
    return _url->GetValue();
}

wxString GameEditDialog::GetPath() const {
    return _path->GetValue();
}

wxString GameEditDialog::GetRegistryKey() const {
    return _registry->GetValue();
}

