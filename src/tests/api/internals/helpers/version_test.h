/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2016    WrinklyNinja

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
<https://www.gnu.org/licenses/>.
*/

#ifndef LOOT_TESTS_BACKEND_HELPERS_VERSION_TEST
#define LOOT_TESTS_BACKEND_HELPERS_VERSION_TEST

#include "api/helpers/version.h"
#include "loot/loot_version.h"

#include <gtest/gtest.h>

namespace loot {
namespace test {
#ifdef _WIN32
TEST(Version, shouldExtractVersionFromApiDll) {
    // Use the API DLL built.
  Version version(boost::filesystem::path("loot_api.dll"));
  std::string expected(LootVersion::string() + ".0");
  EXPECT_EQ(expected, version.AsString());
}
#endif
TEST(Version, defaultConstructorShouldSetEmptyVersionString) {
  EXPECT_EQ("", Version().AsString());
}

TEST(Version, shouldExtractAVersionContainingASingleDigit) {
  Version version(std::string("5"));
  EXPECT_EQ("5", version.AsString());
}

TEST(Version, shouldExtractAVersionContainingMultipleDigits) {
  Version version(std::string("10"));
  EXPECT_EQ("10", version.AsString());
}

TEST(Version, shouldExtractAVersionContainingMultipleNumbers) {
  Version version(std::string("10.11.12.13"));
  EXPECT_EQ("10.11.12.13", version.AsString());
}

TEST(Version, shouldExtractASemanticVersion) {
  Version version(std::string("1.0.0-x.7.z.92+exp.sha.5114f85"));
  EXPECT_EQ("1.0.0-x.7.z.92", version.AsString());
}

TEST(Version, shouldExtractAPseudosemExtendedVersionStoppingAtTheFirstSpaceSeparator) {
  Version version(std::string("01.0.0_alpha:1-2 3"));
  EXPECT_EQ("01.0.0_alpha:1-2", version.AsString());
}

TEST(Version, shouldExtractAVersionSubstring) {
  Version version(std::string("v5.0"));
  EXPECT_EQ("5.0", version.AsString());
}

TEST(Version, shouldBeEmptyIfInputStringContainedNoVersion) {
  Version version(std::string("The quick brown fox jumped over the lazy dog."));
  EXPECT_EQ("", version.AsString());
}

TEST(Version, shouldExtractTimestampWithForwardslashDateSeparators) {
    // Found in a Bashed Patch. Though the timestamp isn't useful to
    // LOOT, it is semantically a version, and extracting it is far
    // easier than trying to skip it and the number of records changed.
  Version version(std::string("Updated: 10/09/2016 13:15:18\r\n\r\nRecords Changed: 43"));
  EXPECT_EQ("10/09/2016 13:15:18", version.AsString());
}

TEST(Version, shouldNotExtractTrailingPeriods) {
    // Found in <http://www.nexusmods.com/fallout4/mods/2955/>.
  Version version(std::string("Version 0.2."));
  EXPECT_EQ("0.2", version.AsString());
}

TEST(Version, shouldExtractVersionAfterTextWhenPrecededByVersionColonString) {
    // Found in <http://www.nexusmods.com/skyrim/mods/71214/>.
  std::string testText("Legendary Edition\r\n\r\nVersion: 3.0.0");
  EXPECT_EQ("3.0.0", Version(testText).AsString());
}

TEST(Version, shouldIgnoreNumbersContainingCommas) {
    // Found in <http://www.nexusmods.com/oblivion/mods/5296/>.
  std::string testText("fixing over 2,300 bugs so far! Version: 3.5.3");
  EXPECT_EQ("3.5.3", Version(testText).AsString());
}

TEST(Version, shouldExtractVersionBeforeText) {
    // Found in <http://www.nexusmods.com/fallout3/mods/19122/>.
  std::string testText("Version: 2.1 The Unofficial Fallout 3 Patch");
  EXPECT_EQ("2.1", Version(testText).AsString());
}

TEST(Version, shouldExtractVersionWithPrecedingV) {
    // Found in <http://www.nexusmods.com/oblivion/mods/22795/>.
  std::string testText("V2.11\r\n\r\n{{BASH:Invent}}");
  EXPECT_EQ("2.11", Version(testText).AsString());
}

TEST(Version, shouldExtractVersionWithPrecedingColonPeriodWhitespace) {
    // Found in <http://www.nexusmods.com/oblivion/mods/45570>.
  std::string testText("Version:. 1.09");
  EXPECT_EQ("1.09", Version(testText).AsString());
}

TEST(Version, shouldExtractVersionWithLettersImmediatelyAfterNumbers) {
    // Found in <http://www.nexusmods.com/skyrim/mods/19>.
  std::string testText("comprehensive bugfixing mod for The Elder Scrolls V: Skyrim\r\n\r\nVersion: 2.1.3b\r\n\r\n");
  EXPECT_EQ("2.1.3b", Version(testText).AsString());
}

TEST(Version, shouldExtractVersionWithPeriodAndNoPrecedingIdentifier) {
    // Found in <http://www.nexusmods.com/skyrim/mods/3863>.
  std::string testText("SkyUI 5.1");
  EXPECT_EQ("5.1", Version(testText).AsString());
}

TEST(Version, shouldNotExtractSingleDigitInSentence) {
    // Found in <http://www.nexusmods.com/skyrim/mods/4708>.
  std::string testText("Adds 8 variants of Triss Merigold's outfit from \"The Witcher 2\"");
  EXPECT_EQ("", Version(testText).AsString());
}

TEST(Version, shouldPreferVersionPrefixedNumbersOverVersionsInSentence) {
    // Found in <http://www.nexusmods.com/skyrim/mods/47327>
  std::string testText("Requires Skyrim patch 1.9.32.0.8 or greater.\n"
                       "Requires Unofficial Skyrim Legendary Edition Patch 3.0.0 or greater.\n"
                       "Version 2.0.0");
  EXPECT_EQ("2.0.0", Version(testText).AsString());
}

TEST(Version, shouldExtractSingleDigitVersionPrecededByV) {
    // Found in <http://www.nexusmods.com/skyrim/mods/19733>
  std::string testText("Immersive Armors v8 Main Plugin");
  EXPECT_EQ("8", Version(testText).AsString());
}

TEST(Version, shouldPreferVersionPrefixedNumbersOverVPrefixedNumber) {
    // Found in <http://www.nexusmods.com/skyrim/mods/43773>
  std::string testText("Compatibility patch for AOS v2.5 and True Storms v1.5 (or later),\nPatch Version: 1.0");
  EXPECT_EQ("1.0", Version(testText).AsString());
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
