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

#include <boost/algorithm/string.hpp>

#include "loot/exception/file_access_error.h"
#include "loot/yaml/plugin_metadata.h"
#include "api/game/game.h"
#include "api/metadata/condition_evaluator.h"
#include "api/plugin/plugin_sorter.h"

namespace loot {
ApiDatabase::ApiDatabase(Game& game) : game_(game) {}

///////////////////////////////////
// Database Loading Functions
///////////////////////////////////

void ApiDatabase::LoadLists(const std::string& masterlistPath,
                            const std::string& userlistPath) {
  Masterlist temp;
  MetadataList userTemp;

  if (!masterlistPath.empty()) {
    if (boost::filesystem::exists(masterlistPath)) {
      temp.Load(masterlistPath);
    } else {
      throw FileAccessError("The given masterlist path does not exist: " + masterlistPath);
    }
  }

  if (!userlistPath.empty()) {
    if (boost::filesystem::exists(userlistPath)) {
      userTemp.Load(userlistPath);
    } else {
      throw FileAccessError("The given userlist path does not exist: " + userlistPath);
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

void ApiDatabase::WriteUserMetadata(const std::string& outputFile, const bool overwrite) const {
  if (!boost::filesystem::exists(boost::filesystem::path(outputFile).parent_path()))
    throw std::invalid_argument("Output directory does not exist.");

  if (boost::filesystem::exists(outputFile) && !overwrite)
    throw FileAccessError("Output file exists but overwrite is not set to true.");

  game_.GetUserlist().Save(outputFile);
}

////////////////////////////////////
// LOOT Functionality Functions
////////////////////////////////////

bool ApiDatabase::UpdateMasterlist(const std::string& masterlistPath,
                                   const std::string& remoteURL,
                                   const std::string& remoteBranch) {
  if (!boost::filesystem::is_directory(boost::filesystem::path(masterlistPath).parent_path()))
    throw std::invalid_argument("Given masterlist path \"" + masterlistPath + "\" does not have a valid parent directory.");

  Masterlist masterlist;
  if (masterlist.Update(masterlistPath, remoteURL, remoteBranch)) {
    game_.GetMasterlist() = masterlist;
    unevaluatedMasterlist_ = masterlist;
    return true;
  }

  return false;
}

MasterlistInfo ApiDatabase::GetMasterlistRevision(const std::string& masterlistPath,
                                                  const bool getShortID) const {
  return Masterlist::GetInfo(masterlistPath, getShortID);
}

//////////////////////////
// DB Access Functions
//////////////////////////

std::set<std::string> ApiDatabase::GetKnownBashTags() const {
  auto masterlistTags = game_.GetMasterlist().BashTags();
  auto userlistTags = game_.GetUserlist().BashTags();

  if (!userlistTags.empty()) {
    masterlistTags.insert(std::begin(userlistTags), std::end(userlistTags));
  }

  return masterlistTags;
}

std::vector<Message> ApiDatabase::GetGeneralMessages(bool evaluateConditions) const {
  auto masterlistMessages = game_.GetMasterlist().Messages();
  auto userlistMessages = game_.GetUserlist().Messages();

  if (!userlistMessages.empty()) {
    masterlistMessages.insert(std::end(masterlistMessages), std::begin(userlistMessages), std::end(userlistMessages));
  }

  if (evaluateConditions) {
    // Evaluate conditions from scratch.
    game_.ClearCachedConditions();
    ConditionEvaluator evaluator(&game_);
    for (auto it = std::begin(masterlistMessages); it != std::end(masterlistMessages);) {
      if (!evaluator.evaluate(it->GetCondition()))
        it = masterlistMessages.erase(it);
      else
        ++it;
    }
  }

  return masterlistMessages;
}

PluginMetadata ApiDatabase::GetPluginMetadata(const std::string& plugin,
                                              bool includeUserMetadata,
                                              bool evaluateConditions) const {
  PluginMetadata metadata = game_.GetMasterlist().FindPlugin(plugin);

  if (includeUserMetadata) {
    metadata.MergeMetadata(game_.GetUserlist().FindPlugin(plugin));
  }

  if (evaluateConditions) {
    ConditionEvaluator evaluator(&game_);
    return evaluator.evaluateAll(metadata);
  }

  return metadata;
}

PluginMetadata ApiDatabase::GetPluginUserMetadata(const std::string& plugin,
                                                  bool evaluateConditions) const {
  PluginMetadata metadata = game_.GetUserlist().FindPlugin(plugin);

  if (evaluateConditions) {
    ConditionEvaluator evaluator(&game_);
    return evaluator.evaluateAll(metadata);
  }

  return metadata;
}

void ApiDatabase::SetPluginUserMetadata(const PluginMetadata& pluginMetadata) {
  game_.GetUserlist().ErasePlugin(pluginMetadata);
  game_.GetUserlist().AddPlugin(pluginMetadata);
}

void ApiDatabase::DiscardPluginUserMetadata(const std::string& plugin) {
  game_.GetUserlist().ErasePlugin(plugin);
}

void ApiDatabase::DiscardAllUserMetadata() {
  game_.GetUserlist().Clear();
}

// Writes a minimal masterlist that only contains mods that have Bash Tag suggestions,
// and/or dirty messages, plus the Tag suggestions and/or messages themselves and their
// conditions, in order to create the Wrye Bash taglist. outputFile is the path to use
// for output. If outputFile already exists, it will only be overwritten if overwrite is true.
void ApiDatabase::WriteMinimalList(const std::string& outputFile, const bool overwrite) const {
  if (!boost::filesystem::exists(boost::filesystem::path(outputFile).parent_path()))
    throw std::invalid_argument("Output directory does not exist.");

  if (boost::filesystem::exists(outputFile) && !overwrite)
    throw FileAccessError("Output file exists but overwrite is not set to true.");

  Masterlist temp = game_.GetMasterlist();
  std::unordered_set<PluginMetadata> minimalPlugins;
  for (const auto &plugin : temp.Plugins()) {
    PluginMetadata p(plugin.GetName());
    p.SetTags(plugin.GetTags());
    p.SetDirtyInfo(plugin.GetDirtyInfo());
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
    throw FileAccessError("Couldn't open output file.");
  out << yout.c_str();
  out.close();
}
}
