/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013-2016    WrinklyNinja

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

#include "api/api_database.h"

#include <vector>
#include <unordered_map>

#include "loot/error.h"
#include "backend/game/game.h"
#include "backend/plugin/plugin_sorter.h"

namespace loot {
ApiDatabase::ApiDatabase(const GameType game, const std::string& gamePath, const std::string& gameLocalDataPath)
  : game_(Game(GameType(game))) {
  game_.SetGamePath(gamePath);
  game_.Init(false, gameLocalDataPath);
}

///////////////////////////////////
// Database Loading Functions
///////////////////////////////////

void ApiDatabase::LoadLists(const std::string& masterlistPath,
                            const std::string& userlistPath) {
  Masterlist temp;
  MetadataList userTemp;

  if (boost::filesystem::exists(masterlistPath)) {
    temp.Load(masterlistPath);
  } else {
    throw Error(Error::Code::path_not_found, std::string("The given masterlist path does not exist: ") + masterlistPath);
  }

  if (!userlistPath.empty()) {
    if (boost::filesystem::exists(userlistPath)) {
      userTemp.Load(userlistPath);
    } else {
      throw Error(Error::Code::path_not_found, std::string("The given userlist path does not exist: ") + userlistPath);
    }
  }

  game_.GetMasterlist() = temp;
  unevaluatedMasterlist_ = temp;
  game_.GetUserlist() = userTemp;
  unevaluatedUserlist_ = userTemp;
}

void ApiDatabase::EvalLists() {
  // Clear caches before evaluating conditions.
  game_.ClearCachedConditions();

  Masterlist temp = unevaluatedMasterlist_;
  MetadataList userTemp = unevaluatedUserlist_;

  // Refresh active plugins before evaluating conditions.
  temp.EvalAllConditions(game_);
  userTemp.EvalAllConditions(game_);

  game_.GetMasterlist() = temp;
  game_.GetUserlist() = userTemp;
}

////////////////////////////////////
// LOOT Functionality Functions
////////////////////////////////////

std::vector<std::string> ApiDatabase::SortPlugins(const std::vector<std::string>& plugins) {
  // Always reload all the plugins.
  game_.LoadPlugins(plugins, false);

  //Sort plugins into their load order.
  PluginSorter sorter;
  auto list = sorter.Sort(game_, LanguageCode::english);

  std::vector<std::string> loadOrder(list.size());
  std::transform(begin(list), end(list), begin(loadOrder), [](const Plugin& plugin) {
    return plugin.Name();
  });

  return loadOrder;
}

bool ApiDatabase::UpdateMasterlist(const std::string& masterlistPath,
                                   const std::string& remoteURL,
                                   const std::string& remoteBranch) {
  if (!boost::filesystem::is_directory(boost::filesystem::path(masterlistPath).parent_path()))
    throw std::invalid_argument("Given masterlist path \"" + masterlistPath + "\" does not have a valid parent directory.");

  Masterlist masterlist;
  return masterlist.Update(masterlistPath, remoteURL, remoteBranch);
}

MasterlistInfo ApiDatabase::GetMasterlistRevision(const std::string& masterlistPath,
                                                  const bool getShortID) {
  MasterlistInfo apiMasterlistInfo;
  apiMasterlistInfo.is_modified = false;
  try {
    Masterlist::Info info = Masterlist::GetInfo(masterlistPath, getShortID);

    if (boost::ends_with(info.revision, " (edited)")) {
      apiMasterlistInfo.revision_id = info.revision.substr(0, info.revision.length() - 9);
      apiMasterlistInfo.revision_date = info.date.substr(0, info.date.length() - 9);
      apiMasterlistInfo.is_modified = true;
    } else {
      apiMasterlistInfo.revision_id = info.revision;
      apiMasterlistInfo.revision_date = info.date;
      apiMasterlistInfo.is_modified = false;
    }
  } catch (Error &e) {
    if (e.code() != Error::Code::ok)
      throw;
  }

  return apiMasterlistInfo;
}

//////////////////////////
// DB Access Functions
//////////////////////////

PluginTags ApiDatabase::GetPluginTags(const std::string& plugin) {
  PluginTags tags;

  PluginMetadata pluginMetadata = game_.GetMasterlist().FindPlugin(PluginMetadata(plugin));
  for (const auto &tag : pluginMetadata.Tags()) {
    if (tag.IsAddition())
      tags.added.insert(tag.Name());
    else
      tags.removed.insert(tag.Name());
  }

  pluginMetadata = game_.GetUserlist().FindPlugin(PluginMetadata(plugin));
  tags.userlist_modified = !pluginMetadata.Tags().empty();
  for (const auto &tag : pluginMetadata.Tags()) {
    if (tag.IsAddition())
      tags.added.insert(tag.Name());
    else
      tags.removed.insert(tag.Name());
  }

  return tags;
}

std::vector<SimpleMessage> ApiDatabase::GetPluginMessages(const std::string& plugin,
                                                          const LanguageCode language) {
  std::vector<SimpleMessage> messages;

  PluginMetadata pluginMetadata = game_.GetMasterlist().FindPlugin(PluginMetadata(plugin));
  for (const auto& message : pluginMetadata.SimpleMessages(language)) {
    messages.push_back(message);
  }

  pluginMetadata = game_.GetUserlist().FindPlugin(PluginMetadata(plugin));
  for (const auto& message : pluginMetadata.SimpleMessages(language)) {
    messages.push_back(message);
  }

  return messages;
}

PluginCleanliness ApiDatabase::GetPluginCleanliness(const std::string& plugin) {
  // Is there any dirty info? Testing for applicability happens in loot_eval_lists().
  if (!game_.GetMasterlist().FindPlugin(PluginMetadata(plugin)).DirtyInfo().empty()
      || !game_.GetUserlist().FindPlugin(PluginMetadata(plugin)).DirtyInfo().empty()) {
    return PluginCleanliness::dirty;
  }

  // Is there a message beginning with the substring "Do not clean."?
  // This isn't a very reliable system, because if the lists have been evaluated in some language
  // other than English, the strings will be in different languages (and the API can't tell what they'd be)
  // and the strings may be non-standard and begin with something other than "Do not clean." anyway.
  std::vector<Message> messages(game_.GetMasterlist().FindPlugin(PluginMetadata(plugin)).Messages());

  for (const auto& message : messages) {
    if (boost::starts_with(message.GetContent(LanguageCode::english).GetText(), "Do not clean")) {
      return PluginCleanliness::do_not_clean;
    }
  }

  messages = game_.GetUserlist().FindPlugin(PluginMetadata(plugin)).Messages();

  for (const auto& message : messages) {
    if (boost::starts_with(message.GetContent(LanguageCode::english).GetText(), "Do not clean")) {
      return PluginCleanliness::do_not_clean;
    }
  }

  return PluginCleanliness::unknown;
}

// Writes a minimal masterlist that only contains mods that have Bash Tag suggestions,
// and/or dirty messages, plus the Tag suggestions and/or messages themselves and their
// conditions, in order to create the Wrye Bash taglist. outputFile is the path to use
// for output. If outputFile already exists, it will only be overwritten if overwrite is true.
void ApiDatabase::WriteMinimalList(const std::string& outputFile, const bool overwrite) {
  if (!boost::filesystem::exists(boost::filesystem::path(outputFile).parent_path()))
    throw std::invalid_argument("Output directory does not exist.");

  if (boost::filesystem::exists(outputFile) && !overwrite)
    throw Error(Error::Code::path_write_fail, "Output file exists but overwrite is not set to true.");

  Masterlist temp = game_.GetMasterlist();
  std::unordered_set<PluginMetadata> minimalPlugins;
  for (const auto &plugin : temp.Plugins()) {
    PluginMetadata p(plugin.Name());
    p.Tags(plugin.Tags());
    p.DirtyInfo(plugin.DirtyInfo());
    minimalPlugins.insert(p);
  }

  YAML::Emitter yout;
  yout.SetIndent(2);
  yout << YAML::BeginMap
    << YAML::Key << "plugins" << YAML::Value << minimalPlugins
    << YAML::EndMap;

  boost::filesystem::path p(outputFile);
  boost::filesystem::ofstream out(p);
  if (out.fail())
    throw Error(Error::Code::path_write_fail, "Couldn't open output file.");
  out << yout.c_str();
  out.close();
}
}
