/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2012    WrinklyNinja

    This file is part of BOSS.

    BOSS is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    BOSS is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BOSS.  If not, see
    <http://www.gnu.org/licenses/>.
*/

#ifndef __BOSS_PARSERS__
#define __BOSS_PARSERS__

#include "metadata.h"

namespace YAML {

    ///////////////////////
    // Parser
    ///////////////////////

    template<>
    struct convert<boss::Message> {
        static Node encode(const boss::Message& rhs) {
            Node node;
            node["condition"] = rhs.condition;
            node["type"] = rhs.type;
            node["content"] = rhs.content;
            node["lang"] = rhs.language;
            return node;
        }

        static bool decode(const Node& node, boss::Message& rhs) {
            if(!node.IsMap())
                return false;

            if (node["condition"])
                rhs.condition = node["condition"].as<std::string>();
            if (node["type"])
                rhs.type = node["type"].as<std::string>();
            if (node["content"])
                rhs.content = node["content"].as<std::string>();
            if (node["lang"])
                rhs.language = node["lang"].as<std::string>();
            return true;
        }
    };

    template<>
    struct convert<boss::File> {
        static Node encode(const boss::File& rhs) {
            Node node;
            node["condition"] = rhs.condition;
            node["name"] = rhs.name;
            node["display"] = rhs.display;
            return node;
        }

        static bool decode(const Node& node, boss::File& rhs) {
            if(node.IsMap()) {
                if (node["condition"])
                    rhs.condition = node["condition"].as<std::string>();
                if (node["name"])
                    rhs.name = node["name"].as<std::string>();
                if (node["display"])
                    rhs.display = node["display"].as<std::string>();
            } else {
                rhs.name = node.as<std::string>();
            }
            return true;
        }
    };

    template<>
    struct convert<boss::Tag> {
        static Node encode(const boss::Tag& rhs) {
            Node node;
            node["condition"] = rhs.condition;
            if (!rhs.addTag)
                node["name"] = "-" + rhs.name;
            else
                node["name"] = rhs.name;
            return node;
        }

        static bool decode(const Node& node, boss::Tag& rhs) {
            std::string condition, tag;
            if(node.IsMap()) {
                if (node["condition"])
                    condition = node["condition"].as<std::string>();
                if (node["name"])
                    tag = node["name"].as<std::string>();
            } else if (node.IsScalar()) {
                tag = node.as<std::string>();
            }
            rhs = boss::Tag(condition, tag);
            return true;
        }
    };

    template<class T, class Compare>
    struct convert< std::set<T, Compare> > {
      static Node encode(const std::set<T, Compare>& rhs) {
          Node node;
          for (typename std::set<T, Compare>::const_iterator it=rhs.begin(), endIt=rhs.end(); it != endIt; ++it) {
              node.push_back(*it);
          }
          return node;
      }

      static bool decode(const Node& node, std::set<T, Compare>& rhs) {
        if(!node.IsSequence())
            return false;

        rhs.clear();
        for(YAML::const_iterator it=node.begin();it!=node.end();++it) {
            rhs.insert(it->as<T>());
        }
        return true;

      }
    };

    template<>
    struct convert<boss::Plugin> {
        static Node encode(const boss::Plugin& rhs) {
            Node node;
            node["name"] = rhs.name;
            node["enabled"] = rhs.enabled;
            node["priority"] = rhs.priority;
            node["after"] = rhs.loadAfter;
            node["req"] = rhs.requirements;
            node["inc"] = rhs.incompatibilities;
            node["msg"] = rhs.messages;
            node["tag"] = rhs.tags;

            return node;
        }

        static bool decode(const Node& node, boss::Plugin& rhs) {
            if(!node.IsMap())
                return false;

            if (node["name"])
                rhs.name = node["name"].as<std::string>();
            if (node["enabled"])
                rhs.enabled = node["enabled"].as<bool>();
            else
                rhs.enabled = true;

            if (node["priority"])
                rhs.priority = node["priority"].as<int>();
            else
                rhs.priority = 0;

            if (node["after"])
                rhs.loadAfter = node["after"].as< std::list<boss::File> >();
            if (node["req"])
                rhs.requirements = node["req"].as< std::list<boss::File> >();
            if (node["inc"])
                rhs.incompatibilities = node["inc"].as< std::set<boss::File, boss::file_comp> >();
            if (node["msg"])
                rhs.messages = node["msg"].as< std::list<boss::Message> >();
            if (node["tag"])
                rhs.tags = node["tag"].as< std::list<boss::Tag> >();
            return true;
        }
    };

    ///////////////////
    // Emitter
    ///////////////////

    template<class T, class Compare>
    Emitter& operator << (Emitter& out, const std::set<T, Compare>& rhs) {
        out << BeginSeq;
        for (typename std::set<T, Compare>::const_iterator it=rhs.begin(), endIt=rhs.end(); it != endIt; ++it) {
            out << *it;
        }
        out << EndSeq;
    }

    Emitter& operator << (Emitter& out, const boss::Message& rhs) {
        out << BeginMap
            << Key << "type" << rhs.type
            << Key << "content" << rhs.content;

        if (!rhs.language.empty())
            out << Key << "lang" << rhs.language;

        if (!rhs.condition.empty())
            out << Key << "condition" << rhs.condition;

        out << EndMap;
    }

    Emitter& operator << (Emitter& out, const boss::File& rhs) {
        if (rhs.condition.empty() && rhs.display.empty())
            out << rhs.name;
        else {
            out << BeginMap
                << Key << "name" << rhs.name;

            if (!rhs.condition.empty())
                out << Key << "condition" << rhs.condition;

            if (!rhs.display.empty())
                out << Key << "display" << rhs.display;

            out << EndMap;
        }
    }

    Emitter& operator << (Emitter& out, const boss::Tag& rhs) {
        if (rhs.condition.empty())
            out << rhs.Data();
        else {
            out << BeginMap
                << Key << "name" << rhs.Data()
                << Key << "condition" << rhs.condition
                << EndMap;
        }
    }

    Emitter& operator << (Emitter& out, const boss::Plugin& rhs) {
        if (!rhs.NameOnly()) {

            out << BeginMap
                << Key << "name" << Value << rhs.name;

            if (rhs.priority != 0)
                out << Key << "priority" << Value << rhs.priority;

            if (!rhs.enabled)
                out << Key << "enabled" << Value << rhs.enabled;

            if (!rhs.loadAfter.empty())
                out << Key << "after" << Value << rhs.loadAfter;

            if (!rhs.requirements.empty())
                out << Key << "req" << Value << rhs.requirements;

            if (!rhs.incompatibilities.empty())
                out << Key << "inc" << Value << rhs.incompatibilities;

            if (!rhs.messages.empty())
                out << Key << "msg" << Value << rhs.messages;

            if (!rhs.tags.empty())
                out << Key << "tag" << Value << rhs.tags;

            out << EndMap;
        }
    }
}

namespace boss {

    /* Conditional parser

    Expression is of the form:

    condition = keyword type

    expression = -'(' condition (operator condition)* -')' operator

    comp_exp = expression (operator expression)*
    -'(' keyword type (operator -'(' keyword type -')' )*

    keyword = if | ifnot
    operator = and | or
    type = file(...) | checksum(...) | version(...) | active(...)

    'and' takes precedence over 'or', and brackets can also be used to indicate
    overriding precedences.

    Split string up into sub-strings according to the bracketing if present, then
    according to operators, until each sub-string only contains one condition.

    {
        expression =
            term
            >> *(string("or") >> term)
            ;

        term =
            factor
            >> *(string("and") >> factor)

        factor =
            condition
            | '(' >> expression >> ')'
            |

        condition = keyword >> type

    }

    */
}
#endif
