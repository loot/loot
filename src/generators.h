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

#include <pugixml.hpp>

#include <stdexcept>
#include <string>
#include <list>

namespace boss {

    void WriteMessage(pugi::xml_node& listItem, std::string content) {

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

    void GenerateHead(pugi::xml_document& doc) {

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
        node.append_attribute("href").set_value("../../examples/style.css");
    }

    void AppendNav(pugi::xml_node& body) {
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

    void AppendSummary(pugi::xml_node& main,
                        bool hasChanged,
                        const std::string& masterlistVersion,
                        bool masterlistUpdateEnabled,
                        int messageNo,
                        int warnNo,
                        int errorNo,
                        const std::list<Message>& messages,
                        const std::string& language) {

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

        if (hasChanged) {
            pugi::xml_node note = summary.append_child();
            note.set_name("div");
            note.append_attribute("id").set_value("noChanges");
            note.text().set("No changes since last run.");
        }

        if (!messages.empty()) {
            pugi::xml_node list = summary.append_child();
            list.set_name("ul");

            for (std::list<Message>::const_iterator it=messages.begin(), endit=messages.end(); it != endit; ++it) {
                //Need to weed out different language messages.
                if (it->Language().empty() || boost::iequals(it->Language(), language)) {

                    pugi::xml_node li = list.append_child();
                    li.set_name("li");
                    li.append_attribute("class").set_value(it->Type().c_str());

                    //Turn any urls into hyperlinks.
                    WriteMessage(li, it->Content());
                    
                }
            }
        }
    }

    void AppendMain(pugi::xml_node& body,
                        bool hasChanged,
                        const std::string& masterlistVersion,
                        bool masterlistUpdateEnabled,
                        int messageNo,
                        int warnNo,
                        int errorNo,
                        const std::list<Message>& messages,
                        const std::string& language) {

        pugi::xml_node main = body.append_child();
        main.set_name("div");
        main.append_attribute("id").set_value("main");

        pugi::xml_node noscript, div;

        noscript = main.append_child();
        noscript.set_name("noscript");
        div = noscript.append_child();
        div.set_name("div");
        div.text().set("The BOSS Report requires Javascript to be enabled in order to function.");

        AppendSummary(main, hasChanged, masterlistVersion, masterlistUpdateEnabled, messageNo, warnNo, errorNo, messages, language);
    }

    void AppendFilters(pugi::xml_node& body, int messageNo, int pluginNo) {

        pugi::xml_node filters = body.append_child();
        filters.set_name("div");
        filters.append_attribute("id").set_value("filters");

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
        input.append_attribute("id").set_value("hideActiveLabel");
        input.append_attribute("data-class").set_value("active");
        input.text().set("Hide 'Active' Label");

        label = filters.append_child();
        label.set_name("label");
        input = label.append_child();
        input.set_name("input");
        input.append_attribute("type").set_value("checkbox");
        input.append_attribute("id").set_value("hideChecksums");
        input.append_attribute("data-class").set_value("crc");
        input.text().set("Hide Checksums");

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
        input.append_attribute("id").set_value("hideInactivePlugins");
        input.text().set("Hide Inactive Plugins");

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

    void AppendScripts(pugi::xml_node& body) {

        pugi::xml_node node;

        node = body.append_child();
        node.set_name("script");
        node.append_attribute("src").set_value("../../examples/eventShim.js");
        node.text().set(" ");

        node = body.append_child();
        node.set_name("script");
        node.append_attribute("src").set_value("../../examples/script.js");
        node.text().set(" ");
        
    }

    void GenerateReport(const std::string& file) {

        pugi::xml_document doc;

        GenerateHead(doc);

        pugi::xml_node body = doc.append_child();
        body.set_name("body");

        AppendNav(body);

        std::list<Message> messages;
        messages.push_back(Message("say", "This is a test message with a link to \"http://www.google.com Google\" in it."));

        AppendMain(body, true, "4308 (2013-06-06)", true, 500, 4, 2, messages, "eng");
        
        AppendFilters(body, 5, 5);
        
        AppendScripts(body);
        

        if (!doc.save_file(file.c_str(), "\t", pugi::format_default | pugi::format_no_declaration))
            throw std::runtime_error("Could not write XML file.");

    }
        
}

#endif
