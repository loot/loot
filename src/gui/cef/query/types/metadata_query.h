/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2018    WrinklyNinja

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

#include <boost/format.hpp>
#include <boost/locale.hpp>

#include "gui/cef/query/derived_plugin_metadata.h"
#include "gui/cef/query/query.h"
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

  PluginMetadata getNonUserMetadata(
      const std::shared_ptr<const PluginInterface>& file,
      const PluginMetadata& masterlistEntry) {
    auto metadata = masterlistEntry;

    auto fileTags = file->GetBashTags();
    auto tags = metadata.GetTags();
    tags.insert(begin(fileTags), end(fileTags));
    metadata.SetTags(tags);

    return metadata;
  }

  DerivedPluginMetadata generateDerivedMetadata(const std::string& pluginName) {
    auto plugin = state_.getCurrentGame().GetPlugin(pluginName);
    if (plugin) {
      return generateDerivedMetadata(plugin);
    } else {
      return DerivedPluginMetadata::none();
    }
  }

  DerivedPluginMetadata generateDerivedMetadata(
      const std::shared_ptr<const PluginInterface>& plugin) {
    auto logger = state_.getLogger();
    if (logger) {
      logger->trace("Getting masterlist metadata for: {}", plugin->GetName());
    }
    auto masterlistMetadata =
        state_.getCurrentGame().GetMasterlistMetadata(plugin->GetName());
    masterlistMetadata = getNonUserMetadata(plugin, masterlistMetadata);

    if (logger) {
      logger->trace("Getting userlist metadata for: {}", plugin->GetName());
    }
    auto userlistMetadata =
        state_.getCurrentGame().GetUserMetadata(plugin->GetName());

    auto evaluatedMetadata = evaluateMasterlistMetadata(plugin->GetName());
    evaluatedMetadata.MergeMetadata(evaluateUserlistMetadata(plugin->GetName()));

    auto messages = evaluatedMetadata.GetMessages();
    auto validityMessages =
      state_.getCurrentGame().CheckInstallValidity(plugin, evaluatedMetadata);
    messages.insert(
      end(messages), begin(validityMessages), end(validityMessages));
    evaluatedMetadata.SetMessages(messages);

    auto derived = DerivedPluginMetadata(state_, plugin, evaluatedMetadata);

    derived.storeUnevaluatedMetadata(
        masterlistMetadata, userlistMetadata);

    return derived;
  }

  std::string generateJsonResponse(const std::string& pluginName) {
    nlohmann::json json = generateDerivedMetadata(pluginName);

    return json.dump();
  }

  template<typename InputIterator>
  std::string generateJsonResponse(InputIterator firstPlugin,
                                   InputIterator lastPlugin) {
    nlohmann::json json = {
      { "folder", state_.getCurrentGame().FolderName() },
      { "masterlist", getMasterlistInfo() },
      { "generalMessages", getGeneralMessages() },
      { "bashTags", state_.getCurrentGame().GetKnownBashTags() },
      { "groups", {
          { "masterlist", state_.getCurrentGame().GetMasterlistGroups() },
          { "userlist", state_.getCurrentGame().GetUserGroups() },
        }
      },
      { "plugins", nlohmann::json::array() },
    };

    for (auto it = firstPlugin; it != lastPlugin; ++it) {
      json["plugins"].push_back(generateDerivedMetadata(*it));
    }

    return json.dump();
  }

private:
  static std::vector<SimpleMessage> toSimpleMessages(
      const std::vector<Message>& messages,
      const std::string& language) {
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
      auto logger = state_.getLogger();
      if (logger) {
        logger->error("\"{}\"'s masterlist metadata contains a condition that "
                      "could not be evaluated. Details: {}",
                      pluginName,
                      e.what());
      }
      master.SetMessages({
          Message(MessageType::error,
                  (boost::format(boost::locale::translate(
                       "\"%1%\" contains a condition that could not be "
                       "evaluated. Details: %2%")) %
                   pluginName % e.what())
                      .str()),
      });
    }

    return master;
  }

  PluginMetadata evaluateUserlistMetadata(const std::string& pluginName) {
    PluginMetadata user(pluginName);
    try {
      user = state_.getCurrentGame().GetUserMetadata(pluginName, true);
    } catch (std::exception& e) {
      auto logger = state_.getLogger();
      if (logger) {
        logger->error("\"{}\"'s user metadata contains a condition that could "
                      "not be evaluated. Details: {}", pluginName, e.what());
      }
      user.SetMessages({
          Message(MessageType::error,
                  (boost::format(boost::locale::translate(
                       "\"%1%\" contains a condition that could not be "
                       "evaluated. Details: %2%")) %
                   pluginName % e.what())
                      .str()),
      });
    }

    return user;
  }

  MasterlistInfo getMasterlistInfo() {
    using boost::locale::translate;

    MasterlistInfo info;
    auto logger = state_.getLogger();
    try {
      info = state_.getCurrentGame().GetMasterlistInfo();
      addSuffixIfModified(info);
    } catch (FileAccessError&) {
      if (logger) {
        logger->warn("No masterlist present at {}",
          state_.getCurrentGame().MasterlistPath().u8string());
      }
      info.revision_id = translate("N/A: No masterlist present").str();
      info.revision_date = translate("N/A: No masterlist present").str();
    } catch (GitStateError&) {
      if (logger) {
        logger->warn("Not a Git repository: {}",
          state_.getCurrentGame().MasterlistPath().parent_path().u8string());
      }
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
