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
    void GetOldReportDetails(const boost::filesystem::path& filepath, YAML::Node& node) {
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

            //Cut off non-JSON part of file.
            contents = contents.substr(11);  //"var data = ".
            //Parse as YAML.
            node = YAML::Load(contents);
            if (node["plugins"]) {
                node = node["plugins"];
            }
        }
    }

    bool AreDetailsEqual(const YAML::Node& lhs, const YAML::Node& rhs) {
        //We want to check for plugin order and messages. 
        if (lhs.IsSequence() && rhs.IsSequence()) {
            std::list<std::string> lhs_names, rhs_names;
            std::list<Message> lhs_messages, rhs_messages;
            
            for (const auto &element: lhs) {
                if (element["name"])
                    lhs_names.push_back(element["name"].as<std::string>());
                if (element["messages"]) {
                    std::list<Message> messages = element["messages"].as< std::list<Message> >();
                    lhs_messages.insert(lhs_messages.end(), messages.begin(), messages.end());
                }
            }
            for (const auto &element: rhs) {
                if (element["name"])
                    rhs_names.push_back(element["name"].as<std::string>());
                if (element["messages"]) {
                    std::list<Message> messages = element["messages"].as< std::list<Message> >();
                    rhs_messages.insert(rhs_messages.end(), messages.begin(), messages.end());
                }
            }

            return lhs_names == rhs_names && lhs_messages == rhs_messages;
        }
        else
            return false;
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

    void WritePlugin(YAML::Emitter& out, const Plugin& plugin, const Game& game) {
        out << YAML::BeginMap;

        out << YAML::Key << "name"
            << YAML::Value << plugin.Name();

        out << YAML::Key << "isActive";
        if (game.IsActive(plugin.Name()))
            out << YAML::Value << true;
        else
            out << YAML::Value << false;

        if (plugin.Crc() != 0) {
            out << YAML::Key << "crc"
                << YAML::Value << IntToHexString(plugin.Crc());
        }

        if (!plugin.Version().empty()) {
            out << YAML::Key << "version"
                << YAML::Value << boost::locale::translate("Version").str() + ": " + plugin.Version();
        }

        std::set<Tag> tags = plugin.Tags();
        std::set<Tag> tagsAdd, tagsRemove;
        if (!tags.empty()) {
            for (const auto &tag: tags) {
                if (tag.IsAddition())
                    tagsAdd.insert(tag);
                else
                    tagsRemove.insert(tag);
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
        for (const auto &element: dirtyInfo) {
            boost::format f;
            if (element.ITMs() > 0 && element.UDRs() > 0 && element.DeletedNavmeshes() > 0)
                f = boost::format(boost::locale::translate("Contains %1% ITM records, %2% UDR records and %3% deleted navmeshes. Clean with %4%.")) % element.ITMs() % element.UDRs() % element.DeletedNavmeshes() % element.CleaningUtility();
            else if (element.ITMs() == 0 && element.UDRs() == 0 && element.DeletedNavmeshes() == 0)
                f = boost::format(boost::locale::translate("Clean with %1%.")) % element.CleaningUtility();


            else if (element.ITMs() == 0 && element.UDRs() > 0 && element.DeletedNavmeshes() > 0)
                f = boost::format(boost::locale::translate("Contains %1% UDR records and %2% deleted navmeshes. Clean with %3%.")) % element.UDRs() % element.DeletedNavmeshes() % element.CleaningUtility();
            else if (element.ITMs() == 0 && element.UDRs() == 0 && element.DeletedNavmeshes() > 0)
                f = boost::format(boost::locale::translate("Contains %1% deleted navmeshes. Clean with %2%.")) % element.DeletedNavmeshes() % element.CleaningUtility();
            else if (element.ITMs() == 0 && element.UDRs() > 0 && element.DeletedNavmeshes() == 0)
                f = boost::format(boost::locale::translate("Contains %1% UDR records. Clean with %2%.")) % element.UDRs() % element.CleaningUtility();

            else if (element.ITMs() > 0 && element.UDRs() == 0 && element.DeletedNavmeshes() > 0)
                f = boost::format(boost::locale::translate("Contains %1% ITM records and %2% deleted navmeshes. Clean with %3%.")) % element.ITMs() % element.DeletedNavmeshes() % element.CleaningUtility();
            else if (element.ITMs() > 0 && element.UDRs() == 0 && element.DeletedNavmeshes() == 0)
                f = boost::format(boost::locale::translate("Contains %1% ITM records. Clean with %2%.")) % element.ITMs() % element.CleaningUtility();

            else if (element.ITMs() > 0 && element.UDRs() > 0 && element.DeletedNavmeshes() == 0)
                f = boost::format(boost::locale::translate("Contains %1% ITM records and %2% UDR records. Clean with %3%.")) % element.ITMs() % element.UDRs() % element.CleaningUtility();

            messages.push_back(loot::Message(loot::Message::warn, f.str()));
        }

        if (!messages.empty()) {
            out << YAML::Key << "messages"
                << YAML::Value << YAML::BeginSeq;
            for (const auto &message: messages) {
                WriteMessage(out, message);
            }
            out << YAML::EndSeq;
        }

        out << YAML::EndMap;
    }

    void GenerateReportData(const Game& game,
        std::list<Message>& messages,
        const std::list<Plugin>& plugins,
        const std::string& masterlistVersion,
        const std::string& masterlistDate,
        const bool masterlistUpdateEnabled) {

        YAML::Node oldDetails;
        GetOldReportDetails(game.ReportDataPath(), oldDetails);
        
        //Need to output YAML as a JSON Javascript variable.
        YAML::Emitter yout;
        yout.SetOutputCharset(YAML::EscapeNonAscii);
        yout.SetStringFormat(YAML::DoubleQuoted);
        yout.SetBoolFormat(YAML::TrueFalseBool);
        yout.SetSeqFormat(YAML::Flow);
        yout.SetMapFormat(YAML::Flow);

        BOOST_LOG_TRIVIAL(debug) << "Generating JSON report data.";

        yout << YAML::BeginMap;

        yout << YAML::Key << "lootVersion"
            << YAML::Value << std::to_string(g_version_major) + "." + std::to_string(g_version_minor) + "." + std::to_string(g_version_patch);

        yout << YAML::Key << "masterlist"
            << YAML::BeginMap
            << YAML::Key << "updaterEnabled"
            << YAML::Value;

        if (masterlistUpdateEnabled)
            yout << boost::locale::translate("Enabled").str();
        else
            yout << boost::locale::translate("Disabled").str();

        yout << YAML::Key << "revision"
            << YAML::Value << masterlistVersion
            << YAML::Key << "date"
            << YAML::Value << masterlistDate
            << YAML::EndMap;

        if (!plugins.empty()) {
            BOOST_LOG_TRIVIAL(debug) << "Generating JSON plugin data.";
            YAML::Emitter tempout;
            tempout.SetOutputCharset(YAML::EscapeNonAscii);
            tempout.SetStringFormat(YAML::DoubleQuoted);
            tempout.SetBoolFormat(YAML::TrueFalseBool);
            tempout.SetSeqFormat(YAML::Flow);
            tempout.SetMapFormat(YAML::Flow);

            tempout << YAML::BeginSeq;
            for (const auto &plugin: plugins) {
                WritePlugin(tempout, plugin, game);
            }
            tempout << YAML::EndSeq;

            YAML::Node node = YAML::Load(tempout.c_str());

            if (AreDetailsEqual(node, oldDetails))
                messages.push_front(loot::Message(loot::Message::say, boost::locale::translate("There have been no changes in the Details tab since LOOT was last run for this game.").str()));
                
            //Need to generate output twice because passing the node causes !<!> to be written before every key and value for some reason.
            yout << YAML::Key << "plugins"
                << YAML::Value << YAML::BeginSeq;
            for (const auto &plugin: plugins) {
                WritePlugin(yout, plugin, game);
            }
            yout << YAML::EndSeq;
        }
        else if (oldDetails.size() == 0)
            messages.push_front(loot::Message(loot::Message::say, boost::locale::translate("There have been no changes in the Details tab since LOOT was last run for this game.").str()));

        if (!messages.empty()) {
            BOOST_LOG_TRIVIAL(debug) << "Generating JSON general message data.";
            yout << YAML::Key << "globalMessages"
                << YAML::Value << YAML::BeginSeq;
            for (const auto &message: messages) {
                WriteMessage(yout, message);
            }
            yout << YAML::EndSeq;
        }

        BOOST_LOG_TRIVIAL(debug) << "Generating text translations.";
        yout << YAML::Key << "l10n"
            << YAML::BeginMap

            << YAML::Key << "txtSummarySec"
            << YAML::Value << boost::locale::translate("Summary").str()

            << YAML::Key << "txtSummary"
            << YAML::Value << boost::locale::translate("Summary").str()

            << YAML::Key << "txtLootVersion"
            << YAML::Value << boost::locale::translate("LOOT Version").str()

            << YAML::Key << "txtMasterlistRevision"
            << YAML::Value << boost::locale::translate("Masterlist Revision").str()

            << YAML::Key << "txtMasterlistDate"
            << YAML::Value << boost::locale::translate("Masterlist Date").str()

            << YAML::Key << "txtMasterlistUpdating"
            << YAML::Value << boost::locale::translate("Masterlist Updating").str()

            << YAML::Key << "txtTotalMessageNo"
            << YAML::Value << boost::locale::translate("Total Number of Messages").str()

            << YAML::Key << "txtTotalWarningNo"
            << YAML::Value << boost::locale::translate("Number of Warnings").str()

            << YAML::Key << "txtTotalErrorNo"
            << YAML::Value << boost::locale::translate("Number of Errors").str()

            << YAML::Key << "txtGeneralMessages"
            << YAML::Value << boost::locale::translate("General Messages").str()

            << YAML::Key << "txtDetailsSec"
            << YAML::Value << boost::locale::translate("Details").str()

            << YAML::Key << "filtersToggle"
            << YAML::Value << boost::locale::translate("Filters").str()

            << YAML::Key << "txtHideVersionNumbers"
            << YAML::Value << boost::locale::translate("Hide Version Numbers").str()

            << YAML::Key << "txtHideCRCs"
            << YAML::Value << boost::locale::translate("Hide CRCs").str()

            << YAML::Key << "txtHideBashTags"
            << YAML::Value << boost::locale::translate("Hide Bash Tag Suggestions").str()

            << YAML::Key << "txtHideNotes"
            << YAML::Value << boost::locale::translate("Hide Notes").str()

            << YAML::Key << "txtHideDoNotCleanMessages"
            << YAML::Value << boost::locale::translate("Hide 'Do Not Clean' Messages").str()

            << YAML::Key << "txtHideInactivePluginMessages"
            << YAML::Value << boost::locale::translate("Hide Inactive Plugin Messages").str()

            << YAML::Key << "txtHideAllPluginMessages"
            << YAML::Value << boost::locale::translate("Hide All Plugin Messages").str()

            << YAML::Key << "txtHideMessagelessPlugins"
            << YAML::Value << boost::locale::translate("Hide Messageless Plugins").str()

            << YAML::Key << "txtPluginsHidden"
            << YAML::Value << boost::locale::translate("Plugins hidden").str()

            << YAML::Key << "txtMessagesHidden"
            << YAML::Value << boost::locale::translate("Messages hidden").str()

            << YAML::EndMap;

        yout << YAML::EndMap;

        loot::ofstream out(game.ReportDataPath());
        out << "var data = " << yout.c_str();
        out.close();
    }

    //Default settings file generation.
    void GenerateDefaultSettingsFile(const std::string& file) {

        BOOST_LOG_TRIVIAL(info) << "Generating default settings file.";

        YAML::Node root;
        std::vector<Game> games;

        root["Language"] = "en";
        root["Game"] = "auto";
        root["Last Game"] = "auto";
        root["Debug Verbosity"] = 0;
        root["Update Masterlist"] = true;

        games.push_back(Game(Game::tes4));
        games.push_back(Game(Game::tes5));
        games.push_back(Game(Game::fo3));
        games.push_back(Game(Game::fonv));
        games.push_back(Game(Game::tes4, "Nehrim").SetDetails("Nehrim - At Fate's Edge", "Nehrim.esm", "https://github.com/loot/oblivion.git", "master", "", "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1"));

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
            << Key << "type" << Value << loot::Game(rhs.Id()).FolderName()
            << Key << "folder" << Value << rhs.FolderName()
            << Key << "name" << Value << rhs.Name()
            << Key << "master" << Value << rhs.Master()
            << Key << "repo" << Value << rhs.RepoURL()
            << Key << "branch" << Value << rhs.RepoBranch()
            << Key << "path" << Value << rhs.GamePath().string()
            << Key << "registry" << Value << rhs.RegistryKey()
            << EndMap;

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

        if (rhs.Content().size() == 1)
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