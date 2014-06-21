/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013-2014    WrinklyNinja

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
#ifndef __LOOT_GENERATORS__
#define __LOOT_GENERATORS__

#include "metadata.h"

#include <yaml-cpp/yaml.h>

#include <string>
#include <list>
#include <boost/filesystem.hpp>

namespace loot {
    //LOOT Report generation stuff.
    void GenerateReportData(const Game& game,
        std::list<Message>& messages,
        const std::list<Plugin>& plugins,
        const std::string& masterlistVersion,
        const std::string& masterlistDate,
        const bool masterlistUpdateEnabled);

    //Default settings file generation.
    void GenerateDefaultSettingsFile(const std::string& file);
}

namespace YAML {
    template<class T, class Compare>
    Emitter& operator << (Emitter& out, const std::set<T, Compare>& rhs) {
        out << BeginSeq;
        for (const auto &element: rhs) {
            out << element;
        }
		out << EndSeq;

		return out;
    }

    Emitter& operator << (Emitter& out, const loot::PluginDirtyInfo& rhs);

    Emitter& operator << (Emitter& out, const loot::Game& rhs);

    Emitter& operator << (Emitter& out, const loot::MessageContent& rhs);

    Emitter& operator << (Emitter& out, const loot::Message& rhs);

    Emitter& operator << (Emitter& out, const loot::File& rhs);

    Emitter& operator << (Emitter& out, const loot::Tag& rhs);

    Emitter& operator << (Emitter& out, const loot::Plugin& rhs);
}

#endif
