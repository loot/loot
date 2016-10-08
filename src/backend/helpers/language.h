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

#ifndef LOOT_BACKEND_HELPERS_LANGUAGE
#define LOOT_BACKEND_HELPERS_LANGUAGE

#include <string>
#include <vector>

#include "loot/enum/language_code.h"

namespace loot {
    //Language class for simpler language support.
class Language {
public:

  static const std::vector<LanguageCode> codes;

  Language(const LanguageCode code);
  Language(const std::string& locale);

  LanguageCode GetCode() const;
  std::string GetName() const;
  std::string GetLocale() const;
private:
  void Construct(const LanguageCode code);

  LanguageCode code_;
  std::string name_;
  std::string locale_;
};
}

#endif
