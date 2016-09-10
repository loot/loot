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

#include "backend/game/game.h"
#include "backend/plugin/plugin.h"
#include "gui/query/query.h"

namespace loot {
class MetadataQuery : public Query {
protected:
  MetadataQuery(Game& game, const LanguageCode language) :
    game_(game), language_(language) {}

  std::vector<SimpleMessage> getGeneralMessages() {
    std::vector<Message> messages;
    appendMessages(messages, game_.GetMasterlist().Messages());
    appendMessages(messages, game_.GetUserlist().Messages());
    appendMessages(messages, game_.GetMessages());

    evaluateMessageConditions(messages);

    return toSimpleMessages(messages, language_);
  }

  YAML::Node generateDerivedMetadata(const Plugin& file,
                                     const PluginMetadata& masterlistEntry,
                                     const PluginMetadata& userlistEntry) {
    Plugin plugin(file);

    plugin.MergeMetadata(masterlistEntry);
    plugin.MergeMetadata(userlistEntry);

    evaluatePlugin(plugin);

    return toYaml(plugin);
  }

  YAML::Node generateDerivedMetadata(const std::string& pluginName) {
      // Now rederive the displayed metadata from the masterlist and userlist.
    try {
      auto plugin = game_.GetPlugin(pluginName);
      PluginMetadata master(game_.GetMasterlist().FindPlugin(plugin));
      PluginMetadata user(game_.GetUserlist().FindPlugin(plugin));

      return generateDerivedMetadata(plugin, master, user);
    } catch (...) {
      return YAML::Node();
    }
  }

private:
  static void appendMessages(std::vector<Message>& destination,
                             const std::vector<Message>& source) {
    destination.insert(end(destination), begin(source), end(source));
  }

  void evaluateMessageConditions(std::vector<Message>& messages) {
    try {
      auto it = begin(messages);
      while (it != end(messages)) {
        if (!it->EvalCondition(game_))
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

  void evaluatePlugin(Plugin& plugin) {
    //Evaluate any conditions
    BOOST_LOG_TRIVIAL(trace) << "Evaluate conditions for merged plugin data.";
    try {
      plugin.EvalAllConditions(game_);
    } catch (std::exception& e) {
      BOOST_LOG_TRIVIAL(error) << "\"" << plugin.Name() << "\" contains a condition that could not be evaluated. Details: " << e.what();
      std::vector<Message> messages(plugin.Messages());
      messages.push_back(Message(MessageType::error, (boost::format(boost::locale::translate("\"%1%\" contains a condition that could not be evaluated. Details: %2%")) % plugin.Name() % e.what()).str()));
      plugin.Messages(messages);
    }

    //Also check install validity.
    plugin.CheckInstallValidity(game_);
  }

  YAML::Node toYaml(const Plugin& plugin) {
    BOOST_LOG_TRIVIAL(info) << "Using message language: " << Language(language_).GetName();

    YAML::Node pluginNode;
    pluginNode["name"] = plugin.Name();
    pluginNode["priority"] = plugin.LocalPriority().getValue();
    pluginNode["globalPriority"] = plugin.GlobalPriority().getValue();
    pluginNode["messages"] = plugin.SimpleMessages(language_);
    pluginNode["tags"] = plugin.Tags();
    pluginNode["isDirty"] = !plugin.DirtyInfo().empty();
    pluginNode["loadOrderIndex"] = game_.GetActiveLoadOrderIndex(plugin.Name());

    if (!plugin.CleanInfo().empty()) {
      pluginNode["cleanedWith"] = plugin.CleanInfo().begin()->CleaningUtility();
    } else {
      pluginNode["cleanedWith"] = "";
    }

    return pluginNode;
  }

  Game& game_;
  const LanguageCode language_;
};
}

#endif
