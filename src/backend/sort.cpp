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

#include "game.h"
#include "helpers.h"

#include <boost/log/trivial.hpp>
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>

using namespace std;

using boost::format;

namespace loc = boost::locale;
namespace fs = boost::filesystem;

namespace loot {

    void Game::SortPlugins(const unsigned int language, std::list<Message>& messages, std::function<void(const std::string&)> callback) {
        boost::thread_group group;

        BOOST_LOG_TRIVIAL(info) << "Using message language: " << Language(language).Name();

        ///////////////////////////////////////////////////////
        // Load Plugins & Lists
        ///////////////////////////////////////////////////////

        callback("Reading installed plugins...");

        group.create_thread([this, language, &messages]() {
            try {
                this->masterlist.Load(*this, language);
            }
            catch (exception &e) {
                messages.push_back(loot::Message(loot::Message::error, (format(loc::translate("Masterlist parsing failed. Details: %1%")) % e.what()).str()));
            }
        });
        group.create_thread([this]() {
            this->LoadPlugins(false);
        });
        group.join_all();

        //Now load userlist.
        if (fs::exists(this->UserlistPath())) {
            BOOST_LOG_TRIVIAL(debug) << "Parsing userlist at: " << this->UserlistPath();

            try {
                this->userlist.Load(this->UserlistPath());
            }
            catch (exception& e) {
                BOOST_LOG_TRIVIAL(error) << "Userlist parsing failed. Details: " << e.what();
                messages.push_back(loot::Message(loot::Message::error, (format(loc::translate("Userlist parsing failed. Details: %1%")) % e.what()).str()));
            }
        }

        ///////////////////////////////////////////////////////
        // Evaluate Global Messages
        ///////////////////////////////////////////////////////

        callback("Evaluating global messages...");

        //Merge all global message lists.
        BOOST_LOG_TRIVIAL(debug) << "Merging all global message lists.";
        if (!this->masterlist.messages.empty())
            messages.insert(messages.end(), this->masterlist.messages.begin(), this->masterlist.messages.end());
        if (!this->userlist.messages.empty())
            messages.insert(messages.end(), this->userlist.messages.begin(), this->userlist.messages.end());

        //Evaluate any conditions in the global messages.
        BOOST_LOG_TRIVIAL(debug) << "Evaluating global message conditions.";
        try {
            list<loot::Message>::iterator it = messages.begin();
            while (it != messages.end()) {
                if (!it->EvalCondition(*this, language))
                    it = messages.erase(it);
                else
                    ++it;
            }
        }
        catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << "A global message contains a condition that could not be evaluated. Details: " << e.what();
            messages.push_back(loot::Message(loot::Message::error, (format(loc::translate("A global message contains a condition that could not be evaluated. Details: %1%")) % e.what()).str()));
        }
    }
}