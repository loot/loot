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

#include "../globals.h"
#include "../metadata.h"
#include "../parsers.h"
#include "main.h"
#include "settings.h"

#include <ostream>
#include <algorithm>
#include <iterator>
#include <ctime>

#include <src/commonSupport.h>
#include <src/fileFormat.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>

#include <wx/snglinst.h>
#include <wx/aboutdlg.h>
#include <wx/progdlg.h>

BEGIN_EVENT_TABLE ( Launcher, wxFrame )
	EVT_MENU ( MENU_Quit, Launcher::OnQuit )
	EVT_MENU ( OPTION_EditMetadata, Launcher::OnEditMetadata )
	EVT_MENU ( OPTION_ViewLastReport, Launcher::OnViewLastReport )
	EVT_MENU ( OPTION_SortPlugins, Launcher::OnSortPlugins )
	EVT_MENU ( MENU_OpenMainReadMe, Launcher::OnOpenFile )
	EVT_MENU ( MENU_OpenSyntaxReadMe, Launcher::OnOpenFile )
	EVT_MENU ( MENU_OpenAPIReadMe, Launcher::OnOpenFile )
	EVT_MENU ( MENU_OpenVersionHistory, Launcher::OnOpenFile )
	EVT_MENU ( MENU_OpenLicenses, Launcher::OnOpenFile )
	EVT_MENU ( MENU_ShowAbout, Launcher::OnAbout )
	EVT_MENU ( MENU_ShowSettings, Launcher::OnOpenSettings )
	EVT_MENU ( MENU_Oblivion, Launcher::OnGameChange )
	EVT_MENU ( MENU_Nehrim, Launcher::OnGameChange )
	EVT_MENU ( MENU_Skyrim, Launcher::OnGameChange )
	EVT_MENU ( MENU_Fallout3, Launcher::OnGameChange )
	EVT_MENU ( MENU_FalloutNewVegas, Launcher::OnGameChange )
	EVT_BUTTON ( OPTION_SortPlugins, Launcher::OnSortPlugins )
	EVT_BUTTON ( OPTION_EditMetadata, Launcher::OnEditMetadata )
	EVT_BUTTON ( OPTION_ViewLastReport, Launcher::OnViewLastReport )
END_EVENT_TABLE()

IMPLEMENT_APP(BossGUI);

using namespace boss;
using namespace std;

namespace fs = boost::filesystem;
namespace loc = boost::locale;

// Temporary hacks.
const fs::path readme_path = fs::path("Docs") / "BOSS Readme.html";
const fs::path syntax_doc_path = fs::path("Docs") / "BOSS Metadata File Syntax.html";
const fs::path api_doc_path = fs::path("Docs") / "BOSS API Readme.html";
const fs::path version_history_path = fs::path("Docs") / "BOSS Version History.html";
const fs::path licenses_path = fs::path("Docs") / "Licenses.txt";

bool BossGUI::OnInit() {
    //Check if GUI is already running.
	wxSingleInstanceChecker *checker = new wxSingleInstanceChecker;

	if (checker->IsAnotherRunning()) {
        //Should print error message here.
		delete checker; // OnExit() won't be called if we return false
		checker = NULL;

		return false;
	}

    //Load settings.
    YAML::Node settings;
    if (fs::exists(settings_path)) {
        settings = YAML::LoadFile(settings_path);
    }

    //Apply logging and language settings (skipping for tester though).

    //Also skipping game detection.
    Game game(GAME_TES5);
    vector<unsigned int> detected;
    detected.push_back(GAME_TES5);

    //Create launcher window.
    Launcher * launcher = new Launcher(wxT("BOSS"), settings, game, detected);

    launcher->SetIcon(wxIconLocation("BOSS.exe"));
	launcher->Show();
	SetTopWindow(launcher);

    return true;
}

Launcher::Launcher(const wxChar *title, const YAML::Node& settings, const Game& game, const vector<unsigned int>& detectedGames) : wxFrame(NULL, wxID_ANY, title), _game(game), _detectedGames(detectedGames), _settings(settings) {

    //Set up menu bar first.
    MenuBar = new wxMenuBar();
    // File Menu
    FileMenu = new wxMenu();
	FileMenu->Append(OPTION_ViewLastReport, translate("&View Last Report"), translate("Opens your last report."));
    FileMenu->Append(OPTION_SortPlugins, translate("&Sort Plugins"), translate("Sorts your installed plugins."));
    FileMenu->AppendSeparator();
    FileMenu->Append(MENU_Quit, translate("&Quit"), translate("Quit BOSS."));
    MenuBar->Append(FileMenu, translate("&File"));
	//Edit Menu
	EditMenu = new wxMenu();
	EditMenu->Append(OPTION_EditMetadata, translate("&Metadata..."), translate("Opens a window where you can edit plugin metadata."));
	EditMenu->Append(MENU_ShowSettings, translate("&Settings..."), translate("Opens the Settings window."));
	MenuBar->Append(EditMenu, translate("&Edit"));
	//Game menu
	GameMenu = new wxMenu();
	GameMenu->AppendRadioItem(MENU_Oblivion, wxT("&Oblivion"), translate("Switch to running BOSS for Oblivion."));
	GameMenu->AppendRadioItem(MENU_Nehrim, wxT("&Nehrim"), translate("Switch to running BOSS for Nehrim."));
	GameMenu->AppendRadioItem(MENU_Skyrim, wxT("&Skyrim"), translate("Switch to running BOSS for Skyrim."));
	GameMenu->AppendRadioItem(MENU_Fallout3, wxT("&Fallout 3"), translate("Switch to running BOSS for Fallout 3."));
	GameMenu->AppendRadioItem(MENU_FalloutNewVegas, wxT("&Fallout: New Vegas"), translate("Switch to running BOSS for Fallout: New Vegas."));
	MenuBar->Append(GameMenu, translate("&Active Game"));
    // About menu
    HelpMenu = new wxMenu();
	HelpMenu->Append(MENU_OpenMainReadMe, translate("Open &Main Readme"), translate("Opens the main BOSS readme in your default web browser."));
	HelpMenu->Append(MENU_OpenSyntaxReadMe, translate("Open &Metadata File Syntax Doc"), translate("Opens the BOSS metadata file syntax documentation in your default web browser."));
	HelpMenu->Append(MENU_OpenAPIReadMe, translate("&Open API Readme"), translate("Opens the BOSS API readme in your default web browser."));
	HelpMenu->Append(MENU_OpenVersionHistory, translate("Open &Version History"), translate("Opens the BOSS version history in your default web browser."));
	HelpMenu->Append(MENU_OpenLicenses, translate("View &Copyright Licenses"), translate("View the GNU General Public License v3.0 and GNU Free Documentation License v1.3."));
	HelpMenu->AppendSeparator();
	HelpMenu->Append(MENU_ShowAbout, translate("&About BOSS..."), translate("Shows information about BOSS."));
    MenuBar->Append(HelpMenu, translate("&Help"));
    SetMenuBar(MenuBar);

	//Set up stuff in the frame.
	SetBackgroundColour(wxColour(255,255,255));

    //Add the three buttons.
    wxBoxSizer *buttonBox = new wxBoxSizer(wxVERTICAL);
	buttonBox->Add(EditButton = new wxButton(this,OPTION_EditMetadata, translate("Edit Metadata")), 1, wxEXPAND|wxALIGN_CENTRE|wxALL, 10);
	buttonBox->Add(SortButton = new wxButton(this,OPTION_SortPlugins, translate("Sort Plugins")), 1, wxEXPAND|wxALIGN_CENTRE|wxLEFT|wxRIGHT, 10);
	buttonBox->Add(ViewButton = new wxButton(this,OPTION_ViewLastReport, translate("View Last Report")), 1, wxEXPAND|wxALIGN_CENTRE|wxALL, 10);

    SortButton->SetDefault();

	if (_game.Id() == GAME_TES4)
		GameMenu->FindItem(MENU_Oblivion)->Check();
	else if (_game.Id() == GAME_NEHRIM)
		GameMenu->FindItem(MENU_Nehrim)->Check();
	else if (_game.Id() == GAME_TES5)
		GameMenu->FindItem(MENU_Skyrim)->Check();
	else if (_game.Id() == GAME_FO3)
		GameMenu->FindItem(MENU_Fallout3)->Check();
	else if (_game.Id() == GAME_FONV)
		GameMenu->FindItem(MENU_FalloutNewVegas)->Check();
        
	DisableUndetectedGames();

    //Now set the layout and sizes.
	SetSizerAndFit(buttonBox);
    SetSize(wxSize(250, 200));
}

//Called when the frame exits.
void Launcher::OnQuit(wxCommandEvent& event) {
	Close(true); // Tells the OS to quit running this process
}

void Launcher::OnSortPlugins(wxCommandEvent& event) {

    wxProgressDialog *progDia = new wxProgressDialog(translate("BOSS: Working..."),translate("BOSS working..."), 1000, this, wxPD_APP_MODAL|wxPD_AUTO_HIDE|wxPD_ELAPSED_TIME);
    
    time_t t0 = time(NULL);

    /* Stuff still missing from a normal execution of BOSS:

      - Reading of settings file.
      - Detecting game to run for.
      - Error handling.
      - Masterlist updating.
      - Checks for metadata self-consistency.
      - Checks for cyclic dependencies and incompatibilities.
      - Setting of load order.

      Need to include libespm setup into game setup, once the former is made to be not global.
    */

    ofstream out("out.txt");

    out << "Setting up libespm and BOSS..." << endl;

    // Set up libesm.
	common::options::setGame(libespm_game);
	ifstream input(libespm_options_path);
	common::readOptions(input);
	input.close();
	
	//Set up BOSS vars.
	boss::Game game(boss::GAME_TES5);

    out << "Reading plugins in Data folder..." << endl;
    time_t start, end;
    start = time(NULL);
    
    // Get a list of the plugins.
    list<boss::Plugin> plugins;
    for (fs::directory_iterator it(game.DataPath()); it != fs::directory_iterator(); ++it) {
        if (fs::is_regular_file(it->status()) && (it->path().extension().string() == ".esp" || it->path().extension().string() == ".esm")) {

            string filename = it->path().filename().string();
			if (filename == "Skyrim.esm"
             || filename == "Tamriel Compendium.esp"
             || filename == "Tamriel Compendium - Skill Books.esp")
				continue;  // Libespm crashes with these plugins.
			
			out << "Reading plugin: " << filename << endl;
			boss::Plugin plugin(game, filename);
            plugins.push_back(plugin);

            progDia->Pulse();
        }
    }

    end = time(NULL);
	out << "Time taken to read plugins: " << (end - start) << " seconds." << endl;
    start = time(NULL);

    YAML::Node mlist, ulist;
    list<boss::Message> messages, mlist_messages, ulist_messages;
    list<boss::Plugin> mlist_plugins, ulist_plugins;

    if (fs::exists(masterlist_path)) {
        out << "Parsing masterlist..." << endl;

        mlist = YAML::LoadFile(masterlist_path);
        if (mlist["globals"])
            mlist_messages = mlist["globals"].as< list<boss::Message> >();
        if (mlist["plugins"])
            mlist_plugins = mlist["plugins"].as< list<boss::Plugin> >();

        end = time(NULL);
        out << "Time taken to parse masterlist: " << (end - start) << " seconds." << endl;
        start = time(NULL);
    }

    if (fs::exists(userlist_path)) {
        out << "Parsing userlist..." << endl;

        ulist = YAML::LoadFile(userlist_path);
        if (ulist["globals"])
            ulist_messages = ulist["globals"].as< list<boss::Message> >();
        if (ulist["plugins"])
            ulist_plugins = ulist["plugins"].as< list<boss::Plugin> >();

        end = time(NULL);
        out << "Time taken to parse userlist: " << (end - start) << " seconds." << endl;
        start = time(NULL);
    }

    progDia->Pulse();

    if (fs::exists(masterlist_path) || fs::exists(userlist_path)) {
        out << "Merging plugin lists..." << endl;

        //Merge all global message lists.
        messages = mlist_messages;
        messages.insert(messages.end(), ulist_messages.begin(), ulist_messages.end());

        //Merge plugin list, masterlist and userlist plugin data.
        for (list<boss::Plugin>::iterator it=plugins.begin(), endIt=plugins.end(); it != endIt; ++it) {
            //Check if there is already a plugin in the 'plugins' list or not.
            list<boss::Plugin>::iterator pos = std::find(mlist_plugins.begin(), mlist_plugins.end(), *it);

            if (pos != mlist_plugins.end()) {
                //Need to merge plugins.
                it->Merge(*pos);
            }

            pos = std::find(ulist_plugins.begin(), ulist_plugins.end(), *it);

            if (pos != ulist_plugins.end()) {
                //Need to merge plugins.
                it->Merge(*pos);
            }
        }

        end = time(NULL);
        out << "Time taken to merge lists: " << (end - start) << " seconds." << endl;
        start = time(NULL);
    }

    progDia->Pulse();
    
    out << "Evaluating any conditions in plugin list..." << endl;

    for (list<boss::Plugin>::iterator it=plugins.begin(), endIt=plugins.end(); it != endIt; ++it) {
        try {
        it->EvalAllConditions(game);
        } catch (boss::error& e) {
            out << e.what() << endl;
        }
    }

    end = time(NULL);
	out << "Time taken to evaluate plugin list: " << (end - start) << " seconds." << endl;
    start = time(NULL);

    progDia->Pulse();

    out << "Checking install validity..." << endl;

    for (list<boss::Plugin>::iterator it=plugins.begin(), endIt = plugins.end(); it != endIt; ++it) {
        map<string, bool> issues = it->CheckInstallValidity(game);
        for (map<string,bool>::const_iterator jt=issues.begin(), endJt=issues.end(); jt != endJt; ++jt) {
            if (jt->second)
                out << "Error: Invalid install detected! \"" << jt->first << "\" is incompatible with \"" << it->Name() << "\" and is present." << endl;
            else
                out << "Error: Invalid install detected! \"" << jt->first << "\" is required by \"" << it->Name() << "\" but is missing." << endl;
        }
    }

    

    progDia->Pulse();

    out << "Printing plugin details..." << endl;    

    for (list<boss::Plugin>::iterator it=plugins.begin(), endIt = plugins.end(); it != endIt; ++it) {
		out << it->Name() << endl
             << '\t' << "Number of records: " << it->FormIDs().size() << endl
			 << '\t' << "Is Master: " << it->IsMaster() << endl
			 << '\t' << "Masters:" << endl;
			 
		vector<std::string> masters = it->Masters();
		for(int i = 0; i < masters.size(); ++i)
			out << '\t' << '\t' << i << ": " << masters[i] << endl;

        out << '\t' << "Conflicts with:" << endl;
        for (list<boss::Plugin>::iterator jt=plugins.begin(), endJt = plugins.end(); jt != endJt; ++jt) {
            if (*jt != *it && !jt->MustLoadAfter(*it)) {
                size_t overlap = jt->OverlapFormIDs(*it).size();
                if (overlap > 0)
                    out << '\t' << '\t' << jt->Name() << " (" << overlap << " records)" << endl;
            }
        }
	}

    progDia->Pulse();
    
    end = time(NULL);
	out << "Time taken to print plugins' details: " << (end - start) << " seconds." << endl;
    start = time(NULL);

    out << "Sorting plugins..." << endl;

    plugins.sort();

    end = time(NULL);
	out << "Time taken to sort plugins: " << (end - start) << " seconds." << endl;

    progDia->Pulse();
    
    for (list<boss::Plugin>::iterator it=plugins.begin(), endIt = plugins.end(); it != endIt; ++it)
        out << it->Name() << endl;

    out << "Writing results file..." << endl;

    YAML::Emitter yout;
    yout.SetIndent(2);
    yout << YAML::BeginMap
         << YAML::Key << "globals" << YAML::Value << messages
         << YAML::Key << "plugins" << YAML::Value << plugins
         << YAML::EndMap;

    ofstream results_out(results_path);
    results_out << yout.c_str();
    results_out.close();

    progDia->Pulse();
    
	out << "Tester finished. Total time taken: " << time(NULL) - t0 << endl;
    out.close();

    progDia->Destroy();
}

void Launcher::OnEditMetadata(wxCommandEvent& event) {

}

void Launcher::OnViewLastReport(wxCommandEvent& event) {

}

void Launcher::OnOpenSettings(wxCommandEvent& event) {
    //Tell the user that stuff is happenining.
	SettingsFrame *settings = new SettingsFrame(translate("BOSS: Settings"), this, _settings);
	settings->SetIcon(wxIconLocation("BOSS.exe"));
	settings->Show();
}

void Launcher::OnGameChange(wxCommandEvent& event) {
    try {
        switch (event.GetId()) {
        case MENU_Oblivion:
            _game = Game(GAME_TES4);
            _settings["Last Game"] = "oblivion";
            break;
        case MENU_Nehrim:
            _game = Game(GAME_NEHRIM);
            _settings["Last Game"] = "nehrim";
            break;
        case MENU_Skyrim:
            _game = Game(GAME_TES5);
            _settings["Last Game"] = "skyrim";
            break;
        case MENU_Fallout3:
            _game = Game(GAME_FO3);
            _settings["Last Game"] = "fallout3";
            break;
        case MENU_FalloutNewVegas:
            _game = Game(GAME_FONV);
            _settings["Last Game"] = "falloutnv";
            break;
        }
	} catch (error& e) {
		wxMessageBox(e.what(), translate("BOSS: Error"), wxOK | wxICON_ERROR, this);
	}
	_game.CreateBOSSGameFolder();
	SetTitle(wxT("BOSS - " + _game.Name()));  //Don't need to convert name, known to be only ASCII chars.
}

void Launcher::OnOpenFile(wxCommandEvent& event) {
    string file;
    if (event.GetId() == MENU_OpenMainReadMe)
        file = readme_path.string();
    else if (event.GetId() == MENU_OpenSyntaxReadMe)
        file = syntax_doc_path.string();
    else if (event.GetId() == MENU_OpenAPIReadMe)
        file = api_doc_path.string();
    else if (event.GetId() == MENU_OpenVersionHistory)
        file = version_history_path.string();
    else if (event.GetId() == MENU_OpenLicenses)
        file = licenses_path.string();
    //Look for file.
    if (fs::exists(file)) {
        wxLaunchDefaultApplication(file);
    } else  //No ReadMe exists, show a pop-up message saying so.
        wxMessageBox(
            FromUTF8(boost::format(loc::translate("Error: \"%1%\" cannot be found.")) % file),
            translate("BOSS: Error"),
            wxOK | wxICON_ERROR,
            this);
}

void Launcher::OnAbout(wxCommandEvent& event) {
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName("BOSS");
    aboutInfo.SetVersion(IntToString(VERSION_MAJOR)+"."+IntToString(VERSION_MINOR)+"."+IntToString(VERSION_PATCH));
    aboutInfo.SetDescription(translate("A utility that optimises \nTES IV: Oblivion, Nehrim - At Fate's Edge, TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders."));
    aboutInfo.SetCopyright("Copyright (C) 2009-2013 BOSS Development Team.");
    aboutInfo.SetWebSite("http://code.google.com/p/better-oblivion-sorting-software/");
	aboutInfo.SetLicence("This program is free software: you can redistribute it and/or modify\n"
    "it under the terms of the GNU General Public License as published by\n"
    "the Free Software Foundation, either version 3 of the License, or\n"
    "(at your option) any later version.\n"
	"\n"
    "This program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n"
	"\n"
    "You should have received a copy of the GNU General Public License\n"
    "along with this program.  If not, see <http://www.gnu.org/licenses/>.");
	aboutInfo.SetIcon(wxIconLocation("BOSS.exe"));
    wxAboutBox(aboutInfo);
}

void Launcher::DisableUndetectedGames() {
	GameMenu->FindItem(MENU_Oblivion)->Enable(false);
	GameMenu->FindItem(MENU_Nehrim)->Enable(false);
	GameMenu->FindItem(MENU_Skyrim)->Enable(false);
	GameMenu->FindItem(MENU_Fallout3)->Enable(false);
	GameMenu->FindItem(MENU_FalloutNewVegas)->Enable(false);
	for (size_t i=0; i < _detectedGames.size(); i++) {
		if (_detectedGames[i] == GAME_TES4)
			GameMenu->FindItem(MENU_Oblivion)->Enable();
		else if (_detectedGames[i] == GAME_NEHRIM)
			GameMenu->FindItem(MENU_Nehrim)->Enable();
		else if (_detectedGames[i] == GAME_TES5)
			GameMenu->FindItem(MENU_Skyrim)->Enable();
		else if (_detectedGames[i] == GAME_FO3)
			GameMenu->FindItem(MENU_Fallout3)->Enable();
		else if (_detectedGames[i] == GAME_FONV)
			GameMenu->FindItem(MENU_FalloutNewVegas)->Enable();
	}

	SetTitle(wxT("BOSS - " + _game.Name()));  //Don't need to convert name, known to be only ASCII chars.
}
