/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2014    WrinklyNinja

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

#include "handler.h"
#include "resource.h"
#include "app.h"

#include "../backend/json.h"
#include "../backend/parsers.h"
#include "../backend/generators.h"

#include <include/cef_app.h>
#include <include/cef_runnable.h>
#include <include/cef_task.h>

#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>

#include <sstream>
#include <string>
#include <cassert>

using namespace std;

using boost::format;

namespace fs = boost::filesystem;
namespace loc = boost::locale;

namespace loot {

    namespace {
        LootHandler * g_instance = NULL;
    }

    // WinHandler methods
    //-------------------

    Handler::Handler() {}

    // Called due to cefQuery execution in binding.html.
    bool Handler::OnQuery(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            int64 query_id,
                            const CefString& request,
                            bool persistent,
                            CefRefPtr<Callback> callback) {

        if (request == "openReadme") {
            try {
                OpenReadme();
                callback->Success("");
            }
            catch (error &e) {
                BOOST_LOG_TRIVIAL(error) << e.what();
                callback->Failure(e.code(), e.what());
            }
            catch (exception &e) {
                BOOST_LOG_TRIVIAL(error) << e.what();
                callback->Failure(-1, e.what());
            }
            return true;
        }
        else if (request == "openLogLocation") {
            try {
                OpenLogLocation();
                callback->Success("");
            }
            catch (error &e) {
                BOOST_LOG_TRIVIAL(error) << e.what();
                callback->Failure(e.code(), e.what());
            }
            catch (exception &e) {
                BOOST_LOG_TRIVIAL(error) << e.what();
                callback->Failure(-1, e.what());
            }
            return true;
        }
        else if (request == "getVersion") {
            callback->Success(GetVersion());
            return true;
        }
        else if (request == "getSettings") {
            callback->Success(GetSettings());
            return true;
        }
        else if (request == "getLanguages") {
            callback->Success(GetLanguages());
            return true;
        }
        else if (request == "getGameTypes") {
            callback->Success(GetGameTypes());
            return true;
        }
        else if (request == "getInstalledGames") {
            callback->Success(GetInstalledGames());
            return true;
        }
        else if (request == "getGameData") {
            BOOST_LOG_TRIVIAL(info) << "Setting LOOT window title bar text to include game name: " << g_app_state.CurrentGame().Name();
            HWND handle = browser->GetHost()->GetWindowHandle();
#ifdef _WIN32
            SetWindowText(handle, ToWinWide("LOOT: " + g_app_state.CurrentGame().Name()).c_str());
#endif
            try {
                callback->Success(GetGameData());
            }
            catch (error &e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to get game data. " << e.what();
                callback->Failure(e.code(), e.what());
            }
            catch (exception &e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to get game data. " << e.what();
                callback->Failure(-1, e.what());
            }
            return true;
        }
        else if (request == "cancelFind") {
            browser->GetHost()->StopFinding(true);
            callback->Success("");
            return true;
        }
        else if (request == "clearAllMetadata") {
            callback->Success(ClearAllMetadata());
            return true;
        }
        else if (request == "redatePlugins") {
            BOOST_LOG_TRIVIAL(debug) << "Redating plugins.";
            try {
                g_app_state.CurrentGame().RedatePlugins();
                callback->Success("");
            }
            catch (error &e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to redate plugins. " << e.what();
                callback->Failure(e.code(), e.what());
            }
            catch (exception &e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to redate plugins. " << e.what();
                callback->Failure(-1, e.what());
            }

            return true;
        }
        else if (request == "updateMasterlist") {
            try {
                callback->Success(UpdateMasterlist());
            }
            catch (error &e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to update the masterlist. " << e.what();
                callback->Failure(e.code(), e.what());
            }
            catch (exception &e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to update the masterlist. " << e.what();
                callback->Failure(-1, e.what());
            }
            return true;
        }
        else if (request == "sortPlugins") {
            callback->Success(SortPlugins());
            return true;
        }
        else {
            // May be a request with arguments.
            YAML::Node req;
            try {
                req = JSON::parse(request.ToString());
            }
            catch (exception &e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to parse CEF query request \"" << request << "\": " << e.what();
                callback->Failure(-1, e.what());
                return true;
            }

            return HandleComplexQuery(browser, req, callback);
        }

        return false;
    }

    // Handle queries with input arguments.
    bool Handler::HandleComplexQuery(CefRefPtr<CefBrowser> browser, YAML::Node& request,
        CefRefPtr<Callback> callback) {


        const string requestName = request["name"].as<string>();

        if (requestName == "find") {
            // Has one arg, which is the search string.
            Find(browser, request["args"][0].as<string>());
            callback->Success("");
            return true;
        }
        else if (requestName == "changeGame") {
            try {
                // Has one arg, which is the folder name of the new game.
                g_app_state.ChangeGame(request["args"][0].as<string>());

                BOOST_LOG_TRIVIAL(info) << "Setting LOOT window title bar text to include game name: " << g_app_state.CurrentGame().Name();
                HWND handle = browser->GetHost()->GetWindowHandle();
#ifdef _WIN32
                SetWindowText(handle, ToWinWide("LOOT: " + g_app_state.CurrentGame().Name()).c_str());
#endif

                callback->Success(GetGameData());
            }
            catch (loot::error &e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to change game. Details: " << e.what();
                callback->Failure(e.code(), string("Failed to change game. Details: ") + e.what());
            }
            catch (std::exception& e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to change game. Details: " << e.what();
                callback->Failure(-1, string("Failed to change game. Details: ") + e.what());
            }
            return true;
        }
        else if (requestName == "getConflictingPlugins") {
            // Has one arg, which is the name of the plugin to get conflicts for.
            callback->Success(GetConflictingPlugins(request["args"][0].as<string>()));
            return true;
        }
        else if (requestName == "copyMetadata") {
            // Has one arg, which is the name of the plugin to copy metadata for.
            try {
                CopyMetadata(request["args"][0].as<string>());
                callback->Success("");
            }
            catch (loot::error &e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to copy plugin metadata. Details: " << e.what();
                callback->Failure(e.code(), string("Failed to copy plugin metadata. Details: ") + e.what());
            }
            catch (std::exception& e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to copy plugin metadata. Details: " << e.what();
                callback->Failure(-1, string("Failed to copy plugin metadata. Details: ") + e.what());
            }
            return true;
        }
        else if (requestName == "clearPluginMetadata") {
            // Has one arg, which is the name of the plugin to copy metadata for.
            callback->Success(ClearPluginMetadata(request["args"][0].as<string>()));
            return true;
        }
        else if (requestName == "editorClosed") {
            BOOST_LOG_TRIVIAL(debug) << "Editor for plugin closed.";
            // One argument, which is the plugin metadata that has changed (+ its name).
            callback->Success(ApplyUserEdits(request["args"][0]));
            return true;
        }
        else if (requestName == "closeSettings") {
            BOOST_LOG_TRIVIAL(trace) << "Settings dialog closed and changes accepted, updating settings object.";
            g_app_state.UpdateSettings(request["args"][0]);
            callback->Success("");
            return true;
        }
        else if (requestName == "applySort") {
            BOOST_LOG_TRIVIAL(trace) << "User has accepted sorted load order, applying it.";
            try {
                g_app_state.CurrentGame().SetLoadOrder(request["args"][0].as<list<string>>());
                callback->Success("");
            }
            catch (error &e) {
                BOOST_LOG_TRIVIAL(error) << e.what();
                callback->Failure(e.code(), e.what());
            }
            catch (exception &e) {
                BOOST_LOG_TRIVIAL(error) << e.what();
                callback->Failure(-1, e.what());
            }

            return true;
        }
        else if (requestName == "saveFilterState") {
            const string id = request["args"][0].as<string>();
            const string value = request["args"][1].as<string>();
            SaveFilterState(id, value);
            callback->Success("");
            return true;
        }
        return false;
    }

    void Handler::Find(CefRefPtr<CefBrowser> browser, const std::string& search) {
        // In case there is a search already running, cancel it.
        browser->GetHost()->StopFinding(true);

        // Only one search at a time is allowed, so give a constant identifier,
        // and we want case-insensitive forward searching, with no repeated
        // searches.
        browser->GetHost()->Find(0, search, true, false, false);
    }

    std::string Handler::GetConflictingPlugins(const std::string& pluginName) {
        BOOST_LOG_TRIVIAL(debug) << "Searching for plugins that conflict with " << pluginName;

        auto pluginIt = g_app_state.CurrentGame().plugins.find(boost::locale::to_lower(pluginName));

        // Checking for FormID overlap will only work if the plugins have been loaded, so check if
        // the first plugin has any FormIDs in memory, and if not load all plugins.
        if (g_app_state.CurrentGame().plugins.begin()->second.FormIDs().size() == 0)
            g_app_state.CurrentGame().LoadPlugins(false);

        map<string, uint32_t> conflictingPlugins;
        YAML::Node node;
        for (const auto& pluginPair : g_app_state.CurrentGame().plugins) {
            if (pluginIt != g_app_state.CurrentGame().plugins.end()) {
                if (pluginIt->second.DoFormIDsOverlap(pluginPair.second)) {
                    BOOST_LOG_TRIVIAL(debug) << "Found conflicting plugin: " << pluginPair.second.Name();
                    conflictingPlugins.emplace(pluginPair.second.Name(), pluginPair.second.Crc());
                    node["conflicts"].push_back(pluginPair.second.Name());
                }
            }
            node["crcs"][pluginPair.second.Name()] = pluginPair.second.Crc();
        }

        if (node.size() > 0)
            return JSON::stringify(node);
        else
            return "null";
    }

    void Handler::CopyMetadata(const std::string& pluginName) {
        BOOST_LOG_TRIVIAL(debug) << "Copying metadata for plugin " << pluginName;

        // Get metadata from masterlist and userlist.
        Plugin plugin = g_app_state.CurrentGame().masterlist.FindPlugin(pluginName);
        plugin.MergeMetadata(g_app_state.CurrentGame().userlist.FindPlugin(pluginName));

        // Generate text representation.
        string text;
        if (plugin.HasNameOnly())
            text = "name: " + plugin.Name();
        else {
            YAML::Emitter yout;
            yout.SetIndent(2);
            yout << plugin;
            text = yout.c_str();
        }

#ifdef _WIN32
        if (!OpenClipboard(NULL)) {
            throw loot::error(loot::error::windows_error, "Failed to open the Windows clipboard.");
        }

        if (!EmptyClipboard()) {
            throw loot::error(loot::error::windows_error, "Failed to empty the Windows clipboard.");
        }

        // The clipboard takes a Unicode (ie. UTF-16) string that it then owns and must not
        // be destroyed by LOOT. Convert the string, then copy it into a new block of 
        // memory for the clipboard.
        wstring wtext = ToWinWide(text);
        wchar_t * wcstr = new wchar_t[wtext.length() + 1];
        wcscpy(wcstr, wtext.c_str());

        if (SetClipboardData(CF_UNICODETEXT, wcstr) == NULL) {
            throw loot::error(loot::error::windows_error, "Failed to copy metadata to the Windows clipboard.");
        }

        if (!CloseClipboard()) {
            throw loot::error(loot::error::windows_error, "Failed to close the Windows clipboard.");
        }
#endif

        BOOST_LOG_TRIVIAL(info) << "Exported userlist metadata text for \"" << pluginName << "\": " << text;
    }

    std::string Handler::ClearPluginMetadata(const std::string& pluginName) {
        BOOST_LOG_TRIVIAL(debug) << "Clearing user metadata for plugin " << pluginName;

        auto ulistPluginIt = find(g_app_state.CurrentGame().userlist.plugins.begin(), g_app_state.CurrentGame().userlist.plugins.end(), Plugin(pluginName));

        if (ulistPluginIt != g_app_state.CurrentGame().userlist.plugins.end()) {
            g_app_state.CurrentGame().userlist.plugins.erase(ulistPluginIt);
        }

        // Now rederive the displayed metadata from the masterlist.
        YAML::Node derivedMetadata = GenerateDerivedMetadata(pluginName);
        if (derivedMetadata.size() > 0)
            return JSON::stringify(derivedMetadata);
        else
            return "null";
    }

    void Handler::SaveFilterState(const std::string& id, const std::string& value) {
        BOOST_LOG_TRIVIAL(trace) << "Saving state of filter " << id << " as " << value;
        YAML::Node settings = g_app_state.GetSettings();

        if (value == "true")
            settings["filters"][id] = true;
        else if (value == "false" || value.empty())
            settings["filters"].remove(id);
        else
            settings["filters"][id] = value;

        g_app_state.UpdateSettings(settings);
    }

    std::string Handler::ApplyUserEdits(const YAML::Node& pluginMetadata) {
        BOOST_LOG_TRIVIAL(trace) << "Applying user edits for: " << pluginMetadata["name"].as<string>();
        // Create new object for userlist entry.
        Plugin newUserlistEntry(pluginMetadata["name"].as<string>());

        // Find existing userlist entry.
        auto ulistPluginIt = find(g_app_state.CurrentGame().userlist.plugins.begin(), g_app_state.CurrentGame().userlist.plugins.end(), newUserlistEntry);

        // First sort out the priority value. This is only given if it was changed.
        BOOST_LOG_TRIVIAL(trace) << "Calculating userlist metadata priority value from Javascript variables.";
        if (pluginMetadata["modPriority"] && pluginMetadata["isGlobalPriority"]) {
            BOOST_LOG_TRIVIAL(trace) << "Priority value was changed, recalculating...";
            // Priority value was changed, so add it to the userlist data.
            int priority = pluginMetadata["modPriority"].as<int>();

            if (priority >= 0) {
                priority += max_priority;
            }
            else {
                priority -= max_priority;
            }
            newUserlistEntry.Priority(priority);
            newUserlistEntry.SetPriorityExplicit(true);
        }
        else {
            // Priority value wasn't changed, use the existing userlist value.
            BOOST_LOG_TRIVIAL(trace) << "Priority value is unchanged, using existing userlist value (if it exists).";
            if (ulistPluginIt != g_app_state.CurrentGame().userlist.plugins.end()) {
                newUserlistEntry.Priority(ulistPluginIt->Priority());
                newUserlistEntry.SetPriorityExplicit(ulistPluginIt->IsPriorityExplicit());
            }
        }

        // Now the enabled flag.
        newUserlistEntry.Enabled(pluginMetadata["userlist"]["enabled"].as<bool>());

        // Now metadata lists. These are given in their entirety, so replace anything that
        // currently exists.
        BOOST_LOG_TRIVIAL(trace) << "Recording metadata lists from Javascript variables.";
        if (pluginMetadata["userlist"]["after"])
            newUserlistEntry.LoadAfter(pluginMetadata["userlist"]["after"].as<set<File>>());
        if (pluginMetadata["userlist"]["req"])
            newUserlistEntry.Reqs(pluginMetadata["userlist"]["req"].as<set<File>>());
        if (pluginMetadata["userlist"]["inc"])
            newUserlistEntry.Incs(pluginMetadata["userlist"]["inc"].as<set<File>>());

        if (pluginMetadata["userlist"]["msg"])
            newUserlistEntry.Messages(pluginMetadata["userlist"]["msg"].as<list<Message>>());
        if (pluginMetadata["userlist"]["tag"])
            newUserlistEntry.Tags(pluginMetadata["userlist"]["tag"].as<set<Tag>>());
        if (pluginMetadata["userlist"]["dirty"])
            newUserlistEntry.DirtyInfo(pluginMetadata["userlist"]["dirty"].as<set<PluginDirtyInfo>>());

        // Now replace existing userlist entry with the new one.
        if (ulistPluginIt != g_app_state.CurrentGame().userlist.plugins.end()) {
            BOOST_LOG_TRIVIAL(trace) << "Replacing existing userlist entry with new metadata.";
            if (newUserlistEntry.HasNameOnly())
                g_app_state.CurrentGame().userlist.plugins.erase(ulistPluginIt);
            else
                *ulistPluginIt = newUserlistEntry;
        }
        else {
            BOOST_LOG_TRIVIAL(trace) << "Adding new metadata to new userlist entry.";
            g_app_state.CurrentGame().userlist.plugins.push_back(newUserlistEntry);
        }

        // Now rederive the derived metadata.
        BOOST_LOG_TRIVIAL(trace) << "Returning newly derived display metadata.";
        YAML::Node derivedMetadata = GenerateDerivedMetadata(newUserlistEntry.Name());
        if (derivedMetadata.size() > 0)
            return JSON::stringify(derivedMetadata);
        else
            return "null";
    }

    void Handler::OpenReadme() {
        BOOST_LOG_TRIVIAL(info) << "Opening LOOT readme.";
        // Open readme in default application.
        HINSTANCE ret = ShellExecute(0, NULL, ToWinWide(ToFileURL(g_path_readme)).c_str(), NULL, NULL, SW_SHOWNORMAL);
        if ((int)ret <= 32)
            throw error(error::windows_error, "Shell execute failed.");
    }

    void Handler::OpenLogLocation() {
        BOOST_LOG_TRIVIAL(info) << "Opening LOOT local appdata folder.";
        //Open debug log folder.
        HINSTANCE ret = ShellExecute(NULL, L"open", ToWinWide(g_path_log.parent_path().string()).c_str(), NULL, NULL, SW_SHOWNORMAL);
        if ((int)ret <= 32)
            throw error(error::windows_error, "Shell execute failed.");
    }

    std::string Handler::GetVersion() {
        BOOST_LOG_TRIVIAL(info) << "Getting LOOT version.";
        YAML::Node version(to_string(g_version_major) + "." + to_string(g_version_minor) + "." + to_string(g_version_patch));
        return JSON::stringify(version);
    }

    std::string Handler::GetSettings() {
        BOOST_LOG_TRIVIAL(info) << "Getting LOOT settings.";
        return JSON::stringify(g_app_state.GetSettings());
    }

    std::string Handler::GetLanguages() {
        BOOST_LOG_TRIVIAL(info) << "Getting LOOT's supported languages.";
        // Need to get an array of language names and their corresponding codes.
        YAML::Node temp;
        vector<string> names = Language::Names();
        for (const auto& name : names) {
            YAML::Node lang;
            lang["name"] = name;
            lang["locale"] = Language(name).Locale();
            temp.push_back(lang);
        }
        return JSON::stringify(temp);
    }

    std::string Handler::GetGameTypes() {
        BOOST_LOG_TRIVIAL(info) << "Getting LOOT's supported game types.";
        YAML::Node temp;
        temp.push_back(Game(Game::tes4).FolderName());
        temp.push_back(Game(Game::tes5).FolderName());
        temp.push_back(Game(Game::fo3).FolderName());
        temp.push_back(Game(Game::fonv).FolderName());
        return JSON::stringify(temp);
    }

    std::string Handler::GetInstalledGames() {
        BOOST_LOG_TRIVIAL(info) << "Getting LOOT's detected games.";
        YAML::Node temp = YAML::Node(g_app_state.InstalledGames());
        if (temp.size() > 0)
            return JSON::stringify(temp);
        else
            return "[]";
    }

    std::string Handler::GetGameData() {
        BOOST_LOG_TRIVIAL(info) << "Getting data specific to LOOT's active game.";
        // Get masterlist revision info and parse if it exists. Also get plugin headers info and parse userlist if it exists.

        g_app_state.CurrentGame().LoadPlugins(true);

        //Sort plugins into their load order.
        list<loot::Plugin> installed;
        list<string> loadOrder;
        g_app_state.CurrentGame().GetLoadOrder(loadOrder);
        for (const auto &pluginName : loadOrder) {
            const auto pos = g_app_state.CurrentGame().plugins.find(boost::locale::to_lower(pluginName));

            if (pos != g_app_state.CurrentGame().plugins.end())
                installed.push_back(pos->second);
        }

        //Parse masterlist, don't update it.
        if (fs::exists(g_app_state.CurrentGame().MasterlistPath())) {
            BOOST_LOG_TRIVIAL(debug) << "Parsing masterlist.";
            g_app_state.CurrentGame().masterlist.MetadataList::Load(g_app_state.CurrentGame().MasterlistPath());
        }

        //Parse userlist.
        if (fs::exists(g_app_state.CurrentGame().UserlistPath())) {
            BOOST_LOG_TRIVIAL(debug) << "Parsing userlist.";
            g_app_state.CurrentGame().userlist.Load(g_app_state.CurrentGame().UserlistPath());
        }

        // Now convert to a single object that can be turned into a JSON string
        //---------------------------------------------------------------------

        //Set language.
        unsigned int language;
        if (g_app_state.GetSettings()["language"])
            language = Language(g_app_state.GetSettings()["language"].as<string>()).Code();
        else
            language = Language::any;
        BOOST_LOG_TRIVIAL(info) << "Using message language: " << Language(language).Name();

        // The data structure is to be set as 'loot.game'.
        YAML::Node gameNode;

        // ID the game using its folder value.
        gameNode["folder"] = g_app_state.CurrentGame().FolderName();

        // Store the masterlist revision and date.
        gameNode["masterlist"]["revision"] = g_app_state.CurrentGame().masterlist.GetRevision(g_app_state.CurrentGame().MasterlistPath());
        gameNode["masterlist"]["date"] = g_app_state.CurrentGame().masterlist.GetDate(g_app_state.CurrentGame().MasterlistPath());

        // Now store plugin data.
        for (const auto& plugin : installed) {
            /* Each plugin has members while hold its raw masterlist and userlist data for
               the editor, and also processed data for the main display.
            */
            YAML::Node pluginNode;
            pluginNode["__type"] = "Plugin";  // For conversion back into a JS typed object.
            pluginNode["name"] = plugin.Name();
            pluginNode["isActive"] = g_app_state.CurrentGame().IsActive(plugin.Name());
            pluginNode["isDummy"] = false; // Set to false for now because null is a bit iffy and we just don't know yet. Although, we could read the record count from the TES4 header... Usual check is (plugin.second.FormIDs().size() == 0);
            pluginNode["loadsBSA"] = plugin.LoadsBSA(g_app_state.CurrentGame());
            pluginNode["crc"] = IntToHexString(plugin.Crc());
            pluginNode["version"] = plugin.Version();

            // Find the masterlist metadata for this plugin.
            BOOST_LOG_TRIVIAL(trace) << "Getting masterlist metadata for: " << plugin.Name();
            Plugin mlistPlugin(plugin);
            mlistPlugin.MergeMetadata(g_app_state.CurrentGame().masterlist.FindPlugin(plugin.Name()));

            if (!mlistPlugin.HasNameOnly()) {
                // Now add the masterlist metadata to the pluginNode.
                pluginNode["masterlist"]["after"] = mlistPlugin.LoadAfter();
                pluginNode["masterlist"]["req"] = mlistPlugin.Reqs();
                pluginNode["masterlist"]["inc"] = mlistPlugin.Incs();
                pluginNode["masterlist"]["msg"] = mlistPlugin.Messages();
                pluginNode["masterlist"]["tag"] = mlistPlugin.Tags();
                pluginNode["masterlist"]["dirty"] = mlistPlugin.DirtyInfo();
            }

            // Now do the same again for any userlist data.
            BOOST_LOG_TRIVIAL(trace) << "Getting userlist metadata for: " << plugin.Name();
            Plugin ulistPlugin(plugin);
            // Clear Bash Tags to prevent false positives.
            ulistPlugin.Tags(set<Tag>());
            ulistPlugin.MergeMetadata(g_app_state.CurrentGame().userlist.FindPlugin(plugin.Name()));

            if (!ulistPlugin.HasNameOnly()) {
                // Now add the userlist metadata to the pluginNode.
                pluginNode["userlist"]["enabled"] = ulistPlugin.Enabled();
                pluginNode["userlist"]["after"] = ulistPlugin.LoadAfter();
                pluginNode["userlist"]["req"] = ulistPlugin.Reqs();
                pluginNode["userlist"]["inc"] = ulistPlugin.Incs();
                pluginNode["userlist"]["msg"] = ulistPlugin.Messages();
                pluginNode["userlist"]["tag"] = ulistPlugin.Tags();
                pluginNode["userlist"]["dirty"] = ulistPlugin.DirtyInfo();
            }

            // Now merge masterlist and userlist metadata and evaluate,
            // putting any resulting metadata into the base of the pluginNode.
            YAML::Node derivedNode = GenerateDerivedMetadata(plugin, mlistPlugin, ulistPlugin);

            for (auto it = derivedNode.begin(); it != derivedNode.end(); ++it) {
                const string key = it->first.as<string>();
                pluginNode[key] = it->second;
            }

            gameNode["plugins"].push_back(pluginNode);
        }

        //Evaluate any conditions in the global messages.
        BOOST_LOG_TRIVIAL(debug) << "Evaluating global message conditions.";
        list<Message> messages = g_app_state.CurrentGame().masterlist.messages;
        try {
            list<Message>::iterator it = messages.begin();
            while (it != messages.end()) {
                if (!it->EvalCondition(g_app_state.CurrentGame(), language))
                    it = messages.erase(it);
                else
                    ++it;
            }
        }
        catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << "A global message contains a condition that could not be evaluated. Details: " << e.what();
            messages.push_back(Message(Message::error, (format(loc::translate("A global message contains a condition that could not be evaluated. Details: %1%")) % e.what()).str()));
        }

        // Now store global messages from masterlist.
        gameNode["globalMessages"] = messages;

        return JSON::stringify(gameNode);
    }

    std::string Handler::UpdateMasterlist() {
        BOOST_LOG_TRIVIAL(debug) << "Updating and parsing masterlist.";

        //Set language.
        unsigned int language;
        if (g_app_state.GetSettings()["language"])
            language = Language(g_app_state.GetSettings()["language"].as<string>()).Code();
        else
            language = Language::any;
        BOOST_LOG_TRIVIAL(info) << "Using message language: " << Language(language).Name();

        // Update / parse masterlist.
        g_app_state.CurrentGame().masterlist.Load(g_app_state.CurrentGame(), language);

        // Now regenerate the JS-side masterlist data.

        // The data structure is to be set as 'loot.game'.
        YAML::Node gameNode;

        // Store the masterlist revision and date.
        gameNode["masterlist"]["revision"] = g_app_state.CurrentGame().masterlist.GetRevision(g_app_state.CurrentGame().MasterlistPath());
        gameNode["masterlist"]["date"] = g_app_state.CurrentGame().masterlist.GetDate(g_app_state.CurrentGame().MasterlistPath());

        for (const auto& pluginPair : g_app_state.CurrentGame().plugins) {
            Plugin mlistPlugin(pluginPair.second);
            mlistPlugin.MergeMetadata(g_app_state.CurrentGame().masterlist.FindPlugin(pluginPair.second.Name()));

            YAML::Node pluginNode;
            if (!mlistPlugin.HasNameOnly()) {
                // Now add the masterlist metadata to the pluginNode.
                pluginNode["masterlist"]["after"] = mlistPlugin.LoadAfter();
                pluginNode["masterlist"]["req"] = mlistPlugin.Reqs();
                pluginNode["masterlist"]["inc"] = mlistPlugin.Incs();
                pluginNode["masterlist"]["msg"] = mlistPlugin.Messages();
                pluginNode["masterlist"]["tag"] = mlistPlugin.Tags();
                pluginNode["masterlist"]["dirty"] = mlistPlugin.DirtyInfo();
            }

            // Now merge masterlist and userlist metadata and evaluate,
            // putting any resulting metadata into the base of the pluginNode.
            YAML::Node derivedNode = GenerateDerivedMetadata(pluginPair.second.Name());

            for (auto it = derivedNode.begin(); it != derivedNode.end(); ++it) {
                const string key = it->first.as<string>();
                pluginNode[key] = it->second;
            }

            gameNode["plugins"].push_back(pluginNode);
        }

        //Evaluate any conditions in the global messages.
        BOOST_LOG_TRIVIAL(debug) << "Evaluating global message conditions.";
        list<Message> messages = g_app_state.CurrentGame().masterlist.messages;
        try {
            list<Message>::iterator it = messages.begin();
            while (it != messages.end()) {
                if (!it->EvalCondition(g_app_state.CurrentGame(), language))
                    it = messages.erase(it);
                else
                    ++it;
            }
        }
        catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << "A global message contains a condition that could not be evaluated. Details: " << e.what();
            messages.push_back(Message(Message::error, (format(loc::translate("A global message contains a condition that could not be evaluated. Details: %1%")) % e.what()).str()));
        }

        // Now store global messages from masterlist.
        gameNode["globalMessages"] = messages;

        return JSON::stringify(gameNode);
    }

    std::string Handler::ClearAllMetadata() {
        BOOST_LOG_TRIVIAL(debug) << "Clearing all user metadata.";
        // Record which plugins have userlist entries.
        vector<string> userlistPlugins;
        for (const auto &plugin : g_app_state.CurrentGame().userlist.plugins) {
            userlistPlugins.push_back(plugin.Name());
        }
        BOOST_LOG_TRIVIAL(trace) << "User metadata exists for " << userlistPlugins.size() << " plugins.";

        // Clear the user metadata.
        g_app_state.CurrentGame().userlist.clear();

        // Regenerate the derived metadata (priority, messages, tags and dirty state)
        // for any plugins with userlist entries.
        YAML::Node pluginsNode;
        for (const auto &plugin : userlistPlugins) {
            pluginsNode.push_back(GenerateDerivedMetadata(plugin));
        }
        BOOST_LOG_TRIVIAL(trace) << "Display metadata rederived for " << pluginsNode.size() << " plugins.";

        if (pluginsNode.size() > 0)
            return JSON::stringify(pluginsNode);
        else
            return "[]";
    }

    std::string Handler::SortPlugins() {
        //Set language.
        unsigned int language;
        if (g_app_state.GetSettings()["language"])
            language = Language(g_app_state.GetSettings()["language"].as<string>()).Code();
        else
            language = Language::any;
        BOOST_LOG_TRIVIAL(info) << "Using message language: " << Language(language).Name();

        // Check if the first plugin has any FormIDs in memory, and if not load all plugins.
        if (g_app_state.CurrentGame().plugins.begin()->second.FormIDs().size() == 0)
            g_app_state.CurrentGame().LoadPlugins(false);

        //Sort plugins into their load order.
        list<Plugin> plugins = g_app_state.CurrentGame().Sort(language, [](const string& message){});

        YAML::Node node;
        for (const auto &plugin : plugins) {
            node["loadOrder"].push_back(plugin.Name());
            node["crcs"][plugin.Name()] = plugin.Crc();
        }

        if (node.size() > 0)
            return JSON::stringify(node);
        else
            return "null";
    }

    YAML::Node Handler::GenerateDerivedMetadata(const Plugin& file, const Plugin& masterlist, const Plugin& userlist) {
        //Set language.
        unsigned int language;
        if (g_app_state.GetSettings()["language"])
            language = Language(g_app_state.GetSettings()["language"].as<string>()).Code();
        else
            language = Language::any;
        BOOST_LOG_TRIVIAL(info) << "Using message language: " << Language(language).Name();

        // Now rederive the displayed metadata from the masterlist and userlist.
        Plugin tempPlugin(file);

        tempPlugin.MergeMetadata(masterlist);
        tempPlugin.MergeMetadata(userlist);

        //Evaluate any conditions
        BOOST_LOG_TRIVIAL(trace) << "Evaluate conditions for merged plugin data.";
        try {
            tempPlugin.EvalAllConditions(g_app_state.CurrentGame(), language);
        }
        catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << "\"" << tempPlugin.Name() << "\" contains a condition that could not be evaluated. Details: " << e.what();
            list<Message> messages(tempPlugin.Messages());
            messages.push_back(Message(Message::error, (format(loc::translate("\"%1%\" contains a condition that could not be evaluated. Details: %2%")) % tempPlugin.Name() % e.what()).str()));
            tempPlugin.Messages(messages);
        }

        //Also check install validity.
        BOOST_LOG_TRIVIAL(trace) << "Checking that the current install is valid according to this plugin's data.";
        bool isDirty = tempPlugin.CheckInstallValidity(g_app_state.CurrentGame());

        // Now add to pluginNode.
        YAML::Node pluginNode;
        pluginNode["name"] = tempPlugin.Name();
        pluginNode["modPriority"] = modulo(tempPlugin.Priority(), max_priority);
        pluginNode["isGlobalPriority"] = (abs(tempPlugin.Priority()) >= max_priority);
        pluginNode["messages"] = tempPlugin.Messages();
        pluginNode["tags"] = tempPlugin.Tags();
        pluginNode["isDirty"] = isDirty;

        return pluginNode;
    }

    YAML::Node Handler::GenerateDerivedMetadata(const std::string& pluginName) {
        
        // Now rederive the displayed metadata from the masterlist and userlist.
        auto pluginIt = g_app_state.CurrentGame().plugins.find(boost::locale::to_lower(pluginName));
        if (pluginIt != g_app_state.CurrentGame().plugins.end()) {

            const Plugin master = g_app_state.CurrentGame().masterlist.FindPlugin(pluginIt->second.Name());
            const Plugin user = g_app_state.CurrentGame().userlist.FindPlugin(pluginIt->second.Name());

            return this->GenerateDerivedMetadata(pluginIt->second, master, user);
        }

        return YAML::Node();
    }

    // LootHandler methods
    //--------------------

    LootHandler::LootHandler() : is_closing_(false) {
        assert(!g_instance);
        g_instance = this;
    }

    LootHandler::~LootHandler() {
        g_instance = NULL;
    }

    LootHandler* LootHandler::GetInstance() {
        return g_instance;
    }

    // CefClient methods
    //------------------

    CefRefPtr<CefDisplayHandler> LootHandler::GetDisplayHandler() {
        return this;
    }

    CefRefPtr<CefLifeSpanHandler> LootHandler::GetLifeSpanHandler() {
        return this;
    }
    
    CefRefPtr<CefLoadHandler> LootHandler::GetLoadHandler() {
        return this;
    }

    bool LootHandler::OnProcessMessageReceived( CefRefPtr<CefBrowser> browser,
                                                CefProcessId source_process,
                                                CefRefPtr<CefProcessMessage> message) {
        return browser_side_router_->OnProcessMessageReceived(browser, source_process, message);
    }

    // CefDisplayHandler methods
    //--------------------------

    void LootHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                      const CefString& title) {
      assert(CefCurrentlyOn(TID_UI));

      CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
#ifdef _WIN32
      SetWindowText(hwnd, std::wstring(title).c_str());
#endif
    }

    // CefLifeSpanHandler methods
    //---------------------------

    void LootHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
        assert(CefCurrentlyOn(TID_UI));

        // Set the title bar icon.
        HWND hWnd = browser->GetHost()->GetWindowHandle();
        HANDLE hIcon = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(MAINICON), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
        HANDLE hIconSm = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(MAINICON), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
        SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSm);

        // Add to the list of existing browsers.
        browser_list_.push_back(browser);

        // Create a message router.
        CefMessageRouterConfig config;
        browser_side_router_ = CefMessageRouterBrowserSide::Create(config);

        browser_side_router_->AddHandler(new Handler(), false);
    }

    bool LootHandler::DoClose(CefRefPtr<CefBrowser> browser) {
        assert(CefCurrentlyOn(TID_UI));

        // Closing the main window requires special handling. See the DoClose()
        // documentation in the CEF header for a detailed destription of this
        // process.
        if (browser_list_.size() == 1) {
            // Set a flag to indicate that the window close should be allowed.
            is_closing_ = true;
        }

        // Allow the close. For windowed browsers this will result in the OS close
        // event being sent.
        return false;
    }

    void LootHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
        assert(CefCurrentlyOn(TID_UI));

        // Cancel any javascript callbacks.
        browser_side_router_->OnBeforeClose(browser);

        // Remove from the list of existing browsers.
        for (BrowserList::iterator bit = browser_list_.begin(); bit != browser_list_.end(); ++bit) {
            if ((*bit)->IsSame(browser)) {
                browser_list_.erase(bit);
                break;
            }
        }

        if (browser_list_.empty()) {
            // All browser windows have closed. Quit the application message loop.
            CefQuitMessageLoop();
        }
    }

    // CefLoadHandler methods
    //-----------------------

    void LootHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            ErrorCode errorCode,
                            const CefString& errorText,
                            const CefString& failedUrl) {
        assert(CefCurrentlyOn(TID_UI));

        // Don't display an error for downloaded files.
        if (errorCode == ERR_ABORTED)
            return;

        // Display a load error message.
        std::stringstream ss;
        ss << "<html><body bgcolor=\"white\">"
           << "<h2>Failed to load URL " << std::string(failedUrl)
           << " with error " << std::string(errorText) << " (" << errorCode
           << ").</h2></body></html>";
        
        frame->LoadString(ss.str(), failedUrl);
    }

    // CefRequestHandler methods
    //--------------------------

    bool LootHandler::OnBeforeBrowse(CefRefPtr< CefBrowser > browser,
        CefRefPtr< CefFrame > frame,
        CefRefPtr< CefRequest > request,
        bool is_redirect) {

        BOOST_LOG_TRIVIAL(trace) << "Attemping to open link: " << request->GetURL().ToString();
        BOOST_LOG_TRIVIAL(trace) << "Comparing with URL: " << ToFileURL(g_path_report);

        if (request->GetURL() == ToFileURL(g_path_report)) {
            BOOST_LOG_TRIVIAL(trace) << "Link is to LOOT page, allowing CEF's default handling.";
            return false;
        }

        BOOST_LOG_TRIVIAL(info) << "Opening link in Windows' default handler.";
        // Open readme in default application.
        HINSTANCE ret = ShellExecute(0, NULL, request->GetURL().ToWString().c_str(), NULL, NULL, SW_SHOWNORMAL);
        if ((int)ret <= 32)
            throw error(error::windows_error, "Shell execute failed.");

        return true;
    }

    void LootHandler::CloseAllBrowsers(bool force_close) {

        if (!CefCurrentlyOn(TID_UI)) {
            // Execute on the UI thread.
            CefPostTask(TID_UI,
            NewCefRunnableMethod(this, &LootHandler::CloseAllBrowsers, force_close));
            return;
        }

        if (browser_list_.empty())
            return;

        for (BrowserList::const_iterator it = browser_list_.begin(); it != browser_list_.end(); ++it) {
            (*it)->GetHost()->CloseBrowser(force_close);
        }
    }

}