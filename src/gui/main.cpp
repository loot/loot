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
#include "main.h"
#include "settings.h"
#include "editor.h"
#include "viewer.h"

#include "../globals.h"
#include "../metadata.h"
#include "../parsers.h"
#include "../error.h"
#include "../helpers.h"
#include "../generators.h"

#include <ostream>
#include <algorithm>
#include <iterator>
#include <ctime>
#include <clocale>

#include <libloadorder.h>
#include <src/playground.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>

#include <wx/snglinst.h>
#include <wx/aboutdlg.h>
#include <wx/progdlg.h>

IMPLEMENT_APP(BossGUI);

using namespace boss;
using namespace std;
using boost::format;

namespace fs = boost::filesystem;
namespace loc = boost::locale;

bool BossGUI::OnInit() {
    
    //Check if GUI is already running.
	wxSingleInstanceChecker *checker = new wxSingleInstanceChecker;

	if (checker->IsAnotherRunning()) {
        wxMessageBox(
			translate("Error: BOSS is already running. This instance will now quit."),
			translate("BOSS: Error"),
			wxOK | wxICON_ERROR,
			NULL);
		delete checker; // OnExit() won't be called if we return false
		checker = NULL;

		return false;
	}

    //Load settings.
    if (fs::exists(settings_path)) {
        try {
            _settings = YAML::LoadFile(settings_path.string());
        } catch (YAML::ParserException& e) {
            //LOG_ERROR("Error: %s", e.getString().c_str());
            wxMessageBox(
				FromUTF8(format(loc::translate("Error: Settings parsing failed. %1%")) % e.what()),
				translate("BOSS: Error"),
				wxOK | wxICON_ERROR,
				NULL);
            return false;
        }
    }

    //Skip logging initialisation for tester.
/*    if (gl_log_debug_output)
		g_logger.setStream(debug_log_path.string().c_str());
	g_logger.setOriginTracking(gl_debug_with_source);
	// it's ok if this number is too high.  setVerbosity will handle it
	g_logger.setVerbosity(static_cast<LogVerbosity>(LV_WARN + gl_debug_verbosity));
*/
    //Specify location of language dictionaries
	boost::locale::generator gen;
	gen.add_messages_path(fs::path("l10n").string());
	gen.add_messages_domain("messages");

    //Set the locale to get encoding and language conversions working correctly.
    if (_settings["Language"]) {
        string localeId = "";
        wxLanguage lang;
        if (_settings["Language"].as<string>() == "eng") {
            localeId = "en.UTF-8";
            lang = wxLANGUAGE_ENGLISH;
        }

        try {
            locale::global(gen(localeId));
            cout.imbue(locale());
            //Need to also set up wxWidgets locale so that its default interface text comes out in the right language.
            wxLoc = new wxLocale();
            if (!wxLoc->Init(lang, wxLOCALE_LOAD_DEFAULT))
                throw runtime_error("System GUI text could not be set.");
            wxLocale::AddCatalogLookupPathPrefix(".\\l10n");
            wxLoc->AddCatalog("wxstd");
        } catch(runtime_error &e) {
           // LOG_ERROR("could not implement translation: %s", e.what());
            wxMessageBox(
                FromUTF8(format(loc::translate("Error: could not apply translation: %1%")) % e.what()),
                translate("BOSS: Error"),
                wxOK | wxICON_ERROR,
                NULL);
        }
        locale global_loc = locale();
        locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
        boost::filesystem::path::imbue(loc);
    }

    //Get games vector.
    

    //Detect installed games.
    try {
        _detectedGames = GetGames(_settings);
        DetectGames(_detectedGames, _undetectedGames);
    } catch (boss::error& e) {
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: Game-specific settings could not be initialised. %1%")) % e.what()),
            translate("BOSS: Error"),
            wxOK | wxICON_ERROR,
            NULL);
        return false;
    }
    if (_detectedGames.empty()) {
        wxMessageBox(
            translate("Error: None of the supported games were detected."),
            translate("BOSS: Error"),
            wxOK | wxICON_ERROR,
            NULL);
        return false;
    }

    string target;
    if (_settings["Game"] && _settings["Game"].as<string>() != "auto")
        target = _settings["Game"].as<string>();
    else if (_settings["Last Game"] && _settings["Last Game"].as<string>() != "auto")
        target = _settings["Last Game"].as<string>();

    if (!target.empty()) {
        for (size_t i=0, max=_detectedGames.size(); i < max; ++i) {
            if (boost::iequals(target, _detectedGames[i].FolderName()))
                _game = _detectedGames[i];
        }
        if (_game == Game())
            _game = _detectedGames[0];
    } else
        _game = _detectedGames[0];

    //Now that game is selected, initialise it.
    _game.Init();

    //Create launcher window.
    Launcher * launcher = new Launcher(wxT("BOSS"), _settings, _game, _detectedGames, _undetectedGames);

    launcher->SetIcon(wxIconLocation("BOSS.exe"));
	launcher->Show();
	SetTopWindow(launcher);

    return true;
}

Launcher::Launcher(const wxChar *title, YAML::Node& settings, Game& game, const vector<boss::Game>& detectedGames, const vector<boss::Game>& undetectedGames) : wxFrame(NULL, wxID_ANY, title), _game(game), _settings(settings), _detectedGames(detectedGames), _undetectedGames(undetectedGames) {

    //Initialise menu items.
    wxMenuBar * MenuBar = new wxMenuBar();
    wxMenu * FileMenu = new wxMenu();
    wxMenu * EditMenu = new wxMenu();
    GameMenu = new wxMenu();
    wxMenu * HelpMenu = new wxMenu();

	//Initialise controls.
    wxButton * EditButton = new wxButton(this, OPTION_EditMetadata, translate("Edit Metadata"));
    wxButton * SortButton = new wxButton(this, OPTION_SortPlugins, translate("Sort Plugins"));
    ViewButton = new wxButton(this,OPTION_ViewLastReport, translate("View Last Report"));

    //Construct menus.
    //File Menu
	FileMenu->Append(OPTION_ViewLastReport, translate("&View Last Report"));
    FileMenu->Append(OPTION_SortPlugins, translate("&Sort Plugins"));
    FileMenu->AppendSeparator();
    FileMenu->Append(wxID_EXIT);
    MenuBar->Append(FileMenu, translate("&File"));
	//Edit Menu
	EditMenu->Append(OPTION_EditMetadata, translate("&Metadata..."));
	EditMenu->Append(MENU_ShowSettings, translate("&Settings..."));
	MenuBar->Append(EditMenu, translate("&Edit"));
	//Game menu - set up initial item states here too.
    for (size_t i=0,max=_detectedGames.size(); i < max; ++i) {
        wxMenuItem * item = GameMenu->AppendRadioItem(MENU_LowestDynamicGameID + i, FromUTF8(_detectedGames[i].Name()));
        if (_game == _detectedGames[i])
            item->Check();
        Bind(wxEVT_COMMAND_MENU_SELECTED, &Launcher::OnGameChange, this, MENU_LowestDynamicGameID + i);
    }
    for (size_t i=0,max=undetectedGames.size(); i < max; ++i) {
        GameMenu->AppendRadioItem(wxID_ANY, FromUTF8(undetectedGames[i].Name()))->Enable(false);
    }
	MenuBar->Append(GameMenu, translate("&Game"));
    //About menu
	HelpMenu->Append(wxID_HELP);
	HelpMenu->AppendSeparator();
	HelpMenu->Append(wxID_ABOUT);
    MenuBar->Append(HelpMenu, translate("&Help"));

    //Set up layout.
    wxBoxSizer *buttonBox = new wxBoxSizer(wxVERTICAL);
	buttonBox->Add(EditButton, 1, wxEXPAND|wxALIGN_CENTRE|wxALL, 10);
	buttonBox->Add(SortButton, 1, wxEXPAND|wxALIGN_CENTRE|wxLEFT|wxRIGHT, 10);
	buttonBox->Add(ViewButton, 1, wxEXPAND|wxALIGN_CENTRE|wxALL, 10);

    //Bind event handlers.
    Bind(wxEVT_COMMAND_MENU_SELECTED, &Launcher::OnQuit, this, wxID_EXIT);
    Bind(wxEVT_COMMAND_MENU_SELECTED, &Launcher::OnViewLastReport, this, OPTION_ViewLastReport);
    Bind(wxEVT_COMMAND_MENU_SELECTED, &Launcher::OnSortPlugins, this, OPTION_SortPlugins);
    Bind(wxEVT_COMMAND_MENU_SELECTED, &Launcher::OnEditMetadata, this, OPTION_EditMetadata);
    Bind(wxEVT_COMMAND_MENU_SELECTED, &Launcher::OnOpenSettings, this, MENU_ShowSettings);
    Bind(wxEVT_COMMAND_MENU_SELECTED, &Launcher::OnHelp, this, wxID_HELP);
    Bind(wxEVT_COMMAND_MENU_SELECTED, &Launcher::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Launcher::OnSortPlugins, this, OPTION_SortPlugins);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Launcher::OnEditMetadata, this, OPTION_EditMetadata);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Launcher::OnViewLastReport, this, OPTION_ViewLastReport);
    Bind(wxEVT_CLOSE_WINDOW, &Launcher::OnClose, this);

    //Set up initial state.
    SortButton->SetDefault();

    if (!fs::exists(_game.ReportPath()))
        ViewButton->Enable(false);

    //Set title bar text.
	SetTitle(FromUTF8("BOSS - " + _game.Name()));

    //Now set the layout and sizes.
    SetMenuBar(MenuBar);
	SetBackgroundColour(wxColour(255,255,255));
	SetSizerAndFit(buttonBox);
    SetSize(wxSize(250, 200));
}

//Called when the frame exits.
void Launcher::OnQuit(wxCommandEvent& event) {
    
	Close(true); // Tells the OS to quit running this process
}

void Launcher::OnClose(wxCloseEvent& event) {


    _settings["Last Game"] = _game.FolderName();
    
    //Save settings.
    YAML::Emitter yout;
    yout.SetIndent(2);
    yout << _settings;
    
    ofstream out(boss::settings_path.string().c_str());
    out << yout.c_str();
    out.close();

    Destroy();
}

void Launcher::OnSortPlugins(wxCommandEvent& event) {

    wxProgressDialog *progDia = new wxProgressDialog(translate("BOSS: Working..."),translate("BOSS working..."), 1000, this, wxPD_APP_MODAL|wxPD_AUTO_HIDE|wxPD_ELAPSED_TIME);
    
    time_t t0 = time(NULL);

    ofstream out("out.txt");

    out << "Reading plugins in Data folder..." << endl;
    time_t start, end;
    start = time(NULL);
    
    // Get a list of the plugins.
    list<boss::Plugin> plugins;
    for (fs::directory_iterator it(_game.DataPath()); it != fs::directory_iterator(); ++it) {
        if (fs::is_regular_file(it->status()) && (it->path().extension().string() == ".esp" || it->path().extension().string() == ".esm")) {

            string filename = it->path().filename().string();
			out << "Reading plugin: " << filename << endl;
			boss::Plugin plugin(_game, filename, false);
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

    if (fs::exists(_game.MasterlistPath())) {
        out << "Parsing masterlist..." << endl;

        try {
            mlist = YAML::LoadFile(_game.MasterlistPath().string());
        } catch (YAML::ParserException& e) {
            //LOG_ERROR("Error: %s", e.getString().c_str());
            wxMessageBox(
				FromUTF8(format(loc::translate("Error: Masterlist parsing failed. %1%")) % e.what()),
				translate("BOSS: Error"),
				wxOK | wxICON_ERROR,
				this);
            return;
        }
        if (mlist["globals"])
            mlist_messages = mlist["globals"].as< list<boss::Message> >();
        if (mlist["plugins"])
            mlist_plugins = mlist["plugins"].as< list<boss::Plugin> >();

        end = time(NULL);
        out << "Time taken to parse masterlist: " << (end - start) << " seconds." << endl;
        start = time(NULL);
    }

    if (fs::exists(_game.UserlistPath())) {
        out << "Parsing userlist..." << endl;

        try {
            ulist = YAML::LoadFile(_game.UserlistPath().string());
        } catch (YAML::ParserException& e) {
            //LOG_ERROR("Error: %s", e.getString().c_str());
            wxMessageBox(
				FromUTF8(format(loc::translate("Error: Userlist parsing failed. %1%")) % e.what()),
				translate("BOSS: Error"),
				wxOK | wxICON_ERROR,
				this);
            return;
        }
        if (ulist["globals"])
            ulist_messages = ulist["globals"].as< list<boss::Message> >();
        if (ulist["plugins"])
            ulist_plugins = ulist["plugins"].as< list<boss::Plugin> >();

        end = time(NULL);
        out << "Time taken to parse userlist: " << (end - start) << " seconds." << endl;
        start = time(NULL);
    }

    progDia->Pulse();

    if (fs::exists(_game.MasterlistPath()) || fs::exists(_game.UserlistPath())) {
        out << "Merging plugin lists..." << endl;

        //Merge all global message lists.
        messages = mlist_messages;
        messages.insert(messages.end(), ulist_messages.begin(), ulist_messages.end());

        //Merge plugin list, masterlist and userlist plugin data.
        for (list<boss::Plugin>::iterator it=plugins.begin(), endIt=plugins.end(); it != endIt; ++it) {
            //Check if there is already a plugin in the 'plugins' list or not.
            list<boss::Plugin>::iterator pos = std::find(mlist_plugins.begin(), mlist_plugins.end(), *it);

            if (pos != mlist_plugins.end())
                it->Merge(*pos);

            pos = std::find(ulist_plugins.begin(), ulist_plugins.end(), *it);

            if (pos != ulist_plugins.end())
                it->Merge(*pos);

            

            progDia->Pulse();
        }

        end = time(NULL);
        out << "Time taken to merge lists: " << (end - start) << " seconds." << endl;
        start = time(NULL);
    }

    progDia->Pulse();
    
    out << "Evaluating any conditions in plugin list..." << endl;

    for (list<boss::Plugin>::iterator it=plugins.begin(), endIt=plugins.end(); it != endIt; ++it) {
        try {
            it->EvalAllConditions(_game, _settings["Language"].as<string>());
        } catch (boss::error& e) {
            //LOG_ERROR("Error: %s", e.what());
            wxMessageBox(
				FromUTF8(format(loc::translate("Error: Condition evaluation failed. %1%")) % e.what()),
				translate("BOSS: Error"),
				wxOK | wxICON_ERROR,
				this);
            return;
        }

        progDia->Pulse();
    }

    end = time(NULL);
	out << "Time taken to evaluate plugin list: " << (end - start) << " seconds." << endl;
    start = time(NULL);

    progDia->Pulse();

    out << "Checking install validity..." << endl;

    for (list<boss::Plugin>::iterator it=plugins.begin(), endIt = plugins.end(); it != endIt; ++it) {
        map<string, bool> issues = it->CheckInstallValidity(_game);
        list<boss::Message> messages = it->Messages();
        for (map<string,bool>::const_iterator jt=issues.begin(), endJt=issues.end(); jt != endJt; ++jt) {
            if (jt->second)
                messages.push_back(boss::Message("error", "\"" + jt->first + "\" is incompatible with \"" + it->Name() + "\" and is present."));
            else
                messages.push_back(boss::Message("error", "\"" + jt->first + "\" is required by \"" + it->Name() + "\" but is missing."));
        }
        if (!issues.empty())
            it->Messages(messages);

        progDia->Pulse();
    }


    end = time(NULL);
	out << "Time taken to check install validity: " << (end - start) << " seconds." << endl;
    start = time(NULL);

    progDia->Pulse();

    out << "Sorting plugins..." << endl;

    plugins.sort();

    end = time(NULL);
	out << "Time taken to sort plugins: " << (end - start) << " seconds." << endl;

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

            progDia->Pulse();
        }
        

        progDia->Pulse();
	}

    progDia->Pulse();
    
    end = time(NULL);
	out << "Time taken to print plugins' details: " << (end - start) << " seconds." << endl;
    start = time(NULL);
    
    for (list<boss::Plugin>::iterator it=plugins.begin(), endIt = plugins.end(); it != endIt; ++it)
        out << it->Name() << endl;

    out << "Writing results file..." << endl;

    try {
        GenerateReport(_game.ReportPath().string(),
                        messages,
                        plugins,
                        "4030 (2020-13-13)",
                        _settings["Update Masterlist"].as<bool>(),
                        true);
    } catch (boss::error& e) {
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: %1%")) % e.what()),
            translate("BOSS: Error"),
            wxOK | wxICON_ERROR,
            this);
        return;
    }

    progDia->Pulse();
    
	out << "Tester finished. Total time taken: " << time(NULL) - t0 << endl;
    out.close();

    //Now a results report definitely exists.
    ViewButton->Enable(true);

    progDia->Destroy();

    if (_settings["View Report Externally"] && _settings["View Report Externally"].as<bool>()) {
        wxLaunchDefaultApplication(_game.ReportPath().string());
    } else {
        //Create viewer window.
        Viewer *viewer = new Viewer(this, translate("BOSS: Report Viewer"), _game.ReportPath().string());
        viewer->Show();
    }
}

void Launcher::OnEditMetadata(wxCommandEvent& event) {

    //Should probably check for masterlist updates before opening metadata editor.
    vector<boss::Plugin> installed, mlist_plugins, ulist_plugins;

    wxProgressDialog *progDia = new wxProgressDialog(translate("BOSS: Working..."),translate("BOSS working..."), 1000, this, wxPD_APP_MODAL|wxPD_AUTO_HIDE|wxPD_ELAPSED_TIME);

    //Scan for installed plugins.
    for (fs::directory_iterator it(_game.DataPath()); it != fs::directory_iterator(); ++it) {
        if (fs::is_regular_file(it->status()) && (it->path().extension().string() == ".esp" || it->path().extension().string() == ".esm")) {
			boss::Plugin plugin(_game, it->path().filename().string(), true);
            installed.push_back(plugin);
            
            progDia->Pulse();
        }
    }

    //Parse masterlist.
    if (fs::exists(_game.MasterlistPath())) {
        YAML::Node mlist;
        try {
            mlist = YAML::LoadFile(_game.MasterlistPath().string());
        } catch (YAML::ParserException& e) {
            //LOG_ERROR("Error: %s", e.getString().c_str());
            wxMessageBox(
                FromUTF8(format(loc::translate("Error: Masterlist parsing failed. %1%")) % e.what()),
                translate("BOSS: Error"),
                wxOK | wxICON_ERROR,
                this);
        }
        if (mlist["plugins"])
            mlist_plugins = mlist["plugins"].as< vector<boss::Plugin> >();
    }
    
    progDia->Pulse();

    //Parse userlist.
    if (fs::exists(_game.UserlistPath())) {
        YAML::Node ulist;
        try {
            ulist = YAML::LoadFile(_game.UserlistPath().string());
        } catch (YAML::ParserException& e) {
            //LOG_ERROR("Error: %s", e.getString().c_str());
            wxMessageBox(
				FromUTF8(format(loc::translate("Error: Userlist parsing failed. %1%")) % e.what()),
				translate("BOSS: Error"),
				wxOK | wxICON_ERROR,
				this);
        }
        if (ulist["plugins"])
            ulist_plugins = ulist["plugins"].as< vector<boss::Plugin> >();
    }
    
    progDia->Pulse();

    //Merge the masterlist down into the installed mods list.
    for (vector<boss::Plugin>::const_iterator it=mlist_plugins.begin(), endit=mlist_plugins.end(); it != endit; ++it) {
        vector<boss::Plugin>::iterator pos = find(installed.begin(), installed.end(), *it);

        if (pos != installed.end())
            pos->Merge(*it);
    }

    
    progDia->Pulse();

    //Add empty entries for any userlist entries that aren't installed.
    for (vector<boss::Plugin>::const_iterator it=ulist_plugins.begin(), endit=ulist_plugins.end(); it != endit; ++it) {
        if (find(installed.begin(), installed.end(), *it) == installed.end())
            installed.push_back(boss::Plugin(it->Name()));
    }
    
    progDia->Pulse();

    //Sort into alphabetical order.
    std::sort(installed.begin(), installed.end(), AlphaSortPlugins);
    
    progDia->Pulse();

    //Create editor window.
    Editor *editor = new Editor(this, translate("BOSS: Metadata Editor"), _game.UserlistPath().string().c_str(), installed, ulist_plugins);
    
    progDia->Destroy();
    
	editor->Show();
}

void Launcher::OnViewLastReport(wxCommandEvent& event) {
    if (_settings["View Report Externally"] && _settings["View Report Externally"].as<bool>()) {
        wxLaunchDefaultApplication(_game.ReportPath().string());
    } else {
        //Create viewer window.
        Viewer *viewer = new Viewer(this, translate("BOSS: Report Viewer"), _game.ReportPath().string());
        viewer->Show();
    }
}

void Launcher::OnOpenSettings(wxCommandEvent& event) {
    vector<boss::Game> games = _detectedGames;
    games.insert(games.end(), _undetectedGames.begin(), _undetectedGames.end());
	SettingsFrame *settings = new SettingsFrame(this, translate("BOSS: Settings"), _settings, games);
	settings->ShowModal();
}

void Launcher::OnGameChange(wxCommandEvent& event) {
    _game = _detectedGames[event.GetId() - MENU_LowestDynamicGameID];
    _game.Init();  //In case it hasn't already been done.
	SetTitle(FromUTF8("BOSS - " + _game.Name()));
}

void Launcher::OnHelp(wxCommandEvent& event) {
    //Look for file.
    if (fs::exists(readme_path.string())) {
        wxLaunchDefaultApplication(readme_path.string());
    } else  //No ReadMe exists, show a pop-up message saying so.
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: \"%1%\" cannot be found.")) % readme_path.string()),
            translate("BOSS: Error"),
            wxOK | wxICON_ERROR,
            this);
}

void Launcher::OnAbout(wxCommandEvent& event) {
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName("BOSS");
    aboutInfo.SetVersion(IntToString(VERSION_MAJOR)+"."+IntToString(VERSION_MINOR)+"."+IntToString(VERSION_PATCH));
    aboutInfo.SetDescription(translate(
    "A utility that optimises TES IV: Oblivion, Nehrim - At Fate's Edge,\nTES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders."));
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

bool Launcher::AlphaSortPlugins(const boss::Plugin& lhs, const boss::Plugin& rhs) {
    return boost::to_lower_copy(lhs.Name()) < boost::to_lower_copy(rhs.Name());
}
