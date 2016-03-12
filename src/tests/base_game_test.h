/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2013-2016    WrinklyNinja

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

#ifndef LOOT_TEST_BASE_GAME_TEST
#define LOOT_TEST_BASE_GAME_TEST

#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

namespace loot {
    namespace test {
        class BaseGameTest : public ::testing::TestWithParam<unsigned int> {
        protected:
            BaseGameTest() :
                missingPath("./missing"),
                dataPath(getPluginsPath()),
                localPath(getLocalPath()),
                masterFile(getMasterFile()),
                blankEsm("Blank.esm"),
                blankDifferentEsm("Blank - Different.esm"),
                blankMasterDependentEsm("Blank - Master Dependent.esm"),
                blankDifferentMasterDependentEsm("Blank - Different Master Dependent.esm"),
                blankEsp("Blank.esp"),
                blankDifferentEsp("Blank - Different.esp"),
                blankMasterDependentEsp("Blank - Master Dependent.esp"),
                blankDifferentMasterDependentEsp("Blank - Different Master Dependent.esp"),
                blankPluginDependentEsp("Blank - Plugin Dependent.esp"),
                blankDifferentPluginDependentEsp("Blank - Different Plugin Dependent.esp") {}

            inline virtual void SetUp() {
                ASSERT_NO_THROW(boost::filesystem::create_directories(localPath));
                ASSERT_TRUE(boost::filesystem::exists(localPath));

                ASSERT_FALSE(boost::filesystem::exists(missingPath));

                ASSERT_TRUE(boost::filesystem::exists(dataPath / blankEsm));
                ASSERT_TRUE(boost::filesystem::exists(dataPath / blankDifferentEsm));
                ASSERT_TRUE(boost::filesystem::exists(dataPath / blankMasterDependentEsm));
                ASSERT_TRUE(boost::filesystem::exists(dataPath / blankDifferentMasterDependentEsm));
                ASSERT_TRUE(boost::filesystem::exists(dataPath / blankEsp));
                ASSERT_TRUE(boost::filesystem::exists(dataPath / blankDifferentEsp));
                ASSERT_TRUE(boost::filesystem::exists(dataPath / blankMasterDependentEsp));
                ASSERT_TRUE(boost::filesystem::exists(dataPath / blankDifferentMasterDependentEsp));
                ASSERT_TRUE(boost::filesystem::exists(dataPath / blankPluginDependentEsp));
                ASSERT_TRUE(boost::filesystem::exists(dataPath / blankDifferentPluginDependentEsp));

                // Make sure the game master file exists.
                ASSERT_FALSE(boost::filesystem::exists(dataPath / masterFile));
                ASSERT_NO_THROW(boost::filesystem::copy_file(dataPath / blankEsm, dataPath / masterFile));
                ASSERT_TRUE(boost::filesystem::exists(dataPath / masterFile));
            }

            inline virtual void TearDown() {
                ASSERT_NO_THROW(boost::filesystem::remove(dataPath / masterFile));
            }

            const boost::filesystem::path missingPath;
            const boost::filesystem::path dataPath;
            const boost::filesystem::path localPath;

            const std::string masterFile;
            const std::string blankEsm;
            const std::string blankDifferentEsm;
            const std::string blankMasterDependentEsm;
            const std::string blankDifferentMasterDependentEsm;
            const std::string blankEsp;
            const std::string blankDifferentEsp;
            const std::string blankMasterDependentEsp;
            const std::string blankDifferentMasterDependentEsp;
            const std::string blankPluginDependentEsp;
            const std::string blankDifferentPluginDependentEsp;

        private:
            inline boost::filesystem::path getLocalPath() const {
                if (GetParam() == loot_game_tes4)
                    return "./local/Oblivion";
                else
                    return "./local/Skyrim";
            }

            inline boost::filesystem::path getPluginsPath() const {
                if (GetParam() == loot_game_tes4)
                    return "./Oblivion/Data";
                else
                    return "./Skyrim/Data";
            }

            inline std::string getMasterFile() const {
                if (GetParam() == loot_game_tes4)
                    return "Oblivion.esm";
                else if (GetParam() == loot_game_tes5)
                    return "Skyrim.esm";
                else if (GetParam() == loot_game_fo3)
                    return "Fallout3.esm";
                else if (GetParam() == loot_game_fonv)
                    return "FalloutNV.esm";
                else
                    return "Fallout4.esm";
            }
        };
    }
}

#endif
