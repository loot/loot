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

#ifndef LOOT_TEST_FIXTURES
#define LOOT_TEST_FIXTURES

#include "printers.h"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

namespace loot {
    namespace test {
        class GenericGameTest : public ::testing::Test {
        protected:
            GenericGameTest(const std::string& gameName)
                : dataPath("./" + gameName + "/Data"),
                localPath("./local/" + gameName),
                missingPath("./missing"),
                masterFile(gameName + ".esm"),
                masterlistPath(localPath / "masterlist.yaml"),
                userlistPath(localPath / "userlist.yaml"),
                resourcePath(dataPath / "resource" / "detail" / "resource.txt") {}

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
                boost::filesystem::ofstream out(dataPath / "EmptyFile.esm");
                out.close();
                ASSERT_TRUE(boost::filesystem::exists(dataPath / "EmptyFile.esm"));

                // Write out an empty resource file.
                ASSERT_NO_THROW(boost::filesystem::create_directories(resourcePath.parent_path()));
                out.open(resourcePath);
                out.close();
                ASSERT_TRUE(boost::filesystem::exists(resourcePath));

                // Write out an non-empty, non-plugin file.
                out.open(dataPath / "NotAPlugin.esm");
                out << "This isn't a valid plugin file.";
                out.close();
                ASSERT_TRUE(boost::filesystem::exists(dataPath / "NotAPlugin.esm"));

                // LOOT expects the game master file to be present, so mock it.
                ASSERT_FALSE(boost::filesystem::exists(dataPath / masterFile));
                ASSERT_NO_THROW(boost::filesystem::copy_file(dataPath / "Blank.esm", dataPath / masterFile));
                ASSERT_TRUE(boost::filesystem::exists(dataPath / masterFile));

                setLoadOrder();

                // Set Oblivion's active plugins to a known list before running the test.
                boost::filesystem::ofstream activePlugins(localPath / "plugins.txt");
                activePlugins
                    << masterFile << std::endl
                    << "Blank.esm" << std::endl
                    << "Blank - Different Master Dependent.esp" << std::endl;
                activePlugins.close();
            }

            inline virtual void TearDown() {
                // Unghost the ghosted plugin.
                ASSERT_TRUE(boost::filesystem::exists(dataPath / "Blank - Master Dependent.esm.ghost"));
                ASSERT_NO_THROW(boost::filesystem::rename(dataPath / "Blank - Master Dependent.esm.ghost", dataPath / "Blank - Master Dependent.esm"));
                ASSERT_FALSE(boost::filesystem::exists(dataPath / "Blank - Master Dependent.esm.ghost"));

                // Delete generated files.
                ASSERT_NO_THROW(boost::filesystem::remove(dataPath / "EmptyFile.esm"));
                ASSERT_NO_THROW(boost::filesystem::remove(resourcePath));
                ASSERT_NO_THROW(boost::filesystem::remove(dataPath / "NotAPlugin.esm"));
                ASSERT_FALSE(boost::filesystem::exists(dataPath / "EmptyFile.esm"));
                ASSERT_FALSE(boost::filesystem::exists(resourcePath));
                ASSERT_FALSE(boost::filesystem::exists(dataPath / "NotAPlugin.esm"));

                // Masterlist & userlist may have been created during test, so delete them.
                ASSERT_NO_THROW(boost::filesystem::remove(masterlistPath));
                ASSERT_NO_THROW(boost::filesystem::remove(userlistPath));

                // Also remove the ".git" folder if it has been created.
                ASSERT_NO_THROW(boost::filesystem::remove_all(localPath / ".git"));

                // Delete the mock Oblivion.esm.
                ASSERT_TRUE(boost::filesystem::exists(dataPath / masterFile));
                ASSERT_NO_THROW(boost::filesystem::remove(dataPath / masterFile));
                ASSERT_FALSE(boost::filesystem::exists(dataPath / masterFile));

                // Delete existing plugins.txt.
                ASSERT_NO_THROW(boost::filesystem::remove(localPath / "plugins.txt"));

                // Delete loadorder.txt if it exists.
                ASSERT_NO_THROW(boost::filesystem::remove(localPath / "loadorder.txt"));
            }

            const boost::filesystem::path dataPath;
            const boost::filesystem::path localPath;
            const boost::filesystem::path missingPath;

            const std::string masterFile;

            const boost::filesystem::path masterlistPath;
            const boost::filesystem::path userlistPath;

            const boost::filesystem::path resourcePath;
        private:
            inline std::list<std::string> GetExpectedSortedOrder() const {
                return std::list<std::string>({
                    masterFile,
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
                });
            }

            inline void setLoadOrder() {
                if (isTimestampLoadOrderMethod()) {
                    time_t modificationTime = time(NULL);  // Current time.
                    for (const auto &plugin : GetExpectedSortedOrder()) {
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
                    boost::filesystem::ofstream loadOrder(localPath / "loadorder.txt");
                    for (const auto &plugin : GetExpectedSortedOrder())
                        loadOrder << plugin << std::endl;
                    loadOrder.close();
                }
            }

            inline bool isTimestampLoadOrderMethod() {
                return masterFile == "Oblivion.esm";
            }
        };

        class OblivionTest : public GenericGameTest {
        protected:
            OblivionTest() : GenericGameTest("Oblivion") {}
        };

        class SkyrimTest : public GenericGameTest {
        protected:
            SkyrimTest() : GenericGameTest("Skyrim") {}

            inline virtual void SetUp() {
                GenericGameTest::SetUp();

                ASSERT_TRUE(boost::filesystem::exists(dataPath / "Blank.bsa"));
                ASSERT_TRUE(boost::filesystem::exists(dataPath / "Blank.bsl"));
            }
        };
    }
}

#endif
