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
#ifndef __LOOT_METADATA_PLUGIN_DIRTY_INFO__
#define __LOOT_METADATA_PLUGIN_DIRTY_INFO__

#include "message.h"

#include <cstdint>
#include <string>

#include <yaml-cpp/yaml.h>

namespace loot {
    class Game;

    class PluginDirtyInfo {
    public:
        PluginDirtyInfo();
        PluginDirtyInfo(uint32_t crc, unsigned int itm, unsigned int ref, unsigned int nav, const std::string& utility);

        bool operator < (const PluginDirtyInfo& rhs) const;
        bool operator == (const PluginDirtyInfo& rhs) const;

        uint32_t CRC() const;
        unsigned int ITMs() const;
        unsigned int DeletedRefs() const;
        unsigned int DeletedNavmeshes() const;
        std::string CleaningUtility() const;

        Message AsMessage() const;

        bool EvalCondition(Game& game, const std::string& pluginName) const;
    private:
        uint32_t _crc;
        unsigned int _itm;
        unsigned int _ref;
        unsigned int _nav;
        std::string _utility;
    };
}

namespace YAML {
    template<>
    struct convert < loot::PluginDirtyInfo > {
        static Node encode(const loot::PluginDirtyInfo& rhs) {
            Node node;
            node["crc"] = rhs.CRC();
            node["util"] = rhs.CleaningUtility();

            if (rhs.ITMs() > 0)
                node["itm"] = rhs.ITMs();
            if (rhs.DeletedRefs() > 0)
                node["udr"] = rhs.DeletedRefs();
            if (rhs.DeletedNavmeshes() > 0)
                node["nav"] = rhs.DeletedNavmeshes();

            return node;
        }

        static bool decode(const Node& node, loot::PluginDirtyInfo& rhs) {
            if (!node.IsMap())
                throw RepresentationException(node.Mark(), "bad conversion: 'dirty info' object must be a map");
            if (!node["crc"])
                throw RepresentationException(node.Mark(), "bad conversion: 'crc' key missing from 'dirty info' object");
            if (!node["util"])
                throw RepresentationException(node.Mark(), "bad conversion: 'util' key missing from 'dirty info' object");

            uint32_t crc = node["crc"].as<uint32_t>();
            int itm = 0, ref = 0, nav = 0;

            if (node["itm"])
                itm = node["itm"].as<unsigned int>();
            if (node["udr"])
                ref = node["udr"].as<unsigned int>();
            if (node["nav"])
                nav = node["nav"].as<unsigned int>();

            std::string utility = node["util"].as<std::string>();

            rhs = loot::PluginDirtyInfo(crc, itm, ref, nav, utility);

            return true;
        }
    };

    Emitter& operator << (Emitter& out, const loot::PluginDirtyInfo& rhs);
}

#endif
