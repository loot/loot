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

#ifndef LOOT_TESTS_GUI_STATE_GAME_DETECTION_DETAIL_TEST
#define LOOT_TESTS_GUI_STATE_GAME_DETECTION_DETAIL_TEST

#include "gui/state/game/detection/detail.h"
#include "tests/common_game_test_fixture.h"
#include "tests/gui/state/game/detection/test_registry.h"

namespace loot::test {
static const std::array<InstallSource, 5> ALL_INSTALL_SOURCES = {
    InstallSource::steam,
    InstallSource::gog,
    InstallSource::epic,
    InstallSource::microsoft,
    InstallSource::unknown};

class GetDefaultLootFolderNameTest : public ::testing::TestWithParam<GameId> {};

INSTANTIATE_TEST_SUITE_P(,
                         GetDefaultLootFolderNameTest,
                         ::testing::ValuesIn(ALL_GAME_IDS));

TEST_P(GetDefaultLootFolderNameTest, shouldNotThrowForAnyValidGameId) {
  EXPECT_NO_THROW(GetDefaultLootFolderName(GetParam()));
}

class GetSourceDescriptionTest
    : public ::testing::TestWithParam<InstallSource> {};

INSTANTIATE_TEST_SUITE_P(,
                         GetSourceDescriptionTest,
                         ::testing::ValuesIn(ALL_INSTALL_SOURCES));

TEST_P(GetSourceDescriptionTest, shouldNotThrowForAnyValidGameId) {
  EXPECT_NO_THROW(GetSourceDescription(GetParam()));
}

class GetNameSourceSuffixTest : public ::testing::TestWithParam<InstallSource> {
};

INSTANTIATE_TEST_SUITE_P(,
                         GetNameSourceSuffixTest,
                         ::testing::ValuesIn(ALL_INSTALL_SOURCES));

TEST_P(GetNameSourceSuffixTest, shouldNotThrowForAnyValidGameId) {
  EXPECT_NO_THROW(GetNameSourceSuffix(GetParam()));
}

class FindGameInstallsTest : public CommonGameTestFixture {
protected:
  FindGameInstallsTest() :
      CommonGameTestFixture(GameId::tes5se),
      epicManifestsPath(gamePath.parent_path() / "Manifests"),
      xboxGamingRootPath(gamePath.parent_path() / "xbox"),
      genericInstallPath(gamePath),
      steamInstallPath(gamePath.parent_path() / "steam"),
      gogInstallPath(gamePath.parent_path() / "gog"),
      epicInstallPath(gamePath.parent_path() / "epic"),
      msInstallPath(xboxGamingRootPath /
                    "The Elder Scrolls V- Skyrim Special Edition (PC)" /
                    "Content") {
    // Create generic install.
    registry.SetStringValue(
        "Software\\Bethesda Softworks\\Skyrim Special Edition",
        genericInstallPath.u8string());

    // Create Steam install.
    CopyInstall(steamInstallPath);
    registry.SetStringValue(
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App "
        "489830",
        steamInstallPath.u8string());

    // Create GOG install.
    CopyInstall(gogInstallPath);
    registry.SetStringValue("Software\\GOG.com\\Games\\1801825368",
                            gogInstallPath.u8string());

    // Create Epic install.
    CreateEpicManifest();
    CopyInstall(epicInstallPath);
    registry.SetStringValue("Software\\Epic Games\\EpicGamesLauncher",
                            epicManifestsPath.parent_path().u8string());

    // Create MS Store install.
    CopyInstall(msInstallPath);
  }

  std::filesystem::path epicManifestsPath;
  std::filesystem::path xboxGamingRootPath;

  std::filesystem::path genericInstallPath;
  std::filesystem::path steamInstallPath;
  std::filesystem::path gogInstallPath;
  std::filesystem::path epicInstallPath;
  std::filesystem::path msInstallPath;

  TestRegistry registry;

private:
  void CreateEpicManifest() {
    std::filesystem::create_directory(epicManifestsPath);

    std::ofstream out(epicManifestsPath / "manifest.item");
    out << "{\"AppName\": \"ac82db5035584c7f8a2c548d98c86b2c\", "
           "\"InstallLocation\": \"" +
               boost::replace_all_copy(
                   epicInstallPath.u8string(), "\\", "\\\\") +
               "\"}";
    out.close();
  }

  void CopyInstall(const std::filesystem::path& destination) {
    std::filesystem::create_directories(destination.parent_path());

    std::filesystem::copy(
        gamePath, destination, std::filesystem::copy_options::recursive);
  }
};

TEST_F(FindGameInstallsTest, shouldReturnAnEmptyVectorIfNoGamesAreInstalled) {
  EXPECT_TRUE(FindGameInstalls(TestRegistry(), {}, {}, {}).empty());
}

TEST_F(FindGameInstallsTest, shouldFindInstallsFromAllSupportedSources) {
  const auto installs =
      FindGameInstalls(registry, {}, {xboxGamingRootPath}, {});

  ASSERT_EQ(5, installs.size());

  EXPECT_EQ(GameId::tes5se, installs[0].gameId);
  EXPECT_EQ(InstallSource::steam, installs[0].source);
  EXPECT_EQ(steamInstallPath, installs[0].installPath);
  EXPECT_EQ("", installs[0].localPath);

  EXPECT_EQ(GameId::tes5se, installs[1].gameId);
  EXPECT_EQ(InstallSource::gog, installs[1].source);
  EXPECT_EQ(gogInstallPath, installs[1].installPath);
  EXPECT_EQ("", installs[1].localPath);

  EXPECT_EQ(GameId::tes5se, installs[2].gameId);
  EXPECT_EQ(InstallSource::unknown, installs[2].source);
  EXPECT_EQ(genericInstallPath, installs[2].installPath);
  EXPECT_EQ("", installs[2].localPath);

  EXPECT_EQ(GameId::tes5se, installs[3].gameId);
  EXPECT_EQ(InstallSource::epic, installs[3].source);
  EXPECT_EQ(epicInstallPath, installs[3].installPath);
  EXPECT_EQ("", installs[3].localPath);

  EXPECT_EQ(GameId::tes5se, installs[4].gameId);
  EXPECT_EQ(InstallSource::microsoft, installs[4].source);
  EXPECT_EQ(msInstallPath, installs[4].installPath);
  EXPECT_EQ("", installs[4].localPath);
}

TEST_F(FindGameInstallsTest, shouldDeduplicateFoundInstalls) {
  registry.SetStringValue(
      "Software\\Bethesda Softworks\\Skyrim Special Edition",
      steamInstallPath.u8string());

  const auto installs =
      FindGameInstalls(registry, {}, {xboxGamingRootPath}, {});

  ASSERT_EQ(4, installs.size());

  EXPECT_EQ(GameId::tes5se, installs[0].gameId);
  EXPECT_EQ(InstallSource::steam, installs[0].source);
  EXPECT_EQ(steamInstallPath, installs[0].installPath);
  EXPECT_EQ("", installs[0].localPath);

  EXPECT_EQ(GameId::tes5se, installs[1].gameId);
  EXPECT_EQ(InstallSource::gog, installs[1].source);
  EXPECT_EQ(gogInstallPath, installs[1].installPath);
  EXPECT_EQ("", installs[1].localPath);

  EXPECT_EQ(GameId::tes5se, installs[2].gameId);
  EXPECT_EQ(InstallSource::epic, installs[2].source);
  EXPECT_EQ(epicInstallPath, installs[2].installPath);
  EXPECT_EQ("", installs[2].localPath);

  EXPECT_EQ(GameId::tes5se, installs[3].gameId);
  EXPECT_EQ(InstallSource::microsoft, installs[3].source);
  EXPECT_EQ(msInstallPath, installs[3].installPath);
  EXPECT_EQ("", installs[3].localPath);
}

TEST(CountGameInstalls, shouldCountConfiguredAndNewInstallsByGameAndSource) {
  const std::vector<GameInstall> configuredInstalls{
      {GameId::tes3, InstallSource::epic},
      {GameId::tes4, InstallSource::gog},
      {GameId::tes4, InstallSource::steam}};

  const std::vector<GameInstall> newInstalls{
      {GameId::tes3, InstallSource::epic},
      {GameId::tes5, InstallSource::microsoft}};

  const auto counts = CountGameInstalls(configuredInstalls, newInstalls);

  EXPECT_EQ(3, counts.size());
  EXPECT_EQ(1, counts.at(GameId::tes3).size());
  EXPECT_EQ(2, counts.at(GameId::tes3).at(InstallSource::epic));
  EXPECT_EQ(2, counts.at(GameId::tes4).size());
  EXPECT_EQ(1, counts.at(GameId::tes4).at(InstallSource::gog));
  EXPECT_EQ(1, counts.at(GameId::tes4).at(InstallSource::steam));
  EXPECT_EQ(1, counts.at(GameId::tes5).size());
  EXPECT_EQ(1, counts.at(GameId::tes5).at(InstallSource::microsoft));
}

TEST(DeriveName, shouldUseBaseNameIfItDoesNotAlreadyExist) {
  const auto baseName = "Morrowind";
  const auto name = DeriveName({GameId::tes3, InstallSource::unknown},
                               baseName,
                               {{GameId::tes3, {{InstallSource::gog, 1}}},
                                {GameId::tes4, {{InstallSource::steam, 1}}}},
                               {});

  EXPECT_EQ(baseName, name);
}

TEST(DeriveName, shouldAddSourceToNameIfAnotherSourceForTheSameGameExists) {
  const std::string baseName = "Morrowind";
  const auto name = DeriveName(
      {GameId::tes3, InstallSource::gog},
      baseName,
      {{GameId::tes3, {{InstallSource::gog, 1}, {InstallSource::steam, 1}}},
       {GameId::tes4, {{InstallSource::steam, 1}}}},
      {});

  EXPECT_EQ(baseName + " (GOG)", name);
}

TEST(DeriveName, shouldAddIndexToNameIfItAlreadyExists) {
  const std::string baseName = "Morrowind";
  const auto name = DeriveName({GameId::tes3, InstallSource::unknown},
                               baseName,
                               {{GameId::tes3, {{InstallSource::gog, 1}}},
                                {GameId::tes4, {{InstallSource::steam, 1}}}},
                               {baseName});

  EXPECT_EQ(baseName + " (1)", name);
}

TEST(DeriveName, shouldAddIndexToNameWithSourceIfItAlreadyExists) {
  const std::string baseName = "Morrowind";
  const auto name = DeriveName(
      {GameId::tes3, InstallSource::gog},
      baseName,
      {{GameId::tes3, {{InstallSource::gog, 1}, {InstallSource::steam, 1}}},
       {GameId::tes4, {{InstallSource::steam, 1}}}},
      {baseName + " (GOG)"});

  EXPECT_EQ(baseName + " (GOG) (1)", name);
}

TEST(DeriveName, shouldIncrementIndexIfNameWithIndexAlreadyExists) {
  const std::string baseName = "Morrowind";
  const auto name = DeriveName({GameId::tes3, InstallSource::gog},
                               baseName,
                               {{GameId::tes3, {{InstallSource::gog, 1}}},
                                {GameId::tes4, {{InstallSource::steam, 1}}}},
                               {baseName, baseName + " (1)"});

  EXPECT_EQ(baseName + " (2)", name);
}

TEST(UpdateSettingsPaths, shouldUpdateInstallPathIfItIsCurrentlyEmpty) {
  GameSettings settings;
  GameInstall install;
  install.installPath = "test";

  ASSERT_TRUE(settings.GamePath().empty());

  UpdateSettingsPaths(settings, install);

  EXPECT_EQ(install.installPath, settings.GamePath());
}

TEST(UpdateSettingsPaths, shouldNotUpdateInstallPathIfItIsNotCurrentlyEmpty) {
  const std::filesystem::path initialPath = "initial";

  GameSettings settings;
  settings.SetGamePath(initialPath);

  GameInstall install;
  install.installPath = "test";

  UpdateSettingsPaths(settings, install);

  EXPECT_EQ(initialPath, settings.GamePath());
}

TEST(UpdateSettingsPaths, shouldUpdateLocalPathIfItIsCurrentlyEmpty) {
  GameSettings settings;
  GameInstall install;
  install.localPath = "test";

  ASSERT_TRUE(settings.GameLocalPath().empty());

  UpdateSettingsPaths(settings, install);

  EXPECT_EQ(install.localPath, settings.GameLocalPath());
}

TEST(UpdateSettingsPaths, shouldNotUpdateLocalPathIfItIsNotCurrentlyEmpty) {
  const std::filesystem::path initialPath = "initial";

  GameSettings settings;
  settings.SetGameLocalPath(initialPath);

  GameInstall install;
  install.localPath = "test";

  UpdateSettingsPaths(settings, install);

  EXPECT_EQ(initialPath, settings.GameLocalPath());
}

TEST(
    UpdateMatchingSettings,
    shouldUpdateSettingsPathsIfThePrimaryComparatorFindsASettingsObjectThatMatchesAnInstall) {
  const std::filesystem::path installPath = "install";
  const std::filesystem::path localPath = "local";
  std::vector<GameSettings> gamesSettings{
      GameSettings(), GameSettings().SetGamePath(installPath)};
  const std::vector<GameInstall> gameInstalls{GameInstall{
      GameId::tes5, InstallSource::unknown, installPath, localPath}};
  const auto comparator = [](const GameSettings& settings,
                             const GameInstall& install) {
    return settings.GamePath() == install.installPath;
  };

  const auto newInstalls =
      UpdateMatchingSettings(gamesSettings, gameInstalls, comparator);

  EXPECT_TRUE(newInstalls.empty());
  EXPECT_TRUE(gamesSettings[0].GamePath().empty());
  EXPECT_TRUE(gamesSettings[0].GameLocalPath().empty());
  EXPECT_EQ(installPath, gamesSettings[1].GamePath());
  EXPECT_EQ(localPath, gamesSettings[1].GameLocalPath());
}

TEST(
    UpdateMatchingSettings,
    shouldTryMatchingSettingsWithEmptyInstallPathByGameTypeIfPrimaryComparatorFindsNoMatch) {
  const std::filesystem::path installPath = "install";
  const std::filesystem::path localPath = "local";
  std::vector<GameSettings> gamesSettings{GameSettings(),
                                          GameSettings(GameId::fo4, "")};
  const std::vector<GameInstall> gameInstalls{
      GameInstall{GameId::fo4, InstallSource::unknown, installPath, localPath}};
  const auto comparator = [](const GameSettings&, const GameInstall&) {
    return false;
  };

  const auto newInstalls =
      UpdateMatchingSettings(gamesSettings, gameInstalls, comparator);

  EXPECT_TRUE(newInstalls.empty());
  EXPECT_TRUE(gamesSettings[0].GamePath().empty());
  EXPECT_TRUE(gamesSettings[0].GameLocalPath().empty());
  EXPECT_EQ(installPath, gamesSettings[1].GamePath());
  EXPECT_EQ(localPath, gamesSettings[1].GameLocalPath());
}

TEST(UpdateMatchingSettings, shouldTrySecondaryMatchingByGameId) {
  const std::filesystem::path installPath1 = "install1";
  const std::filesystem::path installPath2 = "install2";
  const std::filesystem::path localPath1 = "local1";
  const std::filesystem::path localPath2 = "local2";
  std::vector<GameSettings> gamesSettings{GameSettings(GameId::nehrim, ""),
                                          GameSettings(GameId::tes4, ""),
                                          GameSettings(GameId::tes5, "")};
  const std::vector<GameInstall> gameInstalls{
      GameInstall{
          GameId::nehrim, InstallSource::unknown, installPath1, localPath1},
      GameInstall{
          GameId::tes4, InstallSource::unknown, installPath2, localPath2}};
  const auto comparator = [](const GameSettings&, const GameInstall&) {
    return false;
  };

  const auto newInstalls =
      UpdateMatchingSettings(gamesSettings, gameInstalls, comparator);

  EXPECT_TRUE(newInstalls.empty());
  EXPECT_EQ(installPath1, gamesSettings[0].GamePath());
  EXPECT_EQ(localPath1, gamesSettings[0].GameLocalPath());
  EXPECT_EQ(installPath2, gamesSettings[1].GamePath());
  EXPECT_EQ(localPath2, gamesSettings[1].GameLocalPath());
  EXPECT_TRUE(gamesSettings[2].GamePath().empty());
  EXPECT_TRUE(gamesSettings[2].GameLocalPath().empty());
}

TEST(UpdateMatchingSettings,
     shouldIncludeInstallInReturnedVectorIfItMatchesNoSettings) {
  const std::filesystem::path installPath = "install";
  const std::filesystem::path localPath = "local";
  std::vector<GameSettings> gamesSettings{GameSettings(GameId::tes4, "")};
  const std::vector<GameInstall> gameInstalls{GameInstall{
      GameId::tes3, InstallSource::unknown, installPath, localPath}};
  const auto comparator = [](const GameSettings&, const GameInstall&) {
    return false;
  };

  const auto newInstalls =
      UpdateMatchingSettings(gamesSettings, gameInstalls, comparator);

  ASSERT_EQ(1, newInstalls.size());
  EXPECT_EQ(gameInstalls[0].gameId, newInstalls[0].gameId);
  EXPECT_EQ(gameInstalls[0].source, newInstalls[0].source);
  EXPECT_EQ(gameInstalls[0].installPath, newInstalls[0].installPath);
  EXPECT_EQ(gameInstalls[0].localPath, newInstalls[0].localPath);

  EXPECT_TRUE(gamesSettings[0].GamePath().empty());
  EXPECT_TRUE(gamesSettings[0].GameLocalPath().empty());
}

TEST(AppendNewGamesSettings,
     shouldAppendSettingsForEachNewInstallUsingGameDefaults) {
  const std::filesystem::path installPath = "install";
  const std::filesystem::path localPath = "local";
  std::vector<GameSettings> gamesSettings;

  AppendNewGamesSettings(
      gamesSettings,
      {{GameId::enderal, {{InstallSource::unknown, 1}}}},
      {{GameId::enderal, InstallSource::unknown, installPath, localPath},
       {GameId::nehrim}});

  ASSERT_EQ(2, gamesSettings.size());

  EXPECT_EQ(GameId::enderal, gamesSettings[0].Id());
  EXPECT_EQ("Enderal", gamesSettings[0].FolderName());
  EXPECT_EQ("Enderal: Forgotten Stories", gamesSettings[0].Name());
  EXPECT_EQ("Skyrim.esm", gamesSettings[0].Master());
  EXPECT_EQ(
      "https://raw.githubusercontent.com/loot/enderal/v0.21/masterlist.yaml",
      gamesSettings[0].MasterlistSource());
  EXPECT_EQ(installPath, gamesSettings[0].GamePath());
  EXPECT_EQ(localPath, gamesSettings[0].GameLocalPath());

  EXPECT_EQ(GameId::nehrim, gamesSettings[1].Id());
  EXPECT_EQ("Nehrim", gamesSettings[1].FolderName());
  EXPECT_EQ("Nehrim - At Fate's Edge", gamesSettings[1].Name());
  EXPECT_EQ("Nehrim.esm", gamesSettings[1].Master());
  EXPECT_EQ(
      "https://raw.githubusercontent.com/loot/oblivion/v0.21/masterlist.yaml",
      gamesSettings[1].MasterlistSource());
}

TEST(AppendNewGamesSettings,
     shouldDeriveGameAndFolderNamesUsingTheExistingNamesOfPreviousSettings) {
  std::vector<GameSettings> gamesSettings{
      GameSettings(GameId::tes3, "Morrowind").SetName("TES III: Morrowind")};

  AppendNewGamesSettings(
      gamesSettings,
      {{GameId::tes3, {{InstallSource::gog, 3}}}},
      {{GameId::tes3, InstallSource::gog}, {GameId::tes3, InstallSource::gog}});

  ASSERT_EQ(3, gamesSettings.size());
  EXPECT_EQ("Morrowind", gamesSettings[0].FolderName());
  EXPECT_EQ("TES III: Morrowind", gamesSettings[0].Name());
  EXPECT_EQ("Morrowind (1)", gamesSettings[1].FolderName());
  EXPECT_EQ("TES III: Morrowind (1)", gamesSettings[1].Name());
  EXPECT_EQ("Morrowind (2)", gamesSettings[2].FolderName());
  EXPECT_EQ("TES III: Morrowind (2)", gamesSettings[2].Name());
}

class UpdateInstalledGamesSettingsTest : public CommonGameTestFixture {
protected:
  UpdateInstalledGamesSettingsTest() : CommonGameTestFixture(GameId::tes3) {}

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
    std::filesystem::current_path(initialCurrentPath);

    CommonGameTestFixture::TearDown();
  }

private:
  std::filesystem::path initialCurrentPath;
};

#ifdef _WIN32
TEST_F(UpdateInstalledGamesSettingsTest,
       shouldReturnSettingsForGameInParentOfCurrentDirectory) {
  std::vector<GameSettings> gamesSettings;
  UpdateInstalledGamesSettings(gamesSettings, TestRegistry(), {}, {}, {});

  ASSERT_EQ(1, gamesSettings.size());
  EXPECT_EQ(GameId::tes3, gamesSettings[0].Id());
  EXPECT_EQ(std::filesystem::current_path().parent_path(),
            gamesSettings[0].GamePath());
  EXPECT_EQ("", gamesSettings[0].GameLocalPath());
}
#endif
}

#endif
