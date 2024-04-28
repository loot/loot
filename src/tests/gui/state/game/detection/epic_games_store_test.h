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

#ifndef LOOT_TESTS_GUI_STATE_GAME_DETECTION_EPIC_GAMES_STORE_TEST
#define LOOT_TESTS_GUI_STATE_GAME_DETECTION_EPIC_GAMES_STORE_TEST

#include "gui/state/game/detection/epic_games_store.h"
#include "tests/common_game_test_fixture.h"
#include "tests/gui/state/game/detection/test_registry.h"

namespace loot::test {
class Epic_FindGameInstallsExceptionTest
    : public ::testing::TestWithParam<GameId> {};

INSTANTIATE_TEST_SUITE_P(,
                         Epic_FindGameInstallsExceptionTest,
                         ::testing::ValuesIn(ALL_GAME_IDS));

TEST_P(Epic_FindGameInstallsExceptionTest, shouldNotThrowForAnyValidGameId) {
  EXPECT_NO_THROW(loot::epic::FindGameInstalls(TestRegistry(), GetParam(), {}));
}

class Epic_FindGameInstallsTest : public CommonGameTestFixture,
                                  public testing::WithParamInterface<GameId> {
protected:
  Epic_FindGameInstallsTest() :
      CommonGameTestFixture(GetParam()),
      epicManifestsPath(dataPath.parent_path().parent_path() / "Manifests") {
    std::filesystem::create_directory(epicManifestsPath);

    const std::string appName = GetAppName(GetParam());

    if (GetParam() == GameId::fo3) {
      // FO3 has localised subdirectories.
      gamePath = dataPath.parent_path().parent_path() / "game2";

      const std::vector<std::string> folders{"Fallout 3 GOTY English",
                                             "Fallout 3 GOTY French"};
      for (const auto& folder : folders) {
        const auto localisedInstall = gamePath / folder;
        std::filesystem::create_directories(localisedInstall.parent_path());
        std::filesystem::copy(dataPath.parent_path(),
                              localisedInstall,
                              std::filesystem::copy_options::recursive);
      }

    } else {
      gamePath = dataPath.parent_path();
    }

    std::ofstream out(epicManifestsPath / "manifest.item");
    out << "{\"AppName\": \"" + appName + "\", \"InstallLocation\": \"" +
               boost::replace_all_copy(gamePath.u8string(), "\\", "\\\\") +
               "\"}";
    out.close();

    registry.SetStringValue("Software\\Epic Games\\EpicGamesLauncher",
                            epicManifestsPath.parent_path().u8string());
  }

  std::filesystem::path epicManifestsPath;
  std::filesystem::path gamePath;

  TestRegistry registry;

private:
  static std::string GetAppName(const GameId gameId) {
    switch (gameId) {
      case GameId::tes5se:
        return "ac82db5035584c7f8a2c548d98c86b2c";
      case GameId::fo3:
        return "adeae8bbfc94427db57c7dfecce3f1d4";
      case GameId::fo4:
        return "61d52ce4d09d41e48800c22784d13ae8";
      default:
        throw new std::logic_error("Unsupported game ID");
    }
  }
};

// Pass an empty first argument, as it's a prefix for the test instantiation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_SUITE_P(,
                         Epic_FindGameInstallsTest,
                         ::testing::Values(GameId::tes5se,
                                           GameId::fo3,
                                           GameId::fo4));

TEST_P(Epic_FindGameInstallsTest, shouldFindAValidEpicInstall) {
  const auto install = epic::FindGameInstalls(registry, GetParam(), {});

  const auto expectedInstallPath = GetParam() == GameId::fo3
                                       ? gamePath / "Fallout 3 GOTY English"
                                       : gamePath;

  ASSERT_TRUE(install.has_value());
  EXPECT_EQ(GetParam(), install.value().gameId);
  EXPECT_EQ(InstallSource::epic, install.value().source);
  EXPECT_EQ(expectedInstallPath, install.value().installPath);
  EXPECT_EQ("", install.value().localPath);
}

TEST_P(Epic_FindGameInstallsTest, shouldNotFindAnEpicInstallThatIsInvalid) {
  std::filesystem::remove_all(gamePath);

  const auto install = epic::FindGameInstalls(registry, GetParam(), {});

  EXPECT_FALSE(install.has_value());
}

TEST_P(Epic_FindGameInstallsTest,
       shouldNotFindAnEpicInstallIfTheManifestsDirectoryDoesNotExist) {
  std::filesystem::remove_all(epicManifestsPath);

  const auto install = epic::FindGameInstalls(registry, GetParam(), {});

  EXPECT_FALSE(install.has_value());
}

TEST_P(Epic_FindGameInstallsTest,
       shouldPickLocalisedInstallPathAccordingToPreferredUILanguageOrder) {
  const auto install =
      epic::FindGameInstalls(registry, GetParam(), {"fr", "en"});

  const auto expectedInstallPath =
      GetParam() == GameId::fo3 ? gamePath / "Fallout 3 GOTY French" : gamePath;

  ASSERT_TRUE(install.has_value());
  EXPECT_EQ(GetParam(), install.value().gameId);
  EXPECT_EQ(InstallSource::epic, install.value().source);
  EXPECT_EQ(expectedInstallPath, install.value().installPath);
  EXPECT_EQ("", install.value().localPath);
}
}

#endif
