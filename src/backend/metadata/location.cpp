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

#include "location.h"

#include <boost/algorithm/string.hpp>

namespace loot {
Location::Location() {}

Location::Location(const std::string& url, const std::string& name) : url_(url), name_(name) {}

bool Location::operator < (const Location& rhs) const {
  return boost::ilexicographical_compare(url_, rhs.URL());
}

bool Location::operator == (const Location& rhs) const {
  return boost::iequals(url_, rhs.URL());
}

std::string Location::URL() const {
  return url_;
}

std::string Location::Name() const {
  return name_;
}
}

namespace YAML {
Emitter& operator << (Emitter& out, const loot::Location& rhs) {
  if (rhs.Name().empty())
    out << YAML::SingleQuoted << rhs.URL();
  else {
    out << BeginMap
      << Key << "link" << Value << YAML::SingleQuoted << rhs.URL()
      << Key << "name" << Value << YAML::SingleQuoted << rhs.Name()
      << EndMap;
  }
  return out;
}
}
