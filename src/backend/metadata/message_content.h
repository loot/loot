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
#ifndef __LOOT_METADATA_MESSAGE_CONTENT__
#define __LOOT_METADATA_MESSAGE_CONTENT__

#include "../helpers/language.h"

#include <string>

#include <yaml-cpp/yaml.h>

namespace loot {
    class MessageContent {
    public:
        MessageContent();
        MessageContent(const std::string& str, const unsigned int language);

        std::string Str() const;
        unsigned int Language() const;

        bool operator < (const MessageContent& rhs) const;
        bool operator == (const MessageContent& rhs) const;
    private:
        std::string _str;
        unsigned int _language;
    };
}

namespace YAML {
    template<>
    struct convert < loot::MessageContent > {
        static Node encode(const loot::MessageContent& rhs) {
            Node node;
            node["str"] = rhs.Str();
            node["lang"] = loot::Language(rhs.Language()).Locale();

            return node;
        }

        static bool decode(const Node& node, loot::MessageContent& rhs) {
            if (!node.IsMap())
                throw RepresentationException(node.Mark(), "bad conversion: 'message content' object must be a map");
            if (!node["str"])
                throw RepresentationException(node.Mark(), "bad conversion: 'str' key missing from 'message content' object");
            if (!node["lang"])
                throw RepresentationException(node.Mark(), "bad conversion: 'lang' key missing from 'message content' object");

            std::string str = node["str"].as<std::string>();
            unsigned int lang = loot::Language(node["lang"].as<std::string>()).Code();

            rhs = loot::MessageContent(str, lang);

            return true;
        }
    };

    Emitter& operator << (Emitter& out, const loot::MessageContent& rhs);
}

#endif
