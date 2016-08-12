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
#ifndef LOOT_BACKEND_METADATA_MESSAGE_CONTENT
#define LOOT_BACKEND_METADATA_MESSAGE_CONTENT

#include <string>

#include <yaml-cpp/yaml.h>

#include "backend/helpers/language.h"

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

namespace YAML {
template<>
struct convert<loot::MessageContent> {
  static Node encode(const loot::MessageContent& rhs) {
    Node node;
    node["text"] = rhs.GetText();
    node["lang"] = loot::Language(rhs.GetLanguage()).GetLocale();

    return node;
  }

  static bool decode(const Node& node, loot::MessageContent& rhs) {
    if (!node.IsMap())
      throw RepresentationException(node.Mark(), "bad conversion: 'message content' object must be a map");
    if (!node["text"])
      throw RepresentationException(node.Mark(), "bad conversion: 'text' key missing from 'message content' object");
    if (!node["lang"])
      throw RepresentationException(node.Mark(), "bad conversion: 'lang' key missing from 'message content' object");

    std::string text = node["text"].as<std::string>();
    loot::LanguageCode lang = loot::Language(node["lang"].as<std::string>()).GetCode();

    rhs = loot::MessageContent(text, lang);

    return true;
  }
};

Emitter& operator << (Emitter& out, const loot::MessageContent& rhs);
}

#endif
