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

#include <pugixml.hpp>
#include <yaml-cpp/yaml.h>

#include <string>
#include <list>
#include <boost/algorithm/string.hpp>

namespace boss {

    //BOSS Report generation stuff.

    struct xml_string_writer: pugi::xml_writer {
        std::string result;

        virtual void write(const void* data, size_t size) {
            result += std::string(static_cast<const char*>(data), size);
        }
    };

    inline void WriteMessage(pugi::xml_node& listItem, unsigned int type, std::string content) {

        if (type == MESSAGE_SAY)
            content = "Note: " + content;
        else if (type == MESSAGE_TAG) {
            content = "Bash Tag Suggestion(s): " + content;
        } else if (type == MESSAGE_WARN)
            content = "Warning: " + content;
        else
            content = "Error: " + content;

        size_t pos1f = content.find("\"file:");
        size_t pos1h = content.find("\"http");
        size_t pos1;
        if (pos1f < pos1h)
            pos1 = pos1f;
        else
            pos1 = pos1h;
        size_t pos2 = content.find(' ', pos1+1);
        size_t pos3 = content.find('"', pos1+1);

        while (pos1 != std::string::npos && pos2 != std::string::npos && pos3 != std::string::npos) {
            
            std::string url = content.substr(pos1+1, pos2-pos1-1);
            std::string display = content.substr(pos2+1, pos3-pos2-1);

            //Insert normal text preceding hyperlink.
            listItem.append_child(pugi::node_pcdata).set_value(content.substr(0, pos1).c_str());

            //Insert hyperlink.
            pugi::xml_node a = listItem.append_child();
            a.set_name("a");
            a.append_attribute("href").set_value(url.c_str());
            a.text().set(display.c_str());

            //Remove all the processed text from the string.
            content = content.substr(pos3+1);

            //Look for the next hyperlink.
            pos1f = content.find("\"file:");
            pos1h = content.find("\"http");
            if (pos1f < pos1h)
                pos1 = pos1f;
            else
                pos1 = pos1h;
            pos2 = content.find(' ', pos1+1);
            pos3 = content.find('"', pos1+1);
        }

        //Insert any leftover text.
        listItem.append_child(pugi::node_pcdata).set_value(content.c_str());

    }

    inline void GenerateHead(pugi::xml_document& doc) {

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
        node.append_attribute("content").set_value("IE=8");

        node = head.append_child();
        node.set_name("meta");
        node.append_attribute("charset").set_value("utf-8");

        node = head.append_child();
        node.set_name("title");
        node.text().set("BOSS Report");

        node = head.append_child();
        node.set_name("link");
        node.append_attribute("rel").set_value("stylesheet");
        node.append_attribute("href").set_value(ToFileURL(css_path).c_str());

        node = head.append_child();
        node.set_name("script");
        node.append_attribute("src").set_value(ToFileURL(polyfill_path).c_str());
        node.text().set(" ");
    }

    inline void AppendNav(pugi::xml_node& body) {
        pugi::xml_node nav, div;

        nav = body.append_child();
        nav.set_name("div");
        nav.append_attribute("id").set_value("nav");

        div = nav.append_child();
        div.set_name("div");
        div.append_attribute("class").set_value("button current");
        div.append_attribute("data-section").set_value("summary");
        div.text().set("Summary");

        div = nav.append_child();
        div.set_name("div");
        div.append_attribute("class").set_value("button");
        div.append_attribute("data-section").set_value("plugins");
        div.text().set("Details");

        div = nav.append_child();
        div.set_name("div");
        div.append_attribute("class").set_value("button");
        div.append_attribute("id").set_value("filtersToggle");
        div.text().set("Filters");
    }

    inline void AppendMessages(pugi::xml_node& parent, const std::list<Message>& messages, int& warnNo, int& errorNo) {
        if (!messages.empty()) {
            pugi::xml_node list = parent.append_child();
            list.set_name("ul");

            for (std::list<Message>::const_iterator it=messages.begin(), endit=messages.end(); it != endit; ++it) {

                pugi::xml_node li = list.append_child();
                li.set_name("li");

                if (it->Type() == MESSAGE_SAY)
                    li.append_attribute("class").set_value("say");
                if (it->Type() == MESSAGE_TAG)
                    li.append_attribute("class").set_value("tag");
                else if (it->Type() == MESSAGE_WARN) {
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

        pugi::xml_node summary = main.append_child();
        summary.set_name("div");
        summary.append_attribute("id").set_value("summary");

        pugi::xml_node table = summary.append_child();
        table.set_name("table");
        table = table.append_child();
        table.set_name("tbody");

        pugi::xml_node row, cell;

        row = table.append_child();
        row.set_name("tr");
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set("BOSS Version");
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set((IntToString(VERSION_MAJOR)+"."+IntToString(VERSION_MINOR)+"."+IntToString(VERSION_PATCH)).c_str());

        row = table.append_child();
        row.set_name("tr");
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set("Masterlist Version");
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set(masterlistVersion.c_str());

        row = table.append_child();
        row.set_name("tr");
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set("Masterlist Updating");
        cell = row.append_child();
        cell.set_name("td");
        if (masterlistUpdateEnabled)
            cell.text().set("Enabled");
        else
            cell.text().set("Disabled");

        if (!hasChanged) {
            pugi::xml_node note = summary.append_child();
            note.set_name("div");
            note.append_attribute("id").set_value("noChanges");
            note.text().set("No change in details since last run.");
        }

        AppendMessages(summary, messages, warnNo, errorNo);
        messageNo += messages.size();

        row = table.append_child();
        row.set_name("tr");
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set("Total Number of Messages");
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set(IntToString(messageNo).c_str());

        row = table.append_child();
        row.set_name("tr");
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set("Number of Warnings");
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set(IntToString(warnNo).c_str());

        row = table.append_child();
        row.set_name("tr");
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set("Number of Errors");
        cell = row.append_child();
        cell.set_name("td");
        cell.text().set(IntToString(errorNo).c_str());
    }

    inline bool AppendDetails(pugi::xml_node& main, const std::list<Plugin>& plugins, int& messageNo, int& warnNo, int& errorNo, const std::string& oldDetails) {

        pugi::xml_node details = main.append_child();
        details.set_name("div");
        details.append_attribute("id").set_value("plugins");
        details.append_attribute("class").set_value("hidden");

        if (!plugins.empty()) {

            details = details.append_child();
            details.set_name("ul");

            for (std::list<Plugin>::const_iterator it=plugins.begin(), endit=plugins.end(); it != endit; ++it) {
                pugi::xml_node plugin = details.append_child();
                plugin.set_name("li");

                pugi::xml_node node = plugin.append_child();
                node.set_name("span");
                node.append_attribute("class").set_value("mod");
                node.text().set(it->Name().c_str());

                if (!it->Version().empty()) {
                    node = plugin.append_child();
                    node.set_name("span");
                    node.append_attribute("class").set_value("version");
                    node.text().set(("Version: " + it->Version()).c_str());
                }

                std::list<Message> messages = it->Messages();

                std::set<Tag> tags = it->Tags();

                if (!tags.empty()) {
                    std::string add, remove, content;
                    for (std::set<Tag>::const_iterator jt=tags.begin(), endjt=tags.end(); jt != endjt; ++jt) {
                        if (jt->IsAddition())
                            add += ", " + jt->Name();
                        else
                            remove += ", " + jt->Name();
                    }
                    if (!add.empty())
                        content += "Add " + add.substr(2) + ". ";
                    if (!remove.empty())
                        content += "Remove " + remove.substr(2) + ". ";
                    messages.push_back(Message(MESSAGE_TAG, content));  //Special type just for tag suggestions.
                }

                AppendMessages(plugin, messages, warnNo, errorNo);
                messageNo += messages.size();
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

        pugi::xml_node main = body.append_child();
        main.set_name("div");
        main.append_attribute("id").set_value("main");

        pugi::xml_node noscript, div;

        noscript = main.append_child();
        noscript.set_name("noscript");
        div = noscript.append_child();
        div.set_name("div");
        div.text().set("The BOSS Report requires Javascript to be enabled in order to function.");

        int messageNo=0, warnNo=0, errorNo=0;
        bool hasChanged = AppendDetails(main, plugins, messageNo, warnNo, errorNo, oldDetails);
        pluginMessageNo = messageNo;

        AppendSummary(main, hasChanged, masterlistVersion, masterlistUpdateEnabled, messageNo, warnNo, errorNo, messages);
    }

    inline void AppendFilters(pugi::xml_node& body, int messageNo, int pluginNo) {

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
        input.text().set("Hide Version Numbers");

        label = filters.append_child();
        label.set_name("label");
        input = label.append_child();
        input.set_name("input");
        input.append_attribute("type").set_value("checkbox");
        input.append_attribute("id").set_value("hideNotes");
        input.text().set("Hide Notes");

        label = filters.append_child();
        label.set_name("label");
        input = label.append_child();
        input.set_name("input");
        input.append_attribute("type").set_value("checkbox");
        input.append_attribute("id").set_value("hideBashTags");
        input.text().set("Hide Bash Tag Suggestions");

        label = filters.append_child();
        label.set_name("label");
        input = label.append_child();
        input.set_name("input");
        input.append_attribute("type").set_value("checkbox");
        input.append_attribute("id").set_value("hideDoNotCleanMessages");
        input.text().set("Hide 'Do Not Clean' Messages");

        label = filters.append_child();
        label.set_name("label");
        input = label.append_child();
        input.set_name("input");
        input.append_attribute("type").set_value("checkbox");
        input.append_attribute("id").set_value("hideAllPluginMessages");
        input.text().set("Hide All Plugin Messages");

        label = filters.append_child();
        label.set_name("label");
        input = label.append_child();
        input.set_name("input");
        input.append_attribute("type").set_value("checkbox");
        input.append_attribute("id").set_value("hideMessagelessPlugins");
        input.text().set("Hide Messageless Plugins");

        pugi::xml_node span;

        label = filters.append_child();
        label.set_name("div");
        span = label.append_child();
        span.set_name("span");
        span.append_attribute("id").set_value("hiddenPluginNo");
        span.text().set("0");
        label.append_child(pugi::node_pcdata).set_value((" of " + IntToString(pluginNo) + " plugins hidden.").c_str());

        label = filters.append_child();
        label.set_name("div");
        span = label.append_child();
        span.set_name("span");
        span.append_attribute("id").set_value("hiddenMessageNo");
        span.text().set("0");
        label.append_child(pugi::node_pcdata).set_value((" of " + IntToString(messageNo) + " messages hidden.").c_str());
    }

    inline void AppendScripts(pugi::xml_node& body) {

        pugi::xml_node node;

        node = body.append_child();
        node.set_name("script");
        node.append_attribute("src").set_value(ToFileURL(js_path).c_str());
        node.text().set(" ");
        
    }

    inline void GenerateReport(const std::string& file,
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
        
        if (!doc.save_file(file.c_str(), "\t", pugi::format_default | pugi::format_no_declaration))
            throw boss::error(ERROR_PATH_WRITE_FAIL, "Could not write BOSS report.");

    }

    //Default settings file generation.

    inline void GenerateDefaultSettingsFile(const std::string& file) {

        YAML::Node root;
        std::vector<Game> games;

        root["Language"] = "eng";
        root["Game"] = "auto";
        root["Last Game"] = "auto";
        root["Debug Verbosity"] = 0;
        root["Update Masterlist"] = true;
        root["View Report Externally"] = false;

        games.push_back(Game(GAME_TES4));
        games.push_back(Game(GAME_TES5));
        games.push_back(Game(GAME_FO3));
        games.push_back(Game(GAME_FONV));
        games.push_back(Game(GAME_TES4, "Nehrim").SetDetails("Nehrim - At Fate's Edge", "Nehrim.esm", "http://better-oblivion-sorting-software.googlecode.com/svn/data/boss-nehrim/masterlist.yaml", "", "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1"));

        root["Games"] = games;

        //Save settings.
        YAML::Emitter yout;
        yout.SetIndent(2);
        yout << root;
        
        std::ofstream out(file.c_str());
        out << yout.c_str();
        out.close();
    }
}

#endif

namespace YAML {
    template<class T, class Compare>
    Emitter& operator << (Emitter& out, const std::set<T, Compare>& rhs) {
        out << BeginSeq;
        for (typename std::set<T, Compare>::const_iterator it=rhs.begin(), endIt=rhs.end(); it != endIt; ++it) {
            out << *it;
        }
        out << EndSeq;
    }

    inline Emitter& operator << (Emitter& out, const boss::Game& rhs) {
        out << BeginMap
            << Key << "folder" << Value << rhs.FolderName()
            << Key << "name" << Value << rhs.Name()
            << Key << "master" << Value << rhs.Master()
            << Key << "url" << Value << rhs.URL()
            << Key << "path" << Value << rhs.GamePath().string()
            << Key << "registry" << Value << rhs.RegistryKey();

        if (rhs.Id() == boss::GAME_TES4)
            out << Key << Value << boss::Game(boss::GAME_TES4).FolderName();
        else if (rhs.Id() == boss::GAME_TES5)
            out << Key << Value << boss::Game(boss::GAME_TES5).FolderName();
        else if (rhs.Id() == boss::GAME_FO3)
            out << Key << Value << boss::Game(boss::GAME_FO3).FolderName();
        else if (rhs.Id() == boss::GAME_FONV)
            out << Key << Value << boss::Game(boss::GAME_FONV).FolderName();

        out << EndMap;
    }

    inline Emitter& operator << (Emitter& out, const boss::MessageContent& rhs) {
        out << BeginMap;

        if (rhs.Language() == boss::LANG_ENG)
            out << Key << "lang" << Value << "eng";

        out << Key << "str" << Value << rhs.Str();

        out << EndMap;
    }

    inline Emitter& operator << (Emitter& out, const boss::Message& rhs) {
        out << BeginMap;

        if (rhs.Type() == boss::MESSAGE_SAY)
            out << Key << "type" << Value << "say";
        else if (rhs.Type() == boss::MESSAGE_WARN)
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
    }

    inline Emitter& operator << (Emitter& out, const boss::File& rhs) {
        if (!rhs.IsConditional() && rhs.DisplayName().empty())
            out << rhs.Name();
        else {
            out << BeginMap
                << Key << "name" << Value << rhs.Name();

            if (rhs.IsConditional())
                out << Key << "condition" << Value << rhs.Condition();

            if (!rhs.DisplayName().empty())
                out << Key << "display" << Value << rhs.DisplayName();

            out << EndMap;
        }
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
                out << Key << "name" << Value << '-' << rhs.Name();
            
            out << Key << "condition" << Value << rhs.Condition()
                << EndMap;
        }
    }

    inline Emitter& operator << (Emitter& out, const boss::Plugin& rhs) {
        //if (!rhs.HasNameOnly()) {

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

            out << EndMap;
        //}
    }
}
