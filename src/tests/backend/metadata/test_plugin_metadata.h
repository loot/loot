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

#ifndef LOOT_TEST_F_BACKEND_METADATA_PLUGIN_METADATA
#define LOOT_TEST_F_BACKEND_METADATA_PLUGIN_METADATA

#include "backend/metadata/plugin_metadata.h"
#include "tests/fixtures.h"

class PluginMetadata : public SkyrimTest {};

TEST_F(PluginMetadata, Constructors) {
    loot::PluginMetadata pm;
    EXPECT_TRUE(pm.Enabled());
    EXPECT_FALSE(pm.IsPriorityExplicit());
    EXPECT_EQ(0, pm.Priority());

    pm = loot::PluginMetadata("Blank.esm");
    EXPECT_EQ("Blank.esm", pm.Name());
    EXPECT_TRUE(pm.Enabled());
    EXPECT_FALSE(pm.IsPriorityExplicit());
    EXPECT_EQ(0, pm.Priority());
}

TEST_F(PluginMetadata, EqualityOperator) {
    loot::PluginMetadata pm1, pm2;
    EXPECT_TRUE(pm1 == pm2);

    pm1 = loot::PluginMetadata("Blank.esm");
    pm2 = loot::PluginMetadata("blank.esm");
    EXPECT_TRUE(pm1 == pm2);

    pm1 = loot::PluginMetadata("Blank.esm");
    pm2 = loot::PluginMetadata("Blan.\\.esm");
    EXPECT_TRUE(pm1 == pm2);
    EXPECT_TRUE(pm2 == pm1);
}

TEST_F(PluginMetadata, MergeMetadata) {
    loot::PluginMetadata pm1(YAML::Load(
        "name: 'Blank.esp'\n"
        "enabled: false\n"
        "priority: 5\n"
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
        "  - 'http://www.example.com'"
        ).as<loot::PluginMetadata>());

    loot::PluginMetadata pm2(YAML::Load(
        "name: 'Blank - Different.esp'\n"
        "after:\n"
        "  - 'Blank.esm'\n"
        "  - 'Blank.esp'\n"
        "req:\n"
        "  - 'Blank.esm'\n"
        "  - 'Blank.esp'\n"
        "inc:\n"
        "  - 'Blank.esm'\n"
        "  - 'Blank.esp'\n"
        "msg:\n"
        "  - type: say\n"
        "    content: 'content'\n"
        "tag:\n"
        "  - Relev\n"
        "  - -Relev\n"
        "  - Delev\n"
        "dirty:\n"
        "  - crc: 0x5\n"
        "    util: 'utility'\n"
        "    udr: 1\n"
        "    nav: 2\n"
        "  - crc: 0x9\n"
        "    util: 'utility'\n"
        "    udr: 1\n"
        "    nav: 2\n"
        "url:\n"
        "  - 'http://www.example.com'\n"
        "  - 'http://www.example2.com'"
        ).as<loot::PluginMetadata>());

    EXPECT_NO_THROW(pm1.MergeMetadata(pm2));
    EXPECT_EQ("Blank.esp", pm1.Name());
    EXPECT_TRUE(pm1.Enabled());
    EXPECT_EQ(5, pm1.Priority());
    EXPECT_TRUE(pm1.IsPriorityExplicit());
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esm"),
        loot::File("Blank.esp"),
    }), pm1.LoadAfter());
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esm"),
        loot::File("Blank.esp"),
    }), pm1.Reqs());
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esm"),
        loot::File("Blank.esp"),
    }), pm1.Incs());
    EXPECT_EQ(std::list<loot::Message>({
        loot::Message(loot::Message::say, "content"),
        loot::Message(loot::Message::say, "content"),
    }), pm1.Messages());
    EXPECT_EQ(std::set<loot::Tag>({
        loot::Tag("Relev"),
        loot::Tag("Relev", false),
        loot::Tag("Delev"),
    }), pm1.Tags());
    EXPECT_EQ(std::set<loot::PluginDirtyInfo>({
        loot::PluginDirtyInfo(5, 0, 1, 2, "utility"),
        loot::PluginDirtyInfo(9, 0, 1, 2, "utility"),
    }), pm1.DirtyInfo());
    EXPECT_EQ(std::set<loot::Location>({
        loot::Location("http://www.example.com"),
        loot::Location("http://www.example2.com"),
    }), pm1.Locations());

    pm2.SetPriorityExplicit(true);
    EXPECT_NO_THROW(pm1.MergeMetadata(pm2));
    EXPECT_EQ(0, pm1.Priority());
    EXPECT_TRUE(pm1.IsPriorityExplicit());
}

TEST_F(PluginMetadata, DiffMetadata) {
    loot::PluginMetadata pm1(YAML::Load(
        "name: 'Blank.esp'\n"
        "enabled: false\n"
        "after:\n"
        "  - 'Blank.esm'\n"
        "  - 'Blank - Different.esm'\n"
        "req:\n"
        "  - 'Blank.esm'\n"
        "  - 'Blank - Different.esm'\n"
        "inc:\n"
        "  - 'Blank.esm'\n"
        "  - 'Blank - Different.esm'\n"
        "msg:\n"
        "  - type: say\n"
        "    content: 'content'\n"
        "  - type: say\n"
        "    content: 'content2'\n"
        "tag:\n"
        "  - Relev\n"
        "  - NoMerge\n"
        "dirty:\n"
        "  - crc: 0x5\n"
        "    util: 'utility'\n"
        "    udr: 1\n"
        "    nav: 2\n"
        "  - crc: 0x1\n"
        "    util: 'utility'\n"
        "    udr: 1\n"
        "    nav: 2\n"
        "url:\n"
        "  - 'http://www.example.com'\n"
        "  - 'http://www.other-example.com'"
        ).as<loot::PluginMetadata>());

    loot::PluginMetadata pm2(YAML::Load(
        "name: 'Blank - Different.esp'\n"
        "after:\n"
        "  - 'Blank.esm'\n"
        "  - 'Blank.esp'\n"
        "req:\n"
        "  - 'Blank.esm'\n"
        "  - 'Blank.esp'\n"
        "inc:\n"
        "  - 'Blank.esm'\n"
        "  - 'Blank.esp'\n"
        "msg:\n"
        "  - type: say\n"
        "    content: 'content'\n"
        "  - type: say\n"
        "    content: 'content3'\n"
        "tag:\n"
        "  - Relev\n"
        "  - -Relev\n"
        "  - Delev\n"
        "dirty:\n"
        "  - crc: 0x5\n"
        "    util: 'utility'\n"
        "    udr: 1\n"
        "    nav: 2\n"
        "  - crc: 0x9\n"
        "    util: 'utility'\n"
        "    udr: 1\n"
        "    nav: 2\n"
        "url:\n"
        "  - 'http://www.example.com'\n"
        "  - 'http://www.example2.com'"
        ).as<loot::PluginMetadata>());

    loot::PluginMetadata result = pm1.DiffMetadata(pm2);
    EXPECT_EQ("Blank.esp", result.Name());
    EXPECT_FALSE(result.Enabled());
    EXPECT_EQ(0, result.Priority());
    EXPECT_FALSE(result.IsPriorityExplicit());
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esp"),
        loot::File("Blank - Different.esm"),
    }), result.LoadAfter());
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esp"),
        loot::File("Blank - Different.esm"),
    }), result.Reqs());
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esp"),
        loot::File("Blank - Different.esm"),
    }), result.Incs());
    EXPECT_EQ(std::list<loot::Message>({
        loot::Message(loot::Message::say, "content2"),
        loot::Message(loot::Message::say, "content3"),
    }), result.Messages());
    EXPECT_EQ(std::set<loot::Tag>({
        loot::Tag("Relev", false),
        loot::Tag("Delev"),
        loot::Tag("NoMerge"),
    }), result.Tags());
    EXPECT_EQ(std::set<loot::PluginDirtyInfo>({
        loot::PluginDirtyInfo(9, 0, 1, 2, "utility"),
        loot::PluginDirtyInfo(1, 0, 1, 2, "utility"),
    }), result.DirtyInfo());
    EXPECT_EQ(std::set<loot::Location>({
        loot::Location("http://www.example2.com"),
        loot::Location("http://www.other-example.com"),
    }), result.Locations());

    pm1.Priority(5);
    result = pm1.DiffMetadata(pm2);

    EXPECT_EQ(5, result.Priority());
    EXPECT_TRUE(result.IsPriorityExplicit());
}

TEST_F(PluginMetadata, NewMetadata) {
    loot::PluginMetadata pm1(YAML::Load(
        "name: 'Blank.esp'\n"
        "enabled: false\n"
        "after:\n"
        "  - 'Blank.esm'\n"
        "  - 'Blank - Different.esm'\n"
        "req:\n"
        "  - 'Blank.esm'\n"
        "  - 'Blank - Different.esm'\n"
        "inc:\n"
        "  - 'Blank.esm'\n"
        "  - 'Blank - Different.esm'\n"
        "msg:\n"
        "  - type: say\n"
        "    content: 'content'\n"
        "  - type: say\n"
        "    content: 'content2'\n"
        "tag:\n"
        "  - Relev\n"
        "  - NoMerge\n"
        "dirty:\n"
        "  - crc: 0x5\n"
        "    util: 'utility'\n"
        "    udr: 1\n"
        "    nav: 2\n"
        "  - crc: 0x1\n"
        "    util: 'utility'\n"
        "    udr: 1\n"
        "    nav: 2\n"
        "url:\n"
        "  - 'http://www.example.com'\n"
        "  - 'http://www.other-example.com'"
        ).as<loot::PluginMetadata>());

    loot::PluginMetadata pm2(YAML::Load(
        "name: 'Blank - Different.esp'\n"
        "after:\n"
        "  - 'Blank.esm'\n"
        "  - 'Blank.esp'\n"
        "req:\n"
        "  - 'Blank.esm'\n"
        "  - 'Blank.esp'\n"
        "inc:\n"
        "  - 'Blank.esm'\n"
        "  - 'Blank.esp'\n"
        "msg:\n"
        "  - type: say\n"
        "    content: 'content'\n"
        "  - type: say\n"
        "    content: 'content3'\n"
        "tag:\n"
        "  - Relev\n"
        "  - -Relev\n"
        "  - Delev\n"
        "dirty:\n"
        "  - crc: 0x5\n"
        "    util: 'utility'\n"
        "    udr: 1\n"
        "    nav: 2\n"
        "  - crc: 0x9\n"
        "    util: 'utility'\n"
        "    udr: 1\n"
        "    nav: 2\n"
        "url:\n"
        "  - 'http://www.example.com'\n"
        "  - 'http://www.example2.com'"
        ).as<loot::PluginMetadata>());

    loot::PluginMetadata result = pm2.NewMetadata(pm1);
    EXPECT_EQ("Blank - Different.esp", result.Name());
    EXPECT_TRUE(result.Enabled());
    EXPECT_EQ(0, result.Priority());
    EXPECT_FALSE(result.IsPriorityExplicit());
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esp")
    }), result.LoadAfter());
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esp")
    }), result.Reqs());
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esp")
    }), result.Incs());
    EXPECT_EQ(std::list<loot::Message>({
        loot::Message(loot::Message::say, "content3"),
    }), result.Messages());
    EXPECT_EQ(std::set<loot::Tag>({
        loot::Tag("Relev", false),
        loot::Tag("Delev"),
    }), result.Tags());
    EXPECT_EQ(std::set<loot::PluginDirtyInfo>({
        loot::PluginDirtyInfo(9, 0, 1, 2, "utility")
    }), result.DirtyInfo());
    EXPECT_EQ(std::set<loot::Location>({
        loot::Location("http://www.example2.com")
    }), result.Locations());

    pm1.Priority(5);
    result = pm2.NewMetadata(pm1);

    EXPECT_EQ(0, result.Priority());
    EXPECT_FALSE(result.IsPriorityExplicit());
}

TEST_F(PluginMetadata, Enabled) {
    loot::PluginMetadata pm;
    ASSERT_TRUE(pm.Enabled());
    pm.Enabled(false);
    EXPECT_FALSE(pm.Enabled());
}

TEST_F(PluginMetadata, SetPriorityExplicit) {
    loot::PluginMetadata pm;
    ASSERT_FALSE(pm.IsPriorityExplicit());
    pm.SetPriorityExplicit(true);
    EXPECT_TRUE(pm.IsPriorityExplicit());
}

TEST_F(PluginMetadata, Priority) {
    loot::PluginMetadata pm;
    ASSERT_NE(10, pm.Priority());
    pm.Priority(10);
    EXPECT_EQ(10, pm.Priority());
}

TEST_F(PluginMetadata, LoadAfter) {
    loot::PluginMetadata pm;
    ASSERT_TRUE(pm.LoadAfter().empty());
    pm.LoadAfter({
        loot::File("Blank.esm")
    });
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esm")
    }), pm.LoadAfter());
}

TEST_F(PluginMetadata, Reqs) {
    loot::PluginMetadata pm;
    ASSERT_TRUE(pm.Reqs().empty());
    pm.Reqs({
        loot::File("Blank.esm")
    });
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esm")
    }), pm.Reqs());
}

TEST_F(PluginMetadata, Incs) {
    loot::PluginMetadata pm;
    ASSERT_TRUE(pm.Incs().empty());
    pm.Incs({
        loot::File("Blank.esm")
    });
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esm")
    }), pm.Incs());
}

TEST_F(PluginMetadata, Messages) {
    loot::PluginMetadata pm;
    ASSERT_TRUE(pm.Messages().empty());
    pm.Messages({
        loot::Message(loot::Message::say, "content")
    });
    EXPECT_EQ(std::list<loot::Message>({
        loot::Message(loot::Message::say, "content")
    }), pm.Messages());
}

TEST_F(PluginMetadata, Tags) {
    loot::PluginMetadata pm;
    ASSERT_TRUE(pm.Tags().empty());
    pm.Tags({
        loot::Tag("Relev")
    });
    EXPECT_EQ(std::set<loot::Tag>({
        loot::Tag("Relev")
    }), pm.Tags());
}

TEST_F(PluginMetadata, DirtyInfo) {
    loot::PluginMetadata pm;
    ASSERT_TRUE(pm.DirtyInfo().empty());
    pm.DirtyInfo({
        loot::PluginDirtyInfo(5, 0, 1, 2, "utility")
    });
    EXPECT_EQ(std::set<loot::PluginDirtyInfo>({
        loot::PluginDirtyInfo(5, 0, 1, 2, "utility")
    }), pm.DirtyInfo());
}

TEST_F(PluginMetadata, Locations) {
    loot::PluginMetadata pm;
    ASSERT_TRUE(pm.Locations().empty());
    pm.Locations({
        loot::Location("http://www.example.com")
    });
    EXPECT_EQ(std::set<loot::Location>({
        loot::Location("http://www.example.com")
    }), pm.Locations());
}

TEST_F(PluginMetadata, EvalAllConditions) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    loot::PluginMetadata pm(YAML::Load(
        "name: 'Blank.esp'\n"
        "after:\n"
        "  - name: 'Blank.esm'\n"
        "    condition: 'file(\"Blank.esm\")'\n"
        "req:\n"
        "  - name: 'Blank.esm'\n"
        "    condition: 'file(\"Blank.missing.esm\")'\n"
        "inc:\n"
        "  - name: 'Blank.esm'\n"
        "    condition: 'file(\"Blank.esm\")'\n"
        "msg:\n"
        "  - type: say\n"
        "    content: 'content'\n"
        "    condition: 'condition'\n"
        "tag:\n"
        "  - name: Relev\n"
        "    condition: 'file(\"Blank.missing.esm\")'\n"
        "dirty:\n"
        "  - crc: 0xE12EFAAA\n"
        "    util: 'utility'\n"
        "    udr: 1\n"
        "    nav: 2\n"
        "  - crc: 0xDEADBEEF\n"
        "    util: 'utility'\n"
        "    udr: 1\n"
        "    nav: 2"
        ).as<loot::PluginMetadata>());

    EXPECT_ANY_THROW(pm.EvalAllConditions(game, loot::Language::english));

    pm.Messages({loot::Message(loot::Message::say, "content")});
    EXPECT_NO_THROW(pm.EvalAllConditions(game, loot::Language::english));
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esm", "", "file(\"Blank.esm\")")
    }), pm.LoadAfter());
    EXPECT_TRUE(pm.Reqs().empty());
    EXPECT_EQ(std::set<loot::File>({
        loot::File("Blank.esm", "", "file(\"Blank.esm\")")
    }), pm.Incs());
    EXPECT_EQ(std::list<loot::Message>({
        loot::Message(loot::Message::say, "content")
    }), pm.Messages());
    EXPECT_TRUE(pm.Tags().empty());
    EXPECT_EQ(std::set<loot::PluginDirtyInfo>({
        loot::PluginDirtyInfo(0xE12EFAAA, 0, 1, 2, "utility")
    }), pm.DirtyInfo());
}

TEST_F(PluginMetadata, ParseAllConditions) {
    loot::PluginMetadata pm(YAML::Load(
        "name: 'Blank.esp'\n"
        "after:\n"
        "  - name: 'Blank.esm'\n"
        "    condition: 'file(\"Blank.esm\")'\n"
        "req:\n"
        "  - name: 'Blank.esm'\n"
        "    condition: 'file(\"Blank.missing.esm\")'\n"
        "inc:\n"
        "  - name: 'Blank.esm'\n"
        "    condition: 'file(\"Blank.esm\")'\n"
        "msg:\n"
        "  - type: say\n"
        "    content: 'content'\n"
        "    condition: 'condition'\n"
        "tag:\n"
        "  - name: Relev\n"
        "    condition: 'file(\"Blank.missing.esm\")'"
        ).as<loot::PluginMetadata>());

    EXPECT_ANY_THROW(pm.ParseAllConditions());

    pm.Messages({loot::Message(loot::Message::say, "content")});
    EXPECT_NO_THROW(pm.ParseAllConditions());
}

TEST_F(PluginMetadata, HasNameOnly) {
    loot::PluginMetadata pm;
    EXPECT_TRUE(pm.HasNameOnly());

    pm = loot::PluginMetadata("Blank.esp");
    EXPECT_TRUE(pm.HasNameOnly());

    pm = loot::PluginMetadata("Blank.esp");
    pm.Enabled(false);
    EXPECT_TRUE(pm.HasNameOnly());

    pm = loot::PluginMetadata("Blank.esp");
    pm.SetPriorityExplicit(true);
    EXPECT_FALSE(pm.HasNameOnly());

    pm = loot::PluginMetadata("Blank.esp");
    pm.LoadAfter({loot::File("Blank.esm")});
    EXPECT_FALSE(pm.HasNameOnly());

    pm = loot::PluginMetadata("Blank.esp");
    pm.Reqs({loot::File("Blank.esm")});
    EXPECT_FALSE(pm.HasNameOnly());

    pm = loot::PluginMetadata("Blank.esp");
    pm.Incs({loot::File("Blank.esm")});
    EXPECT_FALSE(pm.HasNameOnly());

    pm = loot::PluginMetadata("Blank.esp");
    pm.Messages({loot::Message(loot::Message::say, "content")});
    EXPECT_FALSE(pm.HasNameOnly());

    pm = loot::PluginMetadata("Blank.esp");
    pm.Tags({loot::Tag("Relev")});
    EXPECT_FALSE(pm.HasNameOnly());

    pm = loot::PluginMetadata("Blank.esp");
    pm.DirtyInfo({loot::PluginDirtyInfo(5, 0, 1, 2, "utility")});
    EXPECT_FALSE(pm.HasNameOnly());

    pm = loot::PluginMetadata("Blank.esp");
    pm.Locations({loot::Location("http://www.example.com")});
    EXPECT_FALSE(pm.HasNameOnly());
}

TEST_F(PluginMetadata, IsRegexPlugin) {
    loot::PluginMetadata pm;
    EXPECT_FALSE(pm.IsRegexPlugin());

    pm = loot::PluginMetadata("Blank.esm");
    EXPECT_FALSE(pm.IsRegexPlugin());

    pm = loot::PluginMetadata("Blank[[:blank:]]- Different.esm");
    EXPECT_TRUE(pm.IsRegexPlugin());

    pm = loot::PluginMetadata("Blank\\.esm");
    EXPECT_TRUE(pm.IsRegexPlugin());

    pm = loot::PluginMetadata("Blank - (Different )*Master Dependent.esm");
    EXPECT_TRUE(pm.IsRegexPlugin());

    pm = loot::PluginMetadata("Blank - (Different )?Master Dependent.esm");
    EXPECT_TRUE(pm.IsRegexPlugin());

    pm = loot::PluginMetadata("Blank.es(p|m)");
    EXPECT_TRUE(pm.IsRegexPlugin());
}

TEST_F(PluginMetadata, YamlEmitter) {
    loot::PluginMetadata pm("Blank.esp");
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

TEST_F(PluginMetadata, YamlEncode) {
    YAML::Node node;

    loot::PluginMetadata pm("Blank.esp");
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

TEST_F(PluginMetadata, YamlDecode) {
    YAML::Node node;
    loot::PluginMetadata pm;

    node = YAML::Load("Blank.esp");
    EXPECT_ANY_THROW(node.as<loot::PluginMetadata>());

    node = YAML::Load("name: Blank.esp");
    pm = node.as<loot::PluginMetadata>();
    EXPECT_EQ("Blank.esp", pm.Name());
    EXPECT_EQ(0, pm.Priority());
    EXPECT_FALSE(pm.IsPriorityExplicit());

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

    // Don't allow dirty metadata in regex entries.
    node = YAML::Load("name: 'Blank\\.esp'\n"
                      "dirty:\n"
                      "  - crc: 0x5\n"
                      "    util: 'utility'\n"
                      "    udr: 1\n"
                      "    nav: 2");
    EXPECT_ANY_THROW(node.as<loot::PluginMetadata>());

    node = YAML::Load("scalar");
    EXPECT_ANY_THROW(node.as<loot::PluginMetadata>());

    node = YAML::Load("[0, 1, 2]");
    EXPECT_ANY_THROW(node.as<loot::PluginMetadata>());
}

#endif
