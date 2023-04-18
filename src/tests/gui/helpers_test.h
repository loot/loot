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
#ifndef LOOT_TESTS_GUI_HELPERS_TEST
#define LOOT_TESTS_GUI_HELPERS_TEST

#include <gtest/gtest.h>

#include <boost/locale.hpp>

#include "gui/helpers.h"
#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class FindXboxGamingRootPathTest : public CommonGameTestFixture {
protected:
  FindXboxGamingRootPathTest() : CommonGameTestFixture(GameId::tes3) {}
};

TEST(GetDriveRootPaths, shouldReturnNonEmptyVector) {
  EXPECT_FALSE(GetDriveRootPaths().empty());
}

TEST_F(FindXboxGamingRootPathTest,
       shouldReturnNulloptIfTheDotGamingRootFileDoesNotExist) {
  EXPECT_FALSE(FindXboxGamingRootPath(dataPath).has_value());
}

TEST_F(FindXboxGamingRootPathTest,
       shouldReturnNulloptIfDotGamingRootIsADirectory) {
  std::filesystem::create_directory(dataPath / ".GamingRoot");

  EXPECT_FALSE(FindXboxGamingRootPath(dataPath).has_value());
}

TEST_F(FindXboxGamingRootPathTest,
       shouldThrowIfDotGamingRootContainsAnOddNumberOfBytes) {
  std::ofstream out(dataPath / ".GamingRoot", std::ios::binary);
  out << "12345678901";
  out.close();

  EXPECT_THROW(FindXboxGamingRootPath(dataPath), std::runtime_error);
}

TEST_F(FindXboxGamingRootPathTest, shouldThrowIfDotGamingRootIsTooShort) {
  std::ofstream out(dataPath / ".GamingRoot", std::ios::binary);
  out << "12";
  out.close();

  EXPECT_THROW(FindXboxGamingRootPath(dataPath), std::runtime_error);
}

TEST_F(FindXboxGamingRootPathTest,
       shouldInterpretTheNinthAndFollowingBytesAsANullTerminatedUtf16LeString) {
  std::ofstream out(dataPath / ".GamingRoot", std::ios::binary);
  const char* data = "12345678t\0e\0s\0t\0 \0p\0a\0t\0h\0\0\0";
  out.write(data, 28);
  out.close();

  const auto gamingRootPath = FindXboxGamingRootPath(dataPath);
  const auto expectedPath = dataPath / "test path";

  EXPECT_EQ(expectedPath, gamingRootPath);
}

// MSVC interprets source files in the default code page, so
// for me u8"\xC3\x9C" != u8"\u00DC", which is a lot of fun.
// To avoid insanity, write non-ASCII characters as \uXXXX escapes.
// \u03a1 is greek rho uppercase 'Ρ'
// \u03c1 is greek rho lowercase 'ρ'
// \u03f1 is greek rho 'ϱ'
// \u0130 is turkish 'İ'
// \u0131 is turkish 'ı'

TEST(CompareFilenames, shouldBeCaseInsensitiveAndLocaleInvariant) {
  // ICU sees all three greek rhos as case-insensitively equal, unlike Windows.
  // A small enough deviation that it should hopefully be insignificant.

  EXPECT_EQ(0, CompareFilenames("i", "I"));
  EXPECT_EQ(-1, CompareFilenames("i", u8"\u0130"));
  EXPECT_EQ(-1, CompareFilenames("i", u8"\u0131"));
  EXPECT_EQ(-1, CompareFilenames("I", u8"\u0130"));
  EXPECT_EQ(-1, CompareFilenames("I", u8"\u0131"));
  EXPECT_EQ(-1, CompareFilenames(u8"\u0130", u8"\u0131"));
#ifdef _WIN32
  EXPECT_EQ(1, CompareFilenames(u8"\u03f1", u8"\u03a1"));
  EXPECT_EQ(1, CompareFilenames(u8"\u03f1", u8"\u03c1"));
#else
  EXPECT_EQ(0, CompareFilenames(u8"\u03f1", u8"\u03a1"));
  EXPECT_EQ(0, CompareFilenames(u8"\u03f1", u8"\u03c1"));
#endif
  EXPECT_EQ(0, CompareFilenames(u8"\u03a1", u8"\u03c1"));

  // Set locale to Turkish.
  std::locale::global(boost::locale::generator().generate("tr_TR.UTF-8"));

  EXPECT_EQ(0, CompareFilenames("i", "I"));
  EXPECT_EQ(-1, CompareFilenames("i", u8"\u0130"));
  EXPECT_EQ(-1, CompareFilenames("i", u8"\u0131"));
  EXPECT_EQ(-1, CompareFilenames("I", u8"\u0130"));
  EXPECT_EQ(-1, CompareFilenames("I", u8"\u0131"));
  EXPECT_EQ(-1, CompareFilenames(u8"\u0130", u8"\u0131"));
#ifdef _WIN32
  EXPECT_EQ(1, CompareFilenames(u8"\u03f1", u8"\u03a1"));
  EXPECT_EQ(1, CompareFilenames(u8"\u03f1", u8"\u03c1"));
#else
  EXPECT_EQ(0, CompareFilenames(u8"\u03f1", u8"\u03a1"));
  EXPECT_EQ(0, CompareFilenames(u8"\u03f1", u8"\u03c1"));
#endif
  EXPECT_EQ(0, CompareFilenames(u8"\u03a1", u8"\u03c1"));

  // Set locale to Greek.
  std::locale::global(boost::locale::generator().generate("el_GR.UTF-8"));

  EXPECT_EQ(0, CompareFilenames("i", "I"));
  EXPECT_EQ(-1, CompareFilenames("i", u8"\u0130"));
  EXPECT_EQ(-1, CompareFilenames("i", u8"\u0131"));
  EXPECT_EQ(-1, CompareFilenames("I", u8"\u0130"));
  EXPECT_EQ(-1, CompareFilenames("I", u8"\u0131"));
  EXPECT_EQ(-1, CompareFilenames(u8"\u0130", u8"\u0131"));
#ifdef _WIN32
  EXPECT_EQ(1, CompareFilenames(u8"\u03f1", u8"\u03a1"));
  EXPECT_EQ(1, CompareFilenames(u8"\u03f1", u8"\u03c1"));
#else
  EXPECT_EQ(0, CompareFilenames(u8"\u03f1", u8"\u03a1"));
  EXPECT_EQ(0, CompareFilenames(u8"\u03f1", u8"\u03c1"));
#endif
  EXPECT_EQ(0, CompareFilenames(u8"\u03a1", u8"\u03c1"));

  // Reset locale.
  std::locale::global(boost::locale::generator().generate(""));
}
}
}

#endif
