/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2013    WrinklyNinja

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
#ifndef __BOSS_GENERATORS__
#define __BOSS_GENERATORS__

#include "helpers.h"
#include "metadata.h"
#include "globals.h"
#include "parsers.h"
#include "streams.h"

#include <pugixml.hpp>
#include <yaml-cpp/yaml.h>

#include <string>
#include <list>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>

namespace boss {

    //BOSS Report generation stuff.

    struct xml_string_writer: pugi::xml_writer {
        std::string result;

        virtual void write(const void* data, size_t size) {
            result += std::string(static_cast<const char*>(data), size);
        }
    };

    inline void WriteMessage(pugi::xml_node& listItem, unsigned int type, std::string content) {

        if (type == g_message_say)
            content = boost::locale::translate("Note:").str() + " " + content;
        else if (type == g_message_tag) {
            content = boost::locale::translate("Bash Tag Suggestion(s):").str() + " " + content;
        } else if (type == g_message_warn)
            content = boost::locale::translate("Warning:").str() + " " + content;
        else
            content = boost::locale::translate("Error:").str() + " " + content;

        //Now look for Markdown URL syntax and convert any found.
        boost::regex regex("(\\[([^\\]]+)\\]\\s?\\(|<)((file|https?)://\\S+)(\\)|>)", boost::regex::perl | boost::regex::icase);  // \2 is the label, \3 is the URL.

        boost::match_results<std::string::iterator> results;
        std::string::iterator start, end;
        start = content.begin();
        end = content.end();

        while (boost::regex_search(start, end, results, regex)) {

            BOOST_LOG_TRIVIAL(trace) << "Results size: " << results.size();
            
            //Get data from match.
            std::string url, label;
            if (results[3].matched)
                url = std::string(results[3].first, results[3].second);
            if (results[2].matched)
                label = std::string(results[2].first, results[2].second);
            else
                label = url;

            //Insert normal text preceding hyperlink.
            listItem.append_child(pugi::node_pcdata).set_value(std::string(results.prefix().first, results.prefix().second).c_str());

            //Insert hyperlink.
            pugi::xml_node a = listItem.append_child();
            a.set_name("a");
            a.append_attribute("href").set_value(url.c_str());
            a.text().set(label.c_str());

            BOOST_LOG_TRIVIAL(trace) << "FOO";

            //Set string to end of matched section.
            start = results.suffix().first;
        }

        //Insert any leftover text.
        listItem.append_child(pugi::node_pcdata).set_value(std::string(start, end).c_str());

    }

    inline void GenerateHead(pugi::xml_document& doc) {
        BOOST_LOG_TRIVIAL(trace) << "Creating BOSS report head.";

        //Add DOCTYPE node.
        doc.append_child(pugi::node_doctype).set_value("html");

        //Create 'head' node.
        pugi::xml_node head = doc.append_child();
        head.set_name("head");

        //Now add head elements.
        pugi::xml_node node;

        node = head.append_child();
        node.set_name("meta");
        node.append_attribute("http-equiv").set_value("X-UA-Compatible");
        node.append_attribute("content").set_value("IE=9");

        node = head.append_child();
        node.set_name("meta");
        node.append_attribute("charset").set_value("utf-8");

        node = head.append_child();
        node.set_name("title");
        node.text().set(boost::locale::translate("BOSS Report").str().c_str());

        node = head.append_child();
        node.set_name("link");
        node.append_attribute("rel").set_value("stylesheet");
        node.append_attribute("href").set_value(ToFileURL(g_path_css).c_str());

        node = head.append_child();
        node.set_name("script");
        node.append_attribute("src").set_value(ToFileURL(g_path_polyfill).c_str());
        node.text().set(" ");
    }

    inline void AppendNav(pugi::xml_node& body) {
        BOOST_LOG_TRIVIAL(trace) << "Appending navigation bar to BOSS report.";

        pugi::xml_node nav, div;

        nav = body.append_child();
        nav.set_name("div");
        nav.append_attribute("id").set_value("nav");

        div = nav.append_child();
        div.set_name("div");
        div.append_attribute("class").set_value("button current");
        div.append_attribute("data-section").set_value("summary");
        div.text().set(boost::locale::translate("Summary").str().c_str());

        div = nav.append_child();
        div.set_name("div");
        div.append_attribute("class").set_value("button");
        div.append_attribute("data-section").set_value("plugins");
        div.text().set(boost::locale::translate("Details").str().c_str());

        div = nav.append_child();
        div.set_name("div");
        div.append_attribute("class").set_value("button hidden");
        div.append_attribute("id").set_value("filtersToggle");
        div.text().set(boost::locale::translate("Filters").str().c_str());
    }

    inline void AppendMessages(pugi::xml_node& parent, const std::list<Message>& messages, int& warnNo, int& errorNo) {
        if (!messages.empty()) {
            pugi::xml_node list = parent.append_child();
            list.set_name("ul");

            for (std::list<Message>::const_iterator it=messages.begin(), endit=messages.end(); it != endit; ++it) {

                pugi::xml_node li = list.append_child();
                li.set_name("li");

                if (it->Type() == g_message_say)
                    li.append_attribute("class").set_value("say");
                else if (it->Type() == g_message_tag)
                    li.append_attribute("class").set_value("tag");
                else if (it->Type() == g_message_warn) {
                    li.append_attribute("class").set_value("warn");
                    ++warnNo;
                } else {
                    li.append_attribute("class").set_value("error");
                    ++errorNo;
                }

                //Turn any urls into hyperlinks.
                WriteMessage(li, it->Type(), it->Content().front().Str());
            }
        }
    }

    inline void AppendSummary(pugi::xml_node& main,
                        bool hasChanged,
                        const std::string& masterlistVersion,
                        bool masterlistUpdateEnabled,
                        int messageNo,
                        int warnNo,
                        int errorNo,
                        const std::list<Message>& messages) {

        BOOST_LOG_TRIVIAL(trace) << "Appending summary tab to BOSS report.";

        pugi::xml_node summary = main.append_child();
        summary.set_name("div");
        summary.append_attribute("id").set_value("summary");

        pugi::xml_node div = summary.append_child();
        div.set_name("div");

        pugi::xml_node heading = div.append_child();
        heading.set_name("h2");
        heading.text().set(boost::locale::translate("Summary").str().c_str());

        pugi::xml_node table = div.append_child();
        table.set_name("table");
        table = table.append_child();
        table.set_name("tbody");

        pugi::xml_node row, cell;

        row = table.append_child();
        row.set_name("tr");
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set(boost::locale::translate("BOSS Version").str().c_str());
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set((IntToString(g_version_major)+"."+IntToString(g_version_minor)+"."+IntToString(g_version_patch)).c_str());

        row = table.append_child();
        row.set_name("tr");
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set(boost::locale::translate("Masterlist Version").str().c_str());
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set(masterlistVersion.c_str());

        row = table.append_child();
        row.set_name("tr");
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set(boost::locale::translate("Masterlist Updating").str().c_str());
        cell = row.append_child();
        cell.set_name("td");
        if (masterlistUpdateEnabled)
            cell.text().set(boost::locale::translate("Enabled").str().c_str());
        else
            cell.text().set(boost::locale::translate("Disabled").str().c_str());

        if (!hasChanged) {
            BOOST_LOG_TRIVIAL(info) << "No changes in the BOSS report details tab since the last run.";
            pugi::xml_node note = summary.append_child();
            note.set_name("div");
            note.append_attribute("id").set_value("noChanges");
            note.text().set(boost::locale::translate("No change in details since last run.").str().c_str());
        }

        if (!messages.empty()) {

            div = summary.append_child();
            div.set_name("div");

            heading = div.append_child();
            heading.set_name("h2");
            heading.text().set(boost::locale::translate("General Messages").str().c_str());

            AppendMessages(div, messages, warnNo, errorNo);
            messageNo += messages.size();
        }

        row = table.append_child();
        row.set_name("tr");
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set(boost::locale::translate("Total Number of Messages").str().c_str());
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set(IntToString(messageNo).c_str());

        row = table.append_child();
        row.set_name("tr");
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set(boost::locale::translate("Number of Warnings").str().c_str());
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set(IntToString(warnNo).c_str());

        row = table.append_child();
        row.set_name("tr");
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set(boost::locale::translate("Number of Errors").str().c_str());
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set(IntToString(errorNo).c_str());
    }

    inline bool AppendDetails(pugi::xml_node& main, const std::list<Plugin>& plugins, int& messageNo, int& warnNo, int& errorNo, const std::string& oldDetails) {

        BOOST_LOG_TRIVIAL(trace) << "Appending details tab to BOSS report.";

        pugi::xml_node details = main.append_child();
        details.set_name("div");
        details.append_attribute("id").set_value("plugins");
        details.append_attribute("class").set_value("hidden");

        if (!plugins.empty()) {

            details = details.append_child();
            details.set_name("ul");

            for (std::list<Plugin>::const_iterator it=plugins.begin(), endit=plugins.end(); it != endit; ++it) {
                BOOST_LOG_TRIVIAL(trace) << "Appending details for plugin: " << it->Name();
                pugi::xml_node plugin = details.append_child();
                plugin.set_name("li");

                pugi::xml_node node = plugin.append_child();
                node.set_name("span");
                node.append_attribute("class").set_value("mod");
                node.text().set(it->Name().c_str());

                if (it->Crc() != 0) {
                    node = plugin.append_child();
                    node.set_name("div");
                    node.append_attribute("class").set_value("crc");
                    node.text().set(("CRC: " + IntToHexString(it->Crc())).c_str());
                }

                if (!it->Version().empty()) {
                    node = plugin.append_child();
                    node.set_name("div");
                    node.append_attribute("class").set_value("version");
                    node.text().set((boost::locale::translate("Version:").str() + " " + it->Version()).c_str());
                }

                std::list<Message> messages = it->Messages();
                messageNo += messages.size();

                std::set<Tag> tags = it->Tags();

                if (!tags.empty()) {
                    std::string add, remove, content;
                    for (std::set<Tag>::const_iterator jt=tags.begin(), endjt=tags.end(); jt != endjt; ++jt) {
                        if (jt->IsAddition())
                            add += ", " + jt->Name();
                        else
                            remove += ", " + jt->Name();
                    }
                    if (!add.empty()) {
                        node = plugin.append_child();
                        node.set_name("div");
                        node.append_attribute("class").set_value("tag add");
                        node.text().set((boost::locale::translate("Add:").str() + " " + add.substr(2)).c_str());
                    }
                    if (!remove.empty()) {
                        node = plugin.append_child();
                        node.set_name("div");
                        node.append_attribute("class").set_value("tag remove");
                        node.text().set((boost::locale::translate("Remove:").str() + " " + remove.substr(2)).c_str());
                    }
                }

                std::set<PluginDirtyInfo> dirtyInfo = it->DirtyInfo();
                for (std::set<PluginDirtyInfo>::const_iterator jt=dirtyInfo.begin(), endjt=dirtyInfo.end(); jt != endjt; ++jt) {
                    boost::format f;
                    if (jt->ITMs() > 0 && jt->UDRs() > 0 && jt->DeletedNavmeshes() > 0)
                        f = boost::format(boost::locale::translate("Contains %1% ITM records, %2% UDR records and %3% deleted navmeshes. Clean with %4%.")) % jt->ITMs() % jt->UDRs() % jt->DeletedNavmeshes() % jt->CleaningUtility();
                    else if (jt->ITMs() == 0 && jt->UDRs() == 0 && jt->DeletedNavmeshes() == 0)
                        f = boost::format(boost::locale::translate("Clean with %1%.")) % jt->CleaningUtility();


                    else if (jt->ITMs() == 0 && jt->UDRs() > 0 && jt->DeletedNavmeshes() > 0)
                        f = boost::format(boost::locale::translate("Contains %1% UDR records and %2% deleted navmeshes. Clean with %3%.")) % jt->UDRs() % jt->DeletedNavmeshes() % jt->CleaningUtility();
                    else if (jt->ITMs() == 0 && jt->UDRs() == 0 && jt->DeletedNavmeshes() > 0)
                        f = boost::format(boost::locale::translate("Contains %1% deleted navmeshes. Clean with %2%.")) % jt->DeletedNavmeshes() % jt->CleaningUtility();
                    else if (jt->ITMs() == 0 && jt->UDRs() > 0 && jt->DeletedNavmeshes() == 0)
                        f = boost::format(boost::locale::translate("Contains %1% UDR records. Clean with %2%.")) % jt->UDRs() % jt->CleaningUtility();

                    else if (jt->ITMs() > 0 && jt->UDRs() == 0 && jt->DeletedNavmeshes() > 0)
                        f = boost::format(boost::locale::translate("Contains %1% ITM records and %2% deleted navmeshes. Clean with %3%.")) % jt->ITMs() % jt->DeletedNavmeshes() % jt->CleaningUtility();
                    else if (jt->ITMs() > 0 && jt->UDRs() == 0 && jt->DeletedNavmeshes() == 0)
                        f = boost::format(boost::locale::translate("Contains %1% ITM records. Clean with %2%.")) % jt->ITMs() % jt->CleaningUtility();

                    else if (jt->ITMs() > 0 && jt->UDRs() > 0 && jt->DeletedNavmeshes() == 0)
                        f = boost::format(boost::locale::translate("Contains %1% ITM records and %2% UDR records. Clean with %3%.")) % jt->ITMs() % jt->UDRs() % jt->CleaningUtility();

                    messages.push_back(boss::Message(boss::g_message_warn, f.str()));
                }

                AppendMessages(plugin, messages, warnNo, errorNo);
            }
        }

        xml_string_writer writer;
        details.print(writer, "\t", pugi::format_default | pugi::format_no_declaration);

        return writer.result != oldDetails;
    }

    inline void AppendMain(pugi::xml_node& body,
                        const std::string& oldDetails,
                        const std::string& masterlistVersion,
                        bool masterlistUpdateEnabled,
                        const std::list<Message>& messages,
                        const std::list<Plugin>& plugins,
                        int& pluginMessageNo
                        ) {

        BOOST_LOG_TRIVIAL(trace) << "Appending main content to BOSS report.";

        pugi::xml_node main = body.append_child();
        main.set_name("div");
        main.append_attribute("id").set_value("main");

        pugi::xml_node noscript, div;

        noscript = main.append_child();
        noscript.set_name("noscript");
        div = noscript.append_child();
        div.set_name("div");
        div.text().set(boost::locale::translate("The BOSS Report requires Javascript to be enabled in order to function.").str().c_str());

        int messageNo=0, warnNo=0, errorNo=0;
        bool hasChanged = AppendDetails(main, plugins, messageNo, warnNo, errorNo, oldDetails);
        pluginMessageNo = messageNo;

        AppendSummary(main, hasChanged, masterlistVersion, masterlistUpdateEnabled, messageNo, warnNo, errorNo, messages);
    }

    inline void AppendFilters(pugi::xml_node& body, int messageNo, int pluginNo) {

        BOOST_LOG_TRIVIAL(trace) << "Appending filters to BOSS report.";

        pugi::xml_node filters = body.append_child();
        filters.set_name("div");
        filters.append_attribute("id").set_value("filters");
        filters.append_attribute("class").set_value("hidden");

        pugi::xml_node label, input;

        label = filters.append_child();
        label.set_name("label");
        input = label.append_child();
        input.set_name("input");
        input.append_attribute("type").set_value("checkbox");
        input.append_attribute("id").set_value("hideVersionNumbers");
        input.append_attribute("data-class").set_value("version");
        input.text().set(boost::locale::translate("Hide Version Numbers").str().c_str());

        label = filters.append_child();
        label.set_name("label");
        input = label.append_child();
        input.set_name("input");
        input.append_attribute("type").set_value("checkbox");
        input.append_attribute("id").set_value("hideCRCs");
        input.append_attribute("data-class").set_value("crc");
        input.text().set(boost::locale::translate("Hide CRCs").str().c_str());

        label = filters.append_child();
        label.set_name("label");
        input = label.append_child();
        input.set_name("input");
        input.append_attribute("type").set_value("checkbox");
        input.append_attribute("id").set_value("hideBashTags");
        input.append_attribute("data-class").set_value("tag");
        input.text().set(boost::locale::translate("Hide Bash Tag Suggestions").str().c_str());

        label = filters.append_child();
        label.set_name("label");
        input = label.append_child();
        input.set_name("input");
        input.append_attribute("type").set_value("checkbox");
        input.append_attribute("id").set_value("hideNotes");
        input.text().set(boost::locale::translate("Hide Notes").str().c_str());

        label = filters.append_child();
        label.set_name("label");
        input = label.append_child();
        input.set_name("input");
        input.append_attribute("type").set_value("checkbox");
        input.append_attribute("id").set_value("hideDoNotCleanMessages");
        input.text().set(boost::locale::translate("Hide 'Do Not Clean' Messages").str().c_str());

        label = filters.append_child();
        label.set_name("label");
        input = label.append_child();
        input.set_name("input");
        input.append_attribute("type").set_value("checkbox");
        input.append_attribute("id").set_value("hideAllPluginMessages");
        input.text().set(boost::locale::translate("Hide All Plugin Messages").str().c_str());

        label = filters.append_child();
        label.set_name("label");
        input = label.append_child();
        input.set_name("input");
        input.append_attribute("type").set_value("checkbox");
        input.append_attribute("id").set_value("hideMessagelessPlugins");
        input.text().set(boost::locale::translate("Hide Messageless Plugins").str().c_str());

        pugi::xml_node span;

        label = filters.append_child();
        label.set_name("div");
        span = label.append_child();
        span.set_name("span");
        span.append_attribute("id").set_value("hiddenPluginNo");
        span.text().set("0");
        label.append_child(pugi::node_pcdata).set_value((boost::format(boost::locale::translate("%1% of %2% plugins hidden.")) % "" % IntToString(pluginNo)).str().c_str());  //Blank string is so that translators get full string format.

        label = filters.append_child();
        label.set_name("div");
        span = label.append_child();
        span.set_name("span");
        span.append_attribute("id").set_value("hiddenMessageNo");
        span.text().set("0");
        label.append_child(pugi::node_pcdata).set_value((boost::format(boost::locale::translate("%1% of %2% messages hidden.")) % "" % IntToString(messageNo)).str().c_str());  //Blank string is so that translators get full string format.
    }

    inline void AppendScripts(pugi::xml_node& body) {

        BOOST_LOG_TRIVIAL(trace) << "Appending scripts to BOSS report.";

        pugi::xml_node node;

        node = body.append_child();
        node.set_name("script");
        node.append_attribute("src").set_value(ToFileURL(g_path_js).c_str());
        node.text().set(" ");

    }

    inline void GenerateReport(const boost::filesystem::path& file,
                        const std::list<Message>& messages,
                        const std::list<Plugin>& plugins,
                        const std::string& oldDetails,
                        const std::string& masterlistVersion,
                        const bool masterlistUpdateEnabled) {

        pugi::xml_document doc;

        GenerateHead(doc);

        pugi::xml_node body = doc.append_child();
        body.set_name("body");

        AppendNav(body);

        int messageNo=0;
        AppendMain(body, oldDetails, masterlistVersion, masterlistUpdateEnabled, messages, plugins, messageNo);

        AppendFilters(body, messageNo, plugins.size());

        AppendScripts(body);
        //PugiXML's save_file doesn't handle Unicode paths right (it can't open them right), so use a stream instead.
        boost::filesystem::path outpath(file);
        boss::ofstream out(outpath);
        doc.save(out, "\t", pugi::format_default | pugi::format_no_declaration | pugi::format_raw);
        out.close();

    }

    //Default settings file generation.

    inline void GenerateDefaultSettingsFile(const std::string& file) {

        BOOST_LOG_TRIVIAL(info) << "Generating default settings file.";

        YAML::Node root;
        std::vector<Game> games;

        root["Language"] = "eng";
        root["Game"] = "auto";
        root["Last Game"] = "auto";
        root["Debug Verbosity"] = 0;
        root["Update Masterlist"] = true;
        root["View Report Externally"] = false;

        games.push_back(Game(g_game_tes4));
        games.push_back(Game(g_game_tes5));
        games.push_back(Game(g_game_fo3));
        games.push_back(Game(g_game_fonv));
        games.push_back(Game(g_game_tes4, "Nehrim").SetDetails("Nehrim - At Fate's Edge", "Nehrim.esm", "https://github.com/boss-developers/boss-oblivion.git", "", "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1"));

        root["Games"] = games;

        //Save settings.
        YAML::Emitter yout;
        yout.SetIndent(2);
        yout << root;

        boost::filesystem::path p(file);
        boss::ofstream out(p);
        out << yout.c_str();
        out.close();
    }
}

namespace YAML {
    template<class T, class Compare>
    Emitter& operator << (Emitter& out, const std::set<T, Compare>& rhs) {
        out << BeginSeq;
        for (typename std::set<T, Compare>::const_iterator it=rhs.begin(), endIt=rhs.end(); it != endIt; ++it) {
            out << *it;
        }
		out << EndSeq;

		return out;
    }

    inline Emitter& operator << (Emitter& out, const boss::PluginDirtyInfo& rhs) {
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

    inline Emitter& operator << (Emitter& out, const boss::Game& rhs) {
        out << BeginMap
            << Key << "folder" << Value << rhs.FolderName()
            << Key << "name" << Value << rhs.Name()
            << Key << "master" << Value << rhs.Master()
            << Key << "url" << Value << rhs.URL()
            << Key << "path" << Value << rhs.GamePath().string()
            << Key << "registry" << Value << rhs.RegistryKey();

        if (rhs.Id() == boss::g_game_tes4)
            out << Key << Value << boss::Game(boss::g_game_tes4).FolderName();
        else if (rhs.Id() == boss::g_game_tes5)
            out << Key << Value << boss::Game(boss::g_game_tes5).FolderName();
        else if (rhs.Id() == boss::g_game_fo3)
            out << Key << Value << boss::Game(boss::g_game_fo3).FolderName();
        else if (rhs.Id() == boss::g_game_fonv)
            out << Key << Value << boss::Game(boss::g_game_fonv).FolderName();

		out << EndMap;

		return out;
    }

    inline Emitter& operator << (Emitter& out, const boss::MessageContent& rhs) {
        out << BeginMap;

        out << Key << "lang" << Value << boss::GetLangString(rhs.Language());

        out << Key << "str" << Value << rhs.Str();

		out << EndMap;

		return out;
    }

    inline Emitter& operator << (Emitter& out, const boss::Message& rhs) {
        out << BeginMap;

        if (rhs.Type() == boss::g_message_say)
            out << Key << "type" << Value << "say";
        else if (rhs.Type() == boss::g_message_warn)
            out << Key << "type" << Value << "warn";
        else
            out << Key << "type" << Value << "error";

        if (rhs.Content().size() == 1 && rhs.Content().front().Language() == boss::g_lang_any)
            out << Key << "content" << Value << rhs.Content().front().Str();
        else
            out << Key << "content" << Value << rhs.Content();

        if (!rhs.Condition().empty())
            out << Key << "condition" << Value << rhs.Condition();

		out << EndMap;

		return out;
    }

    inline Emitter& operator << (Emitter& out, const boss::File& rhs) {
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

    inline Emitter& operator << (Emitter& out, const boss::Tag& rhs) {
        if (!rhs.IsConditional()) {
            if (rhs.IsAddition())
                out << rhs.Name();
            else
                out << '-' << rhs.Name();
        } else {
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

    inline Emitter& operator << (Emitter& out, const boss::Plugin& rhs) {
        if (!rhs.HasNameOnly()) {

            out << BeginMap
                << Key << "name" << Value << rhs.Name();

            if (rhs.Priority() != 0)
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

#endif
