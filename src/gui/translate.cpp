/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2026 Oliver Hamlet

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

#include "gui/translate.h"

#include <boost/locale/message.hpp>

namespace loot {
std::string translate(const char* text) {
  return boost::locale::translate(text).str();
}

std::string translate(const char* singularText,
                      const char* pluralText,
                      unsigned int count) {
  return boost::locale::translate(singularText,
                                  pluralText,
                                  static_cast<boost::locale::count_type>(count))
      .str();
}

std::string translate(const char* singularText,
                      const char* pluralText,
                      size_t count) {
  return boost::locale::translate(singularText,
                                  pluralText,
                                  static_cast<boost::locale::count_type>(count))
      .str();
}
}
