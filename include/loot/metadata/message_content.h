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
#ifndef LOOT_METADATA_MESSAGE_CONTENT
#define LOOT_METADATA_MESSAGE_CONTENT

#include <string>

#include "loot/language.h"

namespace loot {
class MessageContent {
public:
  MessageContent();
  MessageContent(const std::string& text, const LanguageCode language);

  std::string GetText() const;
  LanguageCode GetLanguage() const;

  bool operator < (const MessageContent& rhs) const;
  bool operator == (const MessageContent& rhs) const;

  static MessageContent Choose(const std::vector<MessageContent> content,
                               const LanguageCode language);
private:
  std::string text_;
  LanguageCode language_;
};
}

#endif
