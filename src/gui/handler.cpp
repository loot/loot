/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2014-2015    WrinklyNinja

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
#include "loot_app.h"
#include "loot_handler.h"

#include "../backend/error.h"
#include "../backend/globals.h"
#include "../backend/plugin_sorter.h"
#include "../backend/helpers/helpers.h"
#include "../backend/helpers/json.h"

#include <include/cef_app.h>
#include <include/cef_runnable.h>
#include <include/cef_task.h>
#include <include/base/cef_bind.h>
#include <include/wrapper/cef_closure_task.h>

#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>

#include <sstream>
#include <string>
#include <iomanip>

using namespace std;

using boost::format;

namespace fs = boost::filesystem;
namespace loc = boost::locale;

namespace loot {
    Handler::Handler(LootState& lootState) : _lootState(lootState) {}

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
            return CefPostTask(TID_FILE, base::Bind(&Handler::GetGameData, base::Unretained(this), frame, callback));
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
                _lootState.CurrentGame().RedatePlugins();
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
            return CefPostTask(TID_FILE, base::Bind(&Handler::UpdateMasterlist, base::Unretained(this), frame, callback));
        }
        else if (request == "sortPlugins") {
            return CefPostTask(TID_FILE, base::Bind(&Handler::SortPlugins, base::Unretained(this), frame, callback));
        }
        else if (request == "getInitErrors") {
            YAML::Node node(_lootState.InitErrors());
            if (node.size() > 0)
                callback->Success(JSON::stringify(node));
            else
                callback->Success("null");
            return true;
        }
        else if (request == "cancelSort") {
            --_lootState.numUnappliedChanges;
            callback->Success("");
            return true;
        }
        else if (request == "editorOpened") {
            ++_lootState.numUnappliedChanges;
            callback->Success("");
            return true;
        }
        else if (request == "editorClosed") {
            // This version of the editorClosed query has no arguments as it is
            // sent when editing is cancelled. Just update the unapplied changes
            // counter.
            --_lootState.numUnappliedChanges;
            callback->Success("");
            return true;
        }
        else {
            // May be a request with arguments.
            YAML::Node req;
            try {
                req = JSON::parse(request.ToString());
            }
            catch (exception &e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to parse CEF query request \"" << request.ToString() << "\": " << e.what();
                callback->Failure(-1, e.what());
                return true;
            }

            return HandleComplexQuery(browser, frame, req, callback);
        }

        return false;
    }

    // Handle queries with input arguments.
    bool Handler::HandleComplexQuery(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     YAML::Node& request,
                                     CefRefPtr<Callback> callback) {
        const string requestName = request["name"].as<string>();

        if (requestName == "changeGame") {
            try {
                // Has one arg, which is the folder name of the new game.
                _lootState.ChangeGame(request["args"][0].as<string>());

                CefPostTask(TID_FILE, base::Bind(&Handler::GetGameData, base::Unretained(this), frame, callback));
            }
            catch (loot::error &e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to change game. Details: " << e.what();
                callback->Failure(e.code(), (boost::format(loc::translate("Failed to change game. Details: %1%")) % e.what()).str());
            }
            catch (std::exception& e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to change game. Details: " << e.what();
                callback->Failure(-1, (boost::format(loc::translate("Failed to change game. Details: %1%")) % e.what()).str());
            }
            return true;
        }
        else if (requestName == "getConflictingPlugins") {
            // Has one arg, which is the name of the plugin to get conflicts for.
            CefPostTask(TID_FILE, base::Bind(&Handler::GetConflictingPlugins, base::Unretained(this), request["args"][0].as<string>(), frame, callback));
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
                callback->Failure(e.code(), (boost::format(loc::translate("Failed to copy plugin metadata. Details: %1%")) % e.what()).str());
            }
            catch (std::exception& e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to copy plugin metadata. Details: " << e.what();
                callback->Failure(-1, (boost::format(loc::translate("Failed to copy plugin metadata. Details: %1%")) % e.what()).str());
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
            --_lootState.numUnappliedChanges;
            return true;
        }
        else if (requestName == "closeSettings") {
            BOOST_LOG_TRIVIAL(trace) << "Settings dialog closed and changes accepted, updating settings object.";

            // Update the game details and settings.
            _lootState.UpdateSettings(request["args"][0]);
            // If the user has deleted a default game, we don't want to restore it now.
            // It will be restored when LOOT is next loaded.
            try {
                BOOST_LOG_TRIVIAL(trace) << "Updating games object.";
                list<GameSettings> games(request["args"][0]["games"].as< list<GameSettings> >());
                _lootState.UpdateGames(games);

                // Also enable/disable debug logging as required.
                if (request["args"][0]["enableDebugLogging"] && request["args"][0]["enableDebugLogging"].as<bool>())
                    boost::log::core::get()->set_logging_enabled(true);
                else
                    boost::log::core::get()->set_logging_enabled(false);

                // Now send back the new list of installed games to the UI.
                BOOST_LOG_TRIVIAL(trace) << "Getting new list of installed games.";
                callback->Success(GetInstalledGames());
            }
            catch (exception &e) {
                BOOST_LOG_TRIVIAL(error) << e.what();
                callback->Failure(-1, e.what());
            }
            return true;
        }
        else if (requestName == "applySort") {
            --_lootState.numUnappliedChanges;
            BOOST_LOG_TRIVIAL(trace) << "User has accepted sorted load order, applying it.";
            try {
                _lootState.CurrentGame().SetLoadOrder(request["args"][0].as<list<string>>());
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
        else if (requestName == "copyContent") {
            // Has one arg, just convert it to a YAML output string.
            try {
                YAML::Emitter yout;
                yout.SetIndent(2);
                yout << request["args"][0];
                string text = yout.c_str();
                // Get rid of yaml-cpp weirdness.
                boost::replace_all(text, "!<!> ", "");
                text = "[spoiler][code]" + text + "[/code][/spoiler]";
                CopyToClipboard(text);
                callback->Success("");
            }
            catch (loot::error &e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to copy plugin metadata. Details: " << e.what();
                callback->Failure(e.code(), (boost::format(loc::translate("Failed to copy plugin metadata. Details: %1%")) % e.what()).str());
            }
            catch (std::exception& e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to copy plugin metadata. Details: " << e.what();
                callback->Failure(-1, (boost::format(loc::translate("Failed to copy plugin metadata. Details: %1%")) % e.what()).str());
            }
            return true;
        }
        else if (requestName == "copyLoadOrder") {
            // Has one arg, an array of plugins in load order. Output them with indices in dec and hex.
            try {
                stringstream ss;
                vector<string> plugins = request["args"][0].as<vector<string>>();
                int decLength = 1;
                if (plugins.size() > 99) {
                    decLength = 3;
                }
                else if (plugins.size() > 9) {
                    decLength = 2;
                }
                size_t i = 0;
                for (const auto& plugin : plugins) {
                    if (Plugin(plugin).IsActive(_lootState.CurrentGame())) {
                        ss << setw(decLength) << i << " " << hex << setw(2) << i << dec << " ";
                        ++i;
                    }
                    else {
                        ss << setw(decLength + 4) << "     ";
                    }
                    ss << plugin << "\r\n";
                }
                CopyToClipboard(ss.str());
                callback->Success("");
            }
            catch (loot::error &e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to copy plugin metadata. Details: " << e.what();
                callback->Failure(e.code(), (boost::format(loc::translate("Failed to copy plugin metadata. Details: %1%")) % e.what()).str());
            }
            catch (std::exception& e) {
                BOOST_LOG_TRIVIAL(error) << "Failed to copy plugin metadata. Details: " << e.what();
                callback->Failure(-1, (boost::format(loc::translate("Failed to copy plugin metadata. Details: %1%")) % e.what()).str());
            }
            return true;
        }
        else if (requestName == "saveFilterState") {
            // Has two args: the first is the filter ID, the second is the value.
            BOOST_LOG_TRIVIAL(trace) << "Saving filter states.";
            try {
                YAML::Node settings = _lootState.GetSettings();

                settings["filters"][request["args"][0].as<string>()] = request["args"][1];

                _lootState.UpdateSettings(settings);
                callback->Success("");
            }
            catch (exception &e) {
                BOOST_LOG_TRIVIAL(error) << e.what();
                callback->Failure(-1, e.what());
            }
            return true;
        }
        return false;
    }

    void Handler::GetConflictingPlugins(const std::string& pluginName, CefRefPtr<CefFrame> frame, CefRefPtr<Callback> callback) {
        BOOST_LOG_TRIVIAL(debug) << "Searching for plugins that conflict with " << pluginName;

        auto pluginIt = _lootState.CurrentGame().plugins.find(boost::locale::to_lower(pluginName));

        // Checking for FormID overlap will only work if the plugins have been loaded, so check if
        // the plugins have been fully loaded, and if not load all plugins.
        if (!_lootState.CurrentGame().ArePluginsFullyLoaded()) {
            SendProgressUpdate(frame, loc::translate("Loading plugin contents..."));
            _lootState.CurrentGame().LoadPlugins(false);
        }

        SendProgressUpdate(frame, loc::translate("Checking for conflicting plugins..."));
        YAML::Node node;
        for (const auto& pluginPair : _lootState.CurrentGame().plugins) {
            YAML::Node pluginNode;

            pluginNode["crc"] = pluginPair.second.Crc();
            pluginNode["isEmpty"] = pluginPair.second.IsEmpty();
            if (pluginIt != _lootState.CurrentGame().plugins.end() && pluginIt->second.DoFormIDsOverlap(pluginPair.second)) {
                BOOST_LOG_TRIVIAL(debug) << "Found conflicting plugin: " << pluginPair.second.Name();
                pluginNode["conflicts"] = true;
            }
            else {
                pluginNode["conflicts"] = false;
            }

            // Plugin loading may have produced an error message, so rederive displayed data.
            YAML::Node derivedNode = GenerateDerivedMetadata(pluginPair.second.Name());
            for (const auto &pair : derivedNode) {
                const string key = pair.first.as<string>();
                pluginNode[key] = pair.second;
            }

            node[pluginPair.second.Name()] = pluginNode;
        }

        if (node.size() > 0)
            callback->Success(JSON::stringify(node));
        else
            callback->Success("null");
    }

    void Handler::CopyMetadata(const std::string& pluginName) {
        BOOST_LOG_TRIVIAL(debug) << "Copying metadata for plugin " << pluginName;

        // Get metadata from masterlist and userlist.
        PluginMetadata plugin = _lootState.CurrentGame().masterlist.FindPlugin(pluginName);
        plugin.MergeMetadata(_lootState.CurrentGame().userlist.FindPlugin(pluginName));

        // Generate text representation.
        string text;
        YAML::Emitter yout;
        yout.SetIndent(2);
        yout << plugin;
        text = yout.c_str();
        // Get rid of yaml-cpp weirdness.
        boost::replace_all(text, "!<!> ", "");
        text = "[spoiler][code]" + text + "[/code][/spoiler]";

        CopyToClipboard(text);

        BOOST_LOG_TRIVIAL(info) << "Exported userlist metadata text for \"" << pluginName << "\": " << text;
    }

    std::string Handler::ClearPluginMetadata(const std::string& pluginName) {
        BOOST_LOG_TRIVIAL(debug) << "Clearing user metadata for plugin " << pluginName;

        _lootState.CurrentGame().userlist.ErasePlugin(Plugin(pluginName));

        // Save userlist edits.
        _lootState.CurrentGame().userlist.Save(_lootState.CurrentGame().UserlistPath());

        // Now rederive the displayed metadata from the masterlist.
        YAML::Node derivedMetadata = GenerateDerivedMetadata(pluginName);
        if (derivedMetadata.size() > 0)
            return JSON::stringify(derivedMetadata);
        else
            return "null";
    }

    std::string Handler::ApplyUserEdits(const YAML::Node& pluginMetadata) {
        BOOST_LOG_TRIVIAL(trace) << "Applying user edits for: " << pluginMetadata["name"].as<string>();
        // Create new object for userlist entry.
        PluginMetadata newUserlistEntry(pluginMetadata["name"].as<string>());

        // Find existing userlist entry.
        PluginMetadata ulistPlugin = _lootState.CurrentGame().userlist.FindPlugin(newUserlistEntry);

        // First sort out the priority value. This is only given if it was changed.
        BOOST_LOG_TRIVIAL(trace) << "Calculating userlist metadata priority value from Javascript variables.";
        if (pluginMetadata["modPriority"] && pluginMetadata["isGlobalPriority"]) {
            BOOST_LOG_TRIVIAL(trace) << "Priority value was changed, recalculating...";
            // Priority value was changed, so add it to the userlist data.
            int priority = pluginMetadata["modPriority"].as<int>();

            if (pluginMetadata["isGlobalPriority"].as<bool>()) {
                if (priority >= 0) {
                    priority += max_priority;
                }
                else {
                    priority -= max_priority;
                }
            }
            newUserlistEntry.Priority(priority);
            newUserlistEntry.SetPriorityExplicit(true);
        }
        else {
            // Priority value wasn't changed, use the existing userlist value.
            BOOST_LOG_TRIVIAL(trace) << "Priority value is unchanged, using existing userlist value (if it exists).";
            if (!ulistPlugin.HasNameOnly()) {
                newUserlistEntry.Priority(ulistPlugin.Priority());
                newUserlistEntry.SetPriorityExplicit(ulistPlugin.IsPriorityExplicit());
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
        if (pluginMetadata["userlist"]["url"])
            newUserlistEntry.Locations(pluginMetadata["userlist"]["url"].as<set<Location>>());

        // For cleanliness, only data that does not duplicate masterlist and plugin data should be retained, so diff that.
        BOOST_LOG_TRIVIAL(trace) << "Removing any user metadata that duplicates masterlist metadata.";
        auto pluginIt = _lootState.CurrentGame().plugins.find(boost::locale::to_lower(newUserlistEntry.Name()));
        if (pluginIt != _lootState.CurrentGame().plugins.end()) {
            Plugin tempPlugin(pluginIt->second);
            tempPlugin.MergeMetadata(_lootState.CurrentGame().masterlist.FindPlugin(newUserlistEntry));
            newUserlistEntry = newUserlistEntry.NewMetadata(tempPlugin);
        }
        else
            newUserlistEntry = newUserlistEntry.NewMetadata(_lootState.CurrentGame().masterlist.FindPlugin(newUserlistEntry));

        // Now replace existing userlist entry with the new one.
        if (!ulistPlugin.HasNameOnly()) {
            BOOST_LOG_TRIVIAL(trace) << "Replacing existing userlist entry with new metadata.";
            if (newUserlistEntry.HasNameOnly())
                _lootState.CurrentGame().userlist.ErasePlugin(ulistPlugin);
            else {
                // Set members are static, so just erase and add the new data.
                _lootState.CurrentGame().userlist.ErasePlugin(ulistPlugin);
                _lootState.CurrentGame().userlist.AddPlugin(newUserlistEntry);
            }
        }
        else {
            BOOST_LOG_TRIVIAL(trace) << "Adding new metadata to new userlist entry.";
            _lootState.CurrentGame().userlist.AddPlugin(newUserlistEntry);
        }

        // Save edited userlist.
        _lootState.CurrentGame().userlist.Save(_lootState.CurrentGame().UserlistPath());

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
            throw error(error::windows_error, loc::translate("Shell execute failed."));
    }

    void Handler::OpenLogLocation() {
        BOOST_LOG_TRIVIAL(info) << "Opening LOOT local appdata folder.";
        //Open debug log folder.
        HINSTANCE ret = ShellExecute(NULL, L"open", ToWinWide(g_path_log.parent_path().string()).c_str(), NULL, NULL, SW_SHOWNORMAL);
        if ((int)ret <= 32)
            throw error(error::windows_error, loc::translate("Shell execute failed."));
    }

    std::string Handler::GetVersion() {
        BOOST_LOG_TRIVIAL(info) << "Getting LOOT version.";
        YAML::Node version(to_string(g_version_major) + "." + to_string(g_version_minor) + "." + to_string(g_version_patch) + "." + g_build_revision);
        return JSON::stringify(version);
    }

    std::string Handler::GetSettings() {
        BOOST_LOG_TRIVIAL(info) << "Getting LOOT settings.";
        return JSON::stringify(_lootState.GetSettings());
    }

    std::string Handler::GetLanguages() {
        BOOST_LOG_TRIVIAL(info) << "Getting LOOT's supported languages.";
        // Need to get an array of language names and their corresponding codes.
        YAML::Node temp;
        for (const auto& code : Language::Codes) {
            YAML::Node lang;
            Language language(code);
            lang["name"] = language.Name();
            lang["locale"] = language.Locale();
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
        YAML::Node temp = YAML::Node(_lootState.InstalledGames());
        if (temp.size() > 0)
            return JSON::stringify(temp);
        else
            return "[]";
    }

    void Handler::GetGameData(CefRefPtr<CefFrame> frame, CefRefPtr<Callback> callback) {
        try {
            /* GetGameData() can be called for initialising the UI for a game for the first time
               in a session, or it can be called when changing to a game that has previously been
               active. In the first case, all data should be loaded, but in the second, only load
               order and plugin header info should be re-loaded.
               Determine which case it is by checking to see if the game's plugins object is empty.
               */
            BOOST_LOG_TRIVIAL(info) << "Getting data specific to LOOT's active game.";
            // Get masterlist revision info and parse if it exists. Also get plugin headers info and parse userlist if it exists.

            SendProgressUpdate(frame, loc::translate("Loading plugin headers..."));

            // First clear CRC and condition caches, otherwise they could lead to incorrect evaluations.
            _lootState.CurrentGame().conditionCache.clear();
            _lootState.CurrentGame().crcCache.clear();

            // Also refresh active plugins list.
            _lootState.CurrentGame().RefreshActivePluginsList();

            bool isFirstLoad = _lootState.CurrentGame().plugins.empty();
            _lootState.CurrentGame().LoadPlugins(true);

            //Sort plugins into their load order.
            list<loot::Plugin> installed;
            list<string> loadOrder = _lootState.CurrentGame().GetLoadOrder();
            for (const auto &pluginName : loadOrder) {
                const auto pos = _lootState.CurrentGame().plugins.find(boost::locale::to_lower(pluginName));

                if (pos != _lootState.CurrentGame().plugins.end())
                    installed.push_back(pos->second);
            }

            if (isFirstLoad) {
                //Parse masterlist, don't update it.
                if (fs::exists(_lootState.CurrentGame().MasterlistPath())) {
                    SendProgressUpdate(frame, loc::translate("Parsing masterlist..."));
                    BOOST_LOG_TRIVIAL(debug) << "Parsing masterlist.";
                    try {
                        _lootState.CurrentGame().masterlist.MetadataList::Load(_lootState.CurrentGame().MasterlistPath());
                    }
                    catch (exception &e) {
                        _lootState.CurrentGame().masterlist.messages.push_back(Message(Message::error, (boost::format(loc::translate("An error occurred while parsing the masterlist: %1%")) % e.what()).str()));
                    }
                }

                //Parse userlist.
                if (fs::exists(_lootState.CurrentGame().UserlistPath())) {
                    SendProgressUpdate(frame, loc::translate("Parsing userlist..."));
                    BOOST_LOG_TRIVIAL(debug) << "Parsing userlist.";
                    try {
                        _lootState.CurrentGame().userlist.Load(_lootState.CurrentGame().UserlistPath());
                    }
                    catch (exception &e) {
                        _lootState.CurrentGame().userlist.messages.push_back(Message(Message::error, (boost::format(loc::translate("An error occurred while parsing the userlist: %1%")) % e.what()).str()));
                    }
                }
            }

            // Now convert to a single object that can be turned into a JSON string
            //---------------------------------------------------------------------

            // The data structure is to be set as 'loot.game'.
            YAML::Node gameNode;

            // ID the game using its folder value.
            gameNode["folder"] = _lootState.CurrentGame().FolderName();

            // Store the masterlist revision and date.
            try {
                gameNode["masterlist"]["revision"] = _lootState.CurrentGame().masterlist.GetRevision(_lootState.CurrentGame().MasterlistPath(), true);
                gameNode["masterlist"]["date"] = _lootState.CurrentGame().masterlist.GetDate(_lootState.CurrentGame().MasterlistPath());
            }
            catch (error &e) {
                gameNode["masterlist"]["revision"] = e.what();
                gameNode["masterlist"]["date"] = e.what();
            }

            // Now store plugin data.
            SendProgressUpdate(frame, loc::translate("Merging and evaluating plugin metadata..."));
            for (const auto& plugin : installed) {
                /* Each plugin has members while hold its raw masterlist and userlist data for
                   the editor, and also processed data for the main display.
                   */
                YAML::Node pluginNode;
                // Find the masterlist metadata for this plugin. Treat Bash Tags from the plugin
                // description as part of it.
                BOOST_LOG_TRIVIAL(trace) << "Getting masterlist metadata for: " << plugin.Name();
                Plugin mlistPlugin(plugin);
                mlistPlugin.MergeMetadata(_lootState.CurrentGame().masterlist.FindPlugin(plugin));

                // Now do the same again for any userlist data.
                BOOST_LOG_TRIVIAL(trace) << "Getting userlist metadata for: " << plugin.Name();
                PluginMetadata ulistPlugin(_lootState.CurrentGame().userlist.FindPlugin(plugin));

                pluginNode["__type"] = "Plugin";  // For conversion back into a JS typed object.
                pluginNode["name"] = plugin.Name();
                pluginNode["isActive"] = plugin.IsActive(_lootState.CurrentGame());
                pluginNode["isEmpty"] = plugin.IsEmpty();
                pluginNode["isMaster"] = plugin.IsMaster();
                pluginNode["loadsBSA"] = plugin.LoadsBSA(_lootState.CurrentGame());
                pluginNode["crc"] = IntToHexString(plugin.Crc());
                pluginNode["version"] = plugin.Version();

                if (!mlistPlugin.HasNameOnly()) {
                    // Now add the masterlist metadata to the pluginNode.
                    pluginNode["masterlist"]["after"] = mlistPlugin.LoadAfter();
                    pluginNode["masterlist"]["req"] = mlistPlugin.Reqs();
                    pluginNode["masterlist"]["inc"] = mlistPlugin.Incs();
                    pluginNode["masterlist"]["msg"] = mlistPlugin.Messages();
                    pluginNode["masterlist"]["tag"] = mlistPlugin.Tags();
                    pluginNode["masterlist"]["dirty"] = mlistPlugin.DirtyInfo();
                    pluginNode["masterlist"]["url"] = mlistPlugin.Locations();
                }

                if (!ulistPlugin.HasNameOnly()) {
                    // Now add the userlist metadata to the pluginNode.
                    pluginNode["userlist"]["enabled"] = ulistPlugin.Enabled();
                    pluginNode["userlist"]["after"] = ulistPlugin.LoadAfter();
                    pluginNode["userlist"]["req"] = ulistPlugin.Reqs();
                    pluginNode["userlist"]["inc"] = ulistPlugin.Incs();
                    pluginNode["userlist"]["msg"] = ulistPlugin.Messages();
                    pluginNode["userlist"]["tag"] = ulistPlugin.Tags();
                    pluginNode["userlist"]["dirty"] = ulistPlugin.DirtyInfo();
                    pluginNode["userlist"]["url"] = ulistPlugin.Locations();
                    // The raw priority data isn't used, but should be set
                    // that LOOT knows it exists.
                    if (ulistPlugin.IsPriorityExplicit()) {
                        pluginNode["userlist"]["hasExplicitPriority"] = true;
                    }
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

            SendProgressUpdate(frame, loc::translate("Loading general messages..."));
            //Set language.
            unsigned int language;
            if (_lootState.GetSettings()["language"])
                language = Language(_lootState.GetSettings()["language"].as<string>()).Code();
            else
                language = Language::any;
            BOOST_LOG_TRIVIAL(info) << "Using message language: " << Language(language).Name();

            //Evaluate any conditions in the global messages.
            BOOST_LOG_TRIVIAL(debug) << "Evaluating global message conditions.";
            list<Message> messages = _lootState.CurrentGame().masterlist.messages;
            messages.insert(messages.end(), _lootState.CurrentGame().userlist.messages.begin(), _lootState.CurrentGame().userlist.messages.end());
            try {
                list<Message>::iterator it = messages.begin();
                while (it != messages.end()) {
                    if (!it->EvalCondition(_lootState.CurrentGame(), language))
                        it = messages.erase(it);
                    else
                        ++it;
                }
            }
            catch (std::exception& e) {
                BOOST_LOG_TRIVIAL(error) << "A global message contains a condition that could not be evaluated. Details: " << e.what();
                messages.push_back(Message(Message::error, (format(loc::translate("A global message contains a condition that could not be evaluated. Details: %1%")) % e.what()).str()));
            }

            // Now store global messages.
            gameNode["globalMessages"] = messages;

            callback->Success(JSON::stringify(gameNode));
        }
        catch (loot::error &e) {
            BOOST_LOG_TRIVIAL(error) << "Failed to get game data. Details: " << e.what();
            callback->Failure(e.code(), (boost::format(loc::translate("Failed to get game data. Details: %1%")) % e.what()).str());
        }
        catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << "Failed to get game data. Details: " << e.what();
            callback->Failure(-1, (boost::format(loc::translate("Failed to get game data. Details: %1%")) % e.what()).str());
        }
    }

    void Handler::UpdateMasterlist(CefRefPtr<CefFrame> frame, CefRefPtr<Callback> callback) {
        try {
            BOOST_LOG_TRIVIAL(debug) << "Updating and parsing masterlist.";

            //Set language.
            unsigned int language;
            if (_lootState.GetSettings()["language"])
                language = Language(_lootState.GetSettings()["language"].as<string>()).Code();
            else
                language = Language::any;
            BOOST_LOG_TRIVIAL(info) << "Using message language: " << Language(language).Name();

            // Update / parse masterlist.
            bool wasChanged = true;
            try {
                SendProgressUpdate(frame, loc::translate("Updating and parsing masterlist..."));
                wasChanged = _lootState.CurrentGame().masterlist.Load(_lootState.CurrentGame(), language);
            }
            catch (loot::error &e) {
                if (e.code() == loot::error::ok) {
                    // There was a parsing error, but roll-back was successful, so the process

                    // should still complete.
                    _lootState.CurrentGame().masterlist.messages.push_back(Message(Message::error, e.what()));
                    wasChanged = true;
                }
                else
                    throw;
            }

            // Now regenerate the JS-side masterlist data if the masterlist was changed.
            SendProgressUpdate(frame, loc::translate("Regenerating displayed content..."));
            if (wasChanged) {
                // The data structure is to be set as 'loot.game'.
                YAML::Node gameNode;

                // Store the masterlist revision and date.
                try {
                    gameNode["masterlist"]["revision"] = _lootState.CurrentGame().masterlist.GetRevision(_lootState.CurrentGame().MasterlistPath(), true);
                    gameNode["masterlist"]["date"] = _lootState.CurrentGame().masterlist.GetDate(_lootState.CurrentGame().MasterlistPath());
                }
                catch (error &e) {
                    gameNode["masterlist"]["revision"] = e.what();
                    gameNode["masterlist"]["date"] = e.what();
                }

                for (const auto& pluginPair : _lootState.CurrentGame().plugins) {
                    Plugin mlistPlugin(pluginPair.second);
                    mlistPlugin.MergeMetadata(_lootState.CurrentGame().masterlist.FindPlugin(pluginPair.second));

                    YAML::Node pluginNode;
                    if (!mlistPlugin.HasNameOnly()) {
                        // Now add the masterlist metadata to the pluginNode.
                        pluginNode["masterlist"]["after"] = mlistPlugin.LoadAfter();
                        pluginNode["masterlist"]["req"] = mlistPlugin.Reqs();
                        pluginNode["masterlist"]["inc"] = mlistPlugin.Incs();
                        pluginNode["masterlist"]["msg"] = mlistPlugin.Messages();
                        pluginNode["masterlist"]["tag"] = mlistPlugin.Tags();
                        pluginNode["masterlist"]["dirty"] = mlistPlugin.DirtyInfo();
                        pluginNode["masterlist"]["url"] = mlistPlugin.Locations();
                    }

                    // Now merge masterlist and userlist metadata and evaluate,
                    // putting any resulting metadata into the base of the pluginNode.
                    YAML::Node derivedNode = GenerateDerivedMetadata(pluginPair.second.Name());

                    for (const auto &pair : derivedNode) {
                        const string key = pair.first.as<string>();
                        pluginNode[key] = pair.second;
                    }

                    gameNode["plugins"].push_back(pluginNode);
                }

                //Evaluate any conditions in the global messages.
                BOOST_LOG_TRIVIAL(debug) << "Evaluating global message conditions.";
                list<Message> messages = _lootState.CurrentGame().masterlist.messages;
                try {
                    list<Message>::iterator it = messages.begin();
                    while (it != messages.end()) {
                        if (!it->EvalCondition(_lootState.CurrentGame(), language))
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

                callback->Success(JSON::stringify(gameNode));
            }
            else
                callback->Success("null");
        }
        catch (error &e) {
            BOOST_LOG_TRIVIAL(error) << "Failed to update the masterlist. Details: " << e.what();
            callback->Failure(e.code(), (boost::format(loc::translate("Failed to update the masterlist. Details: %1%")) % e.what()).str());
        }
        catch (exception &e) {
            BOOST_LOG_TRIVIAL(error) << "Failed to update the masterlist. Details: " << e.what();
            callback->Failure(-1, (boost::format(loc::translate("Failed to update the masterlist. Details: %1%")) % e.what()).str());
        }
    }

    std::string Handler::ClearAllMetadata() {
        BOOST_LOG_TRIVIAL(debug) << "Clearing all user metadata.";
        // Record which plugins have userlist entries.
        vector<string> userlistPlugins;
        for (const auto &plugin : _lootState.CurrentGame().userlist.Plugins()) {
            userlistPlugins.push_back(plugin.Name());
        }
        BOOST_LOG_TRIVIAL(trace) << "User metadata exists for " << userlistPlugins.size() << " plugins.";

        // Clear the user metadata.
        _lootState.CurrentGame().userlist.clear();

        // Save userlist edits.
        _lootState.CurrentGame().userlist.Save(_lootState.CurrentGame().UserlistPath());

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

    void Handler::SortPlugins(CefRefPtr<CefFrame> frame, CefRefPtr<Callback> callback) {
        BOOST_LOG_TRIVIAL(info) << "Beginning sorting operation.";
        //Set language.
        unsigned int language;
        if (_lootState.GetSettings()["language"])
            language = Language(_lootState.GetSettings()["language"].as<string>()).Code();
        else
            language = Language::any;
        BOOST_LOG_TRIVIAL(info) << "Using message language: " << Language(language).Name();

        try {
            // Always reload all the plugins.
            SendProgressUpdate(frame, loc::translate("Loading plugin contents..."));
            _lootState.CurrentGame().LoadPlugins(false);

            //Sort plugins into their load order.
            PluginSorter sorter;
            list<Plugin> plugins = sorter.Sort(_lootState.CurrentGame(), language, [this, frame](const string& message) {
                this->SendProgressUpdate(frame, message);
            });

            YAML::Node node;
            for (const auto &plugin : plugins) {
                YAML::Node pluginNode;

                pluginNode["name"] = plugin.Name();
                pluginNode["crc"] = plugin.Crc();
                pluginNode["isEmpty"] = plugin.IsEmpty();

                // Sorting may have produced a plugin loading error message, so rederive displayed data.
                YAML::Node derivedNode = GenerateDerivedMetadata(plugin.Name());
                for (const auto &pair : derivedNode) {
                    const string key = pair.first.as<string>();
                    pluginNode[key] = pair.second;
                }

                node.push_back(pluginNode);
            }
            ++_lootState.numUnappliedChanges;

            if (node.size() > 0)
                callback->Success(JSON::stringify(node));
            else
                callback->Success("null");
        }
        catch (loot::error& e) {
            BOOST_LOG_TRIVIAL(error) << "Failed to sort plugins. Details: " << e.what();
            callback->Failure(e.code(), (boost::format(loc::translate("Failed to sort plugins. Details: %1%")) % e.what()).str());
        }
    }

    YAML::Node Handler::GenerateDerivedMetadata(const Plugin& file, const PluginMetadata& masterlist, const PluginMetadata& userlist) {
        //Set language.
        unsigned int language;
        if (_lootState.GetSettings()["language"])
            language = Language(_lootState.GetSettings()["language"].as<string>()).Code();
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
            tempPlugin.EvalAllConditions(_lootState.CurrentGame(), language);
        }
        catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << "\"" << tempPlugin.Name() << "\" contains a condition that could not be evaluated. Details: " << e.what();
            list<Message> messages(tempPlugin.Messages());
            messages.push_back(Message(Message::error, (format(loc::translate("\"%1%\" contains a condition that could not be evaluated. Details: %2%")) % tempPlugin.Name() % e.what()).str()));
            tempPlugin.Messages(messages);
        }

        //Also check install validity.
        bool isDirty = tempPlugin.CheckInstallValidity(_lootState.CurrentGame());

        // Now add to pluginNode.
        YAML::Node pluginNode;
        pluginNode["name"] = tempPlugin.Name();
        pluginNode["modPriority"] = tempPlugin.Priority() % max_priority;
        pluginNode["isGlobalPriority"] = (abs(tempPlugin.Priority()) >= max_priority);
        pluginNode["messages"] = tempPlugin.Messages();
        pluginNode["tags"] = tempPlugin.Tags();
        pluginNode["isDirty"] = isDirty;

        return pluginNode;
    }

    YAML::Node Handler::GenerateDerivedMetadata(const std::string& pluginName) {
        // Now rederive the displayed metadata from the masterlist and userlist.
        auto pluginIt = _lootState.CurrentGame().plugins.find(boost::locale::to_lower(pluginName));
        if (pluginIt != _lootState.CurrentGame().plugins.end()) {
            PluginMetadata master(_lootState.CurrentGame().masterlist.FindPlugin(pluginIt->second));
            PluginMetadata user(_lootState.CurrentGame().userlist.FindPlugin(pluginIt->second));

            return this->GenerateDerivedMetadata(pluginIt->second, master, user);
        }

        return YAML::Node();
    }

    void Handler::CopyToClipboard(const std::string& text) {
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
    }

    void Handler::SendProgressUpdate(CefRefPtr<CefFrame> frame, const std::string& message) {
        BOOST_LOG_TRIVIAL(trace) << "Sending progress update: " << message;
        frame->ExecuteJavaScript("showProgress('" + message + "');", frame->GetURL(), 0);
    }
}
