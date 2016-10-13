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

#include "backend/helpers/language.h"

namespace loot {
const std::vector<LanguageCode> Language::codes({
  LanguageCode::english,
  LanguageCode::spanish,
  LanguageCode::russian,
  LanguageCode::french,
  LanguageCode::chinese,
  LanguageCode::polish,
  LanguageCode::brazilian_portuguese,
  LanguageCode::finnish,
  LanguageCode::german,
  LanguageCode::danish,
  LanguageCode::korean
});

Language::Language(LanguageCode code) : code_(code) {
  if (code_ == LanguageCode::spanish) {
    name_ = "Español";
    locale_ = "es";
  } else if (code_ == LanguageCode::russian) {
    name_ = "Русский";
    locale_ = "ru";
  } else if (code_ == LanguageCode::french) {
    name_ = "Français";
    locale_ = "fr";
  } else if (code_ == LanguageCode::chinese) {
    name_ = "简体中文";
    locale_ = "zh_CN";
  } else if (code_ == LanguageCode::polish) {
    name_ = "Polski";
    locale_ = "pl";
  } else if (code_ == LanguageCode::brazilian_portuguese) {
    name_ = "Português do Brasil";
    locale_ = "pt_BR";
  } else if (code_ == LanguageCode::finnish) {
    name_ = "suomi";
    locale_ = "fi";
  } else if (code_ == LanguageCode::german) {
    name_ = "Deutsch";
    locale_ = "de";
  } else if (code_ == LanguageCode::danish) {
    name_ = "Dansk";
    locale_ = "da";
  } else if (code_ == LanguageCode::korean) {
    name_ = "한국어";
    locale_ = "ko";
  } else {
    code_ = LanguageCode::english;
    name_ = "English";
    locale_ = "en";
  }
}

Language::Language(const std::string& locale) : Language(GetCode(locale)) {}

LanguageCode Language::GetCode(const std::string& locale) {
  for (LanguageCode code : codes) {
    if (locale == Language(code).GetLocale()) {
      return code;
    }
  }

  return LanguageCode::english;
}

LanguageCode Language::GetCode() const {
  return code_;
}

std::string Language::GetName() const {
  return name_;
}

std::string Language::GetLocale() const {
  return locale_;
}
}
