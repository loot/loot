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

#include "loot/metadata/message.h"

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

#include "backend/game/game.h"
#include "loot/language.h"

namespace loot {
Message::Message() : type_(MessageType::say) {}

Message::Message(const MessageType type, const std::string& content,
                 const std::string& condition) : type_(type), ConditionalMetadata(condition) {
  content_.push_back(MessageContent(content, LanguageCode::english));
}

Message::Message(const MessageType type, const std::vector<MessageContent>& content,
                 const std::string& condition) : type_(type), content_(content), ConditionalMetadata(condition) {
  if (content.size() > 1) {
    bool englishStringExists = false;
    for (const auto &mc : content) {
      if (mc.GetLanguage() == LanguageCode::english)
        englishStringExists = true;
    }
    if (!englishStringExists)
      throw std::invalid_argument("bad conversion: multilingual messages must contain an English content string");
  }
}

bool Message::operator < (const Message& rhs) const {
  if (!content_.empty() && !rhs.GetContent().empty())
    return boost::ilexicographical_compare(GetContent(LanguageCode::english).GetText(), rhs.GetContent(LanguageCode::english).GetText());
  else if (content_.empty() && !rhs.GetContent().empty())
    return true;
  else
    return false;
}

bool Message::operator == (const Message& rhs) const {
  return (content_ == rhs.GetContent());
}

MessageType Message::GetType() const {
  return type_;
}

std::vector<MessageContent> Message::GetContent() const {
  return content_;
}
MessageContent Message::GetContent(const LanguageCode language) const {
  return MessageContent::Choose(content_, language);
}
SimpleMessage Message::ToSimpleMessage(const LanguageCode language) const {
  MessageContent content = GetContent(language);
  SimpleMessage simpleMessage;

  simpleMessage.type = GetType();
  simpleMessage.language = content.GetLanguage();
  simpleMessage.text = content.GetText();

  return simpleMessage;
}
}
