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

#ifndef LOOT_GUI_QUERY_METADATA_QUERY
#define LOOT_GUI_QUERY_METADATA_QUERY

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

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

  YAML::Node generateDerivedMetadata(const std::string& pluginName) {
      // Now rederive the displayed metadata from the masterlist and userlist.
    std::shared_ptr<const PluginInterface> plugin = nullptr;
    try {
      plugin = state_.getCurrentGame().GetPlugin(pluginName);
    } catch (...) {
      return YAML::Node();
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

  YAML::Node getMasterlistInfo() {
    using boost::locale::translate;

    YAML::Node masterlistNode;
    try {
      MasterlistInfo info = state_.getCurrentGame().GetMasterlistInfo();
      addSuffixIfModified(info);

      masterlistNode["revision"] = info.revision_id;
      masterlistNode["date"] = info.revision_date;
    } catch (FileAccessError &) {
      BOOST_LOG_TRIVIAL(warning) << "No masterlist present at " << state_.getCurrentGame().MasterlistPath();
      masterlistNode["revision"] = translate("N/A: No masterlist present").str();
      masterlistNode["date"] = translate("N/A: No masterlist present").str();
    } catch (GitStateError &) {
      BOOST_LOG_TRIVIAL(warning) << "Not a Git repository: " << state_.getCurrentGame().MasterlistPath().parent_path();
      masterlistNode["revision"] = translate("Unknown: Git repository missing").str();
      masterlistNode["date"] = translate("Unknown: Git repository missing").str();
    }

    return masterlistNode;
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

  YAML::Node generateDerivedMetadata(const std::shared_ptr<const PluginInterface>& file,
                                     const PluginMetadata& masterlistEntry,
                                     const PluginMetadata& userlistEntry) {
    auto metadata = getNonUserMetadata(file, masterlistEntry);
    metadata.MergeMetadata(userlistEntry);

    return toYaml(file, metadata);
  }

  YAML::Node toYaml(const std::shared_ptr<const PluginInterface>& plugin,
                    const PluginMetadata& metadata) {
    BOOST_LOG_TRIVIAL(info) << "Using message language: " << state_.getLanguage();

    YAML::Node pluginNode;
    pluginNode["name"] = plugin->GetName();
    pluginNode["priority"] = metadata.GetLocalPriority().GetValue();
    pluginNode["globalPriority"] = metadata.GetGlobalPriority().GetValue();
    pluginNode["messages"] = metadata.GetSimpleMessages(state_.getLanguage());
    pluginNode["tags"] = metadata.GetTags();
    pluginNode["isDirty"] = !metadata.GetDirtyInfo().empty();
    pluginNode["loadOrderIndex"] = state_.getCurrentGame().GetActiveLoadOrderIndex(plugin->GetName());

    if (!metadata.GetCleanInfo().empty()) {
      pluginNode["cleanedWith"] = metadata.GetCleanInfo().begin()->GetCleaningUtility();
    } else {
      pluginNode["cleanedWith"] = "";
    }

    return pluginNode;
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
