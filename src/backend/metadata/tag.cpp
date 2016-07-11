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

#include "backend/metadata/tag.h"

#include <boost/algorithm/string.hpp>

namespace loot {
Tag::Tag() : addTag_(true) {}

Tag::Tag(const std::string& tag, const bool isAddition, const std::string& condition) : name_(tag), addTag_(isAddition), ConditionalMetadata(condition) {}

bool Tag::operator < (const Tag& rhs) const {
  if (addTag_ != rhs.IsAddition())
    return (addTag_ && !rhs.IsAddition());
  else
    return boost::ilexicographical_compare(Name(), rhs.Name());
}

bool Tag::operator == (const Tag& rhs) const {
  return (addTag_ == rhs.IsAddition() && boost::iequals(Name(), rhs.Name()));
}

bool Tag::IsAddition() const {
  return addTag_;
}

std::string Tag::Name() const {
  return name_;
}
}

namespace YAML {
Emitter& operator << (Emitter& out, const loot::Tag& rhs) {
  if (!rhs.IsConditional()) {
    if (rhs.IsAddition())
      out << rhs.Name();
    else
      out << ('-' + rhs.Name());
  } else {
    out << BeginMap;
    if (rhs.IsAddition())
      out << Key << "name" << Value << rhs.Name();
    else
      out << Key << "name" << Value << ('-' + rhs.Name());

    out << Key << "condition" << Value << YAML::SingleQuoted << rhs.Condition()
      << EndMap;
  }

  return out;
}
}
