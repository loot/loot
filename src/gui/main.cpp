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
#include <boost/thread/thread.hpp>
#include <boost/unordered_map.hpp>

#include <wx/snglinst.h>
#include <wx/aboutdlg.h>
#include <wx/progdlg.h>

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
    plugin_list_loader(PluginGraph& graph, loot::Game& game) : _graph(graph), _game(game) {}

    void operator () () {
        loot::vertex_it vit, vitend;
        for (boost::tie(vit, vitend) = boost::vertices(_graph); vit != vitend; ++vit) {
            if (skipPlugins.find(_graph[*vit].Name()) == skipPlugins.end()) {
                _graph[*vit] = loot::Plugin(_game, _graph[*vit].Name(), false);
            }
        }
    }

    PluginGraph& _graph;
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
            } catch (loot::error& e) {
                _plugins.clear();
                _messages.clear();
                BOOST_LOG_TRIVIAL(error) << "Masterlist update failed. Details: " << e.what();
                _errors.push_back(loot::Message(loot::Message::error, (format(loc::translate("Masterlist update failed. Details: %1%")) % e.what()).str()));
                //Try getting masterlist revision anyway.
                try {
                    _revision = GetMasterlistRevision(_game);
                }
                catch (loot::error& e) {
                    BOOST_LOG_TRIVIAL(error) << "Masterlist revision check failed. Details: " << e.what();
                    _errors.push_back(loot::Message(loot::Message::error, (format(loc::translate("Masterlist revision check failed. Details: %1%")) % e.what()).str()));
                }
            }
        }
        else {
            BOOST_LOG_TRIVIAL(debug) << "Getting masterlist revision";
            try {
                _revision = GetMasterlistRevision(_game);
            }
            catch (loot::error& e) {
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
				translate("Error: Could not create local app data LOOT folder."),
				translate("LOOT: Error"),
				wxOK | wxICON_ERROR,
				NULL);
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
			NULL);
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


    //Set the locale to get encoding and language conversions working correctly.
    BOOST_LOG_TRIVIAL(debug) << "Initialising language settings.";
    //Defaults in case language string is empty or setting is missing.
    string localeId = loot::Language(loot::Language::any).Locale() + ".UTF-8";
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
                NULL);
        }
    } else {
        wxLoc = new wxLocale(wxLANGUAGE_ENGLISH);

        BOOST_LOG_TRIVIAL(error) << "The selected language is not available on this system.";
        wxMessageBox(
            translate("Error: The selected language is not available on this system."),
            translate("LOOT: Error"),
            wxOK | wxICON_ERROR,
            NULL);
    }

    //Detect installed games.
    BOOST_LOG_TRIVIAL(debug) << "Detecting installed games.";
    try {
        _games = GetGames(_settings);
    } catch (loot::error& e) {
        BOOST_LOG_TRIVIAL(error) << "Game-specific settings could not be initialised. " << e.what();
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: Game-specific settings could not be initialised. %1%")) % e.what()),
            translate("LOOT: Error"),
            wxOK | wxICON_ERROR,
            NULL);
        return false;
    } catch (YAML::Exception& e) {
        BOOST_LOG_TRIVIAL(error) << "Games' settings parsing failed. " << e.what();
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: Games' settings parsing failed. %1%")) % e.what()),
            translate("LOOT: Error"),
            wxOK | wxICON_ERROR,
            NULL);
        return false;
    }

    BOOST_LOG_TRIVIAL(debug) << "Selecting game.";
    string target;
    int gameIndex = -1;
    if (_settings["Game"] && _settings["Game"].as<string>() != "auto")
        target = _settings["Game"].as<string>();
    else if (_settings["Last Game"] && _settings["Last Game"].as<string>() != "auto")
        target = _settings["Last Game"].as<string>();

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
                NULL);
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
    } catch (loot::error& e) {
        BOOST_LOG_TRIVIAL(error) << "Game-specific settings could not be initialised. " << e.what();
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: Game-specific settings could not be initialised. %1%")) % e.what()),
            translate("LOOT: Error"),
            wxOK | wxICON_ERROR,
            NULL);
        return false;
    }

    //Create launcher window.
    BOOST_LOG_TRIVIAL(debug) << "Opening the main LOOT window.";
    Launcher * launcher = new Launcher(wxT("LOOT"), _settings, &_game, _games);

    launcher->SetIcon(wxIconLocation("LOOT.exe"));
	launcher->Show();
	SetTopWindow(launcher);

    return true;
}

Launcher::Launcher(const wxChar *title, YAML::Node& settings, Game * game, vector<loot::Game>& games) : wxFrame(NULL, wxID_ANY, title), _game(game), _settings(settings), _games(games) {

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
    SetSize(wxSize(250, 200));

    SortButton->SetFocus();
}

//Called when the frame exits.
void Launcher::OnQuit(wxCommandEvent& event) {

	Close(true); // Tells the OS to quit running this process
}

void Launcher::OnClose(wxCloseEvent& event) {
    BOOST_LOG_TRIVIAL(debug) << "Quiting LOOT.";

    _settings["Last Game"] = _game->FolderName();

    _settings["Games"] = _games;

    //Save settings.
    BOOST_LOG_TRIVIAL(debug) << "Saving LOOT settings.";
    YAML::Emitter yout;
    yout.SetIndent(2);
    yout << _settings;

    loot::ofstream out(loot::g_path_settings);
    out << yout.c_str();
    out.close();

    Destroy();
}

void Launcher::OnViewLastReport(wxCommandEvent& event) {
    //Create viewer window.
    BOOST_LOG_TRIVIAL(debug) << "Opening viewer window...";
    Viewer *viewer = new Viewer(this, translate("LOOT: Report Viewer"), FromUTF8(ToFileURL(g_path_report.string() + "?data=" + _game->ReportDataPath().string())));
    viewer->Show();
    BOOST_LOG_TRIVIAL(debug) << "Report displayed.";
}

void Launcher::OnOpenSettings(wxCommandEvent& event) {
    BOOST_LOG_TRIVIAL(debug) << "Opening settings window...";
    SettingsFrame *settings = new SettingsFrame(this, translate("LOOT: Settings"), _settings, _games);
    settings->ShowModal();
    BOOST_LOG_TRIVIAL(debug) << "Settings window opened.";
}

void Launcher::OnGameChange(wxCommandEvent& event) {
    BOOST_LOG_TRIVIAL(debug) << "Changing current game...";
    _game = &_games[event.GetId() - MENU_LowestDynamicGameID];
    try {
        _game->Init();  //In case it hasn't already been done.
        BOOST_LOG_TRIVIAL(debug) << "New game is " << _game->Name();
    }
    catch (loot::error& e) {
        BOOST_LOG_TRIVIAL(error) << "Game-specific settings could not be initialised." << e.what();
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: Game-specific settings could not be initialised. %1%")) % e.what()),
            translate("LOOT: Error"),
            wxOK | wxICON_ERROR,
            NULL);
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
    aboutInfo.SetVersion(IntToString(g_version_major) + "." + IntToString(g_version_minor) + "." + IntToString(g_version_patch));
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
    loot::PluginGraph graph;
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
            plugins.push_back(loot::Plugin(it->path().filename().string()));  //Just in case there's an error with the graph.
        }
    }
    meanFileSize /= tempMap.size();

    //Now load plugins.
    plugin_list_loader pll(graph, *_game);
    for (boost::unordered_map<string, size_t>::const_iterator it=tempMap.begin(), endit=tempMap.end(); it != endit; ++it) {

        BOOST_LOG_TRIVIAL(trace) << "Found plugin: " << it->first;

        vertex_t v = boost::add_vertex(loot::Plugin(it->first), graph);

        if (it->second > meanFileSize) {
            pll.skipPlugins.insert(it->first);
            plugin_loader pl(graph[v], *_game);
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

    if (fs::exists(_game->MasterlistPath()) || fs::exists(_game->UserlistPath())) {

        //Set language.
        unsigned int lang;
        if (_settings["Language"])
            lang = Language(_settings["Language"].as<string>()).Code();
        else
            lang = loot::Language::any;


        //Merge all global message lists.
        BOOST_LOG_TRIVIAL(trace) << "Merging all global message lists.";
        if (!mlist_messages.empty())
            messages.insert(messages.end(), mlist_messages.begin(), mlist_messages.end());
        if (!ulist_messages.empty())
            messages.insert(messages.end(), ulist_messages.begin(), ulist_messages.end());

        //Evaluate any conditions in the global messages.
        BOOST_LOG_TRIVIAL(trace) << "Evaluating global message conditions.";
        try {
            list<loot::Message>::iterator it=messages.begin();
            while (it != messages.end()) {
                if (!it->EvalCondition(*_game, lang))
                    it = messages.erase(it);
                else
                    ++it;
            }
        } catch (loot::error& e) {
            BOOST_LOG_TRIVIAL(error) << "A global message contains a condition that could not be evaluated. Details: " << e.what();
            messages.push_back(loot::Message(loot::Message::error, (format(loc::translate("A global message contains a condition that could not be evaluated. Details: %1%")) % e.what()).str()));
        }

        //Merge plugin list, masterlist and userlist plugin data.
        BOOST_LOG_TRIVIAL(debug) << "Merging plugin list, masterlist and userlist data, evaluating conditions and checking for install validity.";
        loot::vertex_it vit, vitend;
        for (boost::tie(vit, vitend) = boost::vertices(graph); vit != vitend; ++vit) {
            BOOST_LOG_TRIVIAL(trace) << "Merging for plugin \"" << graph[*vit].Name() << "\"";

            //Check if there is a plugin entry in the masterlist. This will also find matching regex entries.
            list<loot::Plugin>::iterator pos = std::find(mlist_plugins.begin(), mlist_plugins.end(), graph[*vit]);

            if (pos != mlist_plugins.end()) {
                BOOST_LOG_TRIVIAL(trace) << "Merging masterlist data down to plugin list data.";
                graph[*vit].MergeMetadata(*pos);
            }

            //Check if there is a plugin entry in the userlist. This will also find matching regex entries.
            pos = std::find(ulist_plugins.begin(), ulist_plugins.end(), graph[*vit]);

            if (pos != ulist_plugins.end() && pos->Enabled()) {
                BOOST_LOG_TRIVIAL(trace) << "Merging userlist data down to plugin list data.";
                graph[*vit].MergeMetadata(*pos);
            }

            progDia->Pulse();

            //Now that items are merged, evaluate any conditions they have.
            BOOST_LOG_TRIVIAL(trace) << "Evaluate conditions for merged plugin data.";
            try {
                graph[*vit].EvalAllConditions(*_game, lang);
            } catch (loot::error& e) {
                BOOST_LOG_TRIVIAL(error) << "\"" << graph[*vit].Name() << "\" contains a condition that could not be evaluated. Details: " << e.what();
                messages.push_back(loot::Message(loot::Message::error, (format(loc::translate("\"%1%\" contains a condition that could not be evaluated. Details: %2%")) % graph[*vit].Name() % e.what()).str()));
            }

            progDia->Pulse();

            //Also check install validity.
            BOOST_LOG_TRIVIAL(trace) << "Checking that the current install is valid according to this plugin's data.";
            graph[*vit].CheckInstallValidity(*_game);

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
        long ret;
        do {
            BOOST_LOG_TRIVIAL(debug) << "Building the plugin dependency graph...";

            //Now add the interactions between plugins to the graph as edges.
            BOOST_LOG_TRIVIAL(trace) << "Adding non-overlap edges.";
            AddSpecificEdges(graph);

            BOOST_LOG_TRIVIAL(trace) << "Adding priority edges.";
            AddPriorityEdges(graph);

            BOOST_LOG_TRIVIAL(trace) << "Adding overlap edges.";
            AddOverlapEdges(graph);

            BOOST_LOG_TRIVIAL(debug) << "Checking to see if the graph is cyclic.";
            loot::CheckForCycles(graph);

            progDia->Pulse();

            BOOST_LOG_TRIVIAL(debug) << "Performing a topological sort.";
            loot::Sort(graph, plugins);

            progDia->Destroy();
            progDia = NULL;

            BOOST_LOG_TRIVIAL(debug) << "Displaying load order preview.";
            MiniEditor editor(this, translate("LOOT: Calculated Load Order"), plugins, *_game);

            long ret = editor.ShowModal();
            const std::list<loot::Plugin> edits = editor.GetEditedPlugins();

            if (ret != wxID_APPLY) {
                applyLoadOrder = false;
                break;
            }
            else if (edits.empty()) {
                applyLoadOrder = true;
                break;
            }
            else {
                //Recreate progress dialog.
                progDia = new wxProgressDialog(translate("LOOT: Working..."), translate("Recalculating load order..."), 1000, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_ELAPSED_TIME);

                //User accepted edits, now apply them, then loop.
                //Apply edits to the graph vertices.
                loot::vertex_it vit, vitend;
                for (boost::tie(vit, vitend) = boost::vertices(graph); vit != vitend; ++vit) {
                    std::list<loot::Plugin>::const_iterator pos = std::find(edits.begin(), edits.end(), graph[*vit]);

                    if (pos != edits.end()) {
                        BOOST_LOG_TRIVIAL(trace) << "Merging edits down to plugin list data.";
                        graph[*vit].MergeMetadata(*pos);
                    }
                }

                //Clear all existing edges from the graph.
                BOOST_LOG_TRIVIAL(trace) << "Clearing all existing edges from the plugin graph.";
                for (boost::tie(vit, vitend) = boost::vertices(graph); vit != vitend; ++vit) {
                    boost::clear_vertex(*vit, graph);
                }

                //Merge edits down to the userlist entries.
                BOOST_LOG_TRIVIAL(trace) << "Merging down edits to the userlist.";
                for (list<loot::Plugin>::const_iterator it = edits.begin(), endit = edits.end(); it != endit; ++it) {
                    list<loot::Plugin>::iterator jt = find(ulist_plugins.begin(), ulist_plugins.end(), *it);

                    if (jt != ulist_plugins.end()) {
                        jt->MergeMetadata(*it);
                    }
                    else {
                        ulist_plugins.push_back(*it);
                    }
                }

                //Save edits to userlist.
                BOOST_LOG_TRIVIAL(debug) << "Saving edited userlist.";
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
            //User doesn't want to make any changes. Just go straight to applying the load order.
            BOOST_LOG_TRIVIAL(debug) << "Setting load order.";
            try {
                _game->SetLoadOrder(plugins);
            }
            catch (loot::error& e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to set the load order. Details: " << e.what();
                messages.push_back(loot::Message(loot::Message::error, (format(loc::translate("Failed to set the load order. Details: %1%")) % e.what()).str()));
            }
            BOOST_LOG_TRIVIAL(trace) << "Load order set:";
            for (list<loot::Plugin>::iterator it = plugins.begin(), endIt = plugins.end(); it != endIt; ++it) {
                BOOST_LOG_TRIVIAL(trace) << '\t' << it->Name();
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
        progDia = NULL;
    }

    ///////////////////////////////////////////////////////
    // Build & Display Report
    ///////////////////////////////////////////////////////

    BOOST_LOG_TRIVIAL(debug) << "Generating report...";
    try {
        GenerateReport(_game->ReportDataPath(),
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
        if (progDia != NULL)
            progDia->Destroy();
        return;
    }

    //Now a results report definitely exists.
    ViewButton->Enable(true);

    BOOST_LOG_TRIVIAL(debug) << "Displaying report...";
    //Create viewer window.
    Viewer *viewer = new Viewer(this, translate("LOOT: Report Viewer"), FromUTF8(ToFileURL(g_path_report.string() + "?data=" + _game->ReportDataPath().string())));
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
    for (list<string>::const_iterator it = loadOrder.begin(), itend = loadOrder.end(); it != itend; ++it) {
        boost::unordered_map<string, loot::Plugin>::const_iterator pos = _game->plugins.find(*it);

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
    for (list<loot::Plugin>::const_iterator it = mlist_plugins.begin(), endit = mlist_plugins.end(); it != endit; ++it) {
        list<loot::Plugin>::iterator pos = find(installed.begin(), installed.end(), *it);

        if (pos != installed.end())
            pos->MergeMetadata(*it);
    }

    progDia->Pulse();

    //Add empty entries for any userlist entries that aren't installed.
    BOOST_LOG_TRIVIAL(debug) << "Padding the installed mods list to match the plugins in the userlist.";
    for (list<loot::Plugin>::const_iterator it = ulist_plugins.begin(), endit = ulist_plugins.end(); it != endit; ++it) {
        if (find(installed.begin(), installed.end(), *it) == installed.end())
            installed.push_back(loot::Plugin(it->Name()));
    }

    progDia->Pulse();

    //Set language.
    unsigned int lang;
    if (_settings["Language"])
        lang = Language(_settings["Language"].as<string>()).Code();
    else
        lang = loot::Language::any;

    //Create editor window.
    BOOST_LOG_TRIVIAL(debug) << "Opening editor window.";
    Editor *editor = new Editor(this, translate("LOOT: Metadata Editor"), _game->UserlistPath().string(), installed, ulist_plugins, lang, *_game);

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
