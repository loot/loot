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

#include "message.h"
#include "../helpers/language.h"

#include <boost/log/trivial.hpp>

using namespace std;

namespace loot {
    const unsigned int Message::say = 0;
    const unsigned int Message::warn = 1;
    const unsigned int Message::error = 2;

    Message::Message() : _type(Message::say) {}

    Message::Message(const unsigned int type, const std::string& content,
                     const std::string& condition) : _type(type), ConditionalMetadata(condition) {
        _content.push_back(MessageContent(content, Language::english));
    }

    Message::Message(const unsigned int type, const std::vector<MessageContent>& content,
                     const std::string& condition) : _type(type), _content(content), ConditionalMetadata(condition) {}

    bool Message::operator < (const Message& rhs) const {
        if (!_content.empty() && !rhs.Content().empty())
            return boost::ilexicographical_compare(_content.front().Str(), rhs.Content().front().Str());
        else if (_content.empty())
            return true;
        else
            return false;
    }

    bool Message::operator == (const Message& rhs) const {
        return (_type == rhs.Type() && _content == rhs.Content());
    }

    bool Message::EvalCondition(loot::Game& game, const unsigned int language) {
        BOOST_LOG_TRIVIAL(trace) << "Choosing message content for language: " << Language(language).Name();

        if (_content.size() > 1) {
            if (language == Language::any)  //Can use a message of any language, so use the first string.
                _content.resize(1);
            else {
                MessageContent english, match;
                for (const auto &mc : _content) {
                    if (mc.Language() == language) {
                        match = mc;
                        break;
                    }
                    else if (mc.Language() == Language::english)
                        english = mc;
                }
                _content.resize(1);
                if (!match.Str().empty())
                    _content[0] = match;
                else
                    _content[0] = english;
            }
        }
        return ConditionalMetadata::EvalCondition(game);
    }

    MessageContent Message::ChooseContent(const unsigned int language) const {
        BOOST_LOG_TRIVIAL(trace) << "Choosing message content.";
        if (_content.size() == 1 || language == Language::any)
            return _content[0];
        else {
            MessageContent english, match;
            for (const auto &mc : _content) {
                if (mc.Language() == language) {
                    match = mc;
                    break;
                }
                else if (mc.Language() == Language::english)
                    english = mc;
            }
            if (!match.Str().empty())
                return match;
            else
                return english;
        }
    }

    unsigned int Message::Type() const {
        return _type;
    }

    std::vector<MessageContent> Message::Content() const {
        return _content;
    }
}

namespace YAML {
    Emitter& operator << (Emitter& out, const loot::Message& rhs) {
        out << BeginMap;

        if (rhs.Type() == loot::Message::say)
            out << Key << "type" << Value << "say";
        else if (rhs.Type() == loot::Message::warn)
            out << Key << "type" << Value << "warn";
        else
            out << Key << "type" << Value << "error";

        if (rhs.Content().size() == 1)
            out << Key << "content" << Value << YAML::SingleQuoted << rhs.Content().front().Str();
        else
            out << Key << "content" << Value << rhs.Content();

        if (!rhs.Condition().empty())
            out << Key << "condition" << Value << YAML::SingleQuoted << rhs.Condition();

        out << EndMap;

        return out;
    }
}