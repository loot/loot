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

  std::vector<SimpleMessage> getGeneralMessages() {
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
      // Now rederive the displayed metadata from the masterlist and userlist.
    std::shared_ptr<const PluginInterface> plugin = nullptr;
    try {
      plugin = state_.getCurrentGame().GetPlugin(pluginName);
    } catch (...) {
      return DerivedPluginMetadata::none();
    }

    PluginMetadata master(pluginName);
    try {
      master = state_.getCurrentGame().GetMasterlistMetadata(pluginName, true);
    } catch (std::exception& e) {
      BOOST_LOG_TRIVIAL(error) << "\"" << pluginName << "\"'s masterlist metadata contains a condition that could not be evaluated. Details: " << e.what();
      master.SetMessages({
        Message(MessageType::error, (boost::format(boost::locale::translate("\"%1%\" contains a condition that could not be evaluated. Details: %2%")) % pluginName % e.what()).str()),
      });
    }

    PluginMetadata user(pluginName);
    try {
      user = state_.getCurrentGame().GetUserMetadata(pluginName, true);
    } catch (std::exception& e) {
      BOOST_LOG_TRIVIAL(error) << "\"" << pluginName << "\"'s user metadata contains a condition that could not be evaluated. Details: " << e.what();
      user.SetMessages({
        Message(MessageType::error, (boost::format(boost::locale::translate("\"%1%\" contains a condition that could not be evaluated. Details: %2%")) % pluginName % e.what()).str()),
      });
    }

    return generateDerivedMetadata(plugin, master, user);
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

  std::string generateJsonResponse(const std::string& pluginName) {
    YAML::Node response = generateDerivedMetadata(pluginName).toYaml();

    return JSON::stringify(response);
  }

  static std::vector<EditorMessage> toEditorMessages(const std::vector<Message>& messages, const std::string& language) {
    std::vector<EditorMessage> list;

    for (const auto& message : messages) {
      list.push_back(EditorMessage(message, language));
    }

    return list;
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

  DerivedPluginMetadata generateDerivedMetadata(const std::shared_ptr<const PluginInterface>& file,
                                     const PluginMetadata& masterlistEntry,
                                     const PluginMetadata& userlistEntry) {
    auto metadata = getNonUserMetadata(file, masterlistEntry);
    metadata.MergeMetadata(userlistEntry);

    return DerivedPluginMetadata(state_, file, metadata);
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
