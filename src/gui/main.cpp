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
#include "../backend/network.h"

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
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>

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
    if (!fs::exists(g_path_settings)) {
        try {
            if (!fs::exists(g_path_settings.parent_path()))
                fs::create_directory(g_path_settings.parent_path());
        } catch (fs::filesystem_error& e) {
            wxMessageBox(
				translate("Error: Could not create local app data BOSS folder."),
				translate("BOSS: Error"),
				wxOK | wxICON_ERROR,
				NULL);
            return false;
        }
        GenerateDefaultSettingsFile(g_path_settings.string());
    }
    if (fs::exists(g_path_settings)) {
        try {
            _settings = YAML::LoadFile(g_path_settings.string());
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

    //Set up logging.
    unsigned int verbosity = _settings["Debug Verbosity"].as<unsigned int>();
    if (verbosity > 0) {
        boost::log::add_file_log(
            boost::log::keywords::file_name = g_path_log.string().c_str(),
            boost::log::keywords::format = (
                boost::log::expressions::stream
                    << "[" << boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%H:%M:%S") << "]"
                    << " [" << boost::log::trivial::severity << "]: "
                    << boost::log::expressions::smessage
                )
            );
        if (verbosity == 1)
            boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::warning);  //Log all warnings, errors and fatals.
        else if (verbosity == 2)
            boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);  //Log debugs, infos, warnings, errors and fatals.
        else {
            boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);  //Log everything.
        }
        boost::log::add_common_attributes();
    } else
        boost::log::core::get()->set_logging_enabled(false);  //Disable all logging.

    //Specify location of language dictionaries
	boost::locale::generator gen;
	gen.add_messages_path(g_path_l10n.string());
	gen.add_messages_domain("messages");

    //Set the locale to get encoding and language conversions working correctly.
    BOOST_LOG_TRIVIAL(debug) << "Initialising language settings.";
    if (_settings["Language"]) {
        string localeId = "";
        wxLanguage lang;
        if (_settings["Language"].as<string>() == "eng") {
            localeId = "en.UTF-8";
            lang = wxLANGUAGE_ENGLISH;
            BOOST_LOG_TRIVIAL(debug) << "Setting language to English.";
        } else if (_settings["Language"].as<string>() == "spa") {
            localeId = "es.UTF-8";
            lang = wxLANGUAGE_SPANISH;
            BOOST_LOG_TRIVIAL(debug) << "Setting language to Spanish.";
        } else if (_settings["Language"].as<string>() == "spa") {
            localeId = "ru.UTF-8";
            lang = wxLANGUAGE_RUSSIAN;
            BOOST_LOG_TRIVIAL(debug) << "Setting language to Russian.";
        }

        try {
            locale::global(gen(localeId));
            cout.imbue(locale());
            //Need to also set up wxWidgets locale so that its default interface text comes out in the right language.
            wxLoc = new wxLocale();
            if (!wxLoc->Init(lang, wxLOCALE_LOAD_DEFAULT))
                throw runtime_error("System GUI text could not be set.");
            wxLocale::AddCatalogLookupPathPrefix(g_path_l10n.string().c_str());
            wxLoc->AddCatalog("wxstd");
        } catch(runtime_error &e) {
            BOOST_LOG_TRIVIAL(error) << "Could not implement translation: " << e.what();
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
    BOOST_LOG_TRIVIAL(debug) << "Detecting installed games.";
    try {
        _games = GetGames(_settings);
    } catch (boss::error& e) {
        BOOST_LOG_TRIVIAL(error) << "Game-specific settings could not be initialised. " << e.what();
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: Game-specific settings could not be initialised. %1%")) % e.what()),
            translate("BOSS: Error"),
            wxOK | wxICON_ERROR,
            NULL);
        return false;
    } catch (YAML::Exception& e) {
        BOOST_LOG_TRIVIAL(error) << "Games' settings parsing failed. " << e.what();
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: Games' settings parsing failed. %1%")) % e.what()),
            translate("BOSS: Error"),
            wxOK | wxICON_ERROR,
            NULL);
        return false;
    }

    BOOST_LOG_TRIVIAL(debug) << "Selecting game.";
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
            BOOST_LOG_TRIVIAL(error) << "None of the supported games were detected.";
            wxMessageBox(
                translate("Error: None of the supported games were detected."),
                translate("BOSS: Error"),
                wxOK | wxICON_ERROR,
                NULL);
            return false;
        }
    }

    //Now that game is selected, initialise it.
    BOOST_LOG_TRIVIAL(debug) << "Initialising game-specific settings.";
    try {
        _game.Init();
        *find(_games.begin(), _games.end(), _game) = _game;  //Sync changes.
    } catch (boss::error& e) {
        BOOST_LOG_TRIVIAL(error) << "Game-specific settings could not be initialised. " << e.what();
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
    BOOST_LOG_TRIVIAL(debug) << "Quiting BOSS.";

    _settings["Last Game"] = _game.FolderName();

    _settings["Games"] = _games;
    
    //Save settings.
    BOOST_LOG_TRIVIAL(debug) << "Saving BOSS settings.";
    YAML::Emitter yout;
    yout.SetIndent(2);
    yout << _settings;
    
    ofstream out(boss::g_path_settings.string().c_str());
    out << yout.c_str();
    out.close();

    Destroy();
}

void Launcher::OnSortPlugins(wxCommandEvent& event) {

    BOOST_LOG_TRIVIAL(debug) << "Beginning sorting process.";

    YAML::Node mlist, ulist;
    list<boss::Message> messages, mlist_messages, ulist_messages;
    list<boss::Plugin> mlist_plugins, ulist_plugins;

    wxProgressDialog *progDia = new wxProgressDialog(translate("BOSS: Working..."),translate("BOSS working..."), 1000, this, wxPD_APP_MODAL|wxPD_AUTO_HIDE|wxPD_ELAPSED_TIME);

    BOOST_LOG_TRIVIAL(trace) << "Updating masterlist";

    vector<string> parsingErrors;
    string revision;
    try {
        revision = UpdateMasterlist(_game, parsingErrors);
    } catch (boss::error& e) {
        BOOST_LOG_TRIVIAL(error) << "Masterlist update failed. Details: " << e.what();
        messages.push_back(boss::Message(boss::g_message_error, (format(loc::translate("Masterlist update failed. Details: %1%")) % e.what()).str()));
    }
    for (vector<string>::const_iterator it=parsingErrors.begin(), endit=parsingErrors.end(); it != endit; ++it) {
        messages.push_back(boss::Message(boss::g_message_error, *it));
    }

    BOOST_LOG_TRIVIAL(trace) << "Reading plugins in Data folder...";
    
    // Get a list of the plugins.
    list<boss::Plugin> plugins;
    for (fs::directory_iterator it(_game.DataPath()); it != fs::directory_iterator(); ++it) {
        if (fs::is_regular_file(it->status()) && IsPlugin(it->path().string())) {

            string filename = it->path().filename().string();
			BOOST_LOG_TRIVIAL(trace) << "Reading plugin: " << filename;
			boss::Plugin plugin(_game, filename, false);
            plugins.push_back(plugin);

            progDia->Pulse();
        }
    }

    if (fs::exists(_game.MasterlistPath())) {
        BOOST_LOG_TRIVIAL(trace) << "Parsing masterlist...";

        try {
            mlist = YAML::LoadFile(_game.MasterlistPath().string());
        } catch (YAML::ParserException& e) {
            BOOST_LOG_TRIVIAL(error) << "Masterlist parsing failed. Details: " << e.what();
            messages.push_back(boss::Message(boss::g_message_error, (format(loc::translate("Masterlist parsing failed. Details: %1%")) % e.what()).str()));
        }
        if (mlist["globals"])
            mlist_messages = mlist["globals"].as< list<boss::Message> >();
        if (mlist["plugins"])
            mlist_plugins = mlist["plugins"].as< list<boss::Plugin> >();
    }

    if (fs::exists(_game.UserlistPath())) {
        BOOST_LOG_TRIVIAL(trace) << "Parsing userlist...";

        try {
            ulist = YAML::LoadFile(_game.UserlistPath().string());
        } catch (YAML::ParserException& e) {
            BOOST_LOG_TRIVIAL(error) << "Userlist parsing failed. Details: " << e.what();
            messages.push_back(boss::Message(boss::g_message_error, (format(loc::translate("Userlist parsing failed. Details: %1%")) % e.what()).str()));
        }
        if (ulist["plugins"])
            ulist_plugins = ulist["plugins"].as< list<boss::Plugin> >();
    }

    progDia->Pulse();

    bool cyclicDependenciesExist = false;
    if (fs::exists(_game.MasterlistPath()) || fs::exists(_game.UserlistPath())) {
        BOOST_LOG_TRIVIAL(trace) << "Merging plugin lists, evaluating conditions and and checking for install validity...";

        //Merge all global message lists.
        messages = mlist_messages;
        messages.insert(messages.end(), ulist_messages.begin(), ulist_messages.end());

        //Set language.
        unsigned int lang = GetLangNum(_settings["Language"].as<string>());

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
                BOOST_LOG_TRIVIAL(error) << "\"" << it->Name() << "\" contains a condition that could not be evaluated. Details: " << e.what();
                messages.push_back(boss::Message(boss::g_message_error, (format(loc::translate("\"%1%\" contains a condition that could not be evaluated. Details: %2%")) % it->Name() % e.what()).str()));
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
                if (jt->second) {
                    BOOST_LOG_TRIVIAL(error) << "\"" << jt->first << "\" is incompatible with \"" << it->Name() << "\" and is present.";
                    pluginMessages.push_back(boss::Message(boss::g_message_error, (format(loc::translate("\"%1%\" is incompatible with \"%2%\" and is present.")) % jt->first % it->Name()).str()));
                } else {
                    BOOST_LOG_TRIVIAL(error) << "\"" << jt->first << "\" is required by \"" << it->Name() << "\" but is missing.";
                    pluginMessages.push_back(boss::Message(boss::g_message_error, (format(loc::translate("\"%1%\" is required by \"%2%\" but is missing.")) % jt->first % it->Name()).str()));
                }
            }
            if (!issues.empty())
                it->Messages(pluginMessages);

            progDia->Pulse();
        }

        for (map<string,bool>::const_iterator jt=consistencyIssues.begin(), endJt=consistencyIssues.end(); jt != endJt; ++jt) {
            if (jt->second) {
                BOOST_LOG_TRIVIAL(error) << "\"" << jt->first << "\" is a circular dependency. Plugins will not be sorted.";
                messages.push_back(boss::Message(boss::g_message_error, (format(loc::translate("\"%1%\" is a circular dependency. Plugins will not be sorted.")) % jt->first).str()));
                cyclicDependenciesExist = true;
            } else {
                BOOST_LOG_TRIVIAL(error) << "\"" << jt->first << "\" is given as a dependency and an incompatibility.";
                messages.push_back(boss::Message(boss::g_message_error, (format(loc::translate("\"%1%\" is given as a dependency and an incompatibility.")) % jt->first).str()));
            }
        }
    }

    progDia->Pulse();

    if (!cyclicDependenciesExist) {
        BOOST_LOG_TRIVIAL(trace) << "Sorting plugins...";

        //std::sort doesn't compare each plugin to every other plugin, it seems to compare plugins until no movement is necessary, because it assumes that each plugin will be comparable on all tested data, which isn't the case when dealing with masters, etc. 
        list<boss::Plugin>::iterator it=plugins.begin();
        while (it != plugins.end()) {
            list<boss::Plugin>::iterator jt=it;
            ++jt;

            BOOST_LOG_TRIVIAL(trace) << "Sorting for: " << it->Name();

          /*  vector<string> masters = it->Masters();
            if (!masters.empty()) {
                out << "\t" << "Masters:" << endl;
                for (size_t i=0, max=masters.size(); i < max; ++i)
                    out << "\t\t" << masters[i] << endl;
            }*/

            list<boss::Plugin> moved;
            while (jt != plugins.end()) {
               /* if (it->MustLoadAfter(*jt)
                 || jt->Priority() < it->Priority()) {
                 || (!jt->OverlapFormIDs(*it).empty() && jt->FormIDs().size() != it->FormIDs().size() && jt->FormIDs().size() > it->FormIDs().size())) {
              */
              //  if (load_order_sort(*jt, *it)) {
                if (it->MustLoadAfter(*jt)) {
                    BOOST_LOG_TRIVIAL(trace) << jt->Name() << " should load before " << it->Name();
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

        progDia->Pulse();

        LoadOrderPreview preview(this, translate("BOSS: Calculated Load Order"), plugins);

        if (preview.ShowModal() == wxID_OK) {
            list<boss::Plugin> newPluginsList, editedPlugins;
            newPluginsList = preview.GetLoadOrder();

            for (list<boss::Plugin>::iterator it=newPluginsList.begin(),endit=newPluginsList.end(); it != endit; ++it) {
                list<boss::Plugin>::const_iterator jt = find(plugins.begin(), plugins.end(), *it);
                boss::Plugin diff = it->DiffMetadata(*jt);

                if (!diff.HasNameOnly())
                    editedPlugins.push_back(diff);
            }
            plugins = newPluginsList;

            for (list<boss::Plugin>::const_iterator it=editedPlugins.begin(),endit=editedPlugins.end(); it != endit; ++it) {
                list<boss::Plugin>::iterator jt = find(ulist_plugins.begin(), ulist_plugins.end(), *it);

                if (jt != ulist_plugins.end()) {
                    jt->Merge(*it);
                } else {
                    ulist_plugins.push_back(*it);
                }
            }

            for (list<boss::Plugin>::iterator it=ulist_plugins.begin(),endit=ulist_plugins.end(); it != endit; ++it) {
                //Double-check that the plugin is still loading after any other plugins already in its "load after" metadata.
                list<boss::Plugin>::iterator mainPos = find(plugins.begin(), plugins.end(), *it);

                if (mainPos == plugins.end())
                    continue;

                size_t mainDist = distance(plugins.begin(), mainPos);
                
                set<boss::File> loadAfter = it->LoadAfter();
                set<boss::File>::iterator jt = loadAfter.begin();
                while (jt != loadAfter.end()) {
                    list<boss::Plugin>::iterator filePos = find(plugins.begin(), plugins.end(), *jt);

                    size_t fileDist = distance(plugins.begin(), filePos);

                    if (fileDist > mainDist)
                        loadAfter.erase(jt++);
                    else
                        ++jt;
                }
                it->LoadAfter(loadAfter);
            }

            //Save edits to userlist.
            YAML::Emitter yout;
            yout.SetIndent(2);
            yout << YAML::BeginMap
                 << YAML::Key << "plugins" << YAML::Value << ulist_plugins
                 << YAML::EndMap;

            ofstream uout(_game.UserlistPath().string().c_str());
            uout << yout.c_str();
            uout.close();

            //Now set load order.
            try {
                _game.SetLoadOrder(plugins);
            } catch (boss::error& e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to set the load order. Details: " << e.what();
                messages.push_back(boss::Message(boss::g_message_error, (format(loc::translate("Failed to set the load order. Details: %1%")) % e.what()).str()));
            }
        } else {
            BOOST_LOG_TRIVIAL(debug) << "The load order calculated was not applied as sorting was canceled.";
            messages.push_back(boss::Message(boss::g_message_warn, loc::translate("The load order displayed in the Details tab was not applied as sorting was canceled.")));
        }

        BOOST_LOG_TRIVIAL(trace) << "Set load order:";
        for (list<boss::Plugin>::iterator it=plugins.begin(), endIt = plugins.end(); it != endIt; ++it) {
            BOOST_LOG_TRIVIAL(trace) << '\t' << it->Name();
            /*vector<string> masters = it->Masters();
            if (!masters.empty()) {
                out << "\t" << "Masters:" << endl;
                for (size_t i=0, max=masters.size(); i < max; ++i)
                    out << "\t\t" << masters[i] << endl;
            }*/
        }
    }

    BOOST_LOG_TRIVIAL(trace) << "Generating report...";

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
                        revision,
                        _settings["Update Masterlist"].as<bool>());
    } catch (boss::error& e) {
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: %1%")) % e.what()),
            translate("BOSS: Error"),
            wxOK | wxICON_ERROR,
            this);
        progDia->Destroy();
        return;
    }

    progDia->Pulse();
    
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
    BOOST_LOG_TRIVIAL(debug) << "Reading installed plugins' headers.";
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
            BOOST_LOG_TRIVIAL(error) << "Masterlist parsing failed. " << e.what();
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
            BOOST_LOG_TRIVIAL(error) << "Userlist parsing failed. " << e.what();
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
    BOOST_LOG_TRIVIAL(debug) << "Merging the masterlist down into the installed mods list.";
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
    BOOST_LOG_TRIVIAL(debug) << "Sorting plugin list into alphabetical order.";
    std::sort(installed.begin(), installed.end(), boss::alpha_sort);
    
    progDia->Pulse();
    
    unsigned int lang = GetLangNum(_settings["Language"].as<string>());

    //Create editor window.
    BOOST_LOG_TRIVIAL(debug) << "Opening editor window.";
    Editor *editor = new Editor(this, translate("BOSS: Metadata Editor"), _game.UserlistPath().string(), installed, ulist_plugins, lang);
    
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
    if (fs::exists(g_path_readme.string())) {
        wxLaunchDefaultApplication(g_path_readme.string());
    } else  //No ReadMe exists, show a pop-up message saying so.
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: \"%1%\" cannot be found.")) % g_path_readme.string()),
            translate("BOSS: Error"),
            wxOK | wxICON_ERROR,
            this);
}

void Launcher::OnAbout(wxCommandEvent& event) {
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName("BOSS");
    aboutInfo.SetVersion(IntToString(g_version_major)+"."+IntToString(g_version_minor)+"."+IntToString(g_version_patch));
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

LoadOrderPreview::LoadOrderPreview(wxWindow *parent, const wxString title, const std::list<boss::Plugin>& plugins) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER), _plugins(plugins) {

    //Init controls.
    _loadOrder = new wxListView(this, LIST_LoadOrder, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);

    _moveUp = new wxButton(this, BUTTON_MoveUp, translate("Up"));
    _moveDown = new wxButton(this, BUTTON_MoveDown, translate("Down"));

    //Populate list.
    _loadOrder->AppendColumn(translate("Load Order"));
    size_t i=0;
    for (list<boss::Plugin>::const_iterator it=plugins.begin(), endit=plugins.end(); it != endit; ++it, ++i) {
        _loadOrder->InsertItem(i, FromUTF8(it->Name()));
    }
    _loadOrder->SetColumnWidth(0, wxLIST_AUTOSIZE);

    //Set up event handling.
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &LoadOrderPreview::OnMoveUp, this, BUTTON_MoveUp);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &LoadOrderPreview::OnMoveDown, this, BUTTON_MoveDown);
    
    //Set up layout.
    wxBoxSizer * bigBox = new wxBoxSizer(wxVERTICAL);

    bigBox->Add(_loadOrder, 1, wxEXPAND|wxALL, 10);

    wxBoxSizer * hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(_moveUp, 0, wxRIGHT, 5);
    hbox->Add(_moveDown, 0, wxLEFT, 5);
    bigBox->Add(hbox, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT, 10);

    bigBox->Add(new wxStaticText(this, wxID_ANY, translate("Please submit any alterations made for reasons other than user preference \nto the BOSS team so that they may include the changes in the masterlist.")), 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10); 

    //Need to add 'OK' and 'Cancel' buttons.
	wxSizer * sizer = CreateSeparatedButtonSizer(wxOK|wxCANCEL);

	//Now add TabHolder and OK button to window sizer.
    if (sizer != NULL)
        bigBox->Add(sizer, 0, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT, 15);

    //Now set the layout and sizes.
	SetBackgroundColour(wxColour(255,255,255));
    SetIcon(wxIconLocation("BOSS.exe"));
	SetSizerAndFit(bigBox);
}

void LoadOrderPreview::OnMoveUp(wxCommandEvent& event) {
    long selected = _loadOrder->GetFirstSelected();

    while (selected != -1) {
        if (selected > 0) {
            wxString selectedText = _loadOrder->GetItemText(selected);
            wxString aboveText = _loadOrder->GetItemText(selected - 1);

            _loadOrder->SetItemText(selected, aboveText);
            _loadOrder->SetItemText(selected - 1, selectedText);

            _movedPlugins.insert(string(selectedText.ToUTF8()));
            
            _loadOrder->Select(selected, false);
            _loadOrder->Select(selected - 1, true);
        }
        selected = _loadOrder->GetNextSelected(selected);
    }
}

void LoadOrderPreview::OnMoveDown(wxCommandEvent& event) {
    for (long i=_loadOrder->GetItemCount() - 1; i > -1; --i) {
        if (_loadOrder->IsSelected(i)) {
            wxString selectedText = _loadOrder->GetItemText(i);
            wxString belowText = _loadOrder->GetItemText(i + 1);

            _loadOrder->SetItemText(i, belowText);
            _loadOrder->SetItemText(i + 1, selectedText);

            _movedPlugins.insert(string(selectedText.ToUTF8()));

            _loadOrder->Select(i, false);
            _loadOrder->Select(i + 1, true);
        }
    }
}

std::list<boss::Plugin> LoadOrderPreview::GetLoadOrder() const {
    list<boss::Plugin> plugins;
    bool wasMovedUp = false;
    for (size_t i=0,max=_loadOrder->GetItemCount(); i < max; ++i) {
        string name = string(_loadOrder->GetItemText(i).ToUTF8());

        list<boss::Plugin>::const_iterator it = find(_plugins.begin(), _plugins.end(), boss::Plugin(name));

        plugins.push_back(*it);

        if (wasMovedUp) {
            list<boss::Plugin>::const_iterator jt = ----plugins.end();
            set<boss::File> loadAfter = plugins.back().LoadAfter();
            loadAfter.insert(File(jt->Name()));
            plugins.back().LoadAfter(loadAfter);
            wasMovedUp = false;
        }

        if (_movedPlugins.find(name) != _movedPlugins.end()) {
            //Check if this plugin has been moved earlier or later by comparing distances in the original list and the new one.
            size_t newDist = plugins.size() - 1;
            size_t oldDist = distance(_plugins.begin(), it);

            if (newDist > oldDist) {
                //Record the preceding plugin in this plugin's "load after" set.
                list<boss::Plugin>::const_iterator jt = ----plugins.end();
                set<boss::File> loadAfter = plugins.back().LoadAfter();
                loadAfter.insert(File(jt->Name()));
                plugins.back().LoadAfter(loadAfter);
            } else {
                //Record this plugin in the following plugin's "load after" set.
                wasMovedUp = true;
            }
        }
    }

    return plugins;
}

std::set<std::string> LoadOrderPreview::GetMovedPlugins() const {
    return _movedPlugins;
}
