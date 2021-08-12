/*  LOOT

A load order optimisation tool for
Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

Copyright (C) 2014 WrinklyNinja

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
#include "gui/state/game/helpers.h"
#include "loot/exception/file_access_error.h"
#include "loot/exception/git_state_error.h"

namespace loot {
template<typename G = gui::Game>
class MetadataQuery : public Query {
protected:
  MetadataQuery(G& game, std::string language) :
      game_(game),
      language_(language),
      logger_(getLogger()) {}

  std::vector<SimpleMessage> getGeneralMessages() const {
    std::vector<Message> messages = game_.GetMessages();

    return toSimpleMessages(messages, language_);
  }

  std::optional<PluginMetadata> getNonUserMetadata(
      const std::shared_ptr<const PluginInterface>& file) {
    auto fileBashTags = file->GetBashTags();
    auto masterlistMetadata = game_.GetMasterlistMetadata(file->GetName());

    if (fileBashTags.empty()) {
      return masterlistMetadata;
    }

    PluginMetadata metadata(file->GetName());
    metadata.SetTags(fileBashTags);

    if (masterlistMetadata.has_value()) {
      masterlistMetadata.value().MergeMetadata(metadata);
      return masterlistMetadata.value();
    }

    return metadata;
  }

  std::optional<DerivedPluginMetadata<G>> generateDerivedMetadata(
      const std::string& pluginName) {
    auto plugin = game_.GetPlugin(pluginName);
    if (plugin) {
      return generateDerivedMetadata(plugin);
    }

    return std::nullopt;
  }

  DerivedPluginMetadata<G> generateDerivedMetadata(
      const std::shared_ptr<const PluginInterface>& plugin) {
    auto derived = DerivedPluginMetadata<G>(plugin, game_, language_);

    auto nonUserMetadata = getNonUserMetadata(plugin);
    if (nonUserMetadata.has_value()) {
      derived.setMasterlistMetadata(nonUserMetadata.value());
    }

    auto userMetadata = game_.GetUserMetadata(plugin->GetName());
    if (userMetadata.has_value()) {
      derived.setUserMetadata(userMetadata.value());
    }

    auto evaluatedMetadata = evaluateMetadata(plugin->GetName());
    if (!evaluatedMetadata.has_value()) {
      evaluatedMetadata = PluginMetadata(plugin->GetName());
    }

    auto messages = evaluatedMetadata.value().GetMessages();
    auto validityMessages =
        game_.CheckInstallValidity(plugin, evaluatedMetadata.value());
    messages.insert(
        end(messages), begin(validityMessages), end(validityMessages));
    evaluatedMetadata.value().SetMessages(messages);

    derived.setEvaluatedMetadata(evaluatedMetadata.value());

    return derived;
  }

  std::string generateJsonResponse(const std::string& pluginName) {
    auto derivedMetadata = generateDerivedMetadata(pluginName);
    if (derivedMetadata.has_value()) {
      return nlohmann::json(derivedMetadata.value()).dump();
    }

    return "";
  }

  template<typename InputIterator>
  std::string generateJsonResponse(InputIterator firstPlugin,
                                   InputIterator lastPlugin) {
    nlohmann::json json = {
        {"folder", game_.FolderName()},
        {"masterlist", getMasterlistInfo()},
        {"generalMessages", getGeneralMessages()},
        {"bashTags", game_.GetKnownBashTags()},
        {"groups",
         {
             {"masterlist", game_.GetMasterlistGroups()},
             {"userlist", game_.GetUserGroups()},
         }},
        {"plugins", nlohmann::json::array()},
    };

    for (auto it = firstPlugin; it != lastPlugin; ++it) {
      json["plugins"].push_back(generateDerivedMetadata(*it));
    }

    return json.dump();
  }

  G& getGame() {
    return game_;
  }

  const G& getGame() const {
    return game_;
  }

private:
  static std::vector<SimpleMessage> toSimpleMessages(
      const std::vector<Message>& messages,
      const std::string& language) {
    std::vector<SimpleMessage> simpleMessages;
    for (const auto message : messages) {
      auto simpleMessage = message.ToSimpleMessage(language);
      if (simpleMessage.has_value()) {
        simpleMessages.push_back(simpleMessage.value());
      }
    }

    return simpleMessages;
  }

  std::optional<PluginMetadata> evaluateMetadata(
      const std::string& pluginName) {
    auto evaluatedMasterlistMetadata = evaluateMasterlistMetadata(pluginName);
    auto evaluatedUserMetadata = evaluateUserlistMetadata(pluginName);

    if (!evaluatedMasterlistMetadata.has_value()) {
      return evaluatedUserMetadata;
    }

    if (!evaluatedUserMetadata.has_value()) {
      return evaluatedMasterlistMetadata;
    }

    evaluatedUserMetadata.value().MergeMetadata(
        evaluatedMasterlistMetadata.value());

    return evaluatedUserMetadata;
  }

  std::optional<PluginMetadata> evaluateMasterlistMetadata(
      const std::string& pluginName) {
    try {
      return game_.GetMasterlistMetadata(pluginName, true);
    } catch (std::exception& e) {
      if (logger_) {
        logger_->error(
            "\"{}\"'s masterlist metadata contains a condition that "
            "could not be evaluated. Details: {}",
            pluginName,
            e.what());
      }

      PluginMetadata master(pluginName);
      master.SetMessages({
          PlainTextMessage(MessageType::error,
                  (boost::format(boost::locale::translate(
                       "\"%1%\" contains a condition that could not be "
                       "evaluated. Details: %2%")) %
                   pluginName % e.what())
                      .str()),
      });

      return master;
    }
  }

  std::optional<PluginMetadata> evaluateUserlistMetadata(
      const std::string& pluginName) {
    try {
      return game_.GetUserMetadata(pluginName, true);
    } catch (std::exception& e) {
      if (logger_) {
        logger_->error(
            "\"{}\"'s user metadata contains a condition that could "
            "not be evaluated. Details: {}",
            pluginName,
            e.what());
      }

      PluginMetadata user(pluginName);
      user.SetMessages({
          PlainTextMessage(MessageType::error,
                  (boost::format(boost::locale::translate(
                       "\"%1%\" contains a condition that could not be "
                       "evaluated. Details: %2%")) %
                   pluginName % e.what())
                      .str()),
      });

      return user;
    }
  }

  FileRevision getMasterlistInfo() {
    using boost::locale::translate;

    FileRevision revision;
    try {
      revision = game_.GetMasterlistInfo();
      AddSuffixIfModified(revision);
    } catch (FileAccessError&) {
      if (logger_) {
        logger_->warn("No masterlist present at {}",
                      game_.MasterlistPath().u8string());
      }
      auto text =
          /* translators: N/A is an abbreviation for Not Applicable. A masterlist is a database that contains information for various mods. */
          translate("N/A: No masterlist present").str();
      revision.id = text;
      revision.date = text;
    } catch (GitStateError&) {
      if (logger_) {
        logger_->warn("Not a Git repository: {}",
                      game_.MasterlistPath().parent_path().u8string());
      }
      auto text =
        /* translators: Git is the software LOOT uses to track changes to the source code. */
        translate("Unknown: Git repository missing").str();
      revision.id = text;
      revision.date = text;
    }

    return revision;
  }

  G& game_;
  std::shared_ptr<spdlog::logger> logger_;
  const std::string language_;
};
}

#endif
