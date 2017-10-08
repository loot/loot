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

#ifndef LOOT_GUI_QUERY_PROTOBUF
#define LOOT_GUI_QUERY_PROTOBUF

#undef ERROR

#include "schema/query.pb.h"

namespace loot {
std::string mapMessageType(MessageType type) {
  if (type == MessageType::say) {
    return "say";
  } else if (type == MessageType::warn) {
    return "warn";
  } else {
    return "error";
  }
}

void convert(const SimpleMessage& fromMessage, protobuf::SimpleMessage& toMessage) {
  toMessage.set_type(mapMessageType(fromMessage.type));
  toMessage.set_text(fromMessage.text);
  toMessage.set_language(fromMessage.language);
  toMessage.set_condition(fromMessage.condition);
}

void convert(const Tag& fromTag, protobuf::Tag& toTag) {
  toTag.set_name(fromTag.GetName());
  toTag.set_condition(fromTag.GetCondition());
  toTag.set_is_addition(fromTag.IsAddition());
}

void convert(const PluginCleaningData& fromData, protobuf::CleaningData& toData) {
  toData.set_crc(fromData.GetCRC());
  toData.set_util(fromData.GetCleaningUtility());
  toData.set_itm(fromData.GetITMCount());
  toData.set_udr(fromData.GetDeletedReferenceCount());
  toData.set_nav(fromData.GetDeletedNavmeshCount());

  for (const auto& messageContent : fromData.GetInfo()) {
    auto info = toData.add_info();

    info->set_text(messageContent.GetText());
    info->set_language(messageContent.GetLanguage());
  }
}

void convert(const File& fromFile, protobuf::File& toFile) {
    toFile.set_name(fromFile.GetName());
    toFile.set_display(fromFile.GetDisplayName());
    toFile.set_condition(fromFile.GetCondition());
}

protobuf::PluginMetadata convert(const PluginMetadata& metadata, const std::string& language) {
  protobuf::PluginMetadata pbMetadata;
  pbMetadata.set_name(metadata.GetName());
  pbMetadata.set_enabled(metadata.IsEnabled());
  pbMetadata.set_priority(metadata.GetLocalPriority().GetValue());
  pbMetadata.set_global_priority(metadata.GetGlobalPriority().GetValue());

  for (const auto& file : metadata.GetLoadAfterFiles()) {
    auto pbFile = pbMetadata.add_after();
    convert(file, *pbFile);
  }

  for (const auto& file : metadata.GetRequirements()) {
    auto pbFile = pbMetadata.add_req();
    convert(file, *pbFile);
  }

  for (const auto& file : metadata.GetIncompatibilities()) {
    auto pbFile = pbMetadata.add_inc();
    convert(file, *pbFile);
  }

  for (const auto& message : metadata.GetMessages()) {
    auto simpleMessage = message.ToSimpleMessage(language);
    auto pbMessage = pbMetadata.add_msg();
    convert(simpleMessage, *pbMessage);
  }

  for (const auto& tag : metadata.GetTags()) {
    auto pbTag = pbMetadata.add_tag();
    convert(tag, *pbTag);
  }

  for (const auto& cleaningData : metadata.GetDirtyInfo()) {
    auto dirtyInfo = pbMetadata.add_dirty();
    convert(cleaningData, *dirtyInfo);
  }

  for (const auto& cleaningData : metadata.GetCleanInfo()) {
    auto cleanInfo = pbMetadata.add_clean();
    convert(cleaningData, *cleanInfo);
  }

  for (const auto& location : metadata.GetLocations()) {
    auto pbLocation = pbMetadata.add_url();

    pbLocation->set_link(location.GetURL());
    pbLocation->set_name(location.GetName());
  }

  return pbMetadata;
}
}

#endif
