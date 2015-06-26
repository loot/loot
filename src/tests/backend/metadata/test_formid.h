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

#ifndef LOOT_TEST_BACKEND_METADATA_FORMID
#define LOOT_TEST_BACKEND_METADATA_FORMID

#include "backend/metadata/formid.h"
#include "tests/fixtures.h"

using loot::FormID;

TEST(FormID, ConstructorsAndDataAccess) {
    FormID fid;

    EXPECT_EQ("", fid.Plugin());
    EXPECT_EQ(0, fid.Id());

    fid = FormID("plugin.esp", 1);
    EXPECT_EQ("plugin.esp", fid.Plugin());
    EXPECT_EQ(1, fid.Id());

    std::vector<std::string> masters = {
        "master1.esp",
        "master2.esp",
        "plugin.esp"
    };
    fid = FormID(masters, 0x01000001);
    EXPECT_EQ("master2.esp", fid.Plugin());
    EXPECT_EQ(1, fid.Id());

    fid = FormID(masters, 0x0F000001);
    EXPECT_EQ("plugin.esp", fid.Plugin());
    EXPECT_EQ(1, fid.Id());
}

TEST(FormID, EqualityOperator) {
    FormID fid1, fid2;
    EXPECT_TRUE(fid1 == fid2);

    // Not valid conditions, but not evaluating them in this test.
    fid1 = FormID("plugin.esp", 1);
    fid2 = FormID("plugin.esp", 1);
    EXPECT_TRUE(fid1 == fid2);

    fid1 = FormID("plugin.esp", 1);
    fid2 = FormID("plugin.esp", 2);
    EXPECT_FALSE(fid1 == fid2);

    fid1 = FormID("plugin1.esp", 1);
    fid2 = FormID("plugin2.esp", 1);
    EXPECT_FALSE(fid1 == fid2);
}

TEST(FormID, LessThanOperator) {
    FormID fid1, fid2;
    EXPECT_FALSE(fid1 < fid2);
    EXPECT_FALSE(fid2 < fid1);

    fid1 = FormID("plugin.esp", 1);
    fid2 = FormID("Plugin.esp", 1);
    EXPECT_FALSE(fid1 < fid2);
    EXPECT_FALSE(fid2 < fid1);

    fid1 = FormID("plugin.esp", 1);
    fid2 = FormID("plugin.esp", 2);
    EXPECT_TRUE(fid1 < fid2);
    EXPECT_FALSE(fid2 < fid1);

    fid1 = FormID("plugin1.esp", 1);
    fid2 = FormID("Plugin2.esp", 1);
    EXPECT_TRUE(fid1 < fid2);
    EXPECT_FALSE(fid2 < fid1);
}

#endif
