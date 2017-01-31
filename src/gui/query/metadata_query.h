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

    return toSimpleMessages(messages, state_.getLanguage().GetCode());
  }

  PluginMetadata getNonUserMetadata(std::shared_ptr<const PluginInterface> file,
                                    const PluginMetadata& masterlistEntry) {
    auto metadata = masterlistEntry;

    auto fileTags = file->GetBashTags();
    auto tags = metadata.Tags();
    tags.insert(begin(fileTags), end(fileTags));
    metadata.Tags(tags);

    auto messages = metadata.Messages();
    auto statusMessages = file->GetStatusMessages();
    auto validityMessages = CheckInstallValidity(file, metadata);
    messages.insert(end(messages), begin(statusMessages), end(statusMessages));
    messages.insert(end(messages), begin(validityMessages), end(validityMessages));
    metadata.Messages(messages);

    return metadata;
  }

  YAML::Node generateDerivedMetadata(std::shared_ptr<const PluginInterface> file,
                                     const PluginMetadata& masterlistEntry,
                                     const PluginMetadata& userlistEntry) {
    auto metadata = getNonUserMetadata(file, masterlistEntry);
    metadata.MergeMetadata(userlistEntry);

    return toYaml(file, metadata);
  }

  YAML::Node generateDerivedMetadata(const std::string& pluginName) {
      // Now rederive the displayed metadata from the masterlist and userlist.
    try {
      auto plugin = state_.getCurrentGame().GetPlugin(pluginName);


      PluginMetadata master(state_.getCurrentGame().GetMasterlistMetadata(pluginName));
      PluginMetadata user(state_.getCurrentGame().GetUserMetadata(pluginName));

      return generateDerivedMetadata(plugin, master, user);
    } catch (...) {
      return YAML::Node();
    }
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
                                                     LanguageCode language) {
    BOOST_LOG_TRIVIAL(info) << "Using message language: " << Language(language).GetName();
    std::vector<SimpleMessage> simpleMessages(messages.size());
    std::transform(begin(messages),
                   end(messages),
                   begin(simpleMessages),
                   [&](const Message& message) {
      return message.ToSimpleMessage(language);
    });

    return simpleMessages;
  }

  YAML::Node toYaml(std::shared_ptr<const PluginInterface> plugin, const PluginMetadata& metadata) {
    BOOST_LOG_TRIVIAL(info) << "Using message language: " << state_.getLanguage().GetName();

    YAML::Node pluginNode;
    pluginNode["name"] = plugin->GetName();
    pluginNode["priority"] = metadata.LocalPriority().getValue();
    pluginNode["globalPriority"] = metadata.GlobalPriority().getValue();
    pluginNode["messages"] = metadata.SimpleMessages(state_.getLanguage().GetCode());
    pluginNode["tags"] = metadata.Tags();
    pluginNode["isDirty"] = !metadata.DirtyInfo().empty();
    pluginNode["loadOrderIndex"] = state_.getCurrentGame().GetActiveLoadOrderIndex(plugin->GetName());

    if (!metadata.CleanInfo().empty()) {
      pluginNode["cleanedWith"] = metadata.CleanInfo().begin()->CleaningUtility();
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

  std::vector<Message> CheckInstallValidity(std::shared_ptr<const PluginInterface> plugin, const PluginMetadata& metadata) {
    BOOST_LOG_TRIVIAL(trace) << "Checking that the current install is valid according to " << plugin->GetName() << "'s data.";
    std::vector<Message> messages;
    if (state_.getCurrentGame().IsPluginActive(plugin->GetName())) {
      auto pluginExists = [&](const std::string& file) {
        return boost::filesystem::exists(state_.getCurrentGame().DataPath() / file)
          || ((boost::iends_with(file, ".esp") || boost::iends_with(file, ".esm")) && boost::filesystem::exists(state_.getCurrentGame().DataPath() / (file + ".ghost")));
      };
      auto tags = metadata.Tags();
      if (tags.find(Tag("Filter")) == std::end(tags)) {
        for (const auto &master : plugin->GetMasters()) {
          if (!pluginExists(master)) {
            BOOST_LOG_TRIVIAL(error) << "\"" << plugin->GetName() << "\" requires \"" << master << "\", but it is missing.";
            messages.push_back(Message(MessageType::error, (boost::format(boost::locale::translate("This plugin requires \"%1%\" to be installed, but it is missing.")) % master).str()));
          } else if (!state_.getCurrentGame().IsPluginActive(master)) {
            BOOST_LOG_TRIVIAL(error) << "\"" << plugin->GetName() << "\" requires \"" << master << "\", but it is inactive.";
            messages.push_back(Message(MessageType::error, (boost::format(boost::locale::translate("This plugin requires \"%1%\" to be active, but it is inactive.")) % master).str()));
          }
        }
      }

      for (const auto &req : metadata.Reqs()) {
        if (!pluginExists(req.Name())) {
          BOOST_LOG_TRIVIAL(error) << "\"" << plugin->GetName() << "\" requires \"" << req.Name() << "\", but it is missing.";
          messages.push_back(Message(MessageType::error, (boost::format(boost::locale::translate("This plugin requires \"%1%\" to be installed, but it is missing.")) % req.Name()).str()));
        }
      }
      for (const auto &inc : metadata.Incs()) {
        if (pluginExists(inc.Name()) && state_.getCurrentGame().IsPluginActive(inc.Name())) {
          BOOST_LOG_TRIVIAL(error) << "\"" << plugin->GetName() << "\" is incompatible with \"" << inc.Name() << "\", but both are present.";
          messages.push_back(Message(MessageType::error, (boost::format(boost::locale::translate("This plugin is incompatible with \"%1%\", but both are present.")) % inc.Name()).str()));
        }
      }
    }

    // Also generate dirty messages.
    for (const auto &element : metadata.DirtyInfo()) {
      messages.push_back(element.AsMessage());
    }

    return messages;
  }

  LootState& state_;
};
}

#endif
