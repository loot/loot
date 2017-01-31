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

#include "loot/metadata/message_content.h"

#include <boost/algorithm/string.hpp>

#include "loot/language.h"

namespace loot {
MessageContent::MessageContent() : language_(LanguageCode::english) {}

MessageContent::MessageContent(const std::string& text, const LanguageCode language) : text_(text), language_(language) {}

std::string MessageContent::GetText() const {
  return text_;
}

LanguageCode MessageContent::GetLanguage() const {
  return language_;
}

bool MessageContent::operator < (const MessageContent& rhs) const {
  return boost::ilexicographical_compare(text_, rhs.GetText());
}

bool MessageContent::operator == (const MessageContent& rhs) const {
  return (boost::iequals(text_, rhs.GetText()));
}
MessageContent MessageContent::Choose(const std::vector<MessageContent> content,
                                      const LanguageCode language) {
  if (content.empty())
    return MessageContent();
  else if (content.size() == 1)
    return content[0];
  else {
    MessageContent english;
    for (const auto &mc : content) {
      if (mc.GetLanguage() == language) {
        return mc;
      } else if (mc.GetLanguage() == LanguageCode::english)
        english = mc;
    }
    return english;
  }
}
}
