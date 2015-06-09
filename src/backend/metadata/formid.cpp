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

#include "formid.h"

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

using namespace std;

namespace loot {
    FormID::FormID() : id(0) {}

    FormID::FormID(const std::string& sourcePlugin, const uint32_t objectID) : plugin(sourcePlugin), id(objectID) {}

    FormID::FormID(const std::vector<std::string>& sourcePlugins, const uint32_t formID) {
        unsigned int index = formID >> 24;
        id = formID & ~((uint32_t)index << 24);

        if (index >= sourcePlugins.size()) {
            BOOST_LOG_TRIVIAL(trace) << hex << formID << dec << " in " << sourcePlugins.back() << " has a higher modIndex than expected.";
            index = sourcePlugins.size() - 1;
        }

        plugin = sourcePlugins[index];
    }

    bool FormID::operator == (const FormID& rhs) const {
        return (id == rhs.Id() && boost::iequals(plugin, rhs.Plugin()));
    }

    bool FormID::operator < (const FormID& rhs) const {
        if (id != rhs.Id())
            return id < rhs.Id();
        else
            return boost::ilexicographical_compare(plugin, rhs.Plugin());
    }

    std::string FormID::Plugin() const {
        return plugin;
    }

    uint32_t FormID::Id() const {
        return id;
    }
}
