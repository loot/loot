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
#ifndef __LOOT_METADATA_TAG__
#define __LOOT_METADATA_TAG__

#include "conditional_metadata.h"

#include <string>

#include <yaml-cpp/yaml.h>

namespace loot {
    class Tag : public ConditionalMetadata {
    public:
        Tag();
        Tag(const std::string& tag, const bool isAddition = true, const std::string& condition = "");

        bool operator < (const Tag& rhs) const;
        bool operator == (const Tag& rhs) const;

        bool IsAddition() const;
        std::string Name() const;
    private:
        std::string _name;
        bool addTag;
    };
}

namespace YAML {
    template<>
    struct convert < loot::Tag > {
        static Node encode(const loot::Tag& rhs) {
            Node node;
            node["condition"] = rhs.Condition();
            if (rhs.IsAddition())
                node["name"] = rhs.Name();
            else
                node["name"] = "-" + rhs.Name();
            return node;
        }

        static bool decode(const Node& node, loot::Tag& rhs) {
            std::string condition, tag;
            if (node.IsMap()) {
                if (!node["name"])
                    return false;

                if (node["condition"])
                    condition = node["condition"].as<std::string>();
                if (node["name"])
                    tag = node["name"].as<std::string>();
            }
            else if (node.IsScalar())
                tag = node.as<std::string>();

            if (tag[0] == '-')
                rhs = loot::Tag(tag.substr(1), false, condition);
            else
                rhs = loot::Tag(tag, true, condition);

            return true;
        }
    };

    Emitter& operator << (Emitter& out, const loot::Tag& rhs);
}

#endif
