/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2014-2016    WrinklyNinja

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
    <https://www.gnu.org/licenses/>.
    */

#include "query_handler.h"

#include <sstream>
#include <string>
#include <iomanip>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>
#include <include/base/cef_bind.h>
#include <include/cef_app.h>
#include <include/cef_task.h>
#include <include/wrapper/cef_closure_task.h>

#include "gui/loot_app.h"
#include "gui/loot_handler.h"
#include "gui/resource.h"

#include "backend/error.h"
#include "backend/app/loot_paths.h"
#include "backend/app/loot_version.h"
#include "backend/plugin/plugin_sorter.h"
#include "backend/helpers/helpers.h"
#include "backend/helpers/json.h"
#include "backend/helpers/version.h"

using boost::filesystem::exists;
using boost::format;
using boost::locale::translate;
using std::exception;
using std::list;
using std::set;
using std::string;
using std::vector;

namespace loot {
QueryHandler::QueryHandler(LootState& lootState) : lootState_(lootState) {}

// Called due to cefQuery execution in binding.html.
bool QueryHandler::OnQuery(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           int64 query_id,
                           const CefString& request,
                           bool persistent,
                           CefRefPtr<Callback> callback) {
  if (request == "openReadme") {
    try {
      OpenReadme();
      callback->Success("");
    } catch (Error &e) {
      BOOST_LOG_TRIVIAL(error) << e.what();
      callback->Failure(e.codeAsUnsignedInt(), e.what());
    } catch (exception &e) {
      BOOST_LOG_TRIVIAL(error) << e.what();
      callback->Failure(-1, e.what());
    }
    return true;
  } else if (request == "openLogLocation") {
    try {
      OpenLogLocation();
      callback->Success("");
    } catch (Error &e) {
      BOOST_LOG_TRIVIAL(error) << e.what();
      callback->Failure(e.codeAsUnsignedInt(), e.what());
    } catch (exception &e) {
      BOOST_LOG_TRIVIAL(error) << e.what();
      callback->Failure(-1, e.what());
    }
    return true;
  } else if (request == "getVersion") {
    callback->Success(GetVersion());
    return true;
  } else if (request == "getSettings") {
    callback->Success(GetSettings());
    return true;
  } else if (request == "getLanguages") {
    callback->Success(GetLanguages());
    return true;
  } else if (request == "getGameTypes") {
    callback->Success(GetGameTypes());
    return true;
  } else if (request == "getInstalledGames") {
    callback->Success(GetInstalledGames());
    return true;
  } else if (request == "getGameData") {
    SendProgressUpdate(frame, translate("Parsing, merging and evaluating metadata..."));
    return CefPostTask(TID_FILE, base::Bind(&QueryHandler::GetGameData, base::Unretained(this), frame, callback));
  } else if (request == "cancelFind") {
    browser->GetHost()->StopFinding(true);
    callback->Success("");
    return true;
  } else if (request == "clearAllMetadata") {
    callback->Success(ClearAllMetadata());
    return true;
  } else if (request == "redatePlugins") {
    BOOST_LOG_TRIVIAL(debug) << "Redating plugins.";
    try {
      lootState_.getCurrentGame().RedatePlugins();
      callback->Success("");
    } catch (Error &e) {
      BOOST_LOG_TRIVIAL(error) << "Failed to redate plugins. " << e.what();
      callback->Failure(e.codeAsUnsignedInt(), e.what());
    } catch (exception &e) {
      BOOST_LOG_TRIVIAL(error) << "Failed to redate plugins. " << e.what();
      callback->Failure(-1, e.what());
    }

    return true;
  } else if (request == "updateMasterlist") {
    return CefPostTask(TID_FILE, base::Bind(&QueryHandler::UpdateMasterlist, base::Unretained(this), callback));
  } else if (request == "sortPlugins") {
    return CefPostTask(TID_FILE, base::Bind(&QueryHandler::SortPlugins, base::Unretained(this), frame, callback));
  } else if (request == "getInitErrors") {
    YAML::Node node(lootState_.getInitErrors());
    if (node.size() > 0)
      callback->Success(JSON::stringify(node));
    else
      callback->Success("null");
    return true;
  } else if (request == "cancelSort") {
    lootState_.decrementUnappliedChangeCounter();
    lootState_.getCurrentGame().DecrementLoadOrderSortCount();

    YAML::Node node(GetGeneralMessages());
    callback->Success(JSON::stringify(node));
    return true;
  } else if (request == "editorOpened") {
    lootState_.incrementUnappliedChangeCounter();
    callback->Success("");
    return true;
  } else if (request == "editorClosed") {
      // This version of the editorClosed query has no arguments as it is
      // sent when editing is cancelled. Just update the unapplied changes
      // counter.
    lootState_.decrementUnappliedChangeCounter();
    callback->Success("");
    return true;
  } else if (request == "discardUnappliedChanges") {
    while (lootState_.hasUnappliedChanges())
      lootState_.decrementUnappliedChangeCounter();
    callback->Success("");
    return true;
  } else {
      // May be a request with arguments.
    YAML::Node req;
    try {
        // Can't pass this as a reference directly as GCC
        // complains about it.
      std::string requestString = request.ToString();
      req = JSON::parse(requestString);
    } catch (exception &e) {
      BOOST_LOG_TRIVIAL(error) << "Failed to parse CEF query request \"" << request.ToString() << "\": " << e.what();
      callback->Failure(-1, e.what());
      return true;
    }

    return HandleComplexQuery(browser, frame, req, callback);
  }

  return false;
}

// Handle queries with input arguments.
bool QueryHandler::HandleComplexQuery(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      YAML::Node& request,
                                      CefRefPtr<Callback> callback) {
  const string requestName = request["name"].as<string>();

  if (requestName == "changeGame") {
    try {
        // Has one arg, which is the folder name of the new game.
      lootState_.changeGame(request["args"][0].as<string>());

      CefPostTask(TID_FILE, base::Bind(&QueryHandler::GetGameData, base::Unretained(this), frame, callback));
    } catch (Error &e) {
      BOOST_LOG_TRIVIAL(error) << "Failed to change game. Details: " << e.what();
      callback->Failure(e.codeAsUnsignedInt(), (boost::format(translate("Failed to change game. Details: %1%")) % e.what()).str());
    } catch (std::exception& e) {
      BOOST_LOG_TRIVIAL(error) << "Failed to change game. Details: " << e.what();
      callback->Failure(-1, (boost::format(translate("Failed to change game. Details: %1%")) % e.what()).str());
    }
    return true;
  } else if (requestName == "getConflictingPlugins") {
      // Has one arg, which is the name of the plugin to get conflicts for.
    CefPostTask(TID_FILE, base::Bind(&QueryHandler::GetConflictingPlugins, base::Unretained(this), request["args"][0].as<string>(), callback));
    return true;
  } else if (requestName == "copyMetadata") {
      // Has one arg, which is the name of the plugin to copy metadata for.
    try {
      CopyMetadata(request["args"][0].as<string>());
      callback->Success("");
    } catch (Error &e) {
      BOOST_LOG_TRIVIAL(error) << "Failed to copy plugin metadata. Details: " << e.what();
      callback->Failure(e.codeAsUnsignedInt(), (boost::format(translate("Failed to copy plugin metadata. Details: %1%")) % e.what()).str());
    } catch (std::exception& e) {
      BOOST_LOG_TRIVIAL(error) << "Failed to copy plugin metadata. Details: " << e.what();
      callback->Failure(-1, (boost::format(translate("Failed to copy plugin metadata. Details: %1%")) % e.what()).str());
    }
    return true;
  } else if (requestName == "clearPluginMetadata") {
      // Has one arg, which is the name of the plugin to copy metadata for.
    callback->Success(ClearPluginMetadata(request["args"][0].as<string>()));
    return true;
  } else if (requestName == "editorClosed") {
    BOOST_LOG_TRIVIAL(debug) << "Editor for plugin closed.";
    // One argument, which is the plugin metadata that has changed (+ its name).
    try {
      callback->Success(ApplyUserEdits(request["args"][0]));
      lootState_.decrementUnappliedChangeCounter();
    } catch (Error &e) {
      BOOST_LOG_TRIVIAL(error) << "Failed to apply plugin metadata. Details: " << e.what();
      callback->Failure(e.codeAsUnsignedInt(), (boost::format(translate("Failed to apply plugin metadata. Details: %1%")) % e.what()).str());
    } catch (std::exception& e) {
        // If this was a YAML conversion error, cut off the line and column numbers,
        // since the YAML wasn't written to a file.
      string error = e.what();
      size_t pos = string::npos;
      if ((pos = error.find("bad conversion")) != string::npos) {
        error = error.substr(pos);
      }
      BOOST_LOG_TRIVIAL(error) << "Failed to apply plugin metadata. Details: " << e.what();
      callback->Failure(-1, (boost::format(translate("Failed to apply plugin metadata. Details: %1%")) % error).str());
    }
    return true;
  } else if (requestName == "closeSettings") {
    BOOST_LOG_TRIVIAL(trace) << "Settings dialog closed and changes accepted, updating settings object.";

    try {
        // Update the settings.
        // If the user has deleted a default game, we don't want to restore it now.
        // It will be restored when LOOT is next loaded.
      YAML::Node settings = request["args"][0];
      lootState_.load(settings);

      // Now send back the new list of installed games to the UI.
      BOOST_LOG_TRIVIAL(trace) << "Getting new list of installed games.";
      callback->Success(GetInstalledGames());
    } catch (exception &e) {
      BOOST_LOG_TRIVIAL(error) << e.what();
      callback->Failure(-1, e.what());
    }
    return true;
  } else if (requestName == "applySort") {
    lootState_.decrementUnappliedChangeCounter();
    BOOST_LOG_TRIVIAL(trace) << "User has accepted sorted load order, applying it.";
    try {
      lootState_.getCurrentGame().SetLoadOrder(request["args"][0].as<vector<string>>());
      callback->Success("");
    } catch (Error &e) {
      BOOST_LOG_TRIVIAL(error) << e.what();
      callback->Failure(e.codeAsUnsignedInt(), e.what());
    } catch (exception &e) {
      BOOST_LOG_TRIVIAL(error) << e.what();
      callback->Failure(-1, e.what());
    }

    return true;
  } else if (requestName == "copyContent") {
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
    } catch (Error &e) {
      BOOST_LOG_TRIVIAL(error) << "Failed to copy plugin metadata. Details: " << e.what();
      callback->Failure(e.codeAsUnsignedInt(), (boost::format(translate("Failed to copy plugin metadata. Details: %1%")) % e.what()).str());
    } catch (std::exception& e) {
      BOOST_LOG_TRIVIAL(error) << "Failed to copy plugin metadata. Details: " << e.what();
      callback->Failure(-1, (boost::format(translate("Failed to copy plugin metadata. Details: %1%")) % e.what()).str());
    }
    return true;
  } else if (requestName == "copyLoadOrder") {
      // Has one arg, an array of plugins in load order. Output them with indices in dec and hex.
    try {
      std::stringstream ss;
      vector<string> plugins = request["args"][0].as<vector<string>>();
      int decLength = 1;
      if (plugins.size() > 99) {
        decLength = 3;
      } else if (plugins.size() > 9) {
        decLength = 2;
      }
      size_t i = 0;
      for (const auto& pluginName : plugins) {
        if (lootState_.getCurrentGame().IsPluginActive(pluginName)) {
          ss << std::setw(decLength) << i << " " << std::hex << std::setw(2) << i << std::dec << " ";
          ++i;
        } else {
          ss << std::setw(decLength + 4) << "     ";
        }
        ss << pluginName << "\r\n";
      }
      CopyToClipboard(ss.str());
      callback->Success("");
    } catch (Error &e) {
      BOOST_LOG_TRIVIAL(error) << "Failed to copy plugin metadata. Details: " << e.what();
      callback->Failure(e.codeAsUnsignedInt(), (boost::format(translate("Failed to copy plugin metadata. Details: %1%")) % e.what()).str());
    } catch (std::exception& e) {
      BOOST_LOG_TRIVIAL(error) << "Failed to copy plugin metadata. Details: " << e.what();
      callback->Failure(-1, (boost::format(translate("Failed to copy plugin metadata. Details: %1%")) % e.what()).str());
    }
    return true;
  } else if (requestName == "saveFilterState") {
      // Has two args: the first is the filter ID, the second is the value.
    BOOST_LOG_TRIVIAL(trace) << "Saving filter states.";
    try {
      lootState_.storeFilterState(request["args"][0].as<string>(), request["args"][1].as<bool>());
      callback->Success("");
    } catch (exception &e) {
      BOOST_LOG_TRIVIAL(error) << e.what();
      callback->Failure(-1, e.what());
    }
    return true;
  }
  return false;
}

void QueryHandler::GetConflictingPlugins(const std::string& pluginName, CefRefPtr<Callback> callback) {
  BOOST_LOG_TRIVIAL(debug) << "Searching for plugins that conflict with " << pluginName;

  // Checking for FormID overlap will only work if the plugins have been loaded, so check if
  // the plugins have been fully loaded, and if not load all plugins.
  if (!lootState_.getCurrentGame().ArePluginsFullyLoaded())
    lootState_.getCurrentGame().LoadPlugins(false);

  YAML::Node node;
  auto plugin = lootState_.getCurrentGame().GetPlugin(pluginName);
  for (const auto& otherPlugin : lootState_.getCurrentGame().GetPlugins()) {
      // Plugin loading may have produced an error message, so rederive
      // displayed data.

    YAML::Node pluginNode = GenerateDerivedMetadata(otherPlugin.Name());

    pluginNode["name"] = otherPlugin.Name();
    pluginNode["crc"] = otherPlugin.Crc();
    pluginNode["isEmpty"] = otherPlugin.IsEmpty();
    if (plugin.DoFormIDsOverlap(otherPlugin)) {
      BOOST_LOG_TRIVIAL(debug) << "Found conflicting plugin: " << otherPlugin.Name();
      pluginNode["conflicts"] = true;
    } else {
      pluginNode["conflicts"] = false;
    }

    node.push_back(pluginNode);
  }

  if (node.size() > 0)
    callback->Success(JSON::stringify(node));
  else
    callback->Success("[]");
}

void QueryHandler::CopyMetadata(const std::string& pluginName) {
  BOOST_LOG_TRIVIAL(debug) << "Copying metadata for plugin " << pluginName;

  // Get metadata from masterlist and userlist.
  PluginMetadata plugin = lootState_.getCurrentGame().GetMasterlist().FindPlugin(pluginName);
  plugin.MergeMetadata(lootState_.getCurrentGame().GetUserlist().FindPlugin(pluginName));

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

std::string QueryHandler::ClearPluginMetadata(const std::string& pluginName) {
  BOOST_LOG_TRIVIAL(debug) << "Clearing user metadata for plugin " << pluginName;

  lootState_.getCurrentGame().GetUserlist().ErasePlugin(PluginMetadata(pluginName));

  // Save userlist edits.
  lootState_.getCurrentGame().GetUserlist().Save(lootState_.getCurrentGame().UserlistPath());

  // Now rederive the displayed metadata from the masterlist.
  YAML::Node derivedMetadata = GenerateDerivedMetadata(pluginName);
  if (derivedMetadata.size() > 0)
    return JSON::stringify(derivedMetadata);
  else
    return "null";
}

std::string QueryHandler::ApplyUserEdits(const YAML::Node& pluginMetadata) {
  BOOST_LOG_TRIVIAL(trace) << "Applying user edits for: " << pluginMetadata["name"].as<string>();
  // Create new object for userlist entry.
  PluginMetadata newUserlistEntry(pluginMetadata["name"].as<string>());

  // Find existing userlist entry.
  PluginMetadata ulistPlugin = lootState_.getCurrentGame().GetUserlist().FindPlugin(newUserlistEntry);

  // First sort out the priority value. This is only given if it was changed.
  BOOST_LOG_TRIVIAL(trace) << "Calculating userlist metadata local priority value from Javascript variables.";
  if (pluginMetadata["priority"]) {
    BOOST_LOG_TRIVIAL(trace) << "Local priority value was changed, recalculating...";
    // Priority value was changed, so add it to the userlist data.
    newUserlistEntry.LocalPriority(Priority(pluginMetadata["priority"].as<int>()));
  } else {
    // Priority value wasn't changed, use the existing userlist value.
    BOOST_LOG_TRIVIAL(trace) << "Local priority value is unchanged, using existing userlist value (if it exists).";
    newUserlistEntry.LocalPriority(ulistPlugin.LocalPriority());
  }

  if (pluginMetadata["globalPriority"]) {
    BOOST_LOG_TRIVIAL(trace) << "Global priority value was changed, recalculating...";
    // Priority value was changed, so add it to the userlist data.
    newUserlistEntry.GlobalPriority(Priority(pluginMetadata["globalPriority"].as<int>()));
  } else {
    BOOST_LOG_TRIVIAL(trace) << "Global priority value is unchanged, using existing userlist value (if it exists).";
    newUserlistEntry.GlobalPriority(ulistPlugin.GlobalPriority());
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
    newUserlistEntry.DirtyInfo(pluginMetadata["userlist"]["dirty"].as<set<PluginCleaningData>>());
  if (pluginMetadata["userlist"]["clean"])
    newUserlistEntry.CleanInfo(pluginMetadata["userlist"]["clean"].as<set<PluginCleaningData>>());
  if (pluginMetadata["userlist"]["url"])
    newUserlistEntry.Locations(pluginMetadata["userlist"]["url"].as<set<Location>>());

// For cleanliness, only data that does not duplicate masterlist and plugin data should be retained, so diff that.
  BOOST_LOG_TRIVIAL(trace) << "Removing any user metadata that duplicates masterlist metadata.";
  try {
    Plugin tempPlugin(lootState_.getCurrentGame().GetPlugin(newUserlistEntry.Name()));
    tempPlugin.MergeMetadata(lootState_.getCurrentGame().GetMasterlist().FindPlugin(newUserlistEntry));
    newUserlistEntry = newUserlistEntry.NewMetadata(tempPlugin);
  } catch (...) {
    newUserlistEntry = newUserlistEntry.NewMetadata(lootState_.getCurrentGame().GetMasterlist().FindPlugin(newUserlistEntry));
  }

  // Now erase any existing userlist entry.
  if (!ulistPlugin.HasNameOnly()) {
    BOOST_LOG_TRIVIAL(trace) << "Erasing the existing userlist entry.";
    lootState_.getCurrentGame().GetUserlist().ErasePlugin(ulistPlugin);
  }
  // Add a new userlist entry if necessary.
  if (!newUserlistEntry.HasNameOnly()) {
    BOOST_LOG_TRIVIAL(trace) << "Adding new metadata to new userlist entry.";
    lootState_.getCurrentGame().GetUserlist().AddPlugin(newUserlistEntry);
  }

  // Save edited userlist.
  lootState_.getCurrentGame().GetUserlist().Save(lootState_.getCurrentGame().UserlistPath());

  // Now rederive the derived metadata.
  BOOST_LOG_TRIVIAL(trace) << "Returning newly derived display metadata.";
  YAML::Node derivedMetadata = GenerateDerivedMetadata(newUserlistEntry.Name());
  if (derivedMetadata.size() > 0)
    return JSON::stringify(derivedMetadata);
  else
    return "null";
}

void QueryHandler::OpenReadme() {
  BOOST_LOG_TRIVIAL(info) << "Opening LOOT readme.";
  // Open readme in default application.
  OpenInDefaultApplication(LootPaths::getReadmePath());
}

void QueryHandler::OpenLogLocation() {
  BOOST_LOG_TRIVIAL(info) << "Opening LOOT local appdata folder.";
  //Open debug log folder.
  OpenInDefaultApplication(LootPaths::getLogPath().parent_path());
}

std::string QueryHandler::GetVersion() {
  BOOST_LOG_TRIVIAL(info) << "Getting LOOT version.";
  YAML::Node version(LootVersion::string() + "." + LootVersion::revision);
  return JSON::stringify(version);
}

std::string QueryHandler::GetSettings() {
  BOOST_LOG_TRIVIAL(info) << "Getting LOOT settings.";
  return JSON::stringify(lootState_.toYaml());
}

std::string QueryHandler::GetLanguages() {
  BOOST_LOG_TRIVIAL(info) << "Getting LOOT's supported languages.";
  // Need to get an array of language names and their corresponding codes.
  YAML::Node temp;
  for (const auto& code : Language::codes) {
    YAML::Node lang;
    Language language(code);
    lang["name"] = language.GetName();
    lang["locale"] = language.GetLocale();
    temp.push_back(lang);
  }
  return JSON::stringify(temp);
}

std::string QueryHandler::GetGameTypes() {
  BOOST_LOG_TRIVIAL(info) << "Getting LOOT's supported game types.";
  YAML::Node temp;
  temp.push_back(Game(GameType::tes4).FolderName());
  temp.push_back(Game(GameType::tes5).FolderName());
  temp.push_back(Game(GameType::fo3).FolderName());
  temp.push_back(Game(GameType::fonv).FolderName());
  temp.push_back(Game(GameType::fo4).FolderName());
  return JSON::stringify(temp);
}

std::string QueryHandler::GetInstalledGames() {
  BOOST_LOG_TRIVIAL(info) << "Getting LOOT's detected games.";
  YAML::Node temp = YAML::Node(lootState_.getInstalledGames());
  if (temp.size() > 0)
    return JSON::stringify(temp);
  else
    return "[]";
}

void QueryHandler::GetGameData(CefRefPtr<CefFrame> frame, CefRefPtr<Callback> callback) {
  try {
      /* GetGameData() can be called for initialising the UI for a game for the first time
         in a session, or it can be called when changing to a game that has previously been
         active. In the first case, all data should be loaded, but in the second, only load
         order and plugin header info should be re-loaded.
         Determine which case it is by checking to see if the game's plugins object is empty.
         */
    BOOST_LOG_TRIVIAL(info) << "Getting data specific to LOOT's active game.";
    // Get masterlist revision info and parse if it exists. Also get plugin headers info and parse userlist if it exists.

    // First clear CRC and condition caches, otherwise they could lead to incorrect evaluations.
    lootState_.getCurrentGame().ClearCachedConditions();

    bool isFirstLoad = lootState_.getCurrentGame().GetPlugins().empty();
    lootState_.getCurrentGame().LoadPlugins(true);

    //Sort plugins into their load order.
    list<Plugin> installed;
    vector<string> loadOrder = lootState_.getCurrentGame().GetLoadOrder();
    for (const auto &pluginName : loadOrder) {
      try {
        const auto plugin = lootState_.getCurrentGame().GetPlugin(pluginName);
        installed.push_back(plugin);
      } catch (...) {}
    }

    if (isFirstLoad) {
        //Parse masterlist, don't update it.
      if (exists(lootState_.getCurrentGame().MasterlistPath())) {
        BOOST_LOG_TRIVIAL(debug) << "Parsing masterlist.";
        try {
          lootState_.getCurrentGame().GetMasterlist().Load(lootState_.getCurrentGame().MasterlistPath());
        } catch (exception &e) {
          lootState_.getCurrentGame().GetMasterlist().AppendMessage(Message(MessageType::error, (boost::format(translate(
            "An error occurred while parsing the masterlist: %1%. "
            "This probably happened because an update to LOOT changed "
            "its metadata syntax support. Try updating your masterlist "
            "to resolve the error."
          )) % e.what()).str()));
        }
      }

      //Parse userlist.
      if (exists(lootState_.getCurrentGame().UserlistPath())) {
        BOOST_LOG_TRIVIAL(debug) << "Parsing userlist.";
        try {
          lootState_.getCurrentGame().GetUserlist().Load(lootState_.getCurrentGame().UserlistPath());
        } catch (exception &e) {
          lootState_.getCurrentGame().GetUserlist().AppendMessage(Message(MessageType::error, (boost::format(translate(
            "An error occurred while parsing the userlist: %1%. "
            "This probably happened because an update to LOOT changed "
            "its metadata syntax support. Your user metadata will have "
            "to be updated manually.\n\n"
            "To do so, use the 'Open Debug Log Location' in LOOT's main "
            "menu to open its data folder, then open your 'userlist.yaml' "
            "file in the relevant game folder. You can then edit the "
            "metadata it contains with reference to the "
            "[syntax documentation](https://loot.github.io/docs/%2%.%3%.%4%/LOOT%%20Metadata%%20Syntax.html).\n\n"
            "You can also seek support on LOOT's forum thread, which is "
            "linked to on [LOOT's website](https://loot.github.io/)."
          )) % e.what() % LootVersion::major % LootVersion::minor % LootVersion::patch).str()));
        }
      }
    }

    // Now convert to a single object that can be turned into a JSON string
    //---------------------------------------------------------------------

    // The data structure is to be set as 'loot.game'.
    YAML::Node gameNode;

    // ID the game using its folder value.
    gameNode["folder"] = lootState_.getCurrentGame().FolderName();

    // Store the masterlist revision and date.
    try {
      Masterlist::Info info = lootState_.getCurrentGame().GetMasterlist().GetInfo(lootState_.getCurrentGame().MasterlistPath(), true);
      gameNode["masterlist"]["revision"] = info.revision;
      gameNode["masterlist"]["date"] = info.date;
    } catch (Error &e) {
      gameNode["masterlist"]["revision"] = e.what();
      gameNode["masterlist"]["date"] = e.what();
    }

    // Now store global messages.
    gameNode["globalMessages"] = GetGeneralMessages();

    gameNode["bashTags"] = lootState_.getCurrentGame().GetMasterlist().BashTags();

    // Now store plugin data.
    for (const auto& plugin : installed) {
        /* Each plugin has members while hold its raw masterlist and userlist data for
           the editor, and also processed data for the main display.
           */
      YAML::Node pluginNode;
      // Find the masterlist metadata for this plugin. Treat Bash Tags from the plugin
      // description as part of it.
      BOOST_LOG_TRIVIAL(trace) << "Getting masterlist metadata for: " << plugin.Name();
      Plugin mlistPlugin(plugin);
      mlistPlugin.MergeMetadata(lootState_.getCurrentGame().GetMasterlist().FindPlugin(plugin));

      // Now do the same again for any userlist data.
      BOOST_LOG_TRIVIAL(trace) << "Getting userlist metadata for: " << plugin.Name();
      PluginMetadata ulistPlugin(lootState_.getCurrentGame().GetUserlist().FindPlugin(plugin));

      pluginNode["__type"] = "Plugin";  // For conversion back into a JS typed object.
      pluginNode["name"] = plugin.Name();
      pluginNode["isActive"] = plugin.IsActive();
      pluginNode["isEmpty"] = plugin.IsEmpty();
      pluginNode["isMaster"] = plugin.isMasterFile();
      pluginNode["loadsArchive"] = plugin.LoadsArchive();
      pluginNode["crc"] = plugin.Crc();
      pluginNode["version"] = Version(plugin.getDescription()).AsString();

      if (!mlistPlugin.HasNameOnly()) {
          // Now add the masterlist metadata to the pluginNode.
        pluginNode["masterlist"]["after"] = mlistPlugin.LoadAfter();
        pluginNode["masterlist"]["req"] = mlistPlugin.Reqs();
        pluginNode["masterlist"]["inc"] = mlistPlugin.Incs();
        pluginNode["masterlist"]["msg"] = mlistPlugin.Messages();
        pluginNode["masterlist"]["tag"] = mlistPlugin.Tags();
        pluginNode["masterlist"]["dirty"] = mlistPlugin.DirtyInfo();
        pluginNode["masterlist"]["clean"] = mlistPlugin.CleanInfo();
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
        pluginNode["userlist"]["clean"] = ulistPlugin.CleanInfo();
        pluginNode["userlist"]["url"] = ulistPlugin.Locations();
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

    callback->Success(JSON::stringify(gameNode));
  } catch (Error &e) {
    BOOST_LOG_TRIVIAL(error) << "Failed to get game data. Details: " << e.what();
    callback->Failure(e.codeAsUnsignedInt(), (boost::format(translate("Failed to get game data. Details: %1%")) % e.what()).str());
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << "Failed to get game data. Details: " << e.what();
    callback->Failure(-1, (boost::format(translate("Failed to get game data. Details: %1%")) % e.what()).str());
  }
}

void QueryHandler::UpdateMasterlist(CefRefPtr<Callback> callback) {
  try {
      // Update / parse masterlist.
    BOOST_LOG_TRIVIAL(debug) << "Updating and parsing masterlist.";
    bool wasChanged = true;
    try {
      wasChanged = lootState_.getCurrentGame().GetMasterlist().Update(lootState_.getCurrentGame());
    } catch (Error &e) {
      if (e.code() == Error::Code::ok) {
          // There was a parsing error, but roll-back was successful, so the process

          // should still complete.
        lootState_.getCurrentGame().GetMasterlist().AppendMessage(Message(MessageType::error, e.what()));
        wasChanged = true;
      } else {
          // Error wasn't a parsing error. Need to try parsing masterlist if it exists.
        try {
          lootState_.getCurrentGame().GetMasterlist().Load(lootState_.getCurrentGame().MasterlistPath());
        } catch (...) {}
      }
      throw;
    }

    // Now regenerate the JS-side masterlist data if the masterlist was changed.
    if (wasChanged) {
        // The data structure is to be set as 'loot.game'.
      YAML::Node gameNode;

      // Store the masterlist revision and date.
      try {
        Masterlist::Info info = lootState_.getCurrentGame().GetMasterlist().GetInfo(lootState_.getCurrentGame().MasterlistPath(), true);
        gameNode["masterlist"]["revision"] = info.revision;
        gameNode["masterlist"]["date"] = info.date;
      } catch (Error &e) {
        gameNode["masterlist"]["revision"] = e.what();
        gameNode["masterlist"]["date"] = e.what();
      }

      // Store bash tags in case they have changed.
      gameNode["bashTags"] = lootState_.getCurrentGame().GetMasterlist().BashTags();

      // Store global messages in case they have changed.
      gameNode["globalMessages"] = GetGeneralMessages();

      for (const auto& plugin : lootState_.getCurrentGame().GetPlugins()) {
        Plugin mlistPlugin(plugin);
        mlistPlugin.MergeMetadata(lootState_.getCurrentGame().GetMasterlist().FindPlugin(plugin));

        YAML::Node pluginNode;
        if (!mlistPlugin.HasNameOnly()) {
            // Now add the masterlist metadata to the pluginNode.
          pluginNode["masterlist"]["after"] = mlistPlugin.LoadAfter();
          pluginNode["masterlist"]["req"] = mlistPlugin.Reqs();
          pluginNode["masterlist"]["inc"] = mlistPlugin.Incs();
          pluginNode["masterlist"]["msg"] = mlistPlugin.Messages();
          pluginNode["masterlist"]["tag"] = mlistPlugin.Tags();
          pluginNode["masterlist"]["dirty"] = mlistPlugin.DirtyInfo();
          pluginNode["masterlist"]["clean"] = mlistPlugin.CleanInfo();
          pluginNode["masterlist"]["url"] = mlistPlugin.Locations();
        }

        // Now merge masterlist and userlist metadata and evaluate,
        // putting any resulting metadata into the base of the pluginNode.
        YAML::Node derivedNode = GenerateDerivedMetadata(plugin.Name());

        for (const auto &pair : derivedNode) {
          const string key = pair.first.as<string>();
          pluginNode[key] = pair.second;
        }

        gameNode["plugins"].push_back(pluginNode);
      }

      callback->Success(JSON::stringify(gameNode));
    } else
      callback->Success("null");
  } catch (Error &e) {
    BOOST_LOG_TRIVIAL(error) << "Failed to update the masterlist. Details: " << e.what();
    callback->Failure(e.codeAsUnsignedInt(), (boost::format(translate("Failed to update the masterlist. Details: %1%")) % e.what()).str());
  } catch (exception &e) {
    BOOST_LOG_TRIVIAL(error) << "Failed to update the masterlist. Details: " << e.what();
    callback->Failure(-1, (boost::format(translate("Failed to update the masterlist. Details: %1%")) % e.what()).str());
  }
}

std::string QueryHandler::ClearAllMetadata() {
  BOOST_LOG_TRIVIAL(debug) << "Clearing all user metadata.";
  // Record which plugins have userlist entries.
  vector<string> userlistPlugins;
  for (const auto &plugin : lootState_.getCurrentGame().GetUserlist().Plugins()) {
    userlistPlugins.push_back(plugin.Name());
  }
  BOOST_LOG_TRIVIAL(trace) << "User metadata exists for " << userlistPlugins.size() << " plugins.";

  // Clear the user metadata.
  lootState_.getCurrentGame().GetUserlist().Clear();

  // Save userlist edits.
  lootState_.getCurrentGame().GetUserlist().Save(lootState_.getCurrentGame().UserlistPath());

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

void QueryHandler::SortPlugins(CefRefPtr<CefFrame> frame, CefRefPtr<Callback> callback) {
  BOOST_LOG_TRIVIAL(info) << "Beginning sorting operation.";
  BOOST_LOG_TRIVIAL(info) << "Using message language: " << lootState_.getLanguage().GetName();

  try {
      // Always reload all the plugins.
    SendProgressUpdate(frame, translate("Loading plugin contents..."));
    lootState_.getCurrentGame().LoadPlugins(false);

    //Sort plugins into their load order.
    SendProgressUpdate(frame, translate("Sorting load order..."));
    PluginSorter sorter;
    vector<Plugin> plugins = sorter.Sort(lootState_.getCurrentGame(), lootState_.getLanguage().GetCode());

    // If TESV or FO4, check if load order has been changed.
    if ((lootState_.getCurrentGame().Type() == GameType::tes5 || lootState_.getCurrentGame().Type() == GameType::fo4)
        && equal(begin(plugins), end(plugins), begin(lootState_.getCurrentGame().GetLoadOrder()))) {
        // Load order has not been changed, set it without asking for
        // user input because there are no changes to accept and some
        // plugins' positions may only be inferred and not written to
        // loadorder.txt/plugins.txt.
      std::vector<std::string> newLoadOrder;
      std::transform(begin(plugins),
                     end(plugins),
                     back_inserter(newLoadOrder),
                     [](const Plugin& plugin) {
        return plugin.Name();
      });
      lootState_.getCurrentGame().SetLoadOrder(newLoadOrder);
    }

    YAML::Node node;

    // Store global messages in case they have changed.
    node["globalMessages"] = GetGeneralMessages();

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

      node["plugins"].push_back(pluginNode);
    }
    lootState_.incrementUnappliedChangeCounter();

    if (node.size() > 0)
      callback->Success(JSON::stringify(node));
    else
      callback->Success("null");
  } catch (Error& e) {
    BOOST_LOG_TRIVIAL(error) << "Failed to sort plugins. Details: " << e.what();
    if (e.code() == Error::Code::sorting_error) {
      lootState_.getCurrentGame().AppendMessage(Message(MessageType::error, e.what()));

      YAML::Node node;
      node["globalMessages"] = GetGeneralMessages();
      callback->Success(JSON::stringify(node));
    } else
      callback->Failure(e.codeAsUnsignedInt(), (boost::format(translate("Failed to sort plugins. Details: %1%")) % e.what()).str());
  }
}

std::vector<Message> QueryHandler::GetGeneralMessages() const {
  vector<Message> messages;
  auto metadataListMessages = lootState_.getCurrentGame().GetMasterlist().Messages();
  messages.insert(end(messages),
                  begin(metadataListMessages),
                  end(metadataListMessages));
  metadataListMessages = lootState_.getCurrentGame().GetUserlist().Messages();
  messages.insert(end(messages),
                  begin(metadataListMessages),
                  end(metadataListMessages));
  auto gameMessages = lootState_.getCurrentGame().GetMessages();
  messages.insert(end(messages),
                  begin(gameMessages),
                  end(gameMessages));

  try {
    BOOST_LOG_TRIVIAL(info) << "Using message language: " << lootState_.getLanguage().GetName();
    auto it = begin(messages);
    while (it != end(messages)) {
      if (!it->EvalCondition(lootState_.getCurrentGame(), lootState_.getLanguage().GetCode()))
        it = messages.erase(it);
      else
        ++it;
    }
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << "A global message contains a condition that could not be evaluated. Details: " << e.what();
    messages.push_back(Message(MessageType::error, (format(translate("A global message contains a condition that could not be evaluated. Details: %1%")) % e.what()).str()));
  }

  return messages;
}

YAML::Node QueryHandler::GenerateDerivedMetadata(const Plugin& file, const PluginMetadata& masterlist, const PluginMetadata& userlist) {
  BOOST_LOG_TRIVIAL(info) << "Using message language: " << lootState_.getLanguage().GetName();

  // Now rederive the displayed metadata from the masterlist and userlist.
  Plugin tempPlugin(file);

  tempPlugin.MergeMetadata(masterlist);
  tempPlugin.MergeMetadata(userlist);

  //Evaluate any conditions
  BOOST_LOG_TRIVIAL(trace) << "Evaluate conditions for merged plugin data.";
  try {
    tempPlugin.EvalAllConditions(lootState_.getCurrentGame(), lootState_.getLanguage().GetCode());
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << "\"" << tempPlugin.Name() << "\" contains a condition that could not be evaluated. Details: " << e.what();
    list<Message> messages(tempPlugin.Messages());
    messages.push_back(Message(MessageType::error, (format(translate("\"%1%\" contains a condition that could not be evaluated. Details: %2%")) % tempPlugin.Name() % e.what()).str()));
    tempPlugin.Messages(messages);
  }

  //Also check install validity.
  tempPlugin.CheckInstallValidity(lootState_.getCurrentGame());

  // Now add to pluginNode.
  YAML::Node pluginNode;
  pluginNode["name"] = tempPlugin.Name();
  pluginNode["priority"] = tempPlugin.LocalPriority().getValue();
  pluginNode["globalPriority"] = tempPlugin.GlobalPriority().getValue();
  pluginNode["messages"] = tempPlugin.Messages();
  pluginNode["tags"] = tempPlugin.Tags();
  pluginNode["isDirty"] = !tempPlugin.DirtyInfo().empty();
  pluginNode["loadOrderIndex"] = lootState_.getCurrentGame().GetActiveLoadOrderIndex(tempPlugin.Name());

  if (!tempPlugin.CleanInfo().empty()) {
    pluginNode["cleanedWith"] = tempPlugin.CleanInfo().begin()->CleaningUtility();
  } else {
    pluginNode["cleanedWith"] = "";
  }

  return pluginNode;
}

YAML::Node QueryHandler::GenerateDerivedMetadata(const std::string& pluginName) {
    // Now rederive the displayed metadata from the masterlist and userlist.
  try {
    auto plugin = lootState_.getCurrentGame().GetPlugin(pluginName);
    PluginMetadata master(lootState_.getCurrentGame().GetMasterlist().FindPlugin(plugin));
    PluginMetadata user(lootState_.getCurrentGame().GetUserlist().FindPlugin(plugin));

    return this->GenerateDerivedMetadata(plugin, master, user);
  } catch (...) {
    return YAML::Node();
  }
}

void QueryHandler::CopyToClipboard(const std::string& text) {
#ifdef _WIN32
  if (!OpenClipboard(NULL)) {
    throw Error(Error::Code::windows_error, "Failed to open the Windows clipboard.");
  }

  if (!EmptyClipboard()) {
    throw Error(Error::Code::windows_error, "Failed to empty the Windows clipboard.");
  }

  // The clipboard takes a Unicode (ie. UTF-16) string that it then owns and must not
  // be destroyed by LOOT. Convert the string, then copy it into a new block of
  // memory for the clipboard.
  std::wstring wtext = ToWinWide(text);
  wchar_t * wcstr = new wchar_t[wtext.length() + 1];
  wcscpy(wcstr, wtext.c_str());

  if (SetClipboardData(CF_UNICODETEXT, wcstr) == NULL) {
    throw Error(Error::Code::windows_error, "Failed to copy metadata to the Windows clipboard.");
  }

  if (!CloseClipboard()) {
    throw Error(Error::Code::windows_error, "Failed to close the Windows clipboard.");
  }
#endif
}

void QueryHandler::SendProgressUpdate(CefRefPtr<CefFrame> frame, const std::string& message) {
  BOOST_LOG_TRIVIAL(trace) << "Sending progress update: " << message;
  frame->ExecuteJavaScript("loot.Dialog.showProgress('" + message + "');", frame->GetURL(), 0);
}
}
