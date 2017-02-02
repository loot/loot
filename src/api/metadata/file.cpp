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

#include "loot/metadata/file.h"

#include <boost/algorithm/string.hpp>

#include "loot/yaml/file.h"

namespace loot {
File::File() {}

File::File(const std::string& name, const std::string& display, const std::string& condition)
  : name_(name), display_(display), ConditionalMetadata(condition) {}

bool File::operator < (const File& rhs) const {
  return boost::ilexicographical_compare(GetName(), rhs.GetName());
}

bool File::operator == (const File& rhs) const {
  return boost::iequals(GetName(), rhs.GetName());
}

std::string File::GetName() const {
  return name_;
}

std::string File::GetDisplayName() const {
  if (display_.empty())
    return name_;
  else
    return display_;
}
}
