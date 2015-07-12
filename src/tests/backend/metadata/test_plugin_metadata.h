/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2015    WrinklyNinja

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

#ifndef LOOT_TEST_BACKEND_METADATA_PLUGIN_METADATA
#define LOOT_TEST_BACKEND_METADATA_PLUGIN_METADATA

#include "backend/metadata/plugin_metadata.h"
#include "tests/fixtures.h"

using loot::PluginMetadata;

TEST(PluginMetadata, Constructors) {
    PluginMetadata pm;
    EXPECT_TRUE(pm.Enabled());
    EXPECT_FALSE(pm.IsPriorityExplicit());
    EXPECT_EQ(0, pm.Priority());

    pm = PluginMetadata("Blank.esm");
    EXPECT_EQ("Blank.esm", pm.Name());
    EXPECT_TRUE(pm.Enabled());
    EXPECT_FALSE(pm.IsPriorityExplicit());
    EXPECT_EQ(0, pm.Priority());
}

TEST(PluginMetadata, EqualityOperator) {
    PluginMetadata pm1, pm2;
    EXPECT_TRUE(pm1 == pm2);

    pm1 = PluginMetadata("Blank.esm");
    pm2 = PluginMetadata("blank.esm");
    EXPECT_TRUE(pm1 == pm2);

    pm1 = PluginMetadata("Blank.esm");
    pm2 = PluginMetadata("Blan.\\.esm");
    EXPECT_TRUE(pm1 == pm2);
    EXPECT_TRUE(pm2 == pm1);
}

TEST(PluginMetadata, MergeMetadata) {
    FAIL() << "Test is unimplemented";
}

TEST(PluginMetadata, DiffMetadata) {
    FAIL() << "Test is unimplemented";
}

TEST(PluginMetadata, NewMetadata) {
    FAIL() << "Test is unimplemented";
}

TEST(PluginMetadata, Name) {
    PluginMetadata pm;
    ASSERT_NE("Blank.esm", pm.Name());
    pm.Name("Blank.esm");
    EXPECT_EQ("Blank.esm", pm.Name());
}

TEST(PluginMetadata, Enabled) {
    PluginMetadata pm;
    ASSERT_TRUE(pm.Enabled());
    pm.Enabled(false);
    EXPECT_FALSE(pm.Enabled());
}

TEST(PluginMetadata, SetPriorityExplicit) {
    PluginMetadata pm;
    ASSERT_FALSE(pm.IsPriorityExplicit());
    pm.SetPriorityExplicit(true);
    EXPECT_TRUE(pm.IsPriorityExplicit());
}

TEST(PluginMetadata, Priority) {
    PluginMetadata pm;
    ASSERT_NE(10, pm.Priority());
    pm.Priority(10);
    EXPECT_EQ(10, pm.Priority());
}

TEST(PluginMetadata, LoadAfter) {
    PluginMetadata pm;
    ASSERT_TRUE(pm.LoadAfter().empty());
    pm.LoadAfter({
        loot::File("Blank.esm")
    });
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esm")
    }), pm.LoadAfter());
}

TEST(PluginMetadata, Reqs) {
    PluginMetadata pm;
    ASSERT_TRUE(pm.Reqs().empty());
    pm.Reqs({
        loot::File("Blank.esm")
    });
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esm")
    }), pm.Reqs());
}

TEST(PluginMetadata, Incs) {
    PluginMetadata pm;
    ASSERT_TRUE(pm.Incs().empty());
    pm.Incs({
        loot::File("Blank.esm")
    });
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esm")
    }), pm.Incs());
}

TEST(PluginMetadata, Messages) {
    PluginMetadata pm;
    ASSERT_TRUE(pm.Messages().empty());
    pm.Messages({
        loot::Message(loot::Message::say, "content")
    });
    EXPECT_EQ(std::list<loot::Message>({
        loot::Message(loot::Message::say, "content")
    }), pm.Messages());
}

TEST(PluginMetadata, Tags) {
    PluginMetadata pm;
    ASSERT_TRUE(pm.Tags().empty());
    pm.Tags({
        loot::Tag("Relev")
    });
    EXPECT_EQ(std::set<loot::Tag>({
        loot::Tag("Relev")
    }), pm.Tags());
}

TEST(PluginMetadata, DirtyInfo) {
    PluginMetadata pm;
    ASSERT_TRUE(pm.DirtyInfo().empty());
    pm.DirtyInfo({
        loot::PluginDirtyInfo(5, 0, 1, 2, "utility")
    });
    EXPECT_EQ(std::set<loot::PluginDirtyInfo>({
        loot::PluginDirtyInfo(5, 0, 1, 2, "utility")
    }), pm.DirtyInfo());
}

TEST(PluginMetadata, Locations) {
    PluginMetadata pm;
    ASSERT_TRUE(pm.Locations().empty());
    pm.Locations({
        loot::Location("http://www.example.com")
    });
    EXPECT_EQ(std::set<loot::Location>({
        loot::Location("http://www.example.com")
    }), pm.Locations());
}

TEST(PluginMetadata, EvalAllConditions) {
    FAIL() << "Test is unimplemented";
}

TEST(PluginMetadata, ParseAllConditions) {
    FAIL() << "Test is unimplemented";
}

TEST(PluginMetadata, HasNameOnly) {
    PluginMetadata pm;
    EXPECT_TRUE(pm.HasNameOnly());

    pm = PluginMetadata("Blank.esp");
    EXPECT_TRUE(pm.HasNameOnly());

    pm = PluginMetadata("Blank.esp");
    pm.Enabled(false);
    EXPECT_TRUE(pm.HasNameOnly());

    pm = PluginMetadata("Blank.esp");
    pm.SetPriorityExplicit(true);
    EXPECT_FALSE(pm.HasNameOnly());

    pm = PluginMetadata("Blank.esp");
    pm.LoadAfter({loot::File("Blank.esm")});
    EXPECT_FALSE(pm.HasNameOnly());

    pm = PluginMetadata("Blank.esp");
    pm.Reqs({loot::File("Blank.esm")});
    EXPECT_FALSE(pm.HasNameOnly());

    pm = PluginMetadata("Blank.esp");
    pm.Incs({loot::File("Blank.esm")});
    EXPECT_FALSE(pm.HasNameOnly());

    pm = PluginMetadata("Blank.esp");
    pm.Messages({loot::Message(loot::Message::say, "content")});
    EXPECT_FALSE(pm.HasNameOnly());

    pm = PluginMetadata("Blank.esp");
    pm.Tags({loot::Tag("Relev")});
    EXPECT_FALSE(pm.HasNameOnly());

    pm = PluginMetadata("Blank.esp");
    pm.DirtyInfo({loot::PluginDirtyInfo(5, 0, 1, 2, "utility")});
    EXPECT_FALSE(pm.HasNameOnly());

    pm = PluginMetadata("Blank.esp");
    pm.Locations({loot::Location("http://www.example.com")});
    EXPECT_FALSE(pm.HasNameOnly());
}

TEST(PluginMetadata, IsRegexPlugin) {
    PluginMetadata pm;
    EXPECT_FALSE(pm.IsRegexPlugin());

    pm.Name("Blank.esm");
    EXPECT_FALSE(pm.IsRegexPlugin());

    pm.Name("Blank[[:blank:]]- Different.esm");
    EXPECT_FALSE(pm.IsRegexPlugin());

    pm.Name("Blank\\.esm");
    EXPECT_TRUE(pm.IsRegexPlugin());

    pm.Name("Blank - (Different )*Master Dependent.esm");
    EXPECT_FALSE(pm.IsRegexPlugin());

    pm.Name("Blank - (Different )?Master Dependent.esm");
    EXPECT_FALSE(pm.IsRegexPlugin());

    pm.Name("Blank.es(p|m)");
    EXPECT_FALSE(pm.IsRegexPlugin());
}

TEST(PluginMetadata, YamlEmitter) {
    PluginMetadata pm("Blank.esp");
    YAML::Emitter e1;
    e1 << pm;
    EXPECT_STREQ("", e1.c_str());

    pm.SetPriorityExplicit(true);
    YAML::Emitter e2;
    e2 << pm;
    EXPECT_STREQ("name: 'Blank.esp'\n"
                 "priority: 0", e2.c_str());

    pm.Enabled(false);
    YAML::Emitter e3;
    e3 << pm;
    EXPECT_STREQ("name: 'Blank.esp'\n"
                 "priority: 0\n"
                 "enabled: false", e3.c_str());

    pm.LoadAfter({loot::File("Blank.esm")});
    YAML::Emitter e4;
    e4 << pm;
    EXPECT_STREQ("name: 'Blank.esp'\n"
                 "priority: 0\n"
                 "enabled: false\n"
                 "after:\n"
                 "  - 'Blank.esm'", e4.c_str());

    pm.Reqs({loot::File("Blank.esm")});
    YAML::Emitter e5;
    e5 << pm;
    EXPECT_STREQ("name: 'Blank.esp'\n"
                 "priority: 0\n"
                 "enabled: false\n"
                 "after:\n"
                 "  - 'Blank.esm'\n"
                 "req:\n"
                 "  - 'Blank.esm'", e5.c_str());

    pm.Incs({loot::File("Blank.esm")});
    YAML::Emitter e6;
    e6 << pm;
    EXPECT_STREQ("name: 'Blank.esp'\n"
                 "priority: 0\n"
                 "enabled: false\n"
                 "after:\n"
                 "  - 'Blank.esm'\n"
                 "req:\n"
                 "  - 'Blank.esm'\n"
                 "inc:\n"
                 "  - 'Blank.esm'", e6.c_str());

    pm.Messages({loot::Message(loot::Message::say, "content")});
    YAML::Emitter e7;
    e7 << pm;
    EXPECT_STREQ("name: 'Blank.esp'\n"
                 "priority: 0\n"
                 "enabled: false\n"
                 "after:\n"
                 "  - 'Blank.esm'\n"
                 "req:\n"
                 "  - 'Blank.esm'\n"
                 "inc:\n"
                 "  - 'Blank.esm'\n"
                 "msg:\n"
                 "  - type: say\n"
                 "    content: 'content'", e7.c_str());

    pm.Tags({loot::Tag("Relev")});
    YAML::Emitter e8;
    e8 << pm;
    EXPECT_STREQ("name: 'Blank.esp'\n"
                 "priority: 0\n"
                 "enabled: false\n"
                 "after:\n"
                 "  - 'Blank.esm'\n"
                 "req:\n"
                 "  - 'Blank.esm'\n"
                 "inc:\n"
                 "  - 'Blank.esm'\n"
                 "msg:\n"
                 "  - type: say\n"
                 "    content: 'content'\n"
                 "tag:\n"
                 "  - Relev", e8.c_str());

    pm.DirtyInfo({loot::PluginDirtyInfo(5, 0, 1, 2, "utility")});
    YAML::Emitter e9;
    e9 << pm;
    EXPECT_STREQ("name: 'Blank.esp'\n"
                 "priority: 0\n"
                 "enabled: false\n"
                 "after:\n"
                 "  - 'Blank.esm'\n"
                 "req:\n"
                 "  - 'Blank.esm'\n"
                 "inc:\n"
                 "  - 'Blank.esm'\n"
                 "msg:\n"
                 "  - type: say\n"
                 "    content: 'content'\n"
                 "tag:\n"
                 "  - Relev\n"
                 "dirty:\n"
                 "  - crc: 0x5\n"
                 "    util: 'utility'\n"
                 "    udr: 1\n"
                 "    nav: 2", e9.c_str());

    pm.Locations({loot::Location("http://www.example.com")});
    YAML::Emitter e10;
    e10 << pm;
    EXPECT_STREQ("name: 'Blank.esp'\n"
                 "priority: 0\n"
                 "enabled: false\n"
                 "after:\n"
                 "  - 'Blank.esm'\n"
                 "req:\n"
                 "  - 'Blank.esm'\n"
                 "inc:\n"
                 "  - 'Blank.esm'\n"
                 "msg:\n"
                 "  - type: say\n"
                 "    content: 'content'\n"
                 "tag:\n"
                 "  - Relev\n"
                 "dirty:\n"
                 "  - crc: 0x5\n"
                 "    util: 'utility'\n"
                 "    udr: 1\n"
                 "    nav: 2\n"
                 "url:\n"
                 "  - 'http://www.example.com'", e10.c_str());
}

TEST(PluginMetadata, YamlEncode) {
    YAML::Node node;

    PluginMetadata pm("Blank.esp");
    node = pm;
    EXPECT_EQ("Blank.esp", node["name"].as<std::string>());
    EXPECT_FALSE(node["enabled"]);
    EXPECT_FALSE(node["priority"]);
    EXPECT_FALSE(node["after"]);
    EXPECT_FALSE(node["req"]);
    EXPECT_FALSE(node["inc"]);
    EXPECT_FALSE(node["msg"]);
    EXPECT_FALSE(node["tag"]);
    EXPECT_FALSE(node["dirty"]);
    EXPECT_FALSE(node["url"]);

    pm.SetPriorityExplicit(true);
    node = pm;
    EXPECT_EQ(0, node["priority"].as<int>());

    pm.Enabled(false);
    node = pm;
    EXPECT_FALSE(node["enabled"].as<bool>());

    pm.LoadAfter({loot::File("Blank.esm")});
    node = pm;
    EXPECT_EQ(pm.LoadAfter(), node["after"].as<std::set<loot::File>>());

    pm.Reqs({loot::File("Blank.esm")});
    node = pm;
    EXPECT_EQ(pm.Reqs(), node["req"].as<std::set<loot::File>>());

    pm.Incs({loot::File("Blank.esm")});
    node = pm;
    EXPECT_EQ(pm.Incs(), node["inc"].as<std::set<loot::File>>());

    pm.Messages({loot::Message(loot::Message::say, "content")});
    node = pm;
    EXPECT_EQ(pm.Messages(), node["msg"].as<std::list<loot::Message>>());

    pm.Tags({loot::Tag("Relev")});
    node = pm;
    EXPECT_EQ(pm.Tags(), node["tag"].as<std::set<loot::Tag>>());

    pm.DirtyInfo({loot::PluginDirtyInfo(5, 0, 1, 2, "utility")});
    node = pm;
    EXPECT_EQ(pm.DirtyInfo(), node["dirty"].as<std::set<loot::PluginDirtyInfo>>());

    pm.Locations({loot::Location("http://www.example.com")});
    node = pm;
    EXPECT_EQ(pm.Locations(), node["url"].as<std::set<loot::Location>>());
}

TEST(PluginMetadata, YamlDecode) {
    YAML::Node node;
    PluginMetadata pm;

    node = YAML::Load("Blank.esp");
    EXPECT_ANY_THROW(node.as<PluginMetadata>());

    node = YAML::Load("name: Blank.esp");
    pm = node.as<loot::PluginMetadata>();
    EXPECT_EQ("Blank.esp", pm.Name());

    node = YAML::Load("name: 'Blank.esp'\n"
                      "priority: 5\n"
                      "enabled: false\n"
                      "after:\n"
                      "  - 'Blank.esm'\n"
                      "req:\n"
                      "  - 'Blank.esm'\n"
                      "inc:\n"
                      "  - 'Blank.esm'\n"
                      "msg:\n"
                      "  - type: say\n"
                      "    content: 'content'\n"
                      "tag:\n"
                      "  - Relev\n"
                      "dirty:\n"
                      "  - crc: 0x5\n"
                      "    util: 'utility'\n"
                      "    udr: 1\n"
                      "    nav: 2\n"
                      "url:\n"
                      "  - 'http://www.example.com'");
    pm = node.as<loot::PluginMetadata>();
    EXPECT_EQ("Blank.esp", pm.Name());
    EXPECT_EQ(5, pm.Priority());
    EXPECT_TRUE(pm.IsPriorityExplicit());
    EXPECT_FALSE(pm.Enabled());
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esm")
    }), pm.LoadAfter());
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esm")
    }), pm.Reqs());
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esm")
    }), pm.Incs());
    EXPECT_EQ(std::list<loot::Message>({
        loot::Message(loot::Message::say, "content")
    }), pm.Messages());
    EXPECT_EQ(std::set<loot::Tag>({
        loot::Tag("Relev")
    }), pm.Tags());
    EXPECT_EQ(std::set<loot::PluginDirtyInfo>({
        loot::PluginDirtyInfo(5, 0, 1, 2, "utility")
    }), pm.DirtyInfo());
    EXPECT_EQ(std::set<loot::Location>({
        loot::Location("http://www.example.com")
    }), pm.Locations());

    node = YAML::Load("scalar");
    EXPECT_ANY_THROW(node.as<PluginMetadata>());

    node = YAML::Load("[0, 1, 2]");
    EXPECT_ANY_THROW(node.as<PluginMetadata>());
}

#endif
