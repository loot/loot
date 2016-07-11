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

#include "backend/helpers/language.h"

namespace loot {
const std::vector<Language::Code> Language::codes({
    Code::english,
    Code::spanish,
    Code::russian,
    Code::french,
    Code::chinese,
    Code::polish,
    Code::brazilian_portuguese,
    Code::finnish,
    Code::german,
    Code::danish,
    Code::korean
});

Language::Language(Code code) {
  Construct(code);
}

Language::Language(const std::string& locale) {
  for (Code code : codes) {
    if (locale == Language(code).GetLocale()) {
      Construct(code);
      return;
    }
  }

  Construct(Code::english);
}

void Language::Construct(const Code code) {
  code_ = code;
  if (code_ == Code::spanish) {
    name_ = "Español";
    locale_ = "es";
  } else if (code_ == Code::russian) {
    name_ = "Русский";
    locale_ = "ru";
  } else if (code_ == Code::french) {
    name_ = "Français";
    locale_ = "fr";
  } else if (code_ == Code::chinese) {
    name_ = "简体中文";
    locale_ = "zh_CN";
  } else if (code_ == Code::polish) {
    name_ = "Polski";
    locale_ = "pl";
  } else if (code_ == Code::brazilian_portuguese) {
    name_ = "Português do Brasil";
    locale_ = "pt_BR";
  } else if (code_ == Code::finnish) {
    name_ = "suomi";
    locale_ = "fi";
  } else if (code_ == Code::german) {
    name_ = "Deutsch";
    locale_ = "de";
  } else if (code_ == Code::danish) {
    name_ = "Dansk";
    locale_ = "da";
  } else if (code_ == Code::korean) {
    name_ = "한국어";
    locale_ = "ko";
  } else {
    code_ = Code::english;
    name_ = "English";
    locale_ = "en";
  }
}

Language::Code Language::GetCode() const {
  return code_;
}

std::string Language::GetName() const {
  return name_;
}

std::string Language::GetLocale() const {
  return locale_;
}
}
