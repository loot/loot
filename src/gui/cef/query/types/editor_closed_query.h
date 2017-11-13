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

#ifndef LOOT_GUI_QUERY_EDITOR_CLOSED_QUERY
#define LOOT_GUI_QUERY_EDITOR_CLOSED_QUERY

#undef ERROR

#include "gui/cef/query/types/metadata_query.h"
#include "gui/state/loot_state.h"
#include "schema/request.pb.h"

namespace loot {
class EditorClosedQuery : public MetadataQuery {
public:
  EditorClosedQuery(LootState& state, protobuf::EditorState editorState) :
      MetadataQuery(state),
      state_(state),
      applyEdits_(editorState.apply_edits()) {
    try {
      metadata_ = convert(editorState.metadata());
    } catch (...) {
      state_.decrementUnappliedChangeCounter();
      throw;
    }
  }

  std::string executeLogic() {
    if (applyEdits_) {
      applyUserEdits();
    }
    state_.decrementUnappliedChangeCounter();

    return generateJsonResponse(metadata_.GetName());
  }

private:
  template<typename T>
  using PBFields = ::google::protobuf::RepeatedPtrField<T>;

  PluginMetadata getNonUserMetadata() {
    BOOST_LOG_TRIVIAL(trace)
        << "Getting non-user metadata for: " << metadata_.GetName();
    auto masterlistMetadata =
        state_.getCurrentGame().GetMasterlistMetadata(metadata_.GetName());

    try {
      auto plugin = state_.getCurrentGame().GetPlugin(metadata_.GetName());
      return MetadataQuery::getNonUserMetadata(plugin, masterlistMetadata);
    } catch (...) {
    }

    return masterlistMetadata;
  }

  PluginMetadata getUserMetadata() {
    auto nonUserMetadata = getNonUserMetadata();
    auto userMetadata = metadata_.NewMetadata(nonUserMetadata);

    if (metadata_.GetLocalPriority().GetValue() !=
        nonUserMetadata.GetLocalPriority().GetValue()) {
      userMetadata.SetLocalPriority(metadata_.GetLocalPriority());
    } else {
      userMetadata.SetLocalPriority(Priority());
    }

    if (metadata_.GetGlobalPriority().GetValue() !=
        nonUserMetadata.GetGlobalPriority().GetValue()) {
      userMetadata.SetGlobalPriority(metadata_.GetGlobalPriority());
    } else {
      userMetadata.SetGlobalPriority(Priority());
    }

    return userMetadata;
  }

  void applyUserEdits() {
    BOOST_LOG_TRIVIAL(trace)
        << "Applying user edits for: " << metadata_.GetName();

    // Determine what metadata in the response is user-added.
    auto userMetadata = getUserMetadata();

    // Now erase any existing userlist entry.
    BOOST_LOG_TRIVIAL(trace) << "Erasing the existing userlist entry.";
    state_.getCurrentGame().ClearUserMetadata(userMetadata.GetName());

    // Add a new userlist entry if necessary.
    if (!userMetadata.HasNameOnly()) {
      BOOST_LOG_TRIVIAL(trace) << "Adding new metadata to new userlist entry.";
      state_.getCurrentGame().AddUserMetadata(userMetadata);
    }

    // Save edited userlist.
    state_.getCurrentGame().SaveUserMetadata();
  }

  static PluginMetadata convert(protobuf::PluginMetadata pbMetadata) {
    if (pbMetadata.name().empty()) {
      throw std::runtime_error(
          "PluginMetadata object has an empty 'name' value");
    }

    PluginMetadata metadata(pbMetadata.name());

    if (metadata.IsRegexPlugin()) {
      try {
        std::regex(metadata.GetName(),
                   std::regex::ECMAScript | std::regex::icase);
      } catch (std::regex_error& e) {
        throw std::runtime_error(
            std::string(
                "PluginMetadata object has an invalid regex 'name' value: ") +
            e.what());
      }

      if (pbMetadata.dirty().empty()) {
        throw std::runtime_error(
            "PluginMetadata object cannot have a 'dirty' key with a regex "
            "'name' value");
      }
      if (pbMetadata.clean().empty()) {
        throw std::runtime_error(
            "PluginMetadata object cannot have a 'clean' key with a regex "
            "'name' value");
      }
    }

    metadata.SetEnabled(pbMetadata.enabled());

    // These two will register all priorities as explicit, but that's OK because
    // explicitness is ignored above.
    metadata.SetLocalPriority(Priority(pbMetadata.priority()));
    metadata.SetGlobalPriority(Priority(pbMetadata.global_priority()));

    metadata.SetLoadAfterFiles(convert(pbMetadata.after()));
    metadata.SetRequirements(convert(pbMetadata.req()));
    metadata.SetIncompatibilities(convert(pbMetadata.inc()));
    metadata.SetMessages(convert(pbMetadata.msg()));
    metadata.SetTags(convert(pbMetadata.tag()));
    metadata.SetDirtyInfo(convert(pbMetadata.dirty()));
    metadata.SetCleanInfo(convert(pbMetadata.clean()));
    metadata.SetLocations(convert(pbMetadata.url()));

    return metadata;
  }

  static void testConditionSyntax(const std::string& objectType,
                                  const std::string& condition) {
    try {
      ConditionalMetadata(condition).ParseCondition();
    } catch (std::exception& e) {
      throw std::runtime_error(objectType +
                               " object has an invalid condition: " + e.what());
    }
  }

  static std::set<File> convert(const PBFields<protobuf::File>& pbFiles) {
    std::set<File> files;

    for (const auto& pbFile : pbFiles) {
      if (pbFile.name().empty()) {
        throw std::runtime_error("File object has an empty 'name' value");
      }
      testConditionSyntax("File", pbFile.condition());

      files.insert(File(pbFile.name(), pbFile.display(), pbFile.condition()));
    }

    return files;
  }

  static MessageType mapMessageType(const std::string& type) {
    if (type == "say") {
      return MessageType::say;
    } else if (type == "warn") {
      return MessageType::warn;
    } else {
      return MessageType::error;
    }
  }

  static std::vector<Message> convert(
      const PBFields<protobuf::SimpleMessage>& pbMessages) {
    std::vector<Message> messages;

    for (const auto& pbMessage : pbMessages) {
      if (pbMessage.text().empty()) {
        throw std::runtime_error(
            "SimpleMessage object has an empty 'text' value");
      }
      if (pbMessage.language().empty()) {
        throw std::runtime_error(
            "SimpleMessage object has an empty 'language' value");
      }
      if (pbMessage.type().empty()) {
        throw std::runtime_error(
            "SimpleMessage object has an empty 'type' value");
      }
      testConditionSyntax("SimpleMessage", pbMessage.condition());

      MessageContent content(pbMessage.text(), pbMessage.language());

      messages.push_back(Message(mapMessageType(pbMessage.type()),
                                 std::vector<MessageContent>({content}),
                                 pbMessage.condition()));
    }

    return messages;
  }

  static std::set<Tag> convert(const PBFields<protobuf::Tag>& pbTags) {
    std::set<Tag> tags;

    for (const auto& pbTag : pbTags) {
      if (pbTag.name().empty()) {
        throw std::runtime_error("Tag object has an empty 'name' value");
      }
      testConditionSyntax("Tag", pbTag.condition());

      tags.insert(Tag(pbTag.name(), pbTag.is_addition(), pbTag.condition()));
    }

    return tags;
  }

  static std::vector<MessageContent> convert(
      const PBFields<protobuf::MessageContent>& pbContents) {
    std::vector<MessageContent> contents;

    for (const auto& pbContent : pbContents) {
      if (pbContent.text().empty()) {
        throw std::runtime_error(
            "MessageContent object has an empty 'text' value");
      }
      if (pbContent.language().empty()) {
        throw std::runtime_error(
            "MessageContent object has an empty 'language' value");
      }

      contents.push_back(
          MessageContent(pbContent.text(), pbContent.language()));
    }

    if (contents.size() > 1) {
      bool found = false;
      for (const auto& mc : contents) {
        if (mc.GetLanguage() == loot::MessageContent::defaultLanguage)
          found = true;
      }
      if (!found)
        throw std::runtime_error(
            "MessageContent array does not contain an English MessageContent "
            "object");
    }

    return contents;
  }

  static std::set<PluginCleaningData> convert(
      const PBFields<protobuf::CleaningData>& pbData) {
    std::set<PluginCleaningData> data;

    for (const auto& pb : pbData) {
      if (pb.crc() == 0) {
        throw std::runtime_error(
            "CleaningData object has a 'crc' value of zero");
      }
      if (pb.util().empty()) {
        throw std::runtime_error(
            "CleaningData object has an empty 'util' value");
      }

      data.insert(PluginCleaningData(pb.crc(),
                                     pb.util(),
                                     convert(pb.info()),
                                     pb.itm(),
                                     pb.udr(),
                                     pb.nav()));
    }

    return data;
  }

  static std::set<Location> convert(
      const PBFields<protobuf::Location>& pbLocations) {
    std::set<Location> locations;

    for (const auto& pbLocation : pbLocations) {
      if (pbLocation.link().empty()) {
        throw std::runtime_error("Location object has an empty 'link' value");
      }
      locations.insert(Location(pbLocation.link(), pbLocation.name()));
    }

    return locations;
  }

  LootState& state_;
  const bool applyEdits_;
  PluginMetadata metadata_;
};
}

#endif
