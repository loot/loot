/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014 WrinklyNinja

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

#ifndef LOOT_TESTS_GUI_STATE_GAME_DETECTION_STEAM_TEST
#define LOOT_TESTS_GUI_STATE_GAME_DETECTION_STEAM_TEST

#include "gui/state/game/detection/steam.h"
#include "tests/common_game_test_fixture.h"
#include "tests/gui/state/game/detection/test_registry.h"

namespace loot::test {
#ifdef _WIN32
TEST(GetSteamInstallPath, shouldReturnNulloptWhenTheRegistryEntryDoesNotExist) {
  const auto path = loot::steam::GetSteamInstallPath(TestRegistry());

  EXPECT_FALSE(path.has_value());
}

TEST(GetSteamInstallPath, shouldReturnTheStoredPathWhenTheRegistryEntryExists) {
  TestRegistry registry;
  const auto expectedPath = "C:\\Program Files (x86)\\Steam";
  registry.SetStringValue("Software\\Valve\\Steam", expectedPath);

  const auto path = loot::steam::GetSteamInstallPath(registry);

  ASSERT_TRUE(path.has_value());
  EXPECT_EQ(expectedPath, path.value().u8string());
}
#else
TEST(GetSteamInstallPath, shouldReturnTheSteamFolderInUserLocalShare) {
  const auto path = loot::steam::GetSteamInstallPath(TestRegistry());

  ASSERT_TRUE(path.has_value());
  EXPECT_EQ(std::string(getenv("HOME")) + "/.local/share/Steam",
            path.value().u8string());
}
#endif

class FilesystemTest : public ::testing::Test {
public:
  FilesystemTest() : rootPath_(getTempPath()) {}

protected:
  void SetUp() override { std::filesystem::create_directories(rootPath_); }

  void TearDown() override { std::filesystem::remove_all(rootPath_); }

  const std::filesystem::path rootPath_;
};

class GetSteamAppManifestPathsTest : public FilesystemTest {
public:
  GetSteamAppManifestPathsTest() :
      filePath_(rootPath_ / "config" / "libraryfolders.vdf") {}

protected:
  void SetUp() override {
    FilesystemTest::SetUp();

    std::filesystem::create_directories(filePath_.parent_path());
  }

  const std::filesystem::path filePath_;
};

TEST_F(GetSteamAppManifestPathsTest,
       shouldReturnAnEmptyVectorIfTheLibraryFoldersVdfFileDoesNotExist) {
  const auto paths = loot::steam::GetSteamAppManifestPaths(rootPath_);

  EXPECT_TRUE(paths.empty());
}

TEST_F(
    GetSteamAppManifestPathsTest,
    shouldReturnAppManifestPathsConstructedFromTheLibraryPathsAndAppIdsInTheVdf) {
  std::ofstream out(filePath_);
  out << R"test(
"libraryfolders"
{
	"0"
	{
		"path"		"C:\\Program Files (x86)\\Steam"
		"label"		""
		"contentid"		"1137098260226172853"
		"totalsize"		"0"
		"update_clean_bytes_tally"		"3302272801"
		"time_last_update_corruption"		"0"
		"apps"
		{
			"228980"		"181169252"
		}
	}
	"1"
	{
		"path"		"D:\\Games\\Steam"
		"label"		""
		"contentid"		"7286921146364631392"
		"totalsize"		"252496048128"
		"update_clean_bytes_tally"		"66203891438"
		"time_last_update_corruption"		"1637608828"
		"apps"
		{
			"203770"		"2554786621"
			"221640"		"22708311"
			"226840"		"4035469646"
			"489830"		"15345519558"
			"508440"		"4782028087"
			"736260"		"111159842"
			"763890"		"2204294236"
			"1985690"		"1902985401"
		}
	}
}
)test";
  out.close();

  const auto paths = loot::steam::GetSteamAppManifestPaths(rootPath_);

  ASSERT_EQ(1, paths.size());
  EXPECT_EQ("D:\\Games\\Steam\\steamapps\\appmanifest_489830.acf",
            paths[0].u8string());
}

class Steam_FindGameInstallTest : public FilesystemTest {
public:
  Steam_FindGameInstallTest() : filePath_(rootPath_ / "appmanifest.acf") {}

protected:
  const std::filesystem::path filePath_;
};

TEST_F(Steam_FindGameInstallTest, shouldReturnNulloptIfTheAcfFileDoesNotExist) {
  const auto install = loot::steam::FindGameInstall(filePath_);

  EXPECT_FALSE(install.has_value());
}

TEST_F(Steam_FindGameInstallTest,
       shouldReturnNulloptIfTheAcfFileDoesNotContainAnAppId) {
  std::ofstream out(filePath_);
  out << R"test(
"AppState"
{
	"Universe"		"1"
	"LauncherPath"		"C:\\Program Files (x86)\\Steam\\steam.exe"
	"name"		"The Elder Scrolls V: Skyrim Special Edition"
	"StateFlags"		"4"
	"installdir"		"Skyrim Special Edition"
}
)test";
  out.close();

  const auto install = loot::steam::FindGameInstall(filePath_);

  EXPECT_FALSE(install.has_value());
}

TEST_F(Steam_FindGameInstallTest,
       shouldReturnNulloptIfTheAcfFileDoesNotContainAnInstallDir) {
  std::ofstream out(filePath_);
  out << R"test(
"AppState"
{
	"appid"		"489830"
	"Universe"		"1"
	"LauncherPath"		"C:\\Program Files (x86)\\Steam\\steam.exe"
	"name"		"The Elder Scrolls V: Skyrim Special Edition"
	"StateFlags"		"4"
}
)test";
  out.close();

  const auto install = loot::steam::FindGameInstall(filePath_);

  EXPECT_FALSE(install.has_value());
}

TEST_F(Steam_FindGameInstallTest,
       shouldReturnNulloptIfTheAppIdIsNotOfASupportedGame) {
  std::ofstream out(filePath_);
  out << R"test(
"AppState"
{
	"appid"		"736260"
	"Universe"		"1"
	"LauncherPath"		"C:\\Program Files (x86)\\Steam\\steam.exe"
	"name"		"Baba Is You"
	"StateFlags"		"4"
	"installdir"		"Baba Is You"
}
)test";
  out.close();

  const auto install = loot::steam::FindGameInstall(filePath_);

  EXPECT_FALSE(install.has_value());
}

TEST_F(Steam_FindGameInstallTest,
       shouldReturnNulloptIfTheInstallDirIsNotAValidGameInstall) {
  std::ofstream out(filePath_);
  out << R"test(
"AppState"
{
	"appid"		"489830"
	"installdir"		"Skyrim Special Edition"
}
)test";
  out.close();

  const auto install = loot::steam::FindGameInstall(filePath_);

  EXPECT_FALSE(install.has_value());
}

TEST_F(Steam_FindGameInstallTest,
       shouldReturnTheInstallDetailsIfTheAcfDescribesAValidSupportedGame) {
  std::ofstream out(filePath_);
  out << R"test(
"AppState"
{
	"appid"		"489830"
	"installdir"		"Skyrim Special Edition"
}
)test";
  out.close();

  const auto dataPath =
      rootPath_ / "common" / "Skyrim Special Edition" / "Data";
  std::filesystem::create_directories(dataPath);
  touch(dataPath / "Skyrim.esm");
  touch(dataPath.parent_path() / "SkyrimSE.exe");

  const auto install = loot::steam::FindGameInstall(filePath_);

  ASSERT_TRUE(install.has_value());
  EXPECT_EQ(GameId::tes5se, install.value().gameId);
  EXPECT_EQ(InstallSource::steam, install.value().source);
  EXPECT_EQ(dataPath.parent_path(), install.value().installPath);

#ifdef _WIN32
  EXPECT_TRUE(install.value().localPath.empty());
#else
  const auto localPath = std::filesystem::u8path(
      getenv("HOME") / ".local" / "share" / "Steam" / "steamapps" /
      "compatdata" / "489830" / "pfx" / "drive_c" / "users" / "steamuser" /
      "AppData" / "Local" / "Skyrim Special Edition");
  EXPECT_EQ(localPath, install.value().localPath);
#endif
}

class Steam_FindGameInstallsTest
    : public CommonGameTestFixture,
      public ::testing::WithParamInterface<GameId> {
protected:
  Steam_FindGameInstallsTest() : CommonGameTestFixture(GetParam()) {}

  std::string GetSteamGameId() {
    switch (GetParam()) {
      case GameId::tes3:
        return "22320";
      case GameId::tes4:
        return "22330";
      case GameId::nehrim:
        return "1014940";
      case GameId::tes5:
        return "72850";
      case GameId::enderal:
        return "933480";
      case GameId::tes5se:
        return "489830";
      case GameId::enderalse:
        return "976620";
      case GameId::tes5vr:
        return "611670";
      case GameId::fo3:
        return "22300";
      case GameId::fonv:
        return "22380";
      case GameId::fo4:
        return "377160";
      case GameId::fo4vr:
        return "611660";
      default:
        throw std::logic_error("Unsupported Steam game");
    }
  }
};

INSTANTIATE_TEST_SUITE_P(,
                         Steam_FindGameInstallsTest,
                         ::testing::ValuesIn(ALL_GAME_IDS));

TEST_P(Steam_FindGameInstallsTest,
       shouldReturnAnEmptyVectorIfNoRegistryEntriesExist) {
  const auto installs =
      loot::steam::FindGameInstalls(TestRegistry(), GetParam());

  EXPECT_TRUE(installs.empty());
}

TEST_P(
    Steam_FindGameInstallsTest,
    shouldReturnAnEmptyVectorIfARegistryEntryExistsButItIsNotAValidGamePath) {
  TestRegistry registry;
  const auto subKey =
      "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
      "Steam App " +
      GetSteamGameId();
  registry.SetStringValue(subKey, "invalid");

  const auto installs = loot::steam::FindGameInstalls(registry, GetParam());

  EXPECT_TRUE(installs.empty());
}

TEST_P(Steam_FindGameInstallsTest,
       shouldReturnANonEmptyVectorIfARegistryEntryExistsWithAValidGamePath) {
  auto gamePath = dataPath.parent_path();
  if (GetParam() == GameId::nehrim) {
    // Steam's version of Nehrim puts its files in a NehrimFiles subfolder,
    // so move them there.
    std::filesystem::create_directory(gamePath / "NehrimFiles");
    std::filesystem::copy(dataPath,
                          gamePath / "NehrimFiles" / dataPath.filename());
    gamePath /= "NehrimFiles";
  }

  TestRegistry registry;
  const auto subKey =
      "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
      "Steam App " +
      GetSteamGameId();
  registry.SetStringValue(subKey, dataPath.parent_path().u8string());

  const auto installs = loot::steam::FindGameInstalls(registry, GetParam());

  ASSERT_EQ(1, installs.size());
  EXPECT_EQ(GetParam(), installs[0].gameId);
  EXPECT_EQ(InstallSource::steam, installs[0].source);
  EXPECT_EQ(gamePath, installs[0].installPath);
  EXPECT_EQ("", installs[0].localPath);
}
}

#endif
