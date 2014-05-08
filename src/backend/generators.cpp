/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2013-2014    WrinklyNinja

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

#include "generators.h"
#include "helpers.h"
#include "globals.h"
#include "parsers.h"
#include "streams.h"

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

namespace loot {
    //LOOT Report generation stuff.
    std::string GetOldReportDetails(const boost::filesystem::path& filepath) {
        if (boost::filesystem::exists(filepath)) {
            BOOST_LOG_TRIVIAL(debug) << "Reading the previous report's details section.";
            //Read the whole file in.
            std::string contents;
            loot::ifstream in(filepath, std::ios::binary);
            in.seekg(0, std::ios::end);
            contents.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&contents[0], contents.size());
            in.close();

            //Slim down to only the details section.
            size_t pos1 = contents.find("<div id=\"plugins\"");
            if (pos1 != std::string::npos) {
                pos1 = contents.find("<ul>", pos1);
                if (pos1 != std::string::npos) {
                    size_t pos2 = contents.find("<div id=\"summary\">", pos1);
                    if (pos2 != std::string::npos) {
                        pos2 -= 6; // "</div>"
                        return contents.substr(pos1, pos2 - pos1);
                    }
                }
            }
        }
        return "";
    }

    void WriteMessage(YAML::Emitter& out, const Message& message) {

        //Look for Markdown URL syntax and convert any found.
        boost::regex regex("(\\[([^\\]]+)\\]\\s?\\(|<)((file|https?)://\\S+)(\\)|>)", boost::regex::perl | boost::regex::icase);  // \2 is the label, \3 is the URL.

        boost::match_results<std::string::iterator> results;
        std::string content = message.Content().front().Str();
        std::string converted;
        std::string::iterator start, end;
        start = content.begin();
        end = content.end();

        while (boost::regex_search(start, end, results, regex)) {

            //Get data from match.
            std::string url, label;
            if (results[3].matched)
                url = std::string(results[3].first, results[3].second);
            if (results[2].matched)
                label = std::string(results[2].first, results[2].second);
            else
                label = url;

            //Insert normal text preceding hyperlink.
            converted += std::string(results.prefix().first, results.prefix().second);

            //Insert hyperlink.
            converted += "<a href=\"" + url + "\">" + label + "</a>";

            //Set string to end of matched section.
            start = results.suffix().first;
        }

        //Insert any leftover text.
        converted += std::string(start, end);

        //Now emit as YAML.
        out << YAML::BeginMap;

        if (message.Type() == Message::say) {
            out << YAML::Key << "type" << YAML::Value << "say";
            converted = boost::locale::translate("Note:").str() + " " + converted;
        }
        else if (message.Type() == Message::warn) {
            out << YAML::Key << "type" << YAML::Value << "warn";
            converted = boost::locale::translate("Warning:").str() + " " + converted;
        }
        else {
            out << YAML::Key << "type" << YAML::Value << "error";
            converted = boost::locale::translate("Error:").str() + " " + converted;
        }

        out << YAML::Key << "content" << YAML::Value << converted;

        out << YAML::EndMap;
    }

    void WritePlugin(YAML::Emitter& out, const Plugin& plugin) {
        out << YAML::BeginMap;

        out << YAML::Key << "name"
            << YAML::Value << plugin.Name();

        if (plugin.Crc() != 0) {
            out << YAML::Key << "crc"
                << YAML::Value << IntToHexString(plugin.Crc());
        }

        if (!plugin.Version().empty()) {
            out << YAML::Key << "version"
                << YAML::Value << plugin.Version();
        }

        std::set<Tag> tags = plugin.Tags();
        std::set<Tag> tagsAdd, tagsRemove;
        if (!tags.empty()) {
            for (std::set<Tag>::const_iterator it = tags.begin(), endit = tags.end(); it != endit; ++it) {
                if (it->IsAddition())
                    tagsAdd.insert(*it);
                else
                    tagsRemove.insert(*it);
            }
            if (!tagsAdd.empty()) {
                out << YAML::Key << "tagsAdd"
                    << YAML::Value << tagsAdd;
            }
            if (!tagsRemove.empty()) {
                out << YAML::Key << "tagsRemove"
                    << YAML::Value << tagsRemove;
            }
        }

        std::list<Message> messages = plugin.Messages();
        std::set<PluginDirtyInfo> dirtyInfo = plugin.DirtyInfo();
        for (std::set<PluginDirtyInfo>::const_iterator it = dirtyInfo.begin(), endit = dirtyInfo.end(); it != endit; ++it) {
            boost::format f;
            if (it->ITMs() > 0 && it->UDRs() > 0 && it->DeletedNavmeshes() > 0)
                f = boost::format(boost::locale::translate("Contains %1% ITM records, %2% UDR records and %3% deleted navmeshes. Clean with %4%.")) % it->ITMs() % it->UDRs() % it->DeletedNavmeshes() % it->CleaningUtility();
            else if (it->ITMs() == 0 && it->UDRs() == 0 && it->DeletedNavmeshes() == 0)
                f = boost::format(boost::locale::translate("Clean with %1%.")) % it->CleaningUtility();


            else if (it->ITMs() == 0 && it->UDRs() > 0 && it->DeletedNavmeshes() > 0)
                f = boost::format(boost::locale::translate("Contains %1% UDR records and %2% deleted navmeshes. Clean with %3%.")) % it->UDRs() % it->DeletedNavmeshes() % it->CleaningUtility();
            else if (it->ITMs() == 0 && it->UDRs() == 0 && it->DeletedNavmeshes() > 0)
                f = boost::format(boost::locale::translate("Contains %1% deleted navmeshes. Clean with %2%.")) % it->DeletedNavmeshes() % it->CleaningUtility();
            else if (it->ITMs() == 0 && it->UDRs() > 0 && it->DeletedNavmeshes() == 0)
                f = boost::format(boost::locale::translate("Contains %1% UDR records. Clean with %2%.")) % it->UDRs() % it->CleaningUtility();

            else if (it->ITMs() > 0 && it->UDRs() == 0 && it->DeletedNavmeshes() > 0)
                f = boost::format(boost::locale::translate("Contains %1% ITM records and %2% deleted navmeshes. Clean with %3%.")) % it->ITMs() % it->DeletedNavmeshes() % it->CleaningUtility();
            else if (it->ITMs() > 0 && it->UDRs() == 0 && it->DeletedNavmeshes() == 0)
                f = boost::format(boost::locale::translate("Contains %1% ITM records. Clean with %2%.")) % it->ITMs() % it->CleaningUtility();

            else if (it->ITMs() > 0 && it->UDRs() > 0 && it->DeletedNavmeshes() == 0)
                f = boost::format(boost::locale::translate("Contains %1% ITM records and %2% UDR records. Clean with %3%.")) % it->ITMs() % it->UDRs() % it->CleaningUtility();

            messages.push_back(loot::Message(loot::Message::warn, f.str()));
        }

        if (!messages.empty()) {
            out << YAML::Key << "messages"
                << YAML::Value << YAML::BeginSeq;
            for (std::list<Message>::const_iterator it = messages.begin(), endit = messages.end(); it != endit; ++it) {
                WriteMessage(out, *it);
            }
            out << YAML::EndSeq;
        }

        out << YAML::EndMap;
    }

    void GenerateReport(const boost::filesystem::path& file,
        std::list<Message>& messages,
        const std::list<Plugin>& plugins,
        const std::string& masterlistVersion,
        const bool masterlistUpdateEnabled) {


        //messages.push_front(loot::Message(loot::Message::say, boost::locale::translate("There have been no changes in the Details tab since LOOT was last run for this game.").str()));
        
        //Need to output YAML as a JSON Javascript variable.
        YAML::Emitter yout;
        yout.SetOutputCharset(YAML::EscapeNonAscii);
        yout.SetStringFormat(YAML::DoubleQuoted);
        yout.SetBoolFormat(YAML::TrueFalseBool);
        yout.SetSeqFormat(YAML::Flow);
        yout.SetMapFormat(YAML::Flow);

        yout << YAML::BeginMap;

        yout << YAML::Key << "lootVersion"
            << YAML::Value << IntToString(g_version_major) + "." + IntToString(g_version_minor) + "." + IntToString(g_version_patch);

        yout << YAML::Key << "masterlist"
            << YAML::BeginMap
            << YAML::Key << "updaterEnabled"
            << YAML::Value << masterlistUpdateEnabled
            << YAML::Key << "revision"
            << YAML::Value << masterlistVersion
            << YAML::Key << "date"
            << YAML::Value << ""
            << YAML::EndMap;

        if (!messages.empty()) {
            yout << YAML::Key << "globalMessages"
                << YAML::Value << YAML::BeginSeq;
            for (std::list<Message>::const_iterator it = messages.begin(), endit = messages.end(); it != endit; ++it) {
                WriteMessage(yout, *it);
            }
            yout << YAML::EndSeq;
        }

        if (!plugins.empty()) {
            yout << YAML::Key << "plugins"
                << YAML::Value << YAML::BeginSeq;
            for (std::list<Plugin>::const_iterator it = plugins.begin(), endit = plugins.end(); it != endit; ++it) {
                WritePlugin(yout, *it);
            }
            yout << YAML::EndSeq;
        }

        yout << YAML::EndMap;

        loot::ofstream out(file);
        out << "var data = " << yout.c_str();
        out.close();
    }

    //Default settings file generation.
    void GenerateDefaultSettingsFile(const std::string& file) {

        BOOST_LOG_TRIVIAL(info) << "Generating default settings file.";

        YAML::Node root;
        std::vector<Game> games;

        root["Language"] = "eng";
        root["Game"] = "auto";
        root["Last Game"] = "auto";
        root["Debug Verbosity"] = 0;
        root["Update Masterlist"] = true;

        games.push_back(Game(Game::tes4));
        games.push_back(Game(Game::tes5));
        games.push_back(Game(Game::fo3));
        games.push_back(Game(Game::fonv));
        games.push_back(Game(Game::tes4, "Nehrim").SetDetails("Nehrim - At Fate's Edge", "Nehrim.esm", "https://github.com/loot-developers/loot-oblivion.git", "gh-pages", "", "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1"));

        root["Games"] = games;

        //Save settings.
        YAML::Emitter yout;
        yout.SetIndent(2);
        yout << root;

        boost::filesystem::path p(file);
        loot::ofstream out(p);
        out << yout.c_str();
        out.close();
    }
}

namespace YAML {

    Emitter& operator << (Emitter& out, const loot::PluginDirtyInfo& rhs) {
        out << BeginMap
            << Key << "crc" << Value << Hex << rhs.CRC() << Dec
            << Key << "util" << Value << rhs.CleaningUtility();

        if (rhs.ITMs() > 0)
            out << Key << "itm" << Value << rhs.ITMs();
        if (rhs.UDRs() > 0)
            out << Key << "udr" << Value << rhs.UDRs();
        if (rhs.DeletedNavmeshes() > 0)
            out << Key << "nav" << Value << rhs.DeletedNavmeshes();

        out << EndMap;

        return out;
    }

    Emitter& operator << (Emitter& out, const loot::Game& rhs) {
        out << BeginMap
            << Key << "folder" << Value << rhs.FolderName()
            << Key << "name" << Value << rhs.Name()
            << Key << "master" << Value << rhs.Master()
            << Key << "repo" << Value << rhs.RepoURL()
            << Key << "branch" << Value << rhs.RepoBranch()
            << Key << "path" << Value << rhs.GamePath().string()
            << Key << "registry" << Value << rhs.RegistryKey();

        if (rhs.Id() == loot::Game::tes4)
            out << Key << Value << loot::Game(loot::Game::tes4).FolderName();
        else if (rhs.Id() == loot::Game::tes5)
            out << Key << Value << loot::Game(loot::Game::tes5).FolderName();
        else if (rhs.Id() == loot::Game::fo3)
            out << Key << Value << loot::Game(loot::Game::fo3).FolderName();
        else if (rhs.Id() == loot::Game::fonv)
            out << Key << Value << loot::Game(loot::Game::fonv).FolderName();

        out << EndMap;

        return out;
    }

    Emitter& operator << (Emitter& out, const loot::MessageContent& rhs) {
        out << BeginMap;

        out << Key << "lang" << Value << loot::Language(rhs.Language()).Locale();

        out << Key << "str" << Value << rhs.Str();

        out << EndMap;

        return out;
    }

    Emitter& operator << (Emitter& out, const loot::Message& rhs) {
        out << BeginMap;

        if (rhs.Type() == loot::Message::say)
            out << Key << "type" << Value << "say";
        else if (rhs.Type() == loot::Message::warn)
            out << Key << "type" << Value << "warn";
        else
            out << Key << "type" << Value << "error";

        if (rhs.Content().size() == 1 && rhs.Content().front().Language() == loot::Language::any)
            out << Key << "content" << Value << rhs.Content().front().Str();
        else
            out << Key << "content" << Value << rhs.Content();

        if (!rhs.Condition().empty())
            out << Key << "condition" << Value << rhs.Condition();

        out << EndMap;

        return out;
    }

    Emitter& operator << (Emitter& out, const loot::File& rhs) {
        if (!rhs.IsConditional() && rhs.DisplayName().empty())
            out << rhs.Name();
        else {
            out << BeginMap
                << Key << "name" << Value << rhs.Name();

            if (rhs.IsConditional())
                out << Key << "condition" << Value << rhs.Condition();

            if (rhs.DisplayName() != rhs.Name())
                out << Key << "display" << Value << rhs.DisplayName();

            out << EndMap;
        }

        return out;
    }

    Emitter& operator << (Emitter& out, const loot::Tag& rhs) {
        if (!rhs.IsConditional()) {
            if (rhs.IsAddition())
                out << rhs.Name();
            else
                out << '-' << rhs.Name();
        }
        else {
            out << BeginMap;
            if (rhs.IsAddition())
                out << Key << "name" << Value << rhs.Name();
            else
                out << Key << "name" << Value << ('-' + rhs.Name());

            out << Key << "condition" << Value << rhs.Condition()
                << EndMap;
        }

        return out;
    }

    Emitter& operator << (Emitter& out, const loot::Plugin& rhs) {
        if (!rhs.HasNameOnly()) {

            out << BeginMap
                << Key << "name" << Value << rhs.Name();

            if (rhs.IsPriorityExplicit())
                out << Key << "priority" << Value << rhs.Priority();

            if (!rhs.Enabled())
                out << Key << "enabled" << Value << rhs.Enabled();

            if (!rhs.LoadAfter().empty())
                out << Key << "after" << Value << rhs.LoadAfter();

            if (!rhs.Reqs().empty())
                out << Key << "req" << Value << rhs.Reqs();

            if (!rhs.Incs().empty())
                out << Key << "inc" << Value << rhs.Incs();

            if (!rhs.Messages().empty())
                out << Key << "msg" << Value << rhs.Messages();

            if (!rhs.Tags().empty())
                out << Key << "tag" << Value << rhs.Tags();

            if (!rhs.DirtyInfo().empty())
                out << Key << "dirty" << Value << rhs.DirtyInfo();

            out << EndMap;
        }

        return out;
    }
}