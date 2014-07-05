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
#include "main.h"
#include "settings.h"
#include "editor.h"
#include "viewer.h"
#include "misc.h"

#include "../backend/globals.h"
#include "../backend/metadata.h"
#include "../backend/parsers.h"
#include "../backend/error.h"
#include "../backend/helpers.h"
#include "../backend/generators.h"
#include "../backend/network.h"
#include "../backend/streams.h"
#include "../backend/graph.h"

#include <ostream>
#include <algorithm>
#include <iterator>
#include <ctime>
#include <clocale>
#include <unordered_map>

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
#include <boost/thread.hpp>

#include <wx/snglinst.h>
#include <wx/aboutdlg.h>
#include <wx/progdlg.h>
#include <wx/cmdline.h>

wxIMPLEMENT_APP(LOOT);

using namespace loot;
using namespace std;
using boost::format;

namespace fs = boost::filesystem;
namespace loc = boost::locale;


struct plugin_loader {
    plugin_loader(loot::Plugin& plugin, loot::Game& game) : _plugin(plugin), _game(game) {
    }

    void operator () () {
        _plugin = loot::Plugin(_game, _plugin.Name(), false);
    }

    loot::Plugin& _plugin;
    loot::Game& _game;
    string _filename;
    bool _b;
};

struct plugin_list_loader {
    plugin_list_loader(list<loot::Plugin>& plugins, loot::Game& game) : _plugins(plugins), _game(game) {}

    void operator () () {
        for (auto &plugin : _plugins) {
            if (skipPlugins.find(plugin.Name()) == skipPlugins.end()) {
                plugin = loot::Plugin(_game, plugin.Name(), false);
            }
        }
    }

    list<loot::Plugin>& _plugins;
    loot::Game& _game;
    set<string> skipPlugins;
};

struct masterlist_updater_parser {
    masterlist_updater_parser(bool doUpdate, loot::Game& game, list<loot::Message>& errors, list<loot::Plugin>& plugins, list<loot::Message>& messages, string& revision, string& date) : _doUpdate(doUpdate), _game(game), _errors(errors), _plugins(plugins), _messages(messages), _revision(revision), _date(date) {}

    void operator () () {

        if (_doUpdate) {
            BOOST_LOG_TRIVIAL(debug) << "Updating masterlist";
            try {
                pair<string, string> ret = UpdateMasterlist(_game, _errors, _plugins, _messages);
                _revision = ret.first;
                _date = ret.second;
            } catch (std::exception& e) {
                _plugins.clear();
                _messages.clear();
                BOOST_LOG_TRIVIAL(error) << "Masterlist update failed. Details: " << e.what();
                _errors.push_back(loot::Message(loot::Message::error, (format(loc::translate("Masterlist update failed. Details: %1%")) % e.what()).str()));
                //Try getting masterlist revision anyway.
                try {
                    pair<string, string> ret = GetMasterlistRevision(_game);
                    _revision = ret.first;
                    _date = ret.second;
                }
                catch (std::exception& e) {
                    BOOST_LOG_TRIVIAL(error) << "Masterlist revision check failed. Details: " << e.what();
                    _errors.push_back(loot::Message(loot::Message::error, (format(loc::translate("Masterlist revision check failed. Details: %1%")) % e.what()).str()));
                }
            }
        }
        else {
            BOOST_LOG_TRIVIAL(debug) << "Getting masterlist revision";
            try {
                pair<string, string> ret = GetMasterlistRevision(_game);
                _revision = ret.first;
                _date = ret.second;
            }
            catch (std::exception& e) {
                BOOST_LOG_TRIVIAL(error) << "Masterlist revision check failed. Details: " << e.what();
                _errors.push_back(loot::Message(loot::Message::error, (format(loc::translate("Masterlist revision check failed. Details: %1%")) % e.what()).str()));
            }
        }

        if (_plugins.empty() && _messages.empty() && fs::exists(_game.MasterlistPath())) {

            BOOST_LOG_TRIVIAL(debug) << "Parsing masterlist...";
            try {
                loot::ifstream in(_game.MasterlistPath());
                YAML::Node mlist = YAML::Load(in);
                in.close();

                if (mlist["globals"])
                    _messages = mlist["globals"].as< list<loot::Message> >();
                if (mlist["plugins"])
                    _plugins = mlist["plugins"].as< list<loot::Plugin> >();
            } catch (YAML::Exception& e) {
                BOOST_LOG_TRIVIAL(error) << "Masterlist parsing failed. Details: " << e.what();
                _errors.push_back(loot::Message(loot::Message::error, (format(loc::translate("Masterlist parsing failed. Details: %1%")) % e.what()).str()));
            }
            BOOST_LOG_TRIVIAL(debug) << "Finished parsing masterlist.";

        }

        if (_revision.empty()) {
            if (fs::exists(_game.MasterlistPath()))
                _revision = loc::translate("Unknown");
            else
                _revision = loc::translate("No masterlist");
        }

        if (_date.empty()) {
            if (fs::exists(_game.MasterlistPath()))
                _date = loc::translate("Unknown");
            else
                _date = loc::translate("No masterlist");
        }
    }

    bool _doUpdate;
    loot::Game& _game;
    list<loot::Message>& _errors;
    list<loot::Plugin>& _plugins;
    list<loot::Message>& _messages;
    string& _revision;
    string& _date;
};

bool LOOT::OnInit() {

    //Check if GUI is already running.
	wxSingleInstanceChecker *checker = new wxSingleInstanceChecker;

	if (checker->IsAnotherRunning()) {
        wxMessageBox(
			translate("Error: LOOT is already running. This instance will now quit."),
			translate("LOOT: Error"),
			wxOK | wxICON_ERROR,
			nullptr);
		delete checker; // OnExit() won't be called if we return false
		checker = nullptr;

		return false;
	}

    //Load settings.
    if (!fs::exists(g_path_settings)) {
        try {
            if (!fs::exists(g_path_settings.parent_path()))
                fs::create_directory(g_path_settings.parent_path());
        } catch (fs::filesystem_error& /*e*/) {
            wxMessageBox(
				translate("Error: Could not create local app data LOOT folder."),
				translate("LOOT: Error"),
				wxOK | wxICON_ERROR,
				nullptr);
            return false;
        }
        GenerateDefaultSettingsFile(g_path_settings.string());
    }
    try {
        loot::ifstream in(g_path_settings);
        _settings = YAML::Load(in);
        in.close();
    } catch (YAML::ParserException& e) {
        wxMessageBox(
			FromUTF8(format(loc::translate("Error: Settings parsing failed. %1%")) % e.what()),
			translate("LOOT: Error"),
			wxOK | wxICON_ERROR,
			nullptr);
        return false;
    }

    //Set up logging.
    boost::log::add_file_log(
        boost::log::keywords::file_name = g_path_log.string().c_str(),
        boost::log::keywords::auto_flush = true,
        boost::log::keywords::format = (
            boost::log::expressions::stream
                << "[" << boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%H:%M:%S") << "]"
                << " [" << boost::log::trivial::severity << "]: "
                << boost::log::expressions::smessage
            )
        );
    boost::log::add_common_attributes();
    if (_settings["Debug Verbosity"]) {
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
    }
    BOOST_LOG_TRIVIAL(info) << "LOOT Version: " << g_version_major << "." << g_version_minor << "." << g_version_patch;


    //Set the locale to get encoding and language conversions working correctly.
    BOOST_LOG_TRIVIAL(debug) << "Initialising language settings.";
    //Defaults in case language string is empty or setting is missing.
    string localeId = loot::Language(loot::Language::english).Locale() + ".UTF-8";
    wxLanguage wxLang = wxLANGUAGE_ENGLISH;
    if (_settings["Language"]) {
        loot::Language lang(_settings["Language"].as<string>());
        BOOST_LOG_TRIVIAL(debug) << "Selected language: " << lang.Name();
        localeId = lang.Locale() + ".UTF-8";
        if (lang.Code() == loot::Language::english)
            wxLang = wxLANGUAGE_ENGLISH;
        else if (lang.Code() == loot::Language::spanish)
            wxLang = wxLANGUAGE_SPANISH;
        else if (lang.Code() == loot::Language::russian)
            wxLang = wxLANGUAGE_RUSSIAN;
        else if (lang.Code() == loot::Language::french)
            wxLang = wxLANGUAGE_FRENCH;
        else if (lang.Code() == loot::Language::chinese)
            wxLang = wxLANGUAGE_CHINESE;
        else if (lang.Code() == loot::Language::polish)
            wxLang = wxLANGUAGE_POLISH;
        else if (lang.Code() == loot::Language::brazilian_portuguese)
            wxLang = wxLANGUAGE_PORTUGUESE_BRAZILIAN;
        else if (lang.Code() == loot::Language::finnish)
            wxLang = wxLANGUAGE_FINNISH;
        else if (lang.Code() == loot::Language::german)
            wxLang = wxLANGUAGE_GERMAN;
    }

    //Boost.Locale initialisation: Specify location of language dictionaries.
    boost::locale::generator gen;
    gen.add_messages_path(g_path_l10n.string());
    gen.add_messages_domain("loot");

    //Boost.Locale initialisation: Generate and imbue locales.
    locale::global(gen(localeId));
    cout.imbue(locale());
    boost::filesystem::path::imbue(locale());

    //wxWidgets initalisation.
    if (wxLocale::IsAvailable(wxLang)) {
        BOOST_LOG_TRIVIAL(trace) << "Selected language is available, setting language file paths.";
        wxLoc = new wxLocale(wxLang);

        wxLocale::AddCatalogLookupPathPrefix(g_path_l10n.string().c_str());

        wxLoc->AddCatalog("wxstd");

        if (!wxLoc->IsOk()) {
            BOOST_LOG_TRIVIAL(error) << "Could not load translations.";
            wxMessageBox(
                translate("Error: Could not apply translation."),
                translate("LOOT: Error"),
                wxOK | wxICON_ERROR,
                nullptr);
        }
    } else {
        wxLoc = new wxLocale(wxLANGUAGE_ENGLISH);

        BOOST_LOG_TRIVIAL(error) << "The selected language is not available on this system.";
        wxMessageBox(
            translate("Error: The selected language is not available on this system."),
            translate("LOOT: Error"),
            wxOK | wxICON_ERROR,
            nullptr);
    }

    //Detect installed games.
    BOOST_LOG_TRIVIAL(debug) << "Detecting installed games.";
    try {
        _games = GetGames(_settings);
    } catch (YAML::Exception& e) {
        BOOST_LOG_TRIVIAL(error) << "Games' settings parsing failed. " << e.what();
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: Games' settings parsing failed. %1%")) % e.what()),
            translate("LOOT: Error"),
            wxOK | wxICON_ERROR,
            nullptr);
        return false;
    }
    catch (std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "Game-specific settings could not be initialised. " << e.what();
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: Game-specific settings could not be initialised. %1%")) % e.what()),
            translate("LOOT: Error"),
            wxOK | wxICON_ERROR,
            nullptr);
        return false;
    }

    BOOST_LOG_TRIVIAL(debug) << "Selecting game.";
    string target;
    int gameIndex = -1;

    wxCmdLineEntryDesc cmdLineDesc[2];
    cmdLineDesc[0].kind = wxCMD_LINE_OPTION;
    cmdLineDesc[0].shortName = "g";
    cmdLineDesc[0].longName = "game";
    cmdLineDesc[0].description = "The folder name of the game to run for.";
    cmdLineDesc[0].type = wxCMD_LINE_VAL_STRING;
    cmdLineDesc[0].flags = wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR;
    cmdLineDesc[1].kind = wxCMD_LINE_NONE;

    wxCmdLineParser parser(cmdLineDesc, argc, argv);
    wxString value;
    switch (parser.Parse()) {
    case -1:
        BOOST_LOG_TRIVIAL(info) << "Help was given.";
        break;
    case 0:
        if (parser.Found("game", &value))
            target = value.ToUTF8();
        break;
    default:
        BOOST_LOG_TRIVIAL(error) << "A command line syntax error was detected.";
        break;
    }

    if (target.empty()) {
        if (_settings["Game"] && _settings["Game"].as<string>() != "auto")
            target = _settings["Game"].as<string>();
        else if (_settings["Last Game"] && _settings["Last Game"].as<string>() != "auto")
            target = _settings["Last Game"].as<string>();
    }

    if (!target.empty()) {
        for (size_t i=0, max=_games.size(); i < max; ++i) {
            if (target == _games[i].FolderName() && _games[i].IsInstalled())
                gameIndex = i;
        }
    }
    if (gameIndex < 0) {
        //Set gameIndex to the first installed game.
        for (size_t i=0, max=_games.size(); i < max; ++i) {
            if (_games[i].IsInstalled()) {
                gameIndex = i;
                break;
            }
        }
        if (gameIndex < 0) {
            BOOST_LOG_TRIVIAL(error) << "None of the supported games were detected.";
            wxMessageBox(
                translate("Error: None of the supported games were detected."),
                translate("LOOT: Error"),
                wxOK | wxICON_ERROR,
                nullptr);
            return false;
        }
    }
    loot::Game & _game(_games[gameIndex]);
    BOOST_LOG_TRIVIAL(debug) << "Game selected is " << _game.Name();

    //Now that game is selected, initialise it.
    BOOST_LOG_TRIVIAL(debug) << "Initialising game-specific settings.";
    try {
        _game.Init();
        *find(_games.begin(), _games.end(), _game) = _game;  //Sync changes.
    } catch (std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "Game-specific settings could not be initialised. " << e.what();
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: Game-specific settings could not be initialised. %1%")) % e.what()),
            translate("LOOT: Error"),
            wxOK | wxICON_ERROR,
            nullptr);
        return false;
    }

    //Load window size/pos settings.
    wxSize size = wxDefaultSize;
    wxPoint pos = wxDefaultPosition;
    if (_settings["windows"] && _settings["windows"]["main"]) {
        YAML::Node node = _settings["windows"]["main"];
        if (node["width"] && node["height"] && node["xPos"] && node["yPos"]) {
            size = wxSize(node["width"].as<int>(), node["height"].as<int>());
            pos = wxPoint(node["xPos"].as<int>(), node["yPos"].as<int>());
        }
    }

    //Create launcher window.
    BOOST_LOG_TRIVIAL(debug) << "Opening the main LOOT window.";
    Launcher * launcher = new Launcher(wxT("LOOT"), _settings, &_game, _games, pos, size);

    launcher->SetIcon(wxIconLocation("LOOT.exe"));
	launcher->Show();
	SetTopWindow(launcher);

    return true;
}

Launcher::Launcher(const wxChar *title, YAML::Node& settings, Game * game, vector<loot::Game>& games, wxPoint pos, wxSize size) : wxFrame(nullptr, wxID_ANY, title, pos, size), _game(game), _settings(settings), _games(games) {

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
	FileMenu->Append(MENU_ViewDebugLog, translate("View &Debug Log"));
    FileMenu->Append(OPTION_SortPlugins, translate("&Sort Plugins"));
    RedatePluginsItem = FileMenu->Append(MENU_RedatePlugins, translate("&Redate Plugins"));
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
        if (*_game == _games[i])
            item->Check();

        if (_games[i].IsInstalled())
            Bind(wxEVT_MENU, &Launcher::OnGameChange, this, MENU_LowestDynamicGameID + i);
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
    Bind(wxEVT_MENU, &Launcher::OnQuit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &Launcher::OnViewLastReport, this, OPTION_ViewLastReport);
    Bind(wxEVT_MENU, &Launcher::OnOpenDebugLog, this, MENU_ViewDebugLog);
    Bind(wxEVT_MENU, &Launcher::OnSortPlugins, this, OPTION_SortPlugins);
    Bind(wxEVT_MENU, &Launcher::OnEditMetadata, this, OPTION_EditMetadata);
    Bind(wxEVT_MENU, &Launcher::OnRedatePlugins, this, MENU_RedatePlugins);
    Bind(wxEVT_MENU, &Launcher::OnOpenSettings, this, MENU_ShowSettings);
    Bind(wxEVT_MENU, &Launcher::OnHelp, this, wxID_HELP);
    Bind(wxEVT_MENU, &Launcher::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_BUTTON, &Launcher::OnSortPlugins, this, OPTION_SortPlugins);
    Bind(wxEVT_BUTTON, &Launcher::OnEditMetadata, this, OPTION_EditMetadata);
    Bind(wxEVT_BUTTON, &Launcher::OnViewLastReport, this, OPTION_ViewLastReport);
    Bind(wxEVT_CLOSE_WINDOW, &Launcher::OnClose, this);

    //Set up tooltips.
    EditButton->SetToolTip(translate("Opens the Metadata Editor, where plugins' sorting metadata, messages, dirty info and Bash Tag suggestions can be edited."));
    SortButton->SetToolTip(translate("Sorts your plugins, then displays a report of the results."));
    ViewButton->SetToolTip(translate("Opens the last report generated for the current game."));

    //Set up initial state.
    SortButton->SetDefault();

    if (!fs::exists(g_path_report))
        ViewButton->Enable(false);

    if (_game->Id() == loot::Game::tes5)
        RedatePluginsItem->Enable(true);
    else
        RedatePluginsItem->Enable(false);

    //Set title bar text.
    SetTitle(FromUTF8("LOOT - " + _game->Name()));

    //Now set the layout and sizes.
    SetMenuBar(MenuBar);
	SetBackgroundColour(wxColour(255,255,255));
    SetSizerAndFit(buttonBox);

    if (size != wxDefaultSize)
        SetSize(size);
    else
        SetSize(wxSize(250, 200));

    SortButton->SetFocus();
}

//Called when the frame exits.
void Launcher::OnQuit(wxCommandEvent& event) {

	Close(true); // Tells the OS to quit running this process
}

void Launcher::OnClose(wxCloseEvent& event) {
    BOOST_LOG_TRIVIAL(debug) << "Quiting LOOT.";

    //Record window settings.
    YAML::Node main;
    main["height"] = GetSize().GetHeight();
    main["width"] = GetSize().GetWidth();
    main["xPos"] = GetPosition().x;
    main["yPos"] = GetPosition().y;

    _settings["windows"]["main"] = main;

    //Record game settings.
    _settings["Last Game"] = _game->FolderName();
    _settings["Games"] = _games;

    //Save settings.
    try {
        BOOST_LOG_TRIVIAL(debug) << "Saving LOOT settings.";
        YAML::Emitter yout;
        yout.SetIndent(2);
        yout << _settings;

        loot::ofstream out(loot::g_path_settings);
        out << yout.c_str();
        out.close();
    }
    catch (std::exception &e) {
        BOOST_LOG_TRIVIAL(error) << "Failed to save LOOT's settings. Error: " << e.what();
    }

    Destroy();
}

void Launcher::OnViewLastReport(wxCommandEvent& event) {
    //Load window size/pos settings.
    wxSize size = wxDefaultSize;
    wxPoint pos = wxDefaultPosition;
    if (_settings["windows"] && _settings["windows"]["viewer"]) {
        YAML::Node node = _settings["windows"]["viewer"];
        if (node["width"] && node["height"] && node["xPos"] && node["yPos"]) {
            size = wxSize(node["width"].as<int>(), node["height"].as<int>());
            pos = wxPoint(node["xPos"].as<int>(), node["yPos"].as<int>());
        }
    }
    //Create viewer window.
    BOOST_LOG_TRIVIAL(debug) << "Opening viewer window...";
    Viewer *viewer = new Viewer(this, translate("LOOT: Report Viewer"), FromUTF8(ToFileURL(g_path_report.string() + "?data=" + _game->ReportDataPath().string())), pos, size, _settings);
    viewer->Show();
    BOOST_LOG_TRIVIAL(debug) << "Report displayed.";
}

void Launcher::OnOpenSettings(wxCommandEvent& event) {
    BOOST_LOG_TRIVIAL(debug) << "Opening settings window...";

    //Load window size/pos settings.
    wxSize size = wxDefaultSize;
    wxPoint pos = wxDefaultPosition;
    if (_settings["windows"] && _settings["windows"]["settings"]) {
        YAML::Node node = _settings["windows"]["settings"];
        if (node["width"] && node["height"] && node["xPos"] && node["yPos"]) {
            size = wxSize(node["width"].as<int>(), node["height"].as<int>());
            pos = wxPoint(node["xPos"].as<int>(), node["yPos"].as<int>());
        }
    }

    SettingsFrame settings = SettingsFrame(this, translate("LOOT: Settings"), _settings, _games, pos, size);
    BOOST_LOG_TRIVIAL(debug) << "Settings window opened.";
    settings.ShowModal();

    //Record window settings.
    YAML::Node node;
    node["height"] = settings.GetSize().GetHeight();
    node["width"] = settings.GetSize().GetWidth();
    node["xPos"] = settings.GetPosition().x;
    node["yPos"] = settings.GetPosition().y;

    _settings["windows"]["settings"] = node;
}

void Launcher::OnGameChange(wxCommandEvent& event) {
    BOOST_LOG_TRIVIAL(debug) << "Changing current game...";
    _game = &_games[event.GetId() - MENU_LowestDynamicGameID];
    try {
        _game->Init();  //In case it hasn't already been done.
        BOOST_LOG_TRIVIAL(debug) << "New game is " << _game->Name();
    }
    catch (std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "Game-specific settings could not be initialised." << e.what();
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: Game-specific settings could not be initialised. %1%")) % e.what()),
            translate("LOOT: Error"),
            wxOK | wxICON_ERROR,
            nullptr);
    }
    SetTitle(FromUTF8("LOOT - " + _game->Name()));
    if (_game->Id() == loot::Game::tes5)
        RedatePluginsItem->Enable(true);
    else
        RedatePluginsItem->Enable(false);
}

void Launcher::OnHelp(wxCommandEvent& event) {
    //Look for file.
    BOOST_LOG_TRIVIAL(debug) << "Opening readme at: " << g_path_readme;
    if (fs::exists(g_path_readme)) {
        wxLaunchDefaultBrowser(FromUTF8(ToFileURL(g_path_readme.string())));
    }
    else {  //No readme exists, show a pop-up message saying so.
        BOOST_LOG_TRIVIAL(error) << "File \"" << g_path_readme.string() << "\" could not be found.";
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: \"%1%\" cannot be found.")) % g_path_readme.string()),
            translate("LOOT: Error"),
            wxOK | wxICON_ERROR,
            this);
    }
}

void Launcher::OnOpenDebugLog(wxCommandEvent& event) {
    //Look for file.
    BOOST_LOG_TRIVIAL(debug) << "Opening debug log at: " << g_path_log;
    if (fs::exists(g_path_log)) {
        wxLaunchDefaultApplication(FromUTF8(g_path_log.string()));
    }
    else {  //No log exists, show a pop-up message saying so.
        BOOST_LOG_TRIVIAL(error) << "File \"" << g_path_log.string() << "\" could not be found.";
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: \"%1%\" cannot be found.")) % g_path_log.string()),
            translate("LOOT: Error"),
            wxOK | wxICON_ERROR,
            this);
    }
}

void Launcher::OnAbout(wxCommandEvent& event) {
    BOOST_LOG_TRIVIAL(debug) << "Opening About dialog.";
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName("LOOT");
    aboutInfo.SetVersion(to_string(g_version_major) + "." + to_string(g_version_minor) + "." + to_string(g_version_patch));
    aboutInfo.SetDescription(translate("Load order optimisation for Oblivion, Skyrim, Fallout 3 and Fallout: New Vegas."));
    aboutInfo.SetCopyright("Copyright (C) 2012-2014 LOOT Team.");
    aboutInfo.SetWebSite("http://loot.github.io");
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
    aboutInfo.SetIcon(wxIconLocation("LOOT.exe"));
    wxAboutBox(aboutInfo);
}

void Launcher::OnSortPlugins(wxCommandEvent& event) {

    BOOST_LOG_TRIVIAL(debug) << "Beginning sorting process.";

    YAML::Node mlist, ulist;
    list<loot::Message> messages, mlist_messages, ulist_messages;
    list<loot::Plugin> mlist_plugins, ulist_plugins;
    list<loot::Plugin> plugins;
    boost::thread_group group;
    string revision, date;

    wxProgressDialog *progDia = new wxProgressDialog(translate("LOOT: Working..."),translate("LOOT working..."), 1000, this, wxPD_APP_MODAL|wxPD_AUTO_HIDE|wxPD_ELAPSED_TIME);

    ///////////////////////////////////////////////////////
    // Load Plugins & Lists
    ///////////////////////////////////////////////////////

    bool doUpdate = _settings["Update Masterlist"] && _settings["Update Masterlist"].as<bool>();
    masterlist_updater_parser mup(doUpdate, *_game, messages, mlist_plugins, mlist_messages, revision, date);
    group.create_thread(mup);

    //First calculate the mean plugin size. Store it temporarily in a map to reduce filesystem lookups and file size recalculation.
    size_t meanFileSize = 0;
    boost::unordered_map<std::string, size_t> tempMap;
    for (fs::directory_iterator it(_game->DataPath()); it != fs::directory_iterator(); ++it) {
        if (fs::is_regular_file(it->status()) && IsPlugin(it->path().string())) {

            size_t fileSize = fs::file_size(it->path());
            meanFileSize += fileSize;

            tempMap.emplace(it->path().filename().string(), fileSize);
        }
    }
    meanFileSize /= tempMap.size();

    //Now load plugins.
    plugin_list_loader pll(plugins, *_game);
    for (const auto &pluginPair: tempMap) {

        BOOST_LOG_TRIVIAL(info) << "Found plugin: " << pluginPair.first;

        plugins.push_back(loot::Plugin(pluginPair.first));

        if (pluginPair.second > meanFileSize) {
            pll.skipPlugins.insert(pluginPair.first);
            plugin_loader pl(plugins.back(), *_game);
            group.create_thread(pl);
        }

        progDia->Pulse();
    }
    group.create_thread(pll);
    group.join_all();

    //Now load userlist.
    if (fs::exists(_game->UserlistPath())) {
        BOOST_LOG_TRIVIAL(debug) << "Parsing userlist at: " << _game->UserlistPath();

        try {
            loot::ifstream in(_game->UserlistPath());
            YAML::Node ulist = YAML::Load(in);
            in.close();

            if (ulist["plugins"])
                ulist_plugins = ulist["plugins"].as< list<loot::Plugin> >();
        } catch (YAML::ParserException& e) {
            BOOST_LOG_TRIVIAL(error) << "Userlist parsing failed. Details: " << e.what();
            messages.push_back(loot::Message(loot::Message::error, (format(loc::translate("Userlist parsing failed. Details: %1%")) % e.what()).str()));
        }
        if (ulist["plugins"])
            ulist_plugins = ulist["plugins"].as< list<loot::Plugin> >();
    }

    progDia->Pulse();

    ///////////////////////////////////////////////////////
    // Merge & Check Metadata
    ///////////////////////////////////////////////////////

    //Set language.
    unsigned int lang;
    if (_settings["Language"])
        lang = Language(_settings["Language"].as<string>()).Code();
    else
        lang = loot::Language::any;

    if (fs::exists(_game->MasterlistPath()) || fs::exists(_game->UserlistPath())) {


        //Merge all global message lists.
        BOOST_LOG_TRIVIAL(debug) << "Merging all global message lists.";
        if (!mlist_messages.empty())
            messages.insert(messages.end(), mlist_messages.begin(), mlist_messages.end());
        if (!ulist_messages.empty())
            messages.insert(messages.end(), ulist_messages.begin(), ulist_messages.end());

        //Evaluate any conditions in the global messages.
        BOOST_LOG_TRIVIAL(debug) << "Evaluating global message conditions.";
        try {
            list<loot::Message>::iterator it=messages.begin();
            while (it != messages.end()) {
                if (!it->EvalCondition(*_game, lang))
                    it = messages.erase(it);
                else
                    ++it;
            }
        }
        catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << "A global message contains a condition that could not be evaluated. Details: " << e.what();
            messages.push_back(loot::Message(loot::Message::error, (format(loc::translate("A global message contains a condition that could not be evaluated. Details: %1%")) % e.what()).str()));
        }

        //Merge plugin list and masterlist.
        BOOST_LOG_TRIVIAL(debug) << "Merging plugin list and masterlist data.";
        for (auto &plugin : plugins) {
            BOOST_LOG_TRIVIAL(trace) << "Merging for plugin \"" << plugin.Name() << "\"";

            //Check if there is a plugin entry in the masterlist. This will also find matching regex entries.
            list<loot::Plugin>::iterator pos = std::find(mlist_plugins.begin(), mlist_plugins.end(), plugin);

            if (pos != mlist_plugins.end()) {
                BOOST_LOG_TRIVIAL(trace) << "Merging masterlist data down to plugin list data.";
                plugin.MergeMetadata(*pos);
            }

            progDia->Pulse();
        }
    }

    progDia->Update(800, translate("Building plugin graph..."));

    ///////////////////////////////////////////////////////
    // Build Graph Edges & Sort
    ///////////////////////////////////////////////////////

    /* Need to loop this section. There are 3 ways to exit the loop:

    1. Accept the load order at the preview with no changes.
    2. Cancel at the preview.
    3. Cancel making changes.

    Otherwise, the sorting must loop.

    */

    //Check for back-edges, then perform a topological sort.
    try {
        bool applyLoadOrder = false;

        do {
            //Create a plugin graph containing the plugin and masterlist data.
            loot::PluginGraph graph;
            for (auto &plugin : plugins) {
                vertex_t v = boost::add_vertex(plugin, graph);
            }

            BOOST_LOG_TRIVIAL(info) << "Merging userlist into plugin list/masterlist, evaluating conditions and checking for install validity.";
            loot::vertex_it vit, vitend;
            for (boost::tie(vit, vitend) = boost::vertices(graph); vit != vitend; ++vit) {
                BOOST_LOG_TRIVIAL(trace) << "Merging for plugin \"" << graph[*vit].Name() << "\"";

                //Check if there is a plugin entry in the userlist. This will also find matching regex entries.
                list<loot::Plugin>::iterator pos = std::find(ulist_plugins.begin(), ulist_plugins.end(), graph[*vit]);

                if (pos != ulist_plugins.end() && pos->Enabled()) {
                    BOOST_LOG_TRIVIAL(trace) << "Merging userlist data down to plugin list data.";
                    graph[*vit].MergeMetadata(*pos);
                }

                progDia->Pulse();

                //Now that items are merged, evaluate any conditions they have.
                BOOST_LOG_TRIVIAL(trace) << "Evaluate conditions for merged plugin data.";
                try {
                    graph[*vit].EvalAllConditions(*_game, lang);
                }
                catch (std::exception& e) {
                    BOOST_LOG_TRIVIAL(error) << "\"" << graph[*vit].Name() << "\" contains a condition that could not be evaluated. Details: " << e.what();
                    messages.push_back(loot::Message(loot::Message::error, (format(loc::translate("\"%1%\" contains a condition that could not be evaluated. Details: %2%")) % graph[*vit].Name() % e.what()).str()));
                }

                progDia->Pulse();

                //Also check install validity.
                BOOST_LOG_TRIVIAL(trace) << "Checking that the current install is valid according to this plugin's data.";
                graph[*vit].CheckInstallValidity(*_game);

                progDia->Pulse();
            }

            BOOST_LOG_TRIVIAL(info) << "Building the plugin dependency graph...";

            //Now add the interactions between plugins to the graph as edges.
            std::map<std::string, int> overriddenPriorities;
            BOOST_LOG_TRIVIAL(debug) << "Adding non-overlap edges.";
            loot::AddSpecificEdges(graph, overriddenPriorities);

            BOOST_LOG_TRIVIAL(debug) << "Adding priority edges.";
            loot::AddPriorityEdges(graph);

            BOOST_LOG_TRIVIAL(debug) << "Adding overlap edges.";
            loot::AddOverlapEdges(graph);

            BOOST_LOG_TRIVIAL(info) << "Checking to see if the graph is cyclic.";
            loot::CheckForCycles(graph);

            for (const auto &overriddenPriority: overriddenPriorities) {
                vertex_t vertex;
                if (loot::GetVertexByName(graph, overriddenPriority.first, vertex)) {
                    graph[vertex].Priority(overriddenPriority.second);
                }
            }

            progDia->Pulse();

            BOOST_LOG_TRIVIAL(info) << "Performing a topological sort.";
            loot::Sort(graph, plugins);

            progDia->Destroy();
            progDia = nullptr;

            BOOST_LOG_TRIVIAL(info) << "Displaying load order preview.";

            //Load window size/pos settings.
            wxSize size = wxDefaultSize;
            wxPoint pos = wxDefaultPosition;
            if (_settings["windows"] && _settings["windows"]["editor"]) {
                YAML::Node node = _settings["windows"]["editor"];
                if (node["width"] && node["height"] && node["xPos"] && node["yPos"]) {
                    size = wxSize(node["width"].as<int>(), node["height"].as<int>());
                    pos = wxPoint(node["xPos"].as<int>(), node["yPos"].as<int>());
                }
            }

            MiniEditor editor(this, translate("LOOT: Calculated Load Order"), pos, size, plugins, ulist_plugins, *_game);

            long ret = editor.ShowModal();
            const std::list<loot::Plugin>& newUserlist = editor.GetNewUserlist();

            //Record window settings.
            YAML::Node node;
            node["height"] = editor.GetSize().GetHeight();
            node["width"] = editor.GetSize().GetWidth();
            node["xPos"] = editor.GetPosition().x;
            node["yPos"] = editor.GetPosition().y;

            _settings["windows"]["editor"] = node;

            //Need to determine if any new edits have been made.
            bool haveNewEdits = false;
            if (newUserlist.size() != ulist_plugins.size()) {
                BOOST_LOG_TRIVIAL(info) << "Metadata edited for some plugin, new and old userlists differ in size.";
                haveNewEdits = true;
            }
            else {
                for (const auto& newEdit : newUserlist) {
                    const auto it = std::find(ulist_plugins.begin(), ulist_plugins.end(), newEdit);
                    if (it == ulist_plugins.end()) {
                        BOOST_LOG_TRIVIAL(info) << "Metadata added for plugin: " << it->Name();
                        haveNewEdits = true;
                        break;
                    }

                    if (!it->DiffMetadata(newEdit).HasNameOnly()) {
                        BOOST_LOG_TRIVIAL(info) << "Metadata edited for plugin: " << it->Name();
                        haveNewEdits = true;
                        break;
                    }
                }
            }

            if (ret != wxID_APPLY) {
                applyLoadOrder = false;
                break;
            }
            else if (!haveNewEdits) {
                applyLoadOrder = true;
                break;
            }
            else {
                //Recreate progress dialog.
                progDia = new wxProgressDialog(translate("LOOT: Working..."), translate("Recalculating load order..."), 1000, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_ELAPSED_TIME);

                //User accepted edits, now apply them, then loop.
                ulist_plugins = newUserlist;

                //Save edits to userlist.
                BOOST_LOG_TRIVIAL(info) << "Saving edited userlist.";
                YAML::Emitter yout;
                yout.SetIndent(2);
                yout << YAML::BeginMap
                    << YAML::Key << "plugins" << YAML::Value << ulist_plugins
                    << YAML::EndMap;

                loot::ofstream uout(_game->UserlistPath());
                uout << yout.c_str();
                uout.close();

                //Now loop.
            }
        } while (true);

        if (applyLoadOrder) {
            //Applying the load order.
            BOOST_LOG_TRIVIAL(debug) << "Setting load order.";
            try {
                _game->SetLoadOrder(plugins);
            }
            catch (std::exception& e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to set the load order. Details: " << e.what();
                messages.push_back(loot::Message(loot::Message::error, (format(loc::translate("Failed to set the load order. Details: %1%")) % e.what()).str()));
            }
            BOOST_LOG_TRIVIAL(info) << "Load order set:";
            for (const auto &plugin: plugins) {
                BOOST_LOG_TRIVIAL(info) << '\t' << plugin.Name();
            }
        }
        else {
            //User decided to cancel sorting. Just go straight to displaying the report.
            BOOST_LOG_TRIVIAL(info) << "The load order calculated was not applied as sorting was canceled.";
            messages.push_back(loot::Message(loot::Message::warn, loc::translate("The load order displayed in the Details tab was not applied as sorting was canceled.")));
        }

    } catch (std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "Failed to calculate the load order. Details: " << e.what();
        messages.push_back(loot::Message(loot::Message::error, (format(loc::translate("Failed to calculate the load order. Details: %1%")) % e.what()).str()));

        progDia->Destroy();
        progDia = nullptr;
    }

    ///////////////////////////////////////////////////////
    // Build & Display Report
    ///////////////////////////////////////////////////////

    BOOST_LOG_TRIVIAL(debug) << "Generating report...";
    try {
        GenerateReportData(*_game,
                        messages,
                        plugins,
                        revision,
                        date,
                        doUpdate);
    } catch (std::exception& e) {
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: %1%")) % e.what()),
            translate("LOOT: Error"),
            wxOK | wxICON_ERROR,
            this);
        if (progDia != nullptr)
            progDia->Destroy();
        return;
    }

    //Now a results report definitely exists.
    ViewButton->Enable(true);

    BOOST_LOG_TRIVIAL(debug) << "Displaying report...";
    //Load window size/pos settings.
    wxSize size = wxDefaultSize;
    wxPoint pos = wxDefaultPosition;
    if (_settings["windows"] && _settings["windows"]["viewer"]) {
        YAML::Node node = _settings["windows"]["viewer"];
        if (node["width"] && node["height"] && node["xPos"] && node["yPos"]) {
            size = wxSize(node["width"].as<int>(), node["height"].as<int>());
            pos = wxPoint(node["xPos"].as<int>(), node["yPos"].as<int>());
        }
    }

    //Create viewer window.
    Viewer *viewer = new Viewer(this, translate("LOOT: Report Viewer"), FromUTF8(ToFileURL(g_path_report.string() + "?data=" + _game->ReportDataPath().string())), pos, size, _settings);
    viewer->Show();

    BOOST_LOG_TRIVIAL(debug) << "Report display successful. Sorting process complete.";
}

void Launcher::OnEditMetadata(wxCommandEvent& event) {

    //Should probably check for masterlist updates before opening metadata editor.
    list<loot::Plugin> installed, mlist_plugins, ulist_plugins;

    wxProgressDialog *progDia = new wxProgressDialog(translate("LOOT: Working..."),translate("LOOT working..."), 1000, this, wxPD_APP_MODAL|wxPD_AUTO_HIDE|wxPD_ELAPSED_TIME);

    //Scan for installed plugins.
    BOOST_LOG_TRIVIAL(debug) << "Reading installed plugins' headers.";
    _game->LoadPlugins(true);
    //Sort plugins into their load order.
    list<string> loadOrder;
    _game->GetLoadOrder(loadOrder);
    for (const auto &pluginName: loadOrder) {
        const auto pos = _game->plugins.find(pluginName);

        if (pos != _game->plugins.end())
            installed.push_back(pos->second);
    }

    //Parse masterlist.
    if (fs::exists(_game->MasterlistPath())) {
        BOOST_LOG_TRIVIAL(debug) << "Parsing masterlist.";
        YAML::Node mlist;
        try {
            loot::ifstream in(_game->MasterlistPath());
            mlist = YAML::Load(in);
            in.close();
        } catch (YAML::ParserException& e) {
            BOOST_LOG_TRIVIAL(error) << "Masterlist parsing failed. " << e.what();
            wxMessageBox(
                FromUTF8(format(loc::translate("Error: Masterlist parsing failed. %1%")) % e.what()),
                translate("LOOT: Error"),
                wxOK | wxICON_ERROR,
                this);
        }
        if (mlist["plugins"])
            mlist_plugins = mlist["plugins"].as< list<loot::Plugin> >();
    }

    progDia->Pulse();

    //Parse userlist.
    if (fs::exists(_game->UserlistPath())) {
        BOOST_LOG_TRIVIAL(debug) << "Parsing userlist.";
        YAML::Node ulist;
        try {
            loot::ifstream in(_game->UserlistPath());
            ulist = YAML::Load(in);
            in.close();
        } catch (YAML::ParserException& e) {
            BOOST_LOG_TRIVIAL(error) << "Userlist parsing failed. " << e.what();
            wxMessageBox(
				FromUTF8(format(loc::translate("Error: Userlist parsing failed. %1%")) % e.what()),
				translate("LOOT: Error"),
				wxOK | wxICON_ERROR,
				this);
        }
        if (ulist["plugins"])
            ulist_plugins = ulist["plugins"].as< list<loot::Plugin> >();
    }

    progDia->Pulse();

    //Merge the masterlist down into the installed mods list.
    BOOST_LOG_TRIVIAL(debug) << "Merging the masterlist down into the installed mods list.";
    for (const auto &plugin: mlist_plugins) {
        auto pos = find(installed.begin(), installed.end(), plugin);

        if (pos != installed.end())
            pos->MergeMetadata(plugin);
    }

    progDia->Pulse();

    //Add empty entries for any userlist entries that aren't installed.
    BOOST_LOG_TRIVIAL(debug) << "Padding the installed mods list to match the plugins in the userlist.";
    for (const auto &plugin : ulist_plugins) {
        if (find(installed.begin(), installed.end(), plugin) == installed.end())
            installed.push_back(loot::Plugin(plugin.Name()));
    }

    progDia->Pulse();

    //Set language.
    unsigned int lang;
    if (_settings["Language"])
        lang = Language(_settings["Language"].as<string>()).Code();
    else
        lang = loot::Language::any;

    //Load window size/pos settings.
    wxSize size = wxDefaultSize;
    wxPoint pos = wxDefaultPosition;
    if (_settings["windows"] && _settings["windows"]["editor"]) {
        YAML::Node node = _settings["windows"]["editor"];
        if (node["width"] && node["height"] && node["xPos"] && node["yPos"]) {
            size = wxSize(node["width"].as<int>(), node["height"].as<int>());
            pos = wxPoint(node["xPos"].as<int>(), node["yPos"].as<int>());
        }
    }

    //Create editor window.
    BOOST_LOG_TRIVIAL(debug) << "Opening editor window.";
    FullEditor *editor = new FullEditor(this, translate("LOOT: Metadata Editor"), pos, size, _game->UserlistPath().string(), installed, ulist_plugins, lang, *_game, _settings);

    progDia->Destroy();

	editor->Show();
    BOOST_LOG_TRIVIAL(debug) << "Editor window opened.";
}

void Launcher::OnRedatePlugins(wxCommandEvent& event) {
    wxMessageDialog * dia = new wxMessageDialog(this, translate("This feature is provided so that modders using the Creation Kit may set the load order it uses. A side-effect is that any subscribed Steam Workshop mods will be re-downloaded by Steam. Do you wish to continue?"), translate("LOOT: Warning"), wxYES_NO|wxCANCEL|wxICON_EXCLAMATION);

    if (dia->ShowModal() == wxID_YES) {
        BOOST_LOG_TRIVIAL(debug) << "Redating plugins.";
        try {
            _game->RedatePlugins();
        } catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << "Failed to redate plugins. " << e.what();
            wxMessageBox(
                FromUTF8(format(loc::translate("Error: Failed to redate plugins. %1%")) % e.what()),
                translate("LOOT: Error"),
                wxOK | wxICON_ERROR,
                this);
        }

        wxMessageBox(
            translate("Plugins were successfully redated."),
            translate("LOOT: Plugin Redate"),
            wxOK|wxCENTRE,
            this);
    }
}
