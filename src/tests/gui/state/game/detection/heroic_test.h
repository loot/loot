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

#ifndef LOOT_TESTS_GUI_STATE_GAME_DETECTION_HEROIC_TEST
#define LOOT_TESTS_GUI_STATE_GAME_DETECTION_HEROIC_TEST

#include "gui/state/game/detection/heroic.h"
#include "tests/common_game_test_fixture.h"

namespace loot::test {
#ifdef _WIN32
TEST(GetHeroicGamesLauncherConfigPaths, shouldReturnTheHeroicAppDataPath) {
  const auto paths = loot::heroic::getHeroicGamesLauncherConfigPaths();
  const auto expectedPath =
      getLocalAppDataPath().parent_path() / "Roaming" / "heroic";

  ASSERT_EQ(1, paths.size());
  EXPECT_EQ(expectedPath, paths[0].u8string());
}
#else
TEST(GetHeroicGamesLauncherConfigPaths,
     shouldReturnTheHeroicFolderInUserConfig) {
  const auto paths = loot::heroic::getHeroicGamesLauncherConfigPaths();

  const auto home = std::string(getenv("HOME"));

  const auto xgConfigHome = getenv("XDG_CONFIG_HOME");

  const auto config = xgConfigHome == nullptr ? home + "/.config" : xgConfigHome;

  ASSERT_EQ(2, paths.size());
  EXPECT_EQ(config + "/heroic", paths[0].u8string());
  EXPECT_EQ(home + "/.var/app/com.heroicgameslauncher.hgl/config/heroic",
            paths[1].u8string());
}
#endif

class HeroicTest : public ::testing::Test {
public:
  HeroicTest() :
      rootPath_(getTempPath()),
      gogInstalledPath_(rootPath_ / "gog_store" / "installed.json"),
      egsInstalledPath_(rootPath_ / "legendaryConfig" / "legendary" /
                        "installed.json"),
      gamesConfigPath_(rootPath_ / "GamesConfig") {}

protected:
  void SetUp() override {
    std::filesystem::create_directories(gogInstalledPath_.parent_path());
    std::filesystem::create_directories(egsInstalledPath_.parent_path());
    std::filesystem::create_directories(gamesConfigPath_);
  }

  void TearDown() override { std::filesystem::remove_all(rootPath_); }

  std::filesystem::path rootPath_;
  std::filesystem::path gogInstalledPath_;
  std::filesystem::path egsInstalledPath_;
  std::filesystem::path gamesConfigPath_;
};

class Heroic_GetInstalledGogGamesTest : public HeroicTest {};

TEST_F(Heroic_GetInstalledGogGamesTest,
       shouldReturnNoInstallsIfTheInstalledJsonFileDoesNotExist) {
  const auto installs = loot::heroic::getInstalledGogGames(rootPath_);

  EXPECT_TRUE(installs.empty());
}

TEST_F(
    Heroic_GetInstalledGogGamesTest,
    shouldReturnNoInstallsIfThereIsAnInstalledJsonFileButItHasNoSupportedGames) {
  std::ofstream out(gogInstalledPath_);
  out << R"test({"installed": [{
	"install_path": "/home/user/Games/Heroic/Oblivion",
	"appName": "unsupported",
}]})test";
  out.close();

  const auto installs = loot::heroic::getInstalledGogGames(rootPath_);

  EXPECT_TRUE(installs.empty());
}

TEST_F(Heroic_GetInstalledGogGamesTest, shouldReturnAllSupportedGameInstalls) {
  std::ofstream out(gogInstalledPath_);
  out << R"test({"installed": [
  {
	  "install_path": "/home/user/Games/Heroic/Oblivion",
	  "appName": "unsupported"
  },
  {
	  "install_path": "/home/user/Games/Heroic/Morrowind",
	  "appName": "1435828767"
  },
  {
	  "install_path": "/home/user/Games/Heroic/Fallout 3",
	  "appName": "1454315831"
  }
]})test";
  out.close();

  const auto installs = loot::heroic::getInstalledGogGames(rootPath_);

  ASSERT_EQ(2, installs.size());
  EXPECT_EQ(GameId::tes3, installs[0].gameId);
  EXPECT_EQ("1435828767", installs[0].appName);
  EXPECT_EQ(std::filesystem::u8path("/home/user/Games/Heroic/Morrowind"),
            installs[0].installPath);
  EXPECT_EQ(GameId::fo3, installs[1].gameId);
  EXPECT_EQ("1454315831", installs[1].appName);
  EXPECT_EQ(std::filesystem::u8path("/home/user/Games/Heroic/Fallout 3"),
            installs[1].installPath);
}

class Heroic_GetInstalledEgsGamesTest : public HeroicTest {};

TEST_F(Heroic_GetInstalledEgsGamesTest,
       shouldReturnNoInstallsIfTheInstalledJsonFileDoesNotExist) {
  const auto installs = loot::heroic::getInstalledEgsGames(rootPath_);

  EXPECT_TRUE(installs.empty());
}

TEST_F(
    Heroic_GetInstalledEgsGamesTest,
    shouldReturnNoInstallsIfThereIsAnInstalledJsonFileButItHasNoSupportedGames) {
  std::ofstream out(egsInstalledPath_);
  out << R"test({"installed": [{
	"install_path": "/home/user/Games/Heroic/Oblivion",
	"app_name": "unsupported"
}]})test";
  out.close();

  const auto installs = loot::heroic::getInstalledEgsGames(rootPath_);

  EXPECT_TRUE(installs.empty());
}

TEST_F(Heroic_GetInstalledEgsGamesTest, shouldReturnAllSupportedGameInstalls) {
  std::ofstream out(egsInstalledPath_);
  out << R"test({
  "unsupported": {
	  "install_path": "/home/user/Games/Heroic/Oblivion",
	  "app_name": "unsupported"
  },
  "ac82db5035584c7f8a2c548d98c86b2c": {
	  "install_path": "/home/user/Games/Heroic/Skyrim Special Edition",
	  "app_name": "ac82db5035584c7f8a2c548d98c86b2c"
  },
  "adeae8bbfc94427db57c7dfecce3f1d4": {
	  "install_path": "/home/user/Games/Heroic/Fallout 3",
	  "app_name": "adeae8bbfc94427db57c7dfecce3f1d4"
  }
})test";
  out.close();

  const auto installs = loot::heroic::getInstalledEgsGames(rootPath_);

  ASSERT_EQ(2, installs.size());
  EXPECT_EQ(GameId::tes5se, installs[0].gameId);
  EXPECT_EQ("ac82db5035584c7f8a2c548d98c86b2c", installs[0].appName);
  EXPECT_EQ(
      std::filesystem::u8path("/home/user/Games/Heroic/Skyrim Special Edition"),
      installs[0].installPath);
  EXPECT_EQ(GameId::fo3, installs[1].gameId);
  EXPECT_EQ("adeae8bbfc94427db57c7dfecce3f1d4", installs[1].appName);
  EXPECT_EQ(std::filesystem::u8path("/home/user/Games/Heroic/Fallout 3"),
            installs[1].installPath);
}

class Heroic_GetGameLocalPathTest : public HeroicTest {};

TEST_F(Heroic_GetGameLocalPathTest,
       shouldThrowIfTheGameConfigPathDoesNotExist) {
  EXPECT_THROW(
      loot::heroic::getGameLocalPath(rootPath_, "1454315831", "Fallout3"),
      std::runtime_error);
}

TEST_F(Heroic_GetGameLocalPathTest, shouldThrowIfTheExpectedAppNameIsNotFound) {
  std::ofstream out(gamesConfigPath_ / "1454315831.json");
  out << R"test({"1435828767": {
"winePrefix": "/home/user/Games/Heroic/default"
}})test";
  out.close();

  EXPECT_THROW(
      loot::heroic::getGameLocalPath(rootPath_, "1454315831", "Fallout3"),
      std::runtime_error);
}

TEST_F(Heroic_GetGameLocalPathTest, shouldThrowIfThereIsNoWinePrefix) {
  std::ofstream out(gamesConfigPath_ / "1454315831.json");
  out << R"test({"1454315831": {}})test";
  out.close();

  EXPECT_THROW(
      loot::heroic::getGameLocalPath(rootPath_, "1454315831", "Fallout3"),
      std::runtime_error);
}

TEST_F(Heroic_GetGameLocalPathTest,
       shouldReturnTheLocalPathConstructedFromTheWinePrefixAndGivenGameFolder) {
  std::ofstream out(gamesConfigPath_ / "1454315831.json");
  out << R"test({"1454315831": {
"winePrefix": "/home/user/Games/Heroic/default"
}})test";
  out.close();

  const auto folder = "Fallout3";
  const auto path =
      loot::heroic::getGameLocalPath(rootPath_, "1454315831", folder);

  const auto expectedPath = std::filesystem::u8path(
                                "/home/user/Games/Heroic/default/pfx/drive_c/"
                                "users/steamuser/AppData/Local") /
                            folder;
  EXPECT_EQ(expectedPath.generic_u8string(), path.generic_u8string());
}

class Heroic_FindGameInstallsTest : public HeroicTest {};

TEST_F(Heroic_FindGameInstallsTest,
       shouldReturnNoInstallsIfThereIsAGogGameInstallPathButItIsInvalid) {
  std::ofstream out(gogInstalledPath_);
  out << R"test({"installed": [{
	"install_path": "invalid path",
	"appName": "1458058109"
}]})test";
  out.close();

  const auto installs = loot::heroic::findGameInstalls(rootPath_, {});

  EXPECT_TRUE(installs.empty());
}

TEST_F(Heroic_FindGameInstallsTest,
       shouldReturnNoInstallsIfThereIsAnEgsGameInstallPathButItIsInvalid) {
  std::ofstream out(egsInstalledPath_);
  out << R"test({
  "adeae8bbfc94427db57c7dfecce3f1d4": {
	  "install_path": "invalid path",
	  "app_name": "adeae8bbfc94427db57c7dfecce3f1d4"
  }
})test";
  out.close();

  const auto installs = loot::heroic::findGameInstalls(rootPath_, {});

  EXPECT_TRUE(installs.empty());
}

#ifndef _WIN32
TEST_F(Heroic_FindGameInstallsTest,
       shouldReturnValidMorrowindInstallEvenIfThereIsNoGameConfigFile) {
  const auto morrowindPath = rootPath_ / "Morrowind";
  const auto morrowindEsmPath = morrowindPath / "Data Files" / "Morrowind.esm";
  std::filesystem::create_directories(morrowindEsmPath.parent_path());
  touch(morrowindEsmPath);
  touch(morrowindPath / "Morrowind.exe");

  std::ofstream out(gogInstalledPath_);
  out << R"test({"installed": [{
	"appName": "1435828767",
	"install_path": ")test";
  out << morrowindPath.generic_u8string() << "\"}]}";
  out.close();

  const auto installs = loot::heroic::findGameInstalls(rootPath_, {});

  ASSERT_EQ(1, installs.size());
  EXPECT_EQ(GameId::tes3, installs[0].gameId);
  EXPECT_EQ(InstallSource::gog, installs[0].source);
  EXPECT_EQ(morrowindPath, installs[0].installPath);
  EXPECT_TRUE(installs[0].localPath.empty());
}

TEST_F(
    Heroic_FindGameInstallsTest,
    shouldReturnNoInstallsIfThereIsANonMorrowindInstallWithNoGameConfigFile) {
  const auto oblivionPath = rootPath_ / "Oblivion";
  const auto oblivionEsmPath = oblivionPath / "Data" / "Oblivion.esm";
  std::filesystem::create_directories(oblivionEsmPath.parent_path());
  touch(oblivionEsmPath);
  touch(oblivionPath / "Oblivion.exe");

  std::ofstream out(gogInstalledPath_);
  out << R"test({"installed": [{
	"appName": "1458058109",
	"install_path": ")test";
  out << oblivionPath.generic_u8string() << "\"}]}";
  out.close();

  const auto installs = loot::heroic::findGameInstalls(rootPath_, {});

  EXPECT_TRUE(installs.empty());
}

TEST_F(
    Heroic_FindGameInstallsTest,
    shouldReturnNoInstallsIfThereIsANonMorrowindInstallWithAnInvalidGameConfigFile) {
  const auto oblivionPath = rootPath_ / "Oblivion";
  const auto oblivionEsmPath = oblivionPath / "Data" / "Oblivion.esm";
  std::filesystem::create_directories(oblivionEsmPath.parent_path());
  touch(oblivionEsmPath);
  touch(oblivionPath / "Oblivion.exe");

  std::ofstream out(gogInstalledPath_);
  out << R"test({"installed": [{
	"appName": "1458058109",
	"install_path": ")test";
  out << oblivionPath.generic_u8string() << "\"}]}";
  out.close();

  out = std::ofstream(gamesConfigPath_ / "1458058109.json");
  out << R"test({"invalid": {
"winePrefix": "/home/user/Games/Heroic/Oblivion"
}})test";
  out.close();

  const auto installs = loot::heroic::findGameInstalls(rootPath_, {});

  EXPECT_TRUE(installs.empty());
}
#endif

TEST_F(Heroic_FindGameInstallsTest, shouldReturnValidGogAndEgsInstalls) {
  const auto oblivionPath = rootPath_ / "Oblivion";
  const auto oblivionEsmPath = oblivionPath / "Data" / "Oblivion.esm";
  std::filesystem::create_directories(oblivionEsmPath.parent_path());
  touch(oblivionEsmPath);
  touch(oblivionPath / "Oblivion.exe");

  const auto fallout3Path = rootPath_ / "Fallout 3" / "Fallout 3 GOTY English";
  const auto fallout3EsmPath = fallout3Path / "Data" / "Fallout3.esm";
  std::filesystem::create_directories(fallout3EsmPath.parent_path());
  touch(fallout3EsmPath);
  touch(fallout3Path / "Fallout3.exe");

  std::ofstream out(gogInstalledPath_);
  out << R"test({"installed": [{
	"appName": "1458058109",
	"install_path": ")test";
  out << oblivionPath.generic_u8string() << "\"}]}";
  out.close();

  out = std::ofstream(egsInstalledPath_);
  out << R"test({
  "adeae8bbfc94427db57c7dfecce3f1d4": {
	  "app_name": "adeae8bbfc94427db57c7dfecce3f1d4",
	  "install_path": ")test";
  out << fallout3Path.parent_path().generic_u8string() << "\"}}";
  out.close();

#ifndef _WIN32
  out = std::ofstream(gamesConfigPath_ / "1458058109.json");
  out << R"test({"1458058109": {
"winePrefix": "/home/user/Games/Heroic/Oblivion"
}})test";
  out.close();

  out =
      std::ofstream(gamesConfigPath_ / "adeae8bbfc94427db57c7dfecce3f1d4.json");
  out << R"test({"adeae8bbfc94427db57c7dfecce3f1d4": {
"winePrefix": "/home/user/Games/Heroic/Fallout 3"
}})test";
  out.close();
#endif

  const auto installs = loot::heroic::findGameInstalls(rootPath_, {});

  ASSERT_EQ(2, installs.size());
  EXPECT_EQ(GameId::tes4, installs[0].gameId);
  EXPECT_EQ(InstallSource::gog, installs[0].source);
  EXPECT_EQ(oblivionPath, installs[0].installPath);
  EXPECT_EQ(GameId::fo3, installs[1].gameId);
  EXPECT_EQ(InstallSource::epic, installs[1].source);
  EXPECT_EQ(fallout3Path, installs[1].installPath);

#ifdef _WIN32
  EXPECT_TRUE(installs[0].localPath.empty());
  EXPECT_TRUE(installs[1].localPath.empty());
#else
  EXPECT_EQ(
      "/home/user/Games/Heroic/Oblivion/pfx/drive_c/users/steamuser/AppData/"
      "Local/Oblivion",
      installs[0].localPath.generic_u8string());
  EXPECT_EQ(
      "/home/user/Games/Heroic/Fallout "
      "3/pfx/drive_c/users/steamuser/AppData/Local/Fallout3",
      installs[1].localPath.generic_u8string());
#endif
}

TEST_F(Heroic_FindGameInstallsTest,
       shouldChooseLocalisedGameInstallPathsBasedOnPreferredLanguages) {
  const auto createInstall = [&](const std::filesystem::path& path) {
    const auto fallout3EsmPath = path / "Data" / "Fallout3.esm";
    std::filesystem::create_directories(fallout3EsmPath.parent_path());
    touch(fallout3EsmPath);
    touch(path / "Fallout3.exe");
  };
  const auto fallout3EnPath =
      rootPath_ / "Fallout 3" / "Fallout 3 GOTY English";
  const auto fallout3FrPath = rootPath_ / "Fallout 3" / "Fallout 3 GOTY French";
  createInstall(fallout3EnPath);
  createInstall(fallout3FrPath);

  std::ofstream out(egsInstalledPath_);
  out << R"test({
  "adeae8bbfc94427db57c7dfecce3f1d4": {
	  "app_name": "adeae8bbfc94427db57c7dfecce3f1d4",
	  "install_path": ")test";
  out << fallout3EnPath.parent_path().generic_u8string() << "\"}}";
  out.close();

#ifndef _WIN32
  out =
      std::ofstream(gamesConfigPath_ / "adeae8bbfc94427db57c7dfecce3f1d4.json");
  out << R"test({"adeae8bbfc94427db57c7dfecce3f1d4": {
"winePrefix": "/home/user/Games/Heroic/Fallout 3"
}})test";
  out.close();
#endif

  const auto installs = loot::heroic::findGameInstalls(rootPath_, {"fr", "en"});

  ASSERT_EQ(1, installs.size());
  EXPECT_EQ(GameId::fo3, installs[0].gameId);
  EXPECT_EQ(InstallSource::epic, installs[0].source);
  EXPECT_EQ(fallout3FrPath, installs[0].installPath);

#ifdef _WIN32
  EXPECT_TRUE(installs[0].localPath.empty());
#else
  EXPECT_EQ(
      "/home/user/Games/Heroic/Fallout "
      "3/pfx/drive_c/users/steamuser/AppData/Local/Fallout3",
      installs[0].localPath.generic_u8string());
#endif
}
}

#endif
