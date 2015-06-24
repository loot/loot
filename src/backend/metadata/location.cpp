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

#include "location.h"

#include <boost/algorithm/string.hpp>

using namespace std;

namespace loot {
    Location::Location() {}

    Location::Location(const std::string& url) : _url(url) {}

    Location::Location(const std::string& url, const std::string& name) : _url(url), _name(name) {}

    bool Location::operator < (const Location& rhs) const {
        return boost::ilexicographical_compare(_url, rhs.URL());
    }

    bool Location::operator == (const Location& rhs) const {
        return boost::iequals(_url, rhs.URL());
    }

    std::string Location::URL() const {
        return _url;
    }

    std::string Location::Name() const {
        return _name;
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
