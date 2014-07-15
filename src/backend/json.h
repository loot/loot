/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014    WrinklyNinja

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

#ifndef __LOOT_JSON__
#define __LOOT_JSON__


#include <yaml-cpp/yaml.h>

namespace loot {

    // Handy class for turning YAML objects into JSON and vice-versa.
    class JSON {
    public:

        inline static YAML::Node parse(std::string& json) {
            return YAML::Load(json);
        }

        inline static std::string stringify(YAML::Node& yaml) {

            YAML::Emitter out;
            out.SetOutputCharset(YAML::EscapeNonAscii);
            out.SetStringFormat(YAML::DoubleQuoted);
            out.SetBoolFormat(YAML::TrueFalseBool);
            out.SetSeqFormat(YAML::Flow);
            out.SetMapFormat(YAML::Flow);

            out << yaml;

            // yaml-cpp produces `!<!>` artifacts in its output, so remove them.
            std::string json = out.c_str();
            boost::replace_all(json, "!<!>", "");
            // There's also a bit of weirdness where there are some \x escapes and some \u escapes.
            // They should all be \u, so transform them.
            boost::replace_all(json, "\\x", "\\u00");

            return json;
        }
    };
}


#endif