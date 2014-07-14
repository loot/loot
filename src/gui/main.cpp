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
#include "../backend/streams.h"

#include <algorithm>
#include <clocale>

#include <boost/filesystem.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
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
#include <wx/cmdline.h>

wxIMPLEMENT_APP(LOOT);

using namespace loot;
using namespace std;
using boost::format;

namespace fs = boost::filesystem;
namespace loc = boost::locale;

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

    size_t gameIndex(0);
    try {
        gameIndex = SelectGame(_settings, _games, target);
    }
    catch (exception &) {
        BOOST_LOG_TRIVIAL(error) << "None of the supported games were detected.";
        wxMessageBox(
            translate("Error: None of the supported games were detected."),
            translate("LOOT: Error"),
            wxOK | wxICON_ERROR,
            nullptr);
        return false;
    }
    BOOST_LOG_TRIVIAL(debug) << "Game selected is " << _games[gameIndex].Name();

    //Now that game is selected, initialise it.
    BOOST_LOG_TRIVIAL(debug) << "Initialising game-specific settings.";
    try {
        _games[gameIndex].Init();
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

            // Now check that those values are sensible given current screen dimensions.
            int screenX = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
            int screenY = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
            BOOST_LOG_TRIVIAL(trace) << "Current screen dimensions: " << screenX << "x" << screenY;

            if (size.GetHeight() > screenY || size.GetWidth() > screenX) {
                BOOST_LOG_TRIVIAL(trace) << "Previous window size is larger than primary screeen, using default size.";
                size = wxDefaultSize;
            }
            if (pos.x < 0 || pos.x > screenX || pos.y < 0 || pos.y > screenY) {
                BOOST_LOG_TRIVIAL(trace) << "Previous window position lies outside primary screeen, using default position.";
                pos = wxDefaultPosition;
            }
        }
    }

    //Create launcher window.
    BOOST_LOG_TRIVIAL(debug) << "Opening the main LOOT window.";
    Launcher * launcher = new Launcher(wxT("LOOT"), _settings, _games, gameIndex, pos, size);

    launcher->SetIcon(wxIconLocation("LOOT.exe"));
	launcher->Show();
	SetTopWindow(launcher);

    return true;
}

Launcher::Launcher(const wxChar *title, YAML::Node& settings, vector<loot::Game>& games, size_t currentGame, wxPoint pos, wxSize size) : wxFrame(nullptr, wxID_ANY, title, pos, size), _settings(settings), _games(games), _currentGame(currentGame) {

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
        if (_games[_currentGame] == _games[i])
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

    if (_games[_currentGame].Id() == loot::Game::tes5)
        RedatePluginsItem->Enable(true);
    else
        RedatePluginsItem->Enable(false);

    //Set title bar text.
    SetTitle(FromUTF8("LOOT - " + _games[_currentGame].Name()));

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
    _settings["Last Game"] = _games[_currentGame].FolderName();
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
        GetWindowSizePos(_settings["windows"]["viewer"], pos, size);
    }
    //Create viewer window.
    BOOST_LOG_TRIVIAL(debug) << "Opening viewer window...";
    Viewer *viewer = new Viewer(this, translate("LOOT: Report Viewer"), FromUTF8(ToFileURL(g_path_report.string() + "?data=" + _games[_currentGame].ReportDataPath().string())), pos, size, _settings);
    viewer->Show();
    BOOST_LOG_TRIVIAL(debug) << "Report displayed.";
}

void Launcher::OnOpenSettings(wxCommandEvent& event) {
    BOOST_LOG_TRIVIAL(debug) << "Opening settings window...";

    //Load window size/pos settings.
    wxSize size = wxDefaultSize;
    wxPoint pos = wxDefaultPosition;
    if (_settings["windows"] && _settings["windows"]["settings"]) {
        GetWindowSizePos(_settings["windows"]["settings"], pos, size);
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
    _currentGame = event.GetId() - MENU_LowestDynamicGameID;
    try {
        _games[_currentGame].Init();  //In case it hasn't already been done.
        BOOST_LOG_TRIVIAL(debug) << "New game is " << _games[_currentGame].Name();
    }
    catch (std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "Game-specific settings could not be initialised." << e.what();
        wxMessageBox(
            FromUTF8(format(loc::translate("Error: Game-specific settings could not be initialised. %1%")) % e.what()),
            translate("LOOT: Error"),
            wxOK | wxICON_ERROR,
            nullptr);
    }
    SetTitle(FromUTF8("LOOT - " + _games[_currentGame].Name()));
    if (_games[_currentGame].Id() == loot::Game::tes5)
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

    list<loot::Message> messages;
    unsigned int lang;

    wxProgressDialog *progDia = new wxProgressDialog(translate("LOOT: Working..."), translate("LOOT working..."), 1000, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_ELAPSED_TIME);

    function<void(const std::string&)> progressCallback([progDia](const std::string& message) {
        progDia->Pulse(FromUTF8(message));
    });

    //Set language.
    if (_settings["Language"])
        lang = Language(_settings["Language"].as<string>()).Code();
    else
        lang = loot::Language::any;

    BOOST_LOG_TRIVIAL(info) << "Using message language: " << Language(lang).Name();

    ///////////////////////////////////////////////////////
    // Load Plugins & Lists
    ///////////////////////////////////////////////////////

    _games[_currentGame].SortPrep(lang, messages, progressCallback);

    ///////////////////////////////////////////////////////
    // Build Graph Edges & Sort
    ///////////////////////////////////////////////////////

    /* Need to loop this section. There are 3 ways to exit the loop:

    1. Accept the load order at the preview with no changes.
    2. Cancel at the preview.
    3. Cancel making changes.

    Otherwise, the sorting must loop.

    */

    list<loot::Plugin> plugins;
    try {
        bool applyLoadOrder = false;

        do {
            
            // Perform sort.
            plugins = _games[_currentGame].Sort(lang, messages, progressCallback);

            progDia->Destroy();
            progDia = nullptr;

            BOOST_LOG_TRIVIAL(info) << "Displaying load order preview.";

            //Load window size/pos settings.
            wxSize size = wxDefaultSize;
            wxPoint pos = wxDefaultPosition;
            if (_settings["windows"] && _settings["windows"]["editor"]) {
                GetWindowSizePos(_settings["windows"]["editor"], pos, size);
            }

            // Display mini editor.
            MiniEditor editor(this, translate("LOOT: Calculated Load Order"), pos, size, plugins, _games[_currentGame].userlist.plugins, _games[_currentGame]);

            long ret = editor.ShowModal();
            MetadataList newUserlist = editor.GetNewUserlist();

            //Record window settings.
            YAML::Node node;
            node["height"] = editor.GetSize().GetHeight();
            node["width"] = editor.GetSize().GetWidth();
            node["xPos"] = editor.GetPosition().x;
            node["yPos"] = editor.GetPosition().y;

            _settings["windows"]["editor"] = node;

            if (ret != wxID_APPLY) {
                applyLoadOrder = false;
                break;
            }
            else if (_games[_currentGame].userlist == newUserlist) {
                applyLoadOrder = true;
                break;
            }
            else {
                //Recreate progress dialog.
                progDia = new wxProgressDialog(translate("LOOT: Working..."), translate("Recalculating load order..."), 1000, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_ELAPSED_TIME);

                //User accepted edits, now apply them, then loop.
                _games[_currentGame].userlist = newUserlist;

                //Save edits to userlist.
                _games[_currentGame].userlist.Save(_games[_currentGame].UserlistPath());

                //Now loop.
            }
        } while (true);

        if (applyLoadOrder) {
            //Applying the load order.
            BOOST_LOG_TRIVIAL(debug) << "Setting load order.";
            try {
                _games[_currentGame].SetLoadOrder(plugins);
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
        GenerateReportData(_games[_currentGame],
                        messages,
                        plugins,
                        _games[_currentGame].masterlist.GetRevision(_games[_currentGame].MasterlistPath()),
                        _games[_currentGame].masterlist.GetDate(_games[_currentGame].MasterlistPath()),
                        true);
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
        GetWindowSizePos(_settings["windows"]["viewer"], pos, size);
    }

    //Create viewer window.
    Viewer *viewer = new Viewer(this, translate("LOOT: Report Viewer"), FromUTF8(ToFileURL(g_path_report.string() + "?data=" + _games[_currentGame].ReportDataPath().string())), pos, size, _settings);
    viewer->Show();

    BOOST_LOG_TRIVIAL(debug) << "Report display successful. Sorting process complete.";
}

void Launcher::OnEditMetadata(wxCommandEvent& event) {

    //Should probably check for masterlist updates before opening metadata editor.
    list<loot::Plugin> installed;
    unsigned int lang;

    wxProgressDialog *progDia = new wxProgressDialog(translate("LOOT: Working..."), translate("LOOT working..."), 1000, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_ELAPSED_TIME);

    //Set language.
    if (_settings["Language"])
        lang = Language(_settings["Language"].as<string>()).Code();
    else
        lang = loot::Language::any;

    //Scan for installed plugins.
    BOOST_LOG_TRIVIAL(debug) << "Reading installed plugins' headers.";
    _games[_currentGame].LoadPlugins(true);
    //Sort plugins into their load order.
    list<string> loadOrder;
    _games[_currentGame].GetLoadOrder(loadOrder);
    for (const auto &pluginName: loadOrder) {
        const auto pos = _games[_currentGame].plugins.find(pluginName);

        if (pos != _games[_currentGame].plugins.end())
            installed.push_back(pos->second);
    }

    //Parse masterlist.
    if (fs::exists(_games[_currentGame].MasterlistPath())) {
        BOOST_LOG_TRIVIAL(debug) << "Parsing masterlist.";
        _games[_currentGame].masterlist.Load(_games[_currentGame], lang);
    }

    progDia->Pulse();

    //Parse userlist.
    if (fs::exists(_games[_currentGame].UserlistPath())) {
        BOOST_LOG_TRIVIAL(debug) << "Parsing userlist.";
        _games[_currentGame].userlist.Load(_games[_currentGame].UserlistPath());
    }

    progDia->Pulse();

    //Merge the masterlist down into the installed mods list.
    BOOST_LOG_TRIVIAL(debug) << "Merging the masterlist down into the installed mods list.";
    for (const auto &plugin: _games[_currentGame].masterlist.plugins) {
        auto pos = find(installed.begin(), installed.end(), plugin);

        if (pos != installed.end())
            pos->MergeMetadata(plugin);
    }

    progDia->Pulse();

    //Add empty entries for any userlist entries that aren't installed.
    BOOST_LOG_TRIVIAL(debug) << "Padding the installed mods list to match the plugins in the userlist.";
    for (const auto &plugin : _games[_currentGame].userlist.plugins) {
        if (find(installed.begin(), installed.end(), plugin) == installed.end())
            installed.push_back(loot::Plugin(plugin.Name()));
    }

    progDia->Pulse();

    //Load window size/pos settings.
    wxSize size = wxDefaultSize;
    wxPoint pos = wxDefaultPosition;
    if (_settings["windows"] && _settings["windows"]["editor"]) {
        GetWindowSizePos(_settings["windows"]["editor"], pos, size);
    }

    //Create editor window.
    BOOST_LOG_TRIVIAL(debug) << "Opening editor window.";
    FullEditor *editor = new FullEditor(this, translate("LOOT: Metadata Editor"), pos, size, _games[_currentGame].UserlistPath().string(), installed, _games[_currentGame].userlist.plugins, lang, _games[_currentGame], _settings);

    progDia->Destroy();

	editor->Show();
    BOOST_LOG_TRIVIAL(debug) << "Editor window opened.";
}

void Launcher::OnRedatePlugins(wxCommandEvent& event) {
    wxMessageDialog * dia = new wxMessageDialog(this, translate("This feature is provided so that modders using the Creation Kit may set the load order it uses. A side-effect is that any subscribed Steam Workshop mods will be re-downloaded by Steam. Do you wish to continue?"), translate("LOOT: Warning"), wxYES_NO|wxCANCEL|wxICON_EXCLAMATION);

    if (dia->ShowModal() == wxID_YES) {
        BOOST_LOG_TRIVIAL(debug) << "Redating plugins.";
        try {
            _games[_currentGame].RedatePlugins();
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

void Launcher::GetWindowSizePos(const YAML::Node& node, wxPoint& pos, wxSize& size) {
    if (node["width"] && node["height"] && node["xPos"] && node["yPos"]) {
        size = wxSize(node["width"].as<int>(), node["height"].as<int>());
        pos = wxPoint(node["xPos"].as<int>(), node["yPos"].as<int>());

        // Now check that those values are sensible given current screen dimensions.
        int screenX = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
        int screenY = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
        BOOST_LOG_TRIVIAL(trace) << "Current screen dimensions: " << screenX << "x" << screenY;

        if (size.GetHeight() > screenY || size.GetWidth() > screenX) {
            BOOST_LOG_TRIVIAL(trace) << "Previous window size is larger than primary screeen, using default size.";
            size = wxDefaultSize;
        }
        if (pos.x < 0 || pos.x > screenX || pos.y < 0 || pos.y > screenY) {
            BOOST_LOG_TRIVIAL(trace) << "Previous window position lies outside primary screeen, using default position.";
            pos = wxDefaultPosition;
        }
    }
    else {
        size = wxDefaultSize;
        pos = wxDefaultPosition;
    }
}