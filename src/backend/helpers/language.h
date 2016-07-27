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

namespace loot {
    //Language class for simpler language support.
class Language {
public:
  enum struct Code : unsigned int {
    english = 1,
    spanish = 2,
    russian = 3,
    french = 4,
    chinese = 5,
    polish = 6,
    brazilian_portuguese = 7,
    finnish = 8,
    german = 9,
    danish = 10,
    korean = 11
  };

  static const std::vector<Code> codes;

  Language(const Code code);
  Language(const std::string& locale);

  Code GetCode() const;
  std::string GetName() const;
  std::string GetLocale() const;
private:
  void Construct(const Code code);

  Code code_;
  std::string name_;
  std::string locale_;
};
}

#endif
