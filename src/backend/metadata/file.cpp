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

#include "file.h"

#include <boost/algorithm/string.hpp>

using namespace std;

namespace loot {
    File::File() {}

    File::File(const std::string& name, const std::string& display, const std::string& condition)
        : _name(name), _display(display), ConditionalMetadata(condition) {}

    bool File::operator < (const File& rhs) const {
        return boost::ilexicographical_compare(Name(), rhs.Name());
    }

    bool File::operator == (const File& rhs) const {
        return boost::iequals(Name(), rhs.Name());
    }

    std::string File::Name() const {
        return _name;
    }

    std::string File::DisplayName() const {
        if (_display.empty())
            return _name;
        else
            return _display;
    }
}

namespace YAML {
    Emitter& operator << (Emitter& out, const loot::File& rhs) {
        if (!rhs.IsConditional() && (rhs.DisplayName().empty() || rhs.DisplayName() == rhs.Name()))
            out << YAML::SingleQuoted << rhs.Name();
        else {
            out << BeginMap
                << Key << "name" << Value << YAML::SingleQuoted << rhs.Name();

            if (rhs.IsConditional())
                out << Key << "condition" << Value << YAML::SingleQuoted << rhs.Condition();

            if (rhs.DisplayName() != rhs.Name())
                out << Key << "display" << Value << YAML::SingleQuoted << rhs.DisplayName();

            out << EndMap;
        }

        return out;
    }
}