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
#ifndef __LOOT_METADATA_MESSAGE__
#define __LOOT_METADATA_MESSAGE__

#include "conditional_metadata.h"
#include "message_content.h"
#include "../helpers/language.h"

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include <yaml-cpp/yaml.h>

namespace loot {
    class Game;

    class Message : public ConditionalMetadata {
    public:
        Message();
        Message(const unsigned int type, const std::string& content,
                const std::string& condition = "");
        Message(const unsigned int type, const std::vector<MessageContent>& content,
                const std::string& condition = "");

        bool operator < (const Message& rhs) const;
        bool operator == (const Message& rhs) const;

        bool EvalCondition(Game& game, const unsigned int language);

        unsigned int Type() const;
        std::vector<MessageContent> Content() const;
        MessageContent ChooseContent(const unsigned int language) const;

        static const unsigned int say;
        static const unsigned int warn;
        static const unsigned int error;
    private:
        unsigned int _type;
        std::vector<MessageContent> _content;
    };
}

namespace YAML {
    template<>
    struct convert < loot::Message > {
        static Node encode(const loot::Message& rhs) {
            Node node;
            node["condition"] = rhs.Condition();
            node["content"] = rhs.Content();

            if (rhs.Type() == loot::Message::say)
                node["type"] = "say";
            else if (rhs.Type() == loot::Message::warn)
                node["type"] = "warn";
            else
                node["type"] = "error";

            return node;
        }

        static bool decode(const Node& node, loot::Message& rhs) {
            if (!node.IsMap() || !node["type"] || !node["content"])
                return false;

            unsigned int typeNo = loot::Message::say;
            if (node["type"]) {
                std::string type;
                type = node["type"].as<std::string>();

                if (boost::iequals(type, "say"))
                    typeNo = loot::Message::say;
                else if (boost::iequals(type, "warn"))
                    typeNo = loot::Message::warn;
                else
                    typeNo = loot::Message::error;
            }

            std::vector<loot::MessageContent> content;
            if (node["content"].IsSequence())
                content = node["content"].as< std::vector<loot::MessageContent> >();
            else {
                content.push_back(loot::MessageContent(node["content"].as<std::string>(), loot::Language::english));
            }

            //Check now that at least one item in content is English if there are multiple items.
            if (content.size() > 1) {
                bool found = false;
                for (const auto &mc : content) {
                    if (mc.Language() == loot::Language::english)
                        found = true;
                }
                if (!found)
                    return false;
            }

            // Make any substitutions at this point.
            if (node["subs"]) {
                std::vector<std::string> subs = node["subs"].as<std::vector<std::string>>();
                for (auto& mc : content) {
                    boost::format f(mc.Str());

                    for (const auto& sub : subs) {
                        f = f % sub;
                    }

                    mc = loot::MessageContent(f.str(), mc.Language());
                }
            }

            std::string condition;
            if (node["condition"])
                condition = node["condition"].as<std::string>();

            rhs = loot::Message(typeNo, content, condition);
            return true;
        }
    };

    Emitter& operator << (Emitter& out, const loot::Message& rhs);
}

#endif
