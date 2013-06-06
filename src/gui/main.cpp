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

#include "../backend/globals.h"
#include "../backend/metadata.h"
#include "../backend/parsers.h"
#include "../backend/error.h"
#include "../backend/helpers.h"
#include "../backend/generators.h"

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

    //Detect installed games.
    try {
        _games = GetGames(_settings);
    } catch (boss::error& e) {
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: Game-specific settings could not be initialised. %1%")) % e.what()),
            translate("BOSS: Error"),
            wxOK | wxICON_ERROR,
            NULL);
        return false;
    } catch (YAML::Exception& e) {
        //LOG_ERROR("Error: %s", e.getString().c_str());
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: Games' settings parsing failed. %1%")) % e.what()),
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
        for (size_t i=0, max=_games.size(); i < max; ++i) {
            if (target == _games[i].FolderName() && _games[i].IsInstalled())
                _game = _games[i];
        }
    }
    if (_game == Game()) {
        //Set _game to the first installed game.
        for (size_t i=0, max=_games.size(); i < max; ++i) {
            if (_games[i].IsInstalled()) {
                _game = _games[i];
                break;
            }
        }
        if (_game == Game()) {
            wxMessageBox(
                translate("Error: None of the supported games were detected."),
                translate("BOSS: Error"),
                wxOK | wxICON_ERROR,
                NULL);
            return false;
        }
    }

    //Now that game is selected, initialise it.
    try {
        _game.Init();
        *find(_games.begin(), _games.end(), _game) = _game;  //Sync changes.
    } catch (boss::error& e) {
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: Game-specific settings could not be initialised. %1%")) % e.what()),
            translate("BOSS: Error"),
            wxOK | wxICON_ERROR,
            NULL);
        return false;
    }

    //Create launcher window.
    Launcher * launcher = new Launcher(wxT("BOSS"), _settings, _game, _games);

    launcher->SetIcon(wxIconLocation("BOSS.exe"));
	launcher->Show();
	SetTopWindow(launcher);

    return true;
}

Launcher::Launcher(const wxChar *title, YAML::Node& settings, Game& game, vector<boss::Game>& games) : wxFrame(NULL, wxID_ANY, title), _game(game), _settings(settings), _games(games) {

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
    for (size_t i=0,max=_games.size(); i < max; ++i) {
        wxMenuItem * item = GameMenu->AppendRadioItem(MENU_LowestDynamicGameID + i, FromUTF8(_games[i].Name()));
        if (_game == _games[i])
            item->Check();

        if (_games[i].IsInstalled())
            Bind(wxEVT_COMMAND_MENU_SELECTED, &Launcher::OnGameChange, this, MENU_LowestDynamicGameID + i);
        else
            item->Enable(false);
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

    _settings["Games"] = _games;
    
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
        if (fs::is_regular_file(it->status()) && IsPlugin(it->path().string())) {

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

    bool cyclicDependenciesExist = false;
    if (fs::exists(_game.MasterlistPath()) || fs::exists(_game.UserlistPath())) {
        out << "Merging plugin lists, evaluating conditions and and checking for install validity..." << endl;

        //Merge all global message lists.
        messages = mlist_messages;
        messages.insert(messages.end(), ulist_messages.begin(), ulist_messages.end());

        //Set language.
        unsigned int lang = boss::LANG_AUTO;
        if (_settings["Language"].as<string>() == "eng")
            lang = boss::LANG_ENG;

        //Merge plugin list, masterlist and userlist plugin data.
        map<string, bool> consistencyIssues;
        for (list<boss::Plugin>::iterator it=plugins.begin(), endIt=plugins.end(); it != endIt; ++it) {
            //Check if there is already a plugin in the 'plugins' list or not.
            list<boss::Plugin>::iterator pos = std::find(mlist_plugins.begin(), mlist_plugins.end(), *it);

            if (pos != mlist_plugins.end())
                it->Merge(*pos);

            pos = std::find(ulist_plugins.begin(), ulist_plugins.end(), *it);

            if (pos != ulist_plugins.end())
                it->Merge(*pos);

            progDia->Pulse();

            //Now that items are merged, evaluate any conditions they have.
            try {
                it->EvalAllConditions(_game, lang);
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

            //Check that the metadata is self-consistent.
            set<string> dependencies, incompatibilities, reqs;
            map<string, bool> issues;
            issues = it->CheckSelfConsistency(plugins, dependencies, incompatibilities, reqs);
            consistencyIssues.insert(issues.begin(), issues.end());

            //Also check install validity.
            issues = it->CheckInstallValidity(_game);
            list<boss::Message> pluginMessages = it->Messages();
            for (map<string,bool>::const_iterator jt=issues.begin(), endJt=issues.end(); jt != endJt; ++jt) {
                if (jt->second)
                    pluginMessages.push_back(boss::Message(boss::MESSAGE_ERROR, "\"" + jt->first + "\" is incompatible with \"" + it->Name() + "\" and is present."));
                else
                    pluginMessages.push_back(boss::Message(boss::MESSAGE_ERROR, "\"" + jt->first + "\" is required by \"" + it->Name() + "\" but is missing."));
            }
            if (!issues.empty())
                it->Messages(pluginMessages);

            progDia->Pulse();
        }

        for (map<string,bool>::const_iterator jt=consistencyIssues.begin(), endJt=consistencyIssues.end(); jt != endJt; ++jt) {
            if (jt->second) {
                messages.push_back(boss::Message(boss::MESSAGE_ERROR, "\"" + jt->first + "\" is a circular dependency. Plugins will not be sorted."));
                cyclicDependenciesExist = true;
            } else
                messages.push_back(boss::Message(boss::MESSAGE_ERROR, "\"" + jt->first + "\" is given as a dependency and an incompatibility."));
        }
            
        end = time(NULL);
        out << "Time taken to merge lists, evaluate conditions and check for install validity: " << (end - start) << " seconds." << endl;
        start = time(NULL);
    }

    progDia->Pulse();

    if (!cyclicDependenciesExist) {
        out << "Sorting plugins..." << endl;

        //std::sort doesn't compare each plugin to every other plugin, it seems to compare plugins until no movement is necessary, because it assumes that each plugin will be comparable on all tested data, which isn't the case when dealing with masters, etc. Therefore, loop through the list and move each plugin's dependencies directly before it, then apply std::sort.
        list<boss::Plugin>::iterator it=plugins.begin();
        while (it != plugins.end()) {
            list<boss::Plugin>::iterator jt=it;
            ++jt;

            out << "Sorting for: " << it->Name() << endl;

            list<boss::Plugin> moved;
            while (jt != plugins.end()) {
                if (it->MustLoadAfter(*jt)) {
                    moved.push_back(*jt);
                    jt = plugins.erase(jt);
                } else
                    ++jt;

                progDia->Pulse();
            }
            if (!moved.empty()) {
                plugins.insert(it, moved.begin(), moved.end());
                advance(it, -1*(int)moved.size());
            } else
                ++it;

            progDia->Pulse();
        }

        plugins.sort(boss::load_order_sort);

        end = time(NULL);
        out << "Time taken to sort plugins: " << (end - start) << " seconds." << endl;

        progDia->Pulse();

        for (list<boss::Plugin>::iterator it=plugins.begin(), endIt = plugins.end(); it != endIt; ++it)
            out << it->Name() << endl;
    }

    out << "Generating report..." << endl;

    //Read the details section of the previous report, if it exists.
    string oldDetails;
    if (fs::exists(_game.ReportPath().string())) {
        //Read the whole file in.
        ifstream in(_game.ReportPath().string().c_str(), ios::binary);
        in.seekg(0, std::ios::end);
        oldDetails.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&oldDetails[0], oldDetails.size());
        in.close();

        //Slim down to only the details section.
        size_t pos1 = oldDetails.find("<div id=\"plugins\"");
        pos1 = oldDetails.find("<ul>", pos1);
        size_t pos2 = oldDetails.find("</div>", pos1);
        pos2 = oldDetails.rfind("</ul>", pos2) - 3;  //Remove the 3 tabs preceding the closing tag.

        oldDetails = oldDetails.substr(pos1, pos2 - pos1);
        boost::replace_all(oldDetails, "\t\t\t\t", "\t");
        oldDetails += "</ul>\n";
    }
    

    try {
        GenerateReport(_game.ReportPath().string(),
                        messages,
                        plugins,
                        oldDetails,
                        "4030 (2020-13-13)",
                        _settings["Update Masterlist"].as<bool>());
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
        if (fs::is_regular_file(it->status()) && IsPlugin(it->path().string())) {
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
	SettingsFrame *settings = new SettingsFrame(this, translate("BOSS: Settings"), _settings, _games);
	settings->ShowModal();
}

void Launcher::OnGameChange(wxCommandEvent& event) {
    _game = _games[event.GetId() - MENU_LowestDynamicGameID];
    try {
        _game.Init();  //In case it hasn't already been done.
        *find(_games.begin(), _games.end(), _game) = _game;  //Sync changes.
    } catch (boss::error& e) {
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: Game-specific settings could not be initialised. %1%")) % e.what()),
            translate("BOSS: Error"), 
            wxOK | wxICON_ERROR,
            NULL);
    }
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
