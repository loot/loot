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

#ifndef LOOT_LANGUAGE
#define LOOT_LANGUAGE

#include <string>
#include <vector>

#include "loot/api_decorator.h"
#include "loot/enum/language_code.h"

namespace loot {
/**
 * @brief Represents a language, and used to map between API language codes,
 *        locale codes and language names.
 */
class Language {
public:
  /**
   * @brief A convenience constant that contains all available language codes.
   */
  LOOT_API static const std::vector<LanguageCode> codes;

  /**
   * @brief Construct a Language object.
   * @param  code
   *         A LOOT API language code.
   * @return A Language object for the given code.
   */
  LOOT_API Language(const LanguageCode code);

  /**
   * @brief Construct a Language object.
   * @param  locale
   *         A POSIX locale code.
   * @return A Language object. If the locale code corresponds to a language
   *         with a LanguageCode value, the object is for that language,
   *         otherwise it is for English.
   */
  LOOT_API Language(const std::string& locale);

  /**
   * Get the language's LanguageCode.
   * @return The language's LanguageCode.
   */
  LOOT_API LanguageCode GetCode() const;

  /**
   * Get the language's name for itself.
   *
   * For example, ``Русский``, not ``Russian``.
   * @return The language's name.
   */
  LOOT_API std::string GetName() const;

  /**
   * Get the language's POSIX locale code.
   * @return The language's POSIX locale code.
   */
  LOOT_API std::string GetLocale() const;
private:
  static LanguageCode GetCode(const std::string& locale);

  LanguageCode code_;
  std::string name_;
  std::string locale_;
};
}

#endif
