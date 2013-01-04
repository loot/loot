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
#ifndef __BOSS_METADATA__
#define __BOSS_METADATA__

#include <string>
#include <list>
#include <set>

namespace boss {

    class ConditionalData {
    public:
        ConditionalData();
        ConditionalData(const std::string s);
        std::string condition;

        bool EvalCondition() const;
    };

    class Message : public ConditionalData {
    public:
        std::string type;
        std::string language;
        std::string content;
    };

    class File : public ConditionalData {
    public:
        std::string name;
        std::string display;
    };

    class Tag : public ConditionalData {
    public:
        Tag();
        Tag(const std::string tag);
        Tag(const std::string condition, const std::string tag);

        bool addTag;
        std::string name;

        bool IsAddition() const;
    };

    struct file_comp {
        bool operator() (const File& lhs, const File& rhs) const {
            return true;
        }
    };

    struct tag_comp {
        bool operator() (const Tag& lhs, const Tag& rhs) const {
            return true;
        }
    };

    class Plugin {
    public:
        std::string name;
        bool enabled;
        int priority;
        std::list<File> loadAfter;
        std::list<File> requirements;
        std::set<File, file_comp> incompatibilities;
        std::list<Message> messages;
        std::set<Tag, tag_comp> tags;

        void EvalAllConditions();
    };

    struct plugin_comp {
        bool operator() (const Plugin& lhs, const Plugin& rhs) const {
            return true;
        }
    };
}

namespace YAML {
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
            if(!node.IsMap())
                return false;

            if (node["condition"])
                rhs.condition = node["condition"].as<std::string>();
            if (node["name"])
                rhs.name = node["name"].as<std::string>();
            if (node["display"])
                rhs.name = node["display"].as<std::string>();
            return true;
        }
    };

    template<>
    struct convert<boss::Tag> {
        static Node encode(const boss::Tag& rhs) {
            Node node;
            node["condition"] = rhs.condition;
            node["name"] = rhs.name;
            return node;
        }

        static bool decode(const Node& node, boss::Tag& rhs) {
            if(node.IsMap()) {
                if (node["condition"])
                    rhs.condition = node["condition"].as<std::string>();
                if (node["name"])
                    rhs.name = node["name"].as<std::string>();
            } else if (node.IsScalar()) {
                rhs.condition = "";
                rhs.name = node.as<std::string>();
            }
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
            if (node["priority"])
                rhs.priority = node["priority"].as<int>();
            if (node["after"])
                rhs.loadAfter = node["after"].as< std::list<boss::File> >();
            if (node["req"])
                rhs.requirements = node["req"].as< std::list<boss::File> >();
            if (node["inc"])
                rhs.incompatibilities = node["inc"].as< std::set<boss::File, boss::file_comp> >();
            if (node["msg"])
                rhs.messages = node["msg"].as< std::list<boss::Message> >();
            if (node["tag"])
                rhs.tags = node["tag"].as< std::set<boss::Tag, boss::tag_comp> >();
            return true;
        }
    };
}

#endif
