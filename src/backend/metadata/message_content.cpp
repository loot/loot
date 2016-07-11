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
    <http://www.gnu.org/licenses/>.
    */

#include "backend/metadata/message_content.h"

#include <boost/algorithm/string.hpp>

#include "backend/helpers/language.h"

namespace loot {
MessageContent::MessageContent() : language_(Language::Code::english) {}

MessageContent::MessageContent(const std::string& text, const Language::Code language) : text_(text), language_(language) {}

std::string MessageContent::GetText() const {
  return text_;
}

Language::Code MessageContent::GetLanguage() const {
  return language_;
}

bool MessageContent::operator < (const MessageContent& rhs) const {
  return boost::ilexicographical_compare(text_, rhs.GetText());
}

bool MessageContent::operator == (const MessageContent& rhs) const {
  return (boost::iequals(text_, rhs.GetText()));
}
}

namespace YAML {
Emitter& operator << (Emitter& out, const loot::MessageContent& rhs) {
  out << BeginMap;

  out << Key << "lang" << Value << loot::Language(rhs.GetLanguage()).GetLocale();

  out << Key << "str" << Value << YAML::SingleQuoted << rhs.GetText();

  out << EndMap;

  return out;
}
}
