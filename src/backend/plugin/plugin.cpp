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

#include "plugin.h"
#include "plugin_loader.h"
#include "../game/game.h"
#include "../helpers/helpers.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <regex>

using namespace std;

namespace loot {
    /// REGEX expression definition
    ///  Each expression is composed of three parts:
    ///    1. The marker string "version", "ver", "rev", "v" or "r"
    ///    2. The version string itself.

    const char* regex1 =
        "^(?:\\bversion\\b[ ]*(?:[:.\\-]?)|\\brevision\\b(?:[:.\\-]?))[ ]*"
        "((?:alpha|beta|test|debug)?\\s*[-0-9a-zA-Z._+]+\\s*(?:alpha|beta|test|debug)?\\s*(?:[0-9]*))$"
        ;

    const char* regex2 =
        "(?:\\bversion\\b(?:[ :]?)|\\brevision\\b(?:[:.\\-]?))[ ]*"
        "([0-9][-0-9a-zA-Z._]+\\+?)"
        ;

    const char* regex3 =
        "(?:\\bver(?:[:.]?)|\\brev(?:[:.]?))\\s*"
        "([0-9][-0-9a-zA-Z._]*\\+?)"
        ;

    // Matches "Updated: <date>" for the Bashed patch
    const char* regex4 =
        "(?:Updated:)\\s*"
        "([-0-9aAmMpP/ :]+)$"
        ;

    // Matches isolated versions as last resort
    const char* regex5 =
        "(?:(?:\\bv|\\br)(?:\\s?)(?:[-.:])?(?:\\s*))"
        "((?:(?:\\balpha\\b)?|(?:\\bbeta\\b)?)\\s*[0-9]+([-._]*(?!esp|esm)[0-9a-zA-Z]+)*\\+?)"
        ;

    // Matches isolated versions as last resort
    const char* regex6 =
        "((?:(?:\\balpha\\b)?|(?:\\bbeta\\b)?)\\s*\\b[0-9][-0-9a-zA-Z._]*\\+?)$"
        ;

    const char* regex7 =
        "(^\\bmark\\b\\s*\\b[IVX0-9][-0-9a-zA-Z._+]*\\s*(?:alpha|beta|test|debug)?\\s*(?:[0-9]*)?)$"
        ;

    /// Array used to try each of the expressions defined above using
    /// an iteration for each of them.
    const vector<regex> version_checks({
        regex(regex1, regex::ECMAScript | regex::icase),
        regex(regex2, regex::ECMAScript | regex::icase),
        regex(regex3, regex::ECMAScript | regex::icase),
        regex(regex4, regex::ECMAScript | regex::icase),
        regex(regex5, regex::ECMAScript | regex::icase),  //This incorrectly identifies "OBSE v19" where 19 is any integer.
        //regex(regex6, regex::ECMAScript | regex::icase),  //This is responsible for metallicow's false positive.
        regex(regex7, regex::ECMAScript | regex::icase)
    });

    Plugin::Plugin() : PluginMetadata(), _isEmpty(true), isMaster(false), crc(0), numOverrideRecords(0) {}

    Plugin::Plugin(const std::string& n) : PluginMetadata(n), _isEmpty(true), isMaster(false), crc(0), numOverrideRecords(0) {}

    Plugin::Plugin(loot::Game& game, const std::string& n, const bool headerOnly)
        : PluginMetadata(n), _isEmpty(true), isMaster(false), crc(0), numOverrideRecords(0) {
        try {
            PluginLoader loader;
            loader.Load(game, name, headerOnly, false);
            isMaster = loader.IsMaster();
            masters = loader.Masters();
            _isEmpty = loader.IsEmpty();
            formIDs.insert(loader.FormIDs().begin(), loader.FormIDs().end());

            if (!headerOnly) {
                BOOST_LOG_TRIVIAL(trace) << name << ": Caching CRC value.";
                crc = loader.Crc();
                game.crcCache.insert(pair<string, uint32_t>(boost::locale::to_lower(name), crc));
            }

            BOOST_LOG_TRIVIAL(trace) << name << ": Counting override FormIDs.";
            for (const auto& formID : formIDs) {
                if (!boost::iequals(formID.Plugin(), name))
                    ++numOverrideRecords;
            }

            //Also read Bash Tags applied and version string in description.
            string text = loader.Description();
            BOOST_LOG_TRIVIAL(trace) << name << ": " << "Attempting to read the version from the description.";
            for (size_t i = 0; i < version_checks.size(); ++i) {
                smatch what;
                if (regex_search(text, what, version_checks[i])) {
                    //Use the first sub-expression match.
                    version = string(what[1].first, what[1].second);
                    boost::trim(version);
                    BOOST_LOG_TRIVIAL(info) << name << ": " << "Extracted version \"" << version << "\" using regex " << i + 1;
                    break;
                }
            }
            BOOST_LOG_TRIVIAL(trace) << name << ": " << "Attempting to extract Bash Tags from the description.";
            size_t pos1 = text.find("{{BASH:");
            if (pos1 != string::npos && pos1 + 7 != text.length()) {
                pos1 += 7;

                size_t pos2 = text.find("}}", pos1);
                if (pos2 != string::npos) {
                    text = text.substr(pos1, pos2 - pos1);

                    vector<string> bashTags;
                    boost::split(bashTags, text, boost::is_any_of(","));

                    for (auto &tag : bashTags) {
                        boost::trim(tag);
                        BOOST_LOG_TRIVIAL(trace) << name << ": " << "Extracted Bash Tag: " << tag;
                        tags.insert(Tag(tag));
                    }
                }
            }
        }
        catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << "Cannot read plugin file \"" << name << "\". Details: " << e.what();
            messages.push_back(loot::Message(loot::Message::error, (boost::format(boost::locale::translate("Cannot read \"%1%\". Details: %2%")) % name % e.what()).str()));
        }

        BOOST_LOG_TRIVIAL(trace) << name << ": " << "Plugin loading complete.";
    }

    bool Plugin::operator == (const Plugin& rhs) const {
        return boost::iequals(name, rhs.Name());
    }

    bool Plugin::operator != (const Plugin& rhs) const {
        return !(*this == rhs);
    }

    const std::set<FormID>& Plugin::FormIDs() const {
        return formIDs;
    }

    bool Plugin::DoFormIDsOverlap(const Plugin& plugin) const {
        //Basically std::set_intersection except with an early exit instead of an append to results.
        //BOOST_LOG_TRIVIAL(trace) << "Checking for FormID overlap between \"" << name << "\" and \"" << plugin.Name() << "\".";

        set<FormID>::const_iterator i = formIDs.begin(),
            j = plugin.FormIDs().begin(),
            iend = formIDs.end(),
            jend = plugin.FormIDs().end();

        while (i != iend && j != jend) {
            if (*i < *j)
                ++i;
            else if (*j < *i)
                ++j;
            else
                return true;
        }

        return false;
    }

    size_t Plugin::NumOverrideFormIDs() const {
        return numOverrideRecords;
    }

    std::set<FormID> Plugin::OverlapFormIDs(const Plugin& plugin) const {
        set<FormID> otherFormIDs = plugin.FormIDs();
        set<FormID> overlap;

        set_intersection(formIDs.begin(), formIDs.end(), otherFormIDs.begin(), otherFormIDs.end(), inserter(overlap, overlap.end()));

        return overlap;
    }

    std::set<FormID> Plugin::OverrideFormIDs() const {
        set<FormID> fidSubset;
        for (const auto &formID : formIDs) {
            if (!boost::iequals(formID.Plugin(), name))
                fidSubset.insert(formID);
        }
        return fidSubset;
    }

    std::vector<std::string> Plugin::Masters() const {
        return masters;
    }

    bool Plugin::IsMaster() const {
        return isMaster;
    }

    bool Plugin::IsEmpty() const {
        return _isEmpty;
    }

    bool Plugin::IsValid(const Game& game) const {
        BOOST_LOG_TRIVIAL(trace) << "Checking to see if \"" << name << "\" is a valid plugin.";
        // Rather than just checking the extension, try also parsing the file header, and see if it fails.
        if (!boost::iends_with(name, ".esm") && !boost::iends_with(name, ".esp")) {
            return false;
        }

        try {
            PluginLoader loader;
            return loader.Load(game, name, true, true);
        }
        catch (std::exception& /*e*/) {
            BOOST_LOG_TRIVIAL(warning) << "The .es(p|m) file \"" << name << "\" is not a valid plugin.";
            return false;
        }
        return true;
    }

    bool Plugin::IsActive(const Game& game) const {
        return game.activePlugins.find(boost::locale::to_lower(name)) != game.activePlugins.end();
    }

    std::string Plugin::Version() const {
        return version;
    }

    uint32_t Plugin::Crc() const {
        return crc;
    }

    bool Plugin::CheckInstallValidity(const Game& game) {
        BOOST_LOG_TRIVIAL(trace) << "Checking that the current install is valid according to " << name << "'s data.";
        if (IsActive(game)) {
            if (tags.find(Tag("Filter")) == tags.end()) {
                for (const auto &master : masters) {
                    if (!boost::filesystem::exists(game.DataPath() / master) && !boost::filesystem::exists(game.DataPath() / (master + ".ghost"))) {
                        BOOST_LOG_TRIVIAL(error) << "\"" << name << "\" requires \"" << master << "\", but it is missing.";
                        messages.push_back(Message(Message::error, (boost::format(boost::locale::translate("This plugin requires \"%1%\" to be installed, but it is missing.")) % master).str()));
                    }
                    else if (!Plugin(master).IsActive(game)) {
                        BOOST_LOG_TRIVIAL(error) << "\"" << name << "\" requires \"" << master << "\", but it is inactive.";
                        messages.push_back(Message(Message::error, (boost::format(boost::locale::translate("This plugin requires \"%1%\" to be active, but it is inactive.")) % master).str()));
                    }
                }
            }

            auto pluginExists = [](const Game& game, const File& file) {
                return boost::filesystem::exists(game.DataPath() / file.Name())
                    || ((boost::iends_with(file.Name(), ".esp") || boost::iends_with(file.Name(), ".esm")) && boost::filesystem::exists(game.DataPath() / (file.Name() + ".ghost")));
            };
            for (const auto &req : requirements) {
                if (!pluginExists(game, req)) {
                    BOOST_LOG_TRIVIAL(error) << "\"" << name << "\" requires \"" << req.Name() << "\", but it is missing.";
                    messages.push_back(loot::Message(Message::error, (boost::format(boost::locale::translate("This plugin requires \"%1%\" to be installed, but it is missing.")) % req.Name()).str()));
                }
            }
            for (const auto &inc : incompatibilities) {
                if (pluginExists(game, inc) && Plugin(inc.Name()).IsActive(game)) {
                    BOOST_LOG_TRIVIAL(error) << "\"" << name << "\" is incompatible with \"" << inc.Name() << "\", but both are present.";
                    messages.push_back(loot::Message(Message::error, (boost::format(boost::locale::translate("This plugin is incompatible with \"%1%\", but both are present.")) % inc.Name()).str()));
                }
            }
        }

        // Also generate dirty messages.
        for (const auto &element : _dirtyInfo) {
            messages.push_back(element.AsMessage());
        }

        return !_dirtyInfo.empty();
    }

    bool Plugin::LoadsBSA(const Game& game) const {
        if (IsRegexPlugin())
            return false;
        if (game.Id() == Game::tes5) {
            // Skyrim plugins only load BSAs that exactly match their basename.
            return boost::filesystem::exists(game.DataPath() / (name.substr(0, name.length() - 3) + "bsa"));
        }
        else {
            //Oblivion .esp files and FO3, FNV plugins can load BSAs which begin with the plugin basename.
            if (game.Id() != Game::tes4 || boost::iends_with(name, ".esp")) {
                string basename = name.substr(0, name.length() - 4);
                for (boost::filesystem::directory_iterator it(game.DataPath()); it != boost::filesystem::directory_iterator(); ++it) {
                    if (it->path().extension().string() == ".bsa" && boost::istarts_with(it->path().filename().string(), basename))
                        return true;
                }
            }
            return false;
        }
    }
}
