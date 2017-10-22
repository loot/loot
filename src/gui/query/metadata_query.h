/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2017    WrinklyNinja

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

#ifndef LOOT_GUI_QUERY_METADATA_QUERY
#define LOOT_GUI_QUERY_METADATA_QUERY

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

#include "gui/query/derived_plugin_metadata.h"
#include "gui/query/query.h"
#include "loot/exception/file_access_error.h"
#include "loot/exception/git_state_error.h"

namespace loot {
class MetadataQuery : public Query {
protected:
  MetadataQuery(LootState& state) : state_(state) {}

  std::vector<SimpleMessage> getGeneralMessages() const {
    std::vector<Message> messages = state_.getCurrentGame().GetMessages();

    return toSimpleMessages(messages, state_.getLanguage());
  }

  PluginMetadata getNonUserMetadata(const std::shared_ptr<const PluginInterface>& file,
                                    const PluginMetadata& masterlistEntry) {
    auto metadata = masterlistEntry;

    auto fileTags = file->GetBashTags();
    auto tags = metadata.GetTags();
    tags.insert(begin(fileTags), end(fileTags));
    metadata.SetTags(tags);

    auto messages = metadata.GetMessages();
    auto validityMessages = state_.getCurrentGame().CheckInstallValidity(file, metadata);
    messages.insert(end(messages), begin(validityMessages), end(validityMessages));
    metadata.SetMessages(messages);

    return metadata;
  }

  DerivedPluginMetadata generateDerivedMetadata(const std::string& pluginName) {
    try {
      auto plugin = state_.getCurrentGame().GetPlugin(pluginName);

      return generateDerivedMetadata(plugin);
    } catch (...) {
      return DerivedPluginMetadata::none();
    }
  }

  DerivedPluginMetadata generateDerivedMetadata(const std::shared_ptr<const PluginInterface>& plugin) {
    BOOST_LOG_TRIVIAL(trace) << "Getting masterlist metadata for: " << plugin->GetName();
    auto masterlistMetadata = state_.getCurrentGame().GetMasterlistMetadata(plugin->GetName());
    masterlistMetadata = getNonUserMetadata(plugin, masterlistMetadata);

    BOOST_LOG_TRIVIAL(trace) << "Getting userlist metadata for: " << plugin->GetName();
    auto userlistMetadata = state_.getCurrentGame().GetUserMetadata(plugin->GetName());

    auto master = evaluateMasterlistMetadata(plugin->GetName());
    auto user = evaluateUserlistMetadata(plugin->GetName());

    auto evaluatedMetadata = getNonUserMetadata(plugin, master);
    evaluatedMetadata.MergeMetadata(user);

    auto derived = DerivedPluginMetadata(state_, plugin, evaluatedMetadata);

    derived.storeUnevaluatedMetadata(masterlistMetadata, userlistMetadata);

    return derived;
  }

  std::string generateJsonResponse(const std::string& pluginName) {
    YAML::Node response = generateDerivedMetadata(pluginName).toYaml();

    return JSON::stringify(response);
  }

  template<typename InputIterator>
  std::string generateJsonResponse(InputIterator firstPlugin, InputIterator lastPlugin) {
    YAML::Node response;

    auto masterlistInfo = getMasterlistInfo();

    // ID the game using its folder value.
    response["folder"] = state_.getCurrentGame().FolderName();
    response["masterlist"]["revision"] = masterlistInfo.revision_id;
    response["masterlist"]["date"] = masterlistInfo.revision_date;
    response["generalMessages"] = getGeneralMessages();
    response["bashTags"] = state_.getCurrentGame().GetKnownBashTags();

    // Now store plugin data.
    for (auto it = firstPlugin; it != lastPlugin; ++it) {
      response["plugins"].push_back(generateDerivedMetadata(*it).toYaml());
    }

    return JSON::stringify(response);
  }

private:
  static std::vector<SimpleMessage> toSimpleMessages(const std::vector<Message>& messages,
                                                     const std::string& language) {
    BOOST_LOG_TRIVIAL(info) << "Using message language: " << language;
    std::vector<SimpleMessage> simpleMessages(messages.size());
    std::transform(begin(messages),
                   end(messages),
                   begin(simpleMessages),
                   [&](const Message& message) {
      return message.ToSimpleMessage(language);
    });

    return simpleMessages;
  }

  PluginMetadata evaluateMasterlistMetadata(const std::string& pluginName) {
    PluginMetadata master(pluginName);
    try {
      master = state_.getCurrentGame().GetMasterlistMetadata(pluginName, true);
    } catch (std::exception& e) {
      BOOST_LOG_TRIVIAL(error) << "\"" << pluginName << "\"'s masterlist metadata contains a condition that could not be evaluated. Details: " << e.what();
      master.SetMessages({
        Message(MessageType::error, (boost::format(boost::locale::translate("\"%1%\" contains a condition that could not be evaluated. Details: %2%")) % pluginName % e.what()).str()),
      });
    }

    return master;
  }

  PluginMetadata evaluateUserlistMetadata(const std::string& pluginName) {
    PluginMetadata user(pluginName);
    try {
      user = state_.getCurrentGame().GetUserMetadata(pluginName, true);
    } catch (std::exception& e) {
      BOOST_LOG_TRIVIAL(error) << "\"" << pluginName << "\"'s user metadata contains a condition that could not be evaluated. Details: " << e.what();
      user.SetMessages({
        Message(MessageType::error, (boost::format(boost::locale::translate("\"%1%\" contains a condition that could not be evaluated. Details: %2%")) % pluginName % e.what()).str()),
      });
    }

    return user;
  }

  MasterlistInfo getMasterlistInfo() {
    using boost::locale::translate;

    MasterlistInfo info;
    try {
      info = state_.getCurrentGame().GetMasterlistInfo();
      addSuffixIfModified(info);
    } catch (FileAccessError &) {
      BOOST_LOG_TRIVIAL(warning) << "No masterlist present at " << state_.getCurrentGame().MasterlistPath();
      info.revision_id = translate("N/A: No masterlist present").str();
      info.revision_date = translate("N/A: No masterlist present").str();
    } catch (GitStateError &) {
      BOOST_LOG_TRIVIAL(warning) << "Not a Git repository: " << state_.getCurrentGame().MasterlistPath().parent_path();
      info.revision_id = translate("Unknown: Git repository missing").str();
      info.revision_date = translate("Unknown: Git repository missing").str();
    }

    return info;
  }

  void addSuffixIfModified(MasterlistInfo& info) {
    if (info.is_modified) {
      info.revision_date += " " + boost::locale::translate("(edited)").str();
      info.revision_id += " " + boost::locale::translate("(edited)").str();
    }
  }

  LootState& state_;
};
}

#endif
