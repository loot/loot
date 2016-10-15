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

#ifndef LOOT_LANGUAGE_CODE
#define LOOT_LANGUAGE_CODE

/**
 * The namespace used by the LOOT API.
 */
namespace loot {
/**
 * @brief Codes used to specify the preferred language for messages when
 *        evaluating masterlists and userlists.
 * @details If a message is not available in the preferred language, its English
 *          string will be used. Note that messages with only one language
 *          string are assumed to be written in English, but this cannot be
 *          guaranteed (any violations should be reported as bugs so that they
 *          can be fixed).
 */
enum struct LanguageCode : unsigned int {
  english,
  spanish,
  russian,
  french,
  chinese,
  polish,
  brazilian_portuguese,
  finnish,
  german,
  danish,
  korean,
  swedish
};
}

#endif
