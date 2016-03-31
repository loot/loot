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

#include "backend/game/game_settings.h"

#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>

#include <unordered_set>

namespace loot {
    namespace test {
        class BaseGameTest : public ::testing::TestWithParam<unsigned int> {
        protected:
            BaseGameTest() :
                missingPath("./missing"),
                dataPath(getPluginsPath()),
                localPath(getLocalPath()),
                masterFile(getMasterFile()),
                missingEsp("Blank.missing.esp"),
                blankEsm("Blank.esm"),
                blankDifferentEsm("Blank - Different.esm"),
                blankMasterDependentEsm("Blank - Master Dependent.esm"),
                blankDifferentMasterDependentEsm("Blank - Different Master Dependent.esm"),
                blankEsp("Blank.esp"),
                blankDifferentEsp("Blank - Different.esp"),
                blankMasterDependentEsp("Blank - Master Dependent.esp"),
                blankDifferentMasterDependentEsp("Blank - Different Master Dependent.esp"),
                blankPluginDependentEsp("Blank - Plugin Dependent.esp"),
                blankDifferentPluginDependentEsp("Blank - Different Plugin Dependent.esp"),
                blankEsmCrc(getBlankEsmCrc()) {}

            inline virtual void SetUp() {
                ASSERT_NO_THROW(boost::filesystem::create_directories(localPath));
                ASSERT_TRUE(boost::filesystem::exists(localPath));

                ASSERT_FALSE(boost::filesystem::exists(missingPath));
                ASSERT_FALSE(boost::filesystem::exists(dataPath / missingEsp));

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

                // Set initial load order and active plugins.
                setLoadOrder(getInitialLoadOrder());
                setActivePlugins(getInitialActivePlugins());

                // Ghost a plugin.
                ASSERT_FALSE(boost::filesystem::exists(dataPath / (blankMasterDependentEsm + ".ghost")));
                ASSERT_NO_THROW(boost::filesystem::rename(dataPath / blankMasterDependentEsm, dataPath / (blankMasterDependentEsm + ".ghost")));
                ASSERT_TRUE(boost::filesystem::exists(dataPath / (blankMasterDependentEsm + ".ghost")));
            }

            inline virtual void TearDown() {
                ASSERT_NO_THROW(boost::filesystem::remove(dataPath / masterFile));

                // Unghost the ghosted plugin.
                ASSERT_TRUE(boost::filesystem::exists(dataPath / (blankMasterDependentEsm + ".ghost")));
                ASSERT_NO_THROW(boost::filesystem::rename(dataPath / (blankMasterDependentEsm + ".ghost"), dataPath / blankMasterDependentEsm));
                ASSERT_FALSE(boost::filesystem::exists(dataPath / (blankMasterDependentEsm + ".ghost")));
            }

            inline std::list<std::string> getLoadOrder() {
                std::list<std::string> actual;
                if (isLoadOrderTimestampBased(GetParam())) {
                    std::map<time_t, std::string> loadOrder;
                    for (boost::filesystem::directory_iterator it(dataPath); it != boost::filesystem::directory_iterator(); ++it) {
                        if (boost::filesystem::is_regular_file(it->status()) &&
                            (boost::ends_with(it->path().filename().string(), ".esp") || boost::ends_with(it->path().filename().string(), ".esm"))) {
                            loadOrder.emplace(boost::filesystem::last_write_time(it->path()), it->path().filename().string());
                        }
                    }
                    for (const auto& plugin : loadOrder)
                        actual.push_back(plugin.second);
                }
                else {
                    boost::filesystem::ifstream in(localPath / "loadorder.txt");
                    while (in) {
                        std::string line;
                        std::getline(in, line);

                        if (!line.empty())
                            actual.push_back(line);
                    }
                }

                return actual;
            }

            inline std::vector<std::string> getInitialLoadOrder() const {
                return std::vector<std::string>({
                    masterFile,
                    blankEsm,
                    blankDifferentEsm,
                    blankMasterDependentEsm,
                    blankDifferentMasterDependentEsm,
                    blankEsp,
                    blankDifferentEsp,
                    blankMasterDependentEsp,
                    blankDifferentMasterDependentEsp,
                    blankPluginDependentEsp,
                    blankDifferentPluginDependentEsp,
                });
            }

            inline std::unordered_set<std::string> getInitialActivePlugins() const {
                return std::unordered_set<std::string>({
                    masterFile,
                    blankEsm,
                });
            }

            const boost::filesystem::path missingPath;
            const boost::filesystem::path dataPath;
            const boost::filesystem::path localPath;

            const std::string masterFile;
            const std::string missingEsp;
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

            const uint32_t blankEsmCrc;

        private:
            inline boost::filesystem::path getLocalPath() const {
                if (GetParam() == GameSettings::tes4)
                    return "./local/Oblivion";
                else
                    return "./local/Skyrim";
            }

            inline boost::filesystem::path getPluginsPath() const {
                if (GetParam() == GameSettings::tes4)
                    return "./Oblivion/Data";
                else
                    return "./Skyrim/Data";
            }

            inline std::string getMasterFile() const {
                if (GetParam() == GameSettings::tes4)
                    return "Oblivion.esm";
                else if (GetParam() == GameSettings::tes5)
                    return "Skyrim.esm";
                else if (GetParam() == GameSettings::fo3)
                    return "Fallout3.esm";
                else if (GetParam() == GameSettings::fonv)
                    return "FalloutNV.esm";
                else
                    return "Fallout4.esm";
            }

            inline uint32_t getBlankEsmCrc() const {
                if (GetParam() == GameSettings::tes4)
                    return 0x374E2A6F;
                else
                    return 0x187BE342;
            }

            inline void setLoadOrder(const std::vector<std::string>& loadOrder) {
                if (isLoadOrderTimestampBased(GetParam())) {
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
                }
                else {
                    boost::filesystem::ofstream out(localPath / "loadorder.txt");
                    for (const auto &plugin : loadOrder)
                        out << plugin << std::endl;
                }
            }

            inline void setActivePlugins(const std::unordered_set<std::string>& activePlugins) {
                boost::filesystem::ofstream out(localPath / "plugins.txt");
                for (const auto &plugin : activePlugins)
                    out << plugin << std::endl;
            }

            inline static bool isLoadOrderTimestampBased(unsigned int gameId) {
                return gameId == GameSettings::tes4 || gameId == GameSettings::fo3 || gameId == GameSettings::fonv;
            }
        };
    }
}

#endif
