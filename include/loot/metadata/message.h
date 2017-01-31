/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2016    WrinklyNinja

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
#ifndef LOOT_METADATA_MESSAGE
#define LOOT_METADATA_MESSAGE

#include <string>
#include <vector>

#include "loot/api_decorator.h"
#include "loot/metadata/conditional_metadata.h"
#include "loot/metadata/message_content.h"
#include "loot/enum/language_code.h"
#include "loot/enum/message_type.h"
#include "loot/struct/simple_message.h"

namespace loot {
class Message : public ConditionalMetadata {
public:
  LOOT_API Message();
  LOOT_API Message(const MessageType type, const std::string& content,
                   const std::string& condition = "");
  LOOT_API Message(const MessageType type, const std::vector<MessageContent>& content,
                   const std::string& condition = "");

  LOOT_API bool operator < (const Message& rhs) const;
  LOOT_API bool operator == (const Message& rhs) const;

  LOOT_API MessageType GetType() const;
  LOOT_API std::vector<MessageContent> GetContent() const;
  LOOT_API MessageContent GetContent(const LanguageCode language) const;

  LOOT_API SimpleMessage ToSimpleMessage(const LanguageCode language) const;
private:
  MessageType type_;
  std::vector<MessageContent> content_;
};
}

#endif
