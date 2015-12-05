/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2015    WrinklyNinja

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

#include "tag.h"

#include <boost/algorithm/string.hpp>

using namespace std;

namespace loot {
    Tag::Tag() : addTag(true) {}

    Tag::Tag(const string& tag, const bool isAddition, const string& condition) : _name(tag), addTag(isAddition), ConditionalMetadata(condition) {}

    bool Tag::operator < (const Tag& rhs) const {
        if (addTag != rhs.IsAddition())
            return (addTag && !rhs.IsAddition());
        else
            return boost::ilexicographical_compare(Name(), rhs.Name());
    }

    bool Tag::operator == (const Tag& rhs) const {
        return (addTag == rhs.IsAddition() && boost::iequals(Name(), rhs.Name()));
    }

    bool Tag::IsAddition() const {
        return addTag;
    }

    std::string Tag::Name() const {
        return _name;
    }
}

namespace YAML {
    Emitter& operator << (Emitter& out, const loot::Tag& rhs) {
        if (!rhs.IsConditional()) {
            if (rhs.IsAddition())
                out << rhs.Name();
            else
                out << ('-' + rhs.Name());
        }
        else {
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
