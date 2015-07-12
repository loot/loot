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
#ifndef __LOOT_METADATA_LOCATION__
#define __LOOT_METADATA_LOCATION__

#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

namespace loot {
    class Location {
    public:
        Location();
        Location(const std::string& url);
        Location(const std::string& url, const std::vector<std::string>& versions);

        bool operator < (const Location& rhs) const;
        bool operator == (const Location& rhs) const;

        std::string URL() const;
        std::vector<std::string> Versions() const;
    private:
        std::string _url;
        std::vector<std::string> _versions;
    };
}

namespace YAML {
    template<>
    struct convert < loot::Location > {
        static Node encode(const loot::Location& rhs) {
            Node node;

            node["link"] = rhs.URL();
            node["ver"] = rhs.Versions();

            return node;
        }

        static bool decode(const Node& node, loot::Location& rhs) {
            if (!node.IsMap() && !node.IsScalar())
                return false;

            std::string url;
            std::vector<std::string> versions;

            if (node.IsMap()) {
                if (!node["link"] || !node["ver"])
                    return false;

                url = node["link"].as<std::string>();
                versions = node["ver"].as<std::vector<std::string>>();
            }
            else
                url = node.as<std::string>();

            rhs = loot::Location(url, versions);

            return true;
        }
    };

    Emitter& operator << (Emitter& out, const loot::Location& rhs);
}

#endif
