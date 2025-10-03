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
TEST(GetSteamInstallPaths,
     shouldReturnNulloptWhenTheRegistryEntryDoesNotExist) {
  const auto paths = loot::steam::GetSteamInstallPaths(TestRegistry());

  EXPECT_TRUE(paths.empty());
}

TEST(GetSteamInstallPaths,
     shouldReturnTheStoredPathWhenTheRegistryEntryExists) {
  TestRegistry registry;
  const auto expectedPath = "C:\\Program Files (x86)\\Steam";
  registry.SetStringValue("Software\\Valve\\Steam", expectedPath);

  const auto paths = loot::steam::GetSteamInstallPaths(registry);

  ASSERT_EQ(1, paths.size());
  EXPECT_EQ(expectedPath, paths[0].u8string());
}
#else
TEST(GetSteamInstallPaths, shouldReturnTheSteamFolderInUserLocalShare) {
  const auto paths = loot::steam::GetSteamInstallPaths(TestRegistry());

  const auto home = std::string(getenv("HOME"));

  ASSERT_EQ(2, paths.size());
  EXPECT_EQ(home + "/.local/share/Steam", paths[0].u8string());
  EXPECT_EQ(home + "/.var/app/com.valvesoftware.Steam/.local/share/Steam",
            paths[1].u8string());
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

class GetSteamLibraryPathsTest : public FilesystemTest {
public:
  GetSteamLibraryPathsTest() :
      filePath_(rootPath_ / "config" / "libraryfolders.vdf") {}

protected:
  void SetUp() override {
    FilesystemTest::SetUp();

    std::filesystem::create_directories(filePath_.parent_path());
  }

  const std::filesystem::path filePath_;
};

TEST_F(GetSteamLibraryPathsTest,
       shouldReturnAnEmptyVectorIfTheLibraryFoldersVdfFileDoesNotExist) {
  const auto paths = loot::steam::GetSteamLibraryPaths(rootPath_);

  EXPECT_TRUE(paths.empty());
}

TEST_F(GetSteamLibraryPathsTest,
       shouldReturnAnEmptyVectorIfTheRootNodeHasTheWrongName) {
  std::ofstream out(filePath_);
  out << R"test(
"wrong name"
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

  const auto paths = loot::steam::GetSteamLibraryPaths(rootPath_);

  EXPECT_TRUE(paths.empty());
}

TEST_F(GetSteamLibraryPathsTest, shouldSkipFoldersWithNoPathKey) {
  std::ofstream out(filePath_);
  out << R"test(
"libraryfolders"
{
	"0"
	{
		"label"		""
		"contentid"		"1137098260226172853"
		"totalsize"		"0"
		"update_clean_bytes_tally"		"3302272801"
		"time_last_update_corruption"		"0"
		"apps"
		{
			"228980"		"181169252"
			"22330"		"181169252"
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

  const auto paths = loot::steam::GetSteamLibraryPaths(rootPath_);

  ASSERT_EQ(1, paths.size());
  EXPECT_EQ(std::filesystem::u8path("D:\\Games\\Steam"), paths[0]);
}

TEST_F(GetSteamLibraryPathsTest, shouldReturnAllLibraryPathsInTheVdf) {
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
			"22330"		"181169252"
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

  const auto paths = loot::steam::GetSteamLibraryPaths(rootPath_);

  ASSERT_EQ(2, paths.size());
  EXPECT_EQ(std::filesystem::u8path("C:\\Program Files (x86)\\Steam"),
            paths[0].u8string());
  EXPECT_EQ(std::filesystem::u8path("D:\\Games\\Steam"), paths[1]);
}

TEST(GetSteamAppManifestPaths, shouldReturnAPathForEachSupportedAppId) {
  const std::filesystem::path libraryPath = "test";
  const auto paths =
      loot::steam::GetSteamAppManifestPaths(libraryPath, GameId::tes4);

  ASSERT_EQ(2, paths.size());
  EXPECT_EQ(libraryPath / "steamapps" / "appmanifest_22330.acf", paths[0]);
  EXPECT_EQ(libraryPath / "steamapps" / "appmanifest_900883.acf", paths[1]);
}

class Steam_FindGameInstallTest : public FilesystemTest {
public:
  Steam_FindGameInstallTest() :
      filePath_(rootPath_ / "appmanifest.acf"),
      dataPath_(rootPath_ / "common" / "Skyrim Special Edition" / "Data") {}

protected:
  void SetUp() override {
    FilesystemTest::SetUp();

    std::filesystem::create_directories(dataPath_);
    touch(dataPath_ / "Skyrim.esm");
    touch(dataPath_.parent_path() / "SkyrimSE.exe");
  }

  const std::filesystem::path dataPath_;
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

TEST_F(Steam_FindGameInstallTest, shouldReturnNulloptIfTheAppIdIsEmpty) {
  std::ofstream out(filePath_);
  out << R"test(
"AppState"
{
	"appid"		""
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

TEST_F(Steam_FindGameInstallTest, shouldReturnNulloptIfTheInstallDirIsEmpty) {
  std::ofstream out(filePath_);
  out << R"test(
"AppState"
{
	"appid"		"489830"
	"installdir"		""
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
	"installdir"		"invalid"
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

  const auto install = loot::steam::FindGameInstall(filePath_);

  ASSERT_TRUE(install.has_value());
  EXPECT_EQ(GameId::tes5se, install.value().gameId);
  EXPECT_EQ(InstallSource::steam, install.value().source);
  EXPECT_EQ(dataPath_.parent_path(), install.value().installPath);

#ifdef _WIN32
  EXPECT_TRUE(install.value().localPath.empty());
#else
  const auto localPath = filePath_.parent_path() / "compatdata" / "489830" /
                         "pfx" / "drive_c" / "users" / "steamuser" / "AppData" /
                         "Local" / "Skyrim Special Edition";
  EXPECT_EQ(localPath, install.value().localPath);
#endif
}

class Steam_FindGameInstallsTest
    : public CommonGameTestFixture,
      public ::testing::WithParamInterface<GameId> {
protected:
  Steam_FindGameInstallsTest() : CommonGameTestFixture(GetParam()) {}

  std::optional<std::string> GetSteamGameId() {
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
      case GameId::starfield:
        return "1716740";
      case GameId::openmw:
        return std::nullopt;
      case GameId::oblivionRemastered:
        return "2623190";
      default:
        throw std::logic_error("Unrecognised game ID");
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
  const auto steamGameId = GetSteamGameId();
  if (!steamGameId.has_value()) {
    return;
  }

  TestRegistry registry;
  const auto subKey =
      "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
      "Steam App " +
      steamGameId.value();
  registry.SetStringValue(subKey, "invalid");

  const auto installs = loot::steam::FindGameInstalls(registry, GetParam());

  EXPECT_TRUE(installs.empty());
}

TEST_P(Steam_FindGameInstallsTest,
       shouldReturnANonEmptyVectorIfARegistryEntryExistsWithAValidGamePath) {
  const auto steamGameId = GetSteamGameId();
  if (!steamGameId.has_value()) {
    return;
  }

  std::filesystem::path steamGamePath = gamePath;
  if (GetParam() == GameId::nehrim) {
    // Steam's version of Nehrim puts its files in a NehrimFiles subfolder,
    // so move them there.
    std::filesystem::create_directory(gamePath / "NehrimFiles");
    std::filesystem::copy(dataPath,
                          gamePath / "NehrimFiles" / dataPath.filename());
    steamGamePath /= "NehrimFiles";
  }

  TestRegistry registry;
  const auto subKey =
      "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
      "Steam App " +
      steamGameId.value();
  registry.SetStringValue(subKey, gamePath.u8string());

  const auto installs = loot::steam::FindGameInstalls(registry, GetParam());

  ASSERT_EQ(1, installs.size());
  EXPECT_EQ(GetParam(), installs[0].gameId);
  EXPECT_EQ(InstallSource::steam, installs[0].source);
  EXPECT_EQ(steamGamePath, installs[0].installPath);
  EXPECT_EQ("", installs[0].localPath);
}
}

#endif
