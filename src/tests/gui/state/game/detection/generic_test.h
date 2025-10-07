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

#ifndef LOOT_TESTS_GUI_STATE_GAME_DETECTION_GENERIC_TEST
#define LOOT_TESTS_GUI_STATE_GAME_DETECTION_GENERIC_TEST

#include "gui/state/game/detection/generic.h"
#include "gui/state/game/detection/gog.h"
#include "tests/common_game_test_fixture.h"
#include "tests/gui/state/game/detection/test_registry.h"

namespace loot::test {
class Generic_FindGameInstallsTest
    : public CommonGameTestFixture,
      public testing::WithParamInterface<GameId> {
protected:
  Generic_FindGameInstallsTest() : CommonGameTestFixture(GetParam()) {}

  void SetUp() override {
    CommonGameTestFixture::SetUp();

    initialCurrentPath = std::filesystem::current_path();

    const auto lootPath = gamePath / "LOOT";

    // Change the current path into a game subfolder.
    std::filesystem::create_directory(lootPath);
    std::filesystem::current_path(lootPath);
  }

  void TearDown() override {
    // Restore the previous current path.
    restoreCurrentPath();

    CommonGameTestFixture::TearDown();
  }

  void restoreCurrentPath() const {
    std::filesystem::current_path(initialCurrentPath);
  }

  std::optional<std::string> getSubKey() const {
    switch (GetParam()) {
      case GameId::tes3:
        return "Software\\Bethesda Softworks\\Morrowind";
      case GameId::tes4:
        return "Software\\Bethesda Softworks\\Oblivion";
      case GameId::nehrim:
        return "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim"
               " - At Fate's Edge_is1";
      case GameId::tes5:
        return "Software\\Bethesda Softworks\\Skyrim";
      case GameId::enderal:
        return "SOFTWARE\\SureAI\\Enderal";
      case GameId::tes5se:
        return "Software\\Bethesda Softworks\\Skyrim Special Edition";
      case GameId::enderalse:
        return "SOFTWARE\\SureAI\\EnderalSE";
      case GameId::tes5vr:
        return "Software\\Bethesda Softworks\\Skyrim VR";
      case GameId::fo3:
        return "Software\\Bethesda Softworks\\Fallout3";
      case GameId::fonv:
        return "Software\\Bethesda Softworks\\FalloutNV";
      case GameId::fo4:
        return "Software\\Bethesda Softworks\\Fallout4";
      case GameId::fo4vr:
        return "Software\\Bethesda Softworks\\Fallout 4 VR";
      case GameId::starfield:
      case GameId::oblivionRemastered:
        return std::nullopt;
      case GameId::openmw:
        return "Software\\OpenMW.org\\OpenMW 0.48.0";
      default:
        throw std::logic_error("Unrecognised game ID");
    }
  }

  void createSteamFile() const {
    if (GetParam() == GameId::tes3) {
      touch(gamePath / "steam_autocloud.vdf");
    } else if (GetParam() == GameId::nehrim) {
      touch(gamePath / "steam_api.dll");
    } else if (GetParam() == GameId::starfield) {
      touch(gamePath / "steam_api64.dll");
    } else if (GetParam() == GameId::oblivionRemastered) {
      touch(gamePath / "Engine" / "Binaries" / "ThirdParty" / "Steamworks" /
            "Steamv153" / "Win64" / "steam_api64.dll");
    } else {
      touch(gamePath / "installscript.vdf");
    }
  }

private:
  std::filesystem::path initialCurrentPath;
};

// Pass an empty first argument, as it's a prefix for the test instantiation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_SUITE_P(,
                         Generic_FindGameInstallsTest,
                         ::testing::ValuesIn(ALL_GAME_IDS));

TEST_P(Generic_FindGameInstallsTest, shouldNotFindInvalidSiblingGame) {
  std::filesystem::remove_all(dataPath);

  const auto gameInstalls =
      loot::generic::findGameInstalls(TestRegistry(), GetParam());

  EXPECT_TRUE(gameInstalls.empty());
}

#ifdef _WIN32
TEST_P(Generic_FindGameInstallsTest, shouldFindGameInParentOfCurrentDirectory) {
  const auto gameId = GetParam();
  const auto gameInstalls =
      loot::generic::findGameInstalls(TestRegistry(), gameId);

  auto expectedSource = InstallSource::unknown;
  if (GetParam() == GameId::tes5 || GetParam() == GameId::tes5vr ||
      GetParam() == GameId::fo4vr) {
    expectedSource = InstallSource::steam;
  }

  ASSERT_EQ(1, gameInstalls.size());
  EXPECT_EQ(gameId, gameInstalls[0].gameId);
  EXPECT_EQ(expectedSource, gameInstalls[0].source);
  EXPECT_EQ(std::filesystem::current_path().parent_path(),
            gameInstalls[0].installPath);
  EXPECT_EQ("", gameInstalls[0].localPath);
}

TEST_P(Generic_FindGameInstallsTest, shouldIdentifySteamSiblingGame) {
  createSteamFile();

  const auto gameId = GetParam();
  const auto gameInstalls =
      loot::generic::findGameInstalls(TestRegistry(), gameId);

  const auto expectedSource = GetParam() == GameId::openmw
                                  ? InstallSource::unknown
                                  : InstallSource::steam;

  ASSERT_EQ(1, gameInstalls.size());
  EXPECT_EQ(gameId, gameInstalls[0].gameId);
  EXPECT_EQ(expectedSource, gameInstalls[0].source);
  EXPECT_EQ(std::filesystem::current_path().parent_path(),
            gameInstalls[0].installPath);
  EXPECT_EQ("", gameInstalls[0].localPath);
}

TEST_P(Generic_FindGameInstallsTest, shouldIdentifyGogSiblingGame) {
  const auto gogGameIds = gog::getGogGameIds(GetParam());
  if (!gogGameIds.empty()) {
    touch(gamePath / ("goggame-" + gogGameIds[0] + ".ico"));
  }

  const auto gameId = GetParam();
  const auto gameInstalls =
      loot::generic::findGameInstalls(TestRegistry(), gameId);

  auto expectedSource = InstallSource::gog;
  if (GetParam() == GameId::tes5 || GetParam() == GameId::tes5vr ||
      GetParam() == GameId::fo4vr) {
    expectedSource = InstallSource::steam;
  } else if (GetParam() == GameId::enderal || GetParam() == GameId::starfield ||
             GetParam() == GameId::openmw ||
             GetParam() == GameId::oblivionRemastered) {
    expectedSource = InstallSource::unknown;
  }

  ASSERT_EQ(1, gameInstalls.size());
  EXPECT_EQ(gameId, gameInstalls[0].gameId);
  EXPECT_EQ(expectedSource, gameInstalls[0].source);
  EXPECT_EQ(std::filesystem::current_path().parent_path(),
            gameInstalls[0].installPath);
  EXPECT_EQ("", gameInstalls[0].localPath);
}

TEST_P(Generic_FindGameInstallsTest, shouldIdentifyEpicSiblingGame) {
  const auto gameId = GetParam();

  if (gameId == GameId::tes5se) {
    touch(gamePath / "EOSSDK-Win64-Shipping.dll");
  } else if (gameId == GameId::fo3) {
    touch(gamePath / "FalloutLauncherEpic.exe");
  } else if (gameId == GameId::fonv) {
    touch(gamePath / "EOSSDK-Win32-Shipping.dll");
  }

  const auto gameInstalls =
      loot::generic::findGameInstalls(TestRegistry(), gameId);

  auto expectedSource = InstallSource::unknown;
  if (gameId == GameId::tes5 || gameId == GameId::tes5vr ||
      gameId == GameId::fo4vr) {
    expectedSource = InstallSource::steam;
  } else if (gameId == GameId::tes5se || gameId == GameId::fo3 ||
             gameId == GameId::fonv) {
    expectedSource = InstallSource::epic;
  }

  ASSERT_EQ(1, gameInstalls.size());
  EXPECT_EQ(gameId, gameInstalls[0].gameId);
  EXPECT_EQ(expectedSource, gameInstalls[0].source);
  EXPECT_EQ(std::filesystem::current_path().parent_path(),
            gameInstalls[0].installPath);
  EXPECT_EQ("", gameInstalls[0].localPath);
}

TEST_P(Generic_FindGameInstallsTest, shouldIdentifyMsStoreSiblingGame) {
  if (GetParam() == GameId::tes5se || GetParam() == GameId::fo4 ||
      GetParam() == GameId::starfield || GetParam() == GameId::openmw) {
    touch(gamePath / "appxmanifest.xml");
  } else {
    touch(gamePath.parent_path() / "appxmanifest.xml");
  }

  const auto gameId = GetParam();
  const auto gameInstalls =
      loot::generic::findGameInstalls(TestRegistry(), gameId);

  auto expectedSource = InstallSource::microsoft;
  if (GetParam() == GameId::tes5 || GetParam() == GameId::tes5vr ||
      GetParam() == GameId::fo4vr) {
    expectedSource = InstallSource::steam;
  } else if (GetParam() == GameId::nehrim || GetParam() == GameId::enderal ||
             GetParam() == GameId::enderalse || GetParam() == GameId::openmw ||
             GetParam() == GameId::oblivionRemastered) {
    expectedSource = InstallSource::unknown;
  }

  ASSERT_EQ(1, gameInstalls.size());
  EXPECT_EQ(gameId, gameInstalls[0].gameId);
  EXPECT_EQ(expectedSource, gameInstalls[0].source);
  EXPECT_EQ(std::filesystem::current_path().parent_path(),
            gameInstalls[0].installPath);
  EXPECT_EQ("", gameInstalls[0].localPath);
}
#else
TEST_P(Generic_FindGameInstallsTest,
       shouldNotFindGameInParentOfCurrentDirectory) {
  const auto gameId = GetParam();
  const auto gameInstalls =
      loot::generic::findGameInstalls(TestRegistry(), gameId);

  EXPECT_TRUE(gameInstalls.empty());
}
#endif

TEST_P(Generic_FindGameInstallsTest, shouldFindGameUsingGenericRegistryValue) {
  restoreCurrentPath();

  TestRegistry registry;
  const auto subKey = getSubKey();
  if (subKey.has_value()) {
    registry.SetStringValue(subKey.value(), gamePath.u8string());
  }

  const auto gameInstalls =
      loot::generic::findGameInstalls(registry, GetParam());

  auto expectedSource = InstallSource::unknown;
  if (GetParam() == GameId::tes5 || GetParam() == GameId::tes5vr ||
      GetParam() == GameId::fo4vr) {
    expectedSource = InstallSource::steam;
  }

  if (GetParam() == GameId::starfield ||
      GetParam() == GameId::oblivionRemastered) {
    ASSERT_TRUE(gameInstalls.empty());
  } else {
    ASSERT_EQ(1, gameInstalls.size());
    EXPECT_EQ(GetParam(), gameInstalls[0].gameId);
    EXPECT_EQ(expectedSource, gameInstalls[0].source);
    EXPECT_EQ(gamePath, gameInstalls[0].installPath);
    EXPECT_EQ("", gameInstalls[0].localPath);
  }
}

TEST_P(Generic_FindGameInstallsTest,
       shouldNotFindGameIfPathInRegistryIsInvalid) {
  restoreCurrentPath();

  TestRegistry registry;
  const auto subKey = getSubKey();
  if (subKey.has_value()) {
    registry.SetStringValue(subKey.value(), "invalid");
  }

  const auto gameInstalls =
      loot::generic::findGameInstalls(registry, GetParam());

  EXPECT_TRUE(gameInstalls.empty());
}

TEST_P(Generic_FindGameInstallsTest, shouldIdentifySteamRegistryGame) {
  restoreCurrentPath();
  createSteamFile();

  TestRegistry registry;
  const auto subKey = getSubKey();
  if (subKey.has_value()) {
    registry.SetStringValue(subKey.value(), gamePath.u8string());
  }

  const auto gameInstalls =
      loot::generic::findGameInstalls(registry, GetParam());

  if (GetParam() == GameId::starfield ||
      GetParam() == GameId::oblivionRemastered) {
    ASSERT_TRUE(gameInstalls.empty());
  } else {
    const auto expectedSource = GetParam() == GameId::openmw
                                    ? InstallSource::unknown
                                    : InstallSource::steam;

    ASSERT_EQ(1, gameInstalls.size());
    EXPECT_EQ(GetParam(), gameInstalls[0].gameId);
    EXPECT_EQ(expectedSource, gameInstalls[0].source);
    EXPECT_EQ(gamePath, gameInstalls[0].installPath);
    EXPECT_EQ("", gameInstalls[0].localPath);
  }
}

TEST_P(Generic_FindGameInstallsTest, shouldIdentifyGogRegistryGame) {
  restoreCurrentPath();

  const auto gogGameIds = gog::getGogGameIds(GetParam());
  if (!gogGameIds.empty()) {
    touch(gamePath / ("goggame-" + gogGameIds[0] + ".ico"));
  }

  TestRegistry registry;
  const auto subKey = getSubKey();
  if (subKey.has_value()) {
    registry.SetStringValue(subKey.value(), gamePath.u8string());
  }

  const auto gameInstalls =
      loot::generic::findGameInstalls(registry, GetParam());

  auto expectedSource = InstallSource::gog;
  if (GetParam() == GameId::tes5 || GetParam() == GameId::tes5vr ||
      GetParam() == GameId::fo4vr) {
    expectedSource = InstallSource::steam;
  } else if (GetParam() == GameId::enderal || GetParam() == GameId::openmw) {
    expectedSource = InstallSource::unknown;
  }

  if (GetParam() == GameId::starfield ||
      GetParam() == GameId::oblivionRemastered) {
    ASSERT_TRUE(gameInstalls.empty());
  } else {
    ASSERT_EQ(1, gameInstalls.size());
    EXPECT_EQ(GetParam(), gameInstalls[0].gameId);
    EXPECT_EQ(expectedSource, gameInstalls[0].source);
    EXPECT_EQ(gamePath, gameInstalls[0].installPath);
    EXPECT_EQ("", gameInstalls[0].localPath);
  }
}

class DetectGameInstallTest : public CommonGameTestFixture,
                              public testing::WithParamInterface<GameId> {
protected:
  DetectGameInstallTest() : CommonGameTestFixture(GetParam()) {
    if (GetParam() == GameId::nehrim) {
      touch(gamePath / "NehrimLauncher.exe");
    } else if (GetParam() == GameId::enderal ||
               GetParam() == GameId::enderalse) {
      touch(gamePath / "Enderal Launcher.exe");
    }
  }

  GameSettings getSettings() const {
    GameSettings settings = GameSettings(GetParam(), "").setGamePath(gamePath);
    if (GetParam() == GameId::nehrim) {
      settings.setMaster("Nehrim.esm");
    }

    return settings;
  }
};

// Pass an empty first argument, as it's a prefix for the test instantiation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_SUITE_P(,
                         DetectGameInstallTest,
                         ::testing::ValuesIn(ALL_GAME_IDS));

TEST_P(DetectGameInstallTest, shouldNotDetectAGameInstallThatIsNotValid) {
  const auto install = generic::detectGameInstall(GameSettings(GetParam(), ""));

  EXPECT_FALSE(install.has_value());
}

TEST_P(DetectGameInstallTest, shouldDetectAValidGameInstall) {
  const auto install = generic::detectGameInstall(getSettings());

  auto expectedSource = InstallSource::unknown;
  if (GetParam() == GameId::tes5 || GetParam() == GameId::tes5vr ||
      GetParam() == GameId::fo4vr) {
    expectedSource = InstallSource::steam;
  }

  ASSERT_TRUE(install.has_value());
  EXPECT_EQ(GetParam(), install.value().gameId);
  EXPECT_EQ(expectedSource, install.value().source);
  EXPECT_EQ(gamePath, install.value().installPath);
  EXPECT_EQ("", install.value().localPath);
}

TEST_P(DetectGameInstallTest, shouldDetectASteamInstall) {
  if (GetParam() == GameId::tes3) {
    touch(gamePath / "steam_autocloud.vdf");
  } else if (GetParam() == GameId::nehrim) {
    touch(gamePath / "steam_api.dll");
  } else if (GetParam() == GameId::starfield) {
    touch(gamePath / "steam_api64.dll");
  } else if (GetParam() == GameId::oblivionRemastered) {
    touch(gamePath / "Engine" / "Binaries" / "ThirdParty" / "Steamworks" /
          "Steamv153" / "Win64" / "steam_api64.dll");
  } else {
    touch(gamePath / "installscript.vdf");
  }

  const auto install = generic::detectGameInstall(getSettings());

  const auto expectedSource = GetParam() == GameId::openmw
                                  ? InstallSource::unknown
                                  : InstallSource::steam;

  ASSERT_TRUE(install.has_value());
  EXPECT_EQ(GetParam(), install.value().gameId);
  EXPECT_EQ(expectedSource, install.value().source);
  EXPECT_EQ(gamePath, install.value().installPath);
  EXPECT_EQ("", install.value().localPath);
}

TEST_P(DetectGameInstallTest, shouldDetectAGogInstall) {
  const auto gogGameIds = gog::getGogGameIds(GetParam());
  if (!gogGameIds.empty()) {
    touch(gamePath / ("goggame-" + gogGameIds[0] + ".ico"));
  }

  const auto install = generic::detectGameInstall(getSettings());

  auto expectedSource = InstallSource::gog;
  if (GetParam() == GameId::tes5 || GetParam() == GameId::tes5vr ||
      GetParam() == GameId::fo4vr) {
    expectedSource = InstallSource::steam;
  } else if (GetParam() == GameId::enderal || GetParam() == GameId::starfield ||
             GetParam() == GameId::openmw ||
             GetParam() == GameId::oblivionRemastered) {
    expectedSource = InstallSource::unknown;
  }

  ASSERT_TRUE(install.has_value());
  EXPECT_EQ(GetParam(), install.value().gameId);
  EXPECT_EQ(expectedSource, install.value().source);
  EXPECT_EQ(gamePath, install.value().installPath);
  EXPECT_EQ("", install.value().localPath);
}

TEST_P(DetectGameInstallTest, shouldDetectAnEpicInstall) {
  if (GetParam() == GameId::tes5se) {
    touch(gamePath / "EOSSDK-Win64-Shipping.dll");
  } else if (GetParam() == GameId::fo3) {
    touch(gamePath / "FalloutLauncherEpic.exe");
  } else if (GetParam() == GameId::fonv) {
    touch(gamePath / "EOSSDK-Win32-Shipping.dll");
  }

  const auto install = generic::detectGameInstall(getSettings());

  auto expectedSource = InstallSource::unknown;
  if (GetParam() == GameId::tes5 || GetParam() == GameId::tes5vr ||
      GetParam() == GameId::fo4vr) {
    expectedSource = InstallSource::steam;
  } else if (GetParam() == GameId::tes5se || GetParam() == GameId::fo3 ||
             GetParam() == GameId::fonv) {
    expectedSource = InstallSource::epic;
  }

  ASSERT_TRUE(install.has_value());
  EXPECT_EQ(GetParam(), install.value().gameId);
  EXPECT_EQ(expectedSource, install.value().source);
  EXPECT_EQ(gamePath, install.value().installPath);
  EXPECT_EQ("", install.value().localPath);
}

TEST_P(DetectGameInstallTest, shouldDetectAMicrosoftInstall) {
  if (GetParam() == GameId::tes5se || GetParam() == GameId::fo4 ||
      GetParam() == GameId::starfield ||
      GetParam() == GameId::oblivionRemastered) {
    touch(gamePath / "appxmanifest.xml");
  } else {
    touch(gamePath.parent_path() / "appxmanifest.xml");
  }

  const auto install = generic::detectGameInstall(getSettings());

  auto expectedSource = InstallSource::microsoft;
  if (GetParam() == GameId::tes5 || GetParam() == GameId::tes5vr ||
      GetParam() == GameId::fo4vr) {
    expectedSource = InstallSource::steam;
  } else if (GetParam() == GameId::nehrim || GetParam() == GameId::enderal ||
             GetParam() == GameId::enderalse || GetParam() == GameId::openmw) {
    expectedSource = InstallSource::unknown;
  }

  ASSERT_TRUE(install.has_value());
  EXPECT_EQ(GetParam(), install.value().gameId);
  EXPECT_EQ(expectedSource, install.value().source);
  EXPECT_EQ(gamePath, install.value().installPath);
  EXPECT_EQ("", install.value().localPath);
}
}

#endif
