/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2013-2015    WrinklyNinja

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

#ifndef LOOT_TEST_FIXTURES
#define LOOT_TEST_FIXTURES

#include "backend/helpers/streams.h"

#include <gtest/gtest.h>

#ifdef __GNUC__  // Workaround for GCC linking error.
#pragma message("GCC detected: Defining BOOST_NO_CXX11_SCOPED_ENUMS and BOOST_NO_SCOPED_ENUMS to avoid linking errors for boost::filesystem::copy_file().")
#define BOOST_NO_CXX11_SCOPED_ENUMS
#define BOOST_NO_SCOPED_ENUMS  // For older versions.
#endif
#include <boost/filesystem.hpp>

class GameTest : public ::testing::Test {
protected:
    GameTest(const boost::filesystem::path& gameDataPath, const boost::filesystem::path& gameLocalPath)
        : dataPath(gameDataPath), localPath(gameLocalPath), missingPath("./missing"), masterlistPath(localPath / "masterlist.yaml"), userlistPath(localPath / "userlist.yaml"), db(nullptr) {}

    inline virtual void SetUp() {
        ASSERT_NO_THROW(boost::filesystem::create_directories(localPath));
        ASSERT_TRUE(boost::filesystem::exists(localPath));

        ASSERT_FALSE(boost::filesystem::exists(masterlistPath));
        ASSERT_FALSE(boost::filesystem::exists(userlistPath));
        ASSERT_FALSE(boost::filesystem::exists(localPath / ".git"));
        ASSERT_FALSE(boost::filesystem::exists(missingPath));

        ASSERT_TRUE(boost::filesystem::exists(dataPath / "Blank.esm"));
        ASSERT_TRUE(boost::filesystem::exists(dataPath / "Blank - Different.esm"));
        ASSERT_TRUE(boost::filesystem::exists(dataPath / "Blank - Master Dependent.esm"));
        ASSERT_TRUE(boost::filesystem::exists(dataPath / "Blank - Different Master Dependent.esm"));
        ASSERT_TRUE(boost::filesystem::exists(dataPath / "Blank.esp"));
        ASSERT_TRUE(boost::filesystem::exists(dataPath / "Blank - Different.esp"));
        ASSERT_TRUE(boost::filesystem::exists(dataPath / "Blank - Master Dependent.esp"));
        ASSERT_TRUE(boost::filesystem::exists(dataPath / "Blank - Different Master Dependent.esp"));
        ASSERT_TRUE(boost::filesystem::exists(dataPath / "Blank - Plugin Dependent.esp"));
        ASSERT_TRUE(boost::filesystem::exists(dataPath / "Blank - Different Plugin Dependent.esp"));

        ASSERT_FALSE(boost::filesystem::exists(dataPath / "Blank.missing.esm"));
        ASSERT_FALSE(boost::filesystem::exists(dataPath / "Blank.missing.esp"));

        // Ghost a plugin.
        ASSERT_FALSE(boost::filesystem::exists(dataPath / "Blank - Master Dependent.esm.ghost"));
        ASSERT_NO_THROW(boost::filesystem::rename(dataPath / "Blank - Master Dependent.esm", dataPath / "Blank - Master Dependent.esm.ghost"));
        ASSERT_TRUE(boost::filesystem::exists(dataPath / "Blank - Master Dependent.esm.ghost"));

        // Write out an empty file.
        loot::ofstream out(dataPath / "EmptyFile.esm");
        out.close();
        ASSERT_TRUE(boost::filesystem::exists(dataPath / "EmptyFile.esm"));

        // Write out an non-empty, non-plugin file.
        out.open(dataPath / "NotAPlugin.esm");
        out << "This isn't a valid plugin file.";
        out.close();
        ASSERT_TRUE(boost::filesystem::exists(dataPath / "NotAPlugin.esm"));
    }

    inline virtual void TearDown() {
        // Unghost the ghosted plugin.
        ASSERT_TRUE(boost::filesystem::exists(dataPath / "Blank - Master Dependent.esm.ghost"));
        ASSERT_NO_THROW(boost::filesystem::rename(dataPath / "Blank - Master Dependent.esm.ghost", dataPath / "Blank - Master Dependent.esm"));
        ASSERT_FALSE(boost::filesystem::exists(dataPath / "Blank - Master Dependent.esm.ghost"));

        // Delete generated files.
        ASSERT_NO_THROW(boost::filesystem::remove(dataPath / "EmptyFile.esm"));
        ASSERT_NO_THROW(boost::filesystem::remove(dataPath / "NotAPlugin.esm"));
        ASSERT_FALSE(boost::filesystem::exists(dataPath / "EmptyFile.esm"));
        ASSERT_FALSE(boost::filesystem::exists(dataPath / "NotAPlugin.esm"));

        // Masterlist & userlist may have been created during test, so delete them.
        ASSERT_NO_THROW(boost::filesystem::remove(masterlistPath));
        ASSERT_NO_THROW(boost::filesystem::remove(userlistPath));

        // Also remove the ".git" folder if it has been created.
        ASSERT_NO_THROW(boost::filesystem::remove_all(localPath / ".git"));

        ASSERT_NO_THROW(loot_destroy_db(db));
    }

    const boost::filesystem::path dataPath;
    const boost::filesystem::path localPath;
    const boost::filesystem::path missingPath;

    const boost::filesystem::path masterlistPath;
    const boost::filesystem::path userlistPath;

    loot_db db;
};

class OblivionTest : public GameTest {
protected:
    OblivionTest() : GameTest("./Oblivion/Data", "./local/Oblivion") {}

    inline virtual void SetUp() {
        GameTest::SetUp();

        // LOOT expects the game master file to be present, so mock it.
        ASSERT_FALSE(boost::filesystem::exists(dataPath / "Oblivion.esm"));
        ASSERT_NO_THROW(boost::filesystem::copy_file(dataPath / "Blank.esm", dataPath / "Oblivion.esm"));
        ASSERT_TRUE(boost::filesystem::exists(dataPath / "Oblivion.esm"));

        // Oblivion's load order is decided through timestamps, so reset them to a known order before each test.
        std::list<std::string> loadOrder = {
            "Oblivion.esm",
            "Blank.esm",
            "Blank - Different.esm",
            "Blank - Master Dependent.esm",  // Ghosted
            "Blank - Different Master Dependent.esm",
            "Blank.esp",
            "Blank - Different.esp",
            "Blank - Master Dependent.esp",
            "Blank - Different Master Dependent.esp",
            "Blank - Plugin Dependent.esp",
            "Blank - Different Plugin Dependent.esp"
        };
        time_t modificationTime = time(NULL);  // Current time.
        for (const auto &plugin : loadOrder) {
            if (boost::filesystem::exists(dataPath / boost::filesystem::path(plugin + ".ghost"))) {
                boost::filesystem::last_write_time(dataPath / boost::filesystem::path(plugin + ".ghost"), modificationTime);
            }
            else {
                boost::filesystem::last_write_time(dataPath / plugin, modificationTime);
            }
            modificationTime += 60;
        }

        // Set Oblivion's active plugins to a known list before running the test.
        loot::ofstream activePlugins(localPath / "plugins.txt");
        activePlugins
            << "Oblivion.esm" << std::endl
            << "Blank.esm" << std::endl;
        activePlugins.close();
    }

    inline virtual void TearDown() {
        GameTest::TearDown();

        // Delete the mock Oblivion.esm.
        ASSERT_TRUE(boost::filesystem::exists(dataPath / "Oblivion.esm"));
        ASSERT_NO_THROW(boost::filesystem::remove(dataPath / "Oblivion.esm"));
        ASSERT_FALSE(boost::filesystem::exists(dataPath / "Oblivion.esm"));

        // Delete existing plugins.txt.
        ASSERT_NO_THROW(boost::filesystem::remove(localPath / "plugins.txt"));
    };

    inline void GenerateMasterlist() {
        loot::ofstream masterlist(masterlistPath);
        masterlist
            << "plugins:" << std::endl
            << "  - name: Oblivion.esm" << std::endl
            << "  - name: Silgrad_Tower.esm" << std::endl
            << "  - name: No Lights Flicker.esm" << std::endl
            << "    msg:" << std::endl
            << "      - type: say" << std::endl
            << "        content: Use Wrye Bash Bashed Patch tweak instead." << std::endl
            << "  - name: bookplacing.esm" << std::endl
            << "    msg:" << std::endl
            << "      - type: warn" << std::endl
            << "        content: 'Check you are using v2+. If not, Update. v1 has a severe bug with the Mystic Emporium disappearing.'" << std::endl
            << "  - name: EnhancedWeatherSIOnly.esm" << std::endl
            << "    msg:" << std::endl
            << "      - type: error" << std::endl
            << "        content: Obsolete. Remove this and install Enhanced Weather." << std::endl
            << "  - name: Hammerfell.esm" << std::endl
            << "    dirty:" << std::endl
            << "      - crc: 0x7d22f9df" << std::endl
            << "        util: TES4Edit" << std::endl
            << "        udr: 4" << std::endl
            << "  - name: nVidia Black Screen Fix.esp" << std::endl
            << "    msg:" << std::endl
            << "      - type: say" << std::endl
            << "        content: Use Wrye Bash Bashed Patch tweak instead." << std::endl
            << "      - type: say" << std::endl
            << "        content: 'Alternatively, remove this and use UOP v3.0.1+ instead.'" << std::endl
            << "  - name: Unofficial Oblivion Patch.esp" << std::endl
            << "    msg:" << std::endl
            << "      - type: say" << std::endl
            << "        content: 'Do not clean ITM records, they are intentional and required for the mod to function.'" << std::endl
            << "    tag:" << std::endl
            << "      - Actors.ACBS" << std::endl
            << "      - Actors.AIData" << std::endl
            << "      - Actors.AIPackages" << std::endl
            << "      - Actors.CombatStyle" << std::endl
            << "      - Actors.DeathItem" << std::endl
            << "      - Actors.Stats" << std::endl
            << "      - C.Climate" << std::endl
            << "      - C.Light" << std::endl
            << "      - C.Music" << std::endl
            << "      - C.Name" << std::endl
            << "      - C.Owner" << std::endl
            << "      - Creatures.Blood" << std::endl
            << "      - Delev" << std::endl
            << "      - Factions" << std::endl
            << "      - Invent" << std::endl
            << "      - Names" << std::endl
            << "      - NPC.Class" << std::endl
            << "      - Relations" << std::endl
            << "      - Relev" << std::endl
            << "      - Scripts" << std::endl
            << "      - Stats" << std::endl
            << "      - '-C.Water'" << std::endl;

        masterlist.close();
    }
};

class OblivionAPIOperationsTest : public OblivionTest {
protected:
    inline virtual void SetUp() {
        OblivionTest::SetUp();

        ASSERT_EQ(loot_ok, loot_create_db(&db, loot_game_tes4, dataPath.parent_path().string().c_str(), localPath.string().c_str()));
    }
};

class SkyrimTest : public GameTest {
protected:
    SkyrimTest() : GameTest("./Skyrim/Data", "./local/Skyrim") {}

    inline virtual void SetUp() {
        GameTest::SetUp();

        // Can't change Skyrim's main master file, so mock it.
        ASSERT_FALSE(boost::filesystem::exists(dataPath / "Skyrim.esm"));
        ASSERT_NO_THROW(boost::filesystem::copy_file(dataPath / "Blank.esm", dataPath / "Skyrim.esm"));
        ASSERT_TRUE(boost::filesystem::exists(dataPath / "Skyrim.esm"));

        // Set Skyrim's load order to a known list before running the test.
        loot::ofstream loadOrder(localPath / "loadorder.txt");
        loadOrder
            << "Skyrim.esm" << std::endl
            << "Blank.esm" << std::endl
            << "Blank - Different.esm" << std::endl
            << "Blank - Master Dependent.esm" << std::endl  // Ghosted
            << "Blank - Different Master Dependent.esm" << std::endl
            << "Blank.esp" << std::endl
            << "Blank - Different.esp" << std::endl
            << "Blank - Master Dependent.esp" << std::endl
            << "Blank - Different Master Dependent.esp" << std::endl
            << "Blank - Plugin Dependent.esp" << std::endl
            << "Blank - Different Plugin Dependent.esp" << std::endl;
        loadOrder.close();

        // Set Skyrim's active plugins to a known list before running the test.
        loot::ofstream activePlugins(localPath / "plugins.txt");
        activePlugins
            << "Blank.esm" << std::endl;
        activePlugins.close();
    }

    inline virtual void TearDown() {
        GameTest::TearDown();

        // Delete the mock Skyrim.esm.
        ASSERT_TRUE(boost::filesystem::exists(dataPath / "Skyrim.esm"));
        ASSERT_NO_THROW(boost::filesystem::remove(dataPath / "Skyrim.esm"));
        ASSERT_FALSE(boost::filesystem::exists(dataPath / "Skyrim.esm"));

        // Delete existing plugins.txt and loadorder.txt.
        ASSERT_NO_THROW(boost::filesystem::remove(localPath / "plugins.txt"));
        ASSERT_NO_THROW(boost::filesystem::remove(localPath / "loadorder.txt"));
    };
};

class SkyrimAPIOperationsTest : public SkyrimTest {
protected:
    inline virtual void SetUp() {
        SkyrimTest::SetUp();

        ASSERT_EQ(loot_ok, loot_create_db(&db, loot_game_tes5, dataPath.parent_path().string().c_str(), localPath.string().c_str()));
    }
};

#endif