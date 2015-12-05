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

#ifndef LOOT_TEST_BACKEND_HELPERS_VERSION
#define LOOT_TEST_BACKEND_HELPERS_VERSION

#include "backend/globals.h"
#include "backend/helpers/version.h"
#include "tests/fixtures.h"

namespace loot {
    namespace test {
        TEST(Version, ConstructorsAndDataAccess) {
            Version version;
            EXPECT_EQ("", version.AsString());

            version = Version(std::string("5"));
            EXPECT_EQ("5", version.AsString());

#ifdef _WIN32
            // Use the API DLL built.
            version = Version(boost::filesystem::path("loot32.dll"));
            std::string expected(
                std::to_string(g_version_major) + "." +
                std::to_string(g_version_minor) + "." +
                std::to_string(g_version_patch) + ".0"
                );
            EXPECT_EQ(expected, version.AsString());
#endif
        }

        TEST(Version, GreaterThan) {
            Version version1, version2;
            EXPECT_FALSE(version1 > version2);
            EXPECT_FALSE(version2 > version1);

            version1 = Version(std::string("5"));
            version2 = Version(std::string("5"));
            EXPECT_FALSE(version1 > version2);
            EXPECT_FALSE(version2 > version1);

            version1 = Version(std::string("4"));
            version2 = Version(std::string("5"));
            EXPECT_FALSE(version1 > version2);
            EXPECT_TRUE(version2 > version1);
        }

        TEST(Version, LessThan) {
            Version version1, version2;
            EXPECT_FALSE(version1 < version2);
            EXPECT_FALSE(version2 < version1);

            version1 = Version(std::string("5"));
            version2 = Version(std::string("5"));
            EXPECT_FALSE(version1 < version2);
            EXPECT_FALSE(version2 < version1);

            version1 = Version(std::string("4"));
            version2 = Version(std::string("5"));
            EXPECT_TRUE(version1 < version2);
            EXPECT_FALSE(version2 < version1);
        }

        TEST(Version, GreaterThanEqual) {
            Version version1, version2;
            EXPECT_TRUE(version1 >= version2);
            EXPECT_TRUE(version2 >= version1);

            version1 = Version(std::string("5"));
            version2 = Version(std::string("5"));
            EXPECT_TRUE(version1 >= version2);
            EXPECT_TRUE(version2 >= version1);

            version1 = Version(std::string("4"));
            version2 = Version(std::string("5"));
            EXPECT_FALSE(version1 >= version2);
            EXPECT_TRUE(version2 >= version1);
        }

        TEST(Version, LessThanEqual) {
            Version version1, version2;
            EXPECT_TRUE(version1 <= version2);
            EXPECT_TRUE(version2 <= version1);

            version1 = Version(std::string("5"));
            version2 = Version(std::string("5"));
            EXPECT_TRUE(version1 <= version2);
            EXPECT_TRUE(version2 <= version1);

            version1 = Version(std::string("4"));
            version2 = Version(std::string("5"));
            EXPECT_TRUE(version1 <= version2);
            EXPECT_FALSE(version2 <= version1);
        }

        TEST(Version, Equal) {
            Version version1, version2;
            EXPECT_TRUE(version1 == version2);
            EXPECT_TRUE(version2 == version1);

            version1 = Version(std::string("5"));
            version2 = Version(std::string("5"));
            EXPECT_TRUE(version1 == version2);
            EXPECT_TRUE(version2 == version1);

            version1 = Version(std::string("4"));
            version2 = Version(std::string("5"));
            EXPECT_FALSE(version1 == version2);
            EXPECT_FALSE(version2 == version1);
        }

        TEST(Version, NotEqual) {
            Version version1, version2;
            EXPECT_FALSE(version1 != version2);
            EXPECT_FALSE(version2 != version1);

            version1 = Version(std::string("5"));
            version2 = Version(std::string("5"));
            EXPECT_FALSE(version1 != version2);
            EXPECT_FALSE(version2 != version1);

            version1 = Version(std::string("4"));
            version2 = Version(std::string("5"));
            EXPECT_TRUE(version1 != version2);
            EXPECT_TRUE(version2 != version1);
        }
    }
}

#endif
