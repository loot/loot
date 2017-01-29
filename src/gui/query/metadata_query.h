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

#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

#include "backend/plugin/plugin.h"
#include "backend/metadata/condition_evaluator.h"
#include "gui/query/query.h"
#include "loot/exception/file_access_error.h"
#include "loot/exception/git_state_error.h"

namespace loot {
class MetadataQuery : public Query {
protected:
  MetadataQuery(LootState& state) : state_(state) {}

  std::vector<SimpleMessage> getGeneralMessages() {
    std::vector<Message> messages;
    appendMessages(messages, state_.getCurrentGame().GetMasterlist().Messages());
    appendMessages(messages, state_.getCurrentGame().GetUserlist().Messages());
    appendMessages(messages, state_.getCurrentGame().GetMessages());

    evaluateMessageConditions(messages);

    return toSimpleMessages(messages, state_.getLanguage().GetCode());
  }

  YAML::Node generateDerivedMetadata(const Plugin& file,
                                     const PluginMetadata& masterlistEntry,
                                     const PluginMetadata& userlistEntry) {
    Plugin plugin(file);

    plugin.MergeMetadata(evaluateMetadata(masterlistEntry));
    plugin.MergeMetadata(evaluateMetadata(userlistEntry));

    plugin.CheckInstallValidity(state_.getCurrentGame());

    return toYaml(plugin);
  }

  YAML::Node generateDerivedMetadata(const std::string& pluginName) {
      // Now rederive the displayed metadata from the masterlist and userlist.
    try {
      auto plugin = state_.getCurrentGame().GetPlugin(pluginName);
      PluginMetadata master(state_.getCurrentGame().GetMasterlist().FindPlugin(plugin));
      PluginMetadata user(state_.getCurrentGame().GetUserlist().FindPlugin(plugin));

      return generateDerivedMetadata(plugin, master, user);
    } catch (...) {
      return YAML::Node();
    }
  }

  YAML::Node getMasterlistInfo() {
    using boost::locale::translate;

    YAML::Node masterlistNode;
    try {
      MasterlistInfo info = state_.getCurrentGame().GetMasterlist().GetInfo(state_.getCurrentGame().MasterlistPath(), true);
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
  static void appendMessages(std::vector<Message>& destination,
                             const std::vector<Message>& source) {
    destination.insert(end(destination), begin(source), end(source));
  }

  void evaluateMessageConditions(std::vector<Message>& messages) {
    try {
      ConditionEvaluator evaluator(&state_.getCurrentGame());
      auto it = begin(messages);
      while (it != end(messages)) {
        if (!evaluator.evaluate(it->Condition()))
          it = messages.erase(it);
        else
          ++it;
      }
    } catch (std::exception& e) {
      BOOST_LOG_TRIVIAL(error) << "A global message contains a condition that could not be evaluated. Details: " << e.what();
      messages.push_back(Message(MessageType::error, (boost::format(boost::locale::translate("A global message contains a condition that could not be evaluated. Details: %1%")) % e.what()).str()));
    }
  }

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

  PluginMetadata evaluateMetadata(const PluginMetadata& pluginMetadata) {
    BOOST_LOG_TRIVIAL(trace) << "Evaluate conditions for merged plugin data.";
    try {
      ConditionEvaluator evaluator(&state_.getCurrentGame());
      return evaluator.evaluateAll(pluginMetadata);
    } catch (std::exception& e) {
      BOOST_LOG_TRIVIAL(error) << "\"" << pluginMetadata.Name() << "\" contains a condition that could not be evaluated. Details: " << e.what();
      std::vector<Message> messages(pluginMetadata.Messages());
      messages.push_back(Message(MessageType::error, (boost::format(boost::locale::translate("\"%1%\" contains a condition that could not be evaluated. Details: %2%")) % pluginMetadata.Name() % e.what()).str()));
      
      PluginMetadata newMetadata(pluginMetadata);
      newMetadata.Messages(messages);
      
      return newMetadata;
    }
  }

  YAML::Node toYaml(const Plugin& plugin) {
    BOOST_LOG_TRIVIAL(info) << "Using message language: " << state_.getLanguage().GetName();

    YAML::Node pluginNode;
    pluginNode["name"] = plugin.Name();
    pluginNode["priority"] = plugin.LocalPriority().getValue();
    pluginNode["globalPriority"] = plugin.GlobalPriority().getValue();
    pluginNode["messages"] = plugin.SimpleMessages(state_.getLanguage().GetCode());
    pluginNode["tags"] = plugin.Tags();
    pluginNode["isDirty"] = !plugin.DirtyInfo().empty();
    pluginNode["loadOrderIndex"] = state_.getCurrentGame().GetActiveLoadOrderIndex(plugin.Name());

    if (!plugin.CleanInfo().empty()) {
      pluginNode["cleanedWith"] = plugin.CleanInfo().begin()->CleaningUtility();
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
