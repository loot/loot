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

#ifndef LOOT_TESTS_COMMON_GAME_TEST_FIXTURE
#define LOOT_TESTS_COMMON_GAME_TEST_FIXTURE

#include <gtest/gtest.h>

#include <array>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <filesystem>
#include <fstream>
#include <map>

#include "gui/state/game/detection/game_install.h"
#include "loot/enum/game_type.h"
#include "tests/gui/test_helpers.h"

namespace loot {
namespace test {
static const std::array<GameType, 9> ALL_GAME_TYPES = {GameType::tes3,
                                                       GameType::tes4,
                                                       GameType::tes5,
                                                       GameType::tes5se,
                                                       GameType::tes5vr,
                                                       GameType::fo3,
                                                       GameType::fonv,
                                                       GameType::fo4,
                                                       GameType::fo4vr};

class CommonGameTestFixture : public ::testing::Test {
protected:
  CommonGameTestFixture(const GameId gameId) :
      gameId_(gameId),
      rootTestPath(getTempPath()),
      missingPath(rootTestPath / "missing"),
      dataPath(rootTestPath / "games" / "game" / getPluginsFolder()),
      localPath(rootTestPath / "local" / "game"),
      lootDataPath(rootTestPath / "local" / "LOOT"),
      masterFile(getMasterFile()),
      missingEsp("Blank.missing.esp"),
      blankEsm("Blank.esm"),
      blankDifferentEsm("Blank - Different.esm"),
      blankMasterDependentEsm("Blank - Master Dependent.esm"),
      blankDifferentMasterDependentEsm(
          "Blank - Different Master Dependent.esm"),
      blankEsp("Blank.esp"),
      blankDifferentEsp("Blank - Different.esp"),
      blankMasterDependentEsp("Blank - Master Dependent.esp"),
      blankDifferentMasterDependentEsp(
          "Blank - Different Master Dependent.esp"),
      blankPluginDependentEsp("Blank - Plugin Dependent.esp"),
      blankDifferentPluginDependentEsp(
          "Blank - Different Plugin Dependent.esp"),
      nonAsciiEsp(u8"non\u00C1scii.esp"),
      blankEsmCrc(getBlankEsmCrc()) {
    assertInitialState();
  }

  void assertInitialState() {
    using std::filesystem::create_directories;
    using std::filesystem::exists;

    create_directories(dataPath);
    ASSERT_TRUE(exists(dataPath));

    create_directories(localPath);
    ASSERT_TRUE(exists(localPath));

    create_directories(lootDataPath);
    ASSERT_TRUE(exists(lootDataPath));

    if (isExecutableNeeded()) {
      touch(dataPath.parent_path() / getGameExecutable());
      ASSERT_TRUE(exists(dataPath.parent_path() / getGameExecutable()));
    }

    auto sourcePluginsPath = getSourcePluginsPath();

    copyPlugin(sourcePluginsPath, blankEsm);
    copyPlugin(sourcePluginsPath, blankDifferentEsm);
    copyPlugin(sourcePluginsPath, blankMasterDependentEsm);
    copyPlugin(sourcePluginsPath, blankDifferentMasterDependentEsm);
    copyPlugin(sourcePluginsPath, blankEsp);
    copyPlugin(sourcePluginsPath, blankDifferentEsp);
    copyPlugin(sourcePluginsPath, blankMasterDependentEsp);
    copyPlugin(sourcePluginsPath, blankDifferentMasterDependentEsp);
    copyPlugin(sourcePluginsPath, blankPluginDependentEsp);
    copyPlugin(sourcePluginsPath, blankDifferentPluginDependentEsp);

    // Make sure the game master plugin exists.
    ASSERT_NO_THROW(
        std::filesystem::copy_file(dataPath / blankEsm, dataPath / masterFile));
    ASSERT_TRUE(exists(dataPath / masterFile));

    // Create the non-ASCII plugin.
    ASSERT_NO_THROW(std::filesystem::copy_file(
        dataPath / blankEsp, dataPath / std::filesystem::u8path(nonAsciiEsp)));
    ASSERT_TRUE(exists(dataPath / std::filesystem::u8path(nonAsciiEsp)));

    // Set initial load order and active plugins.
    setLoadOrder(getInitialLoadOrder());

    // Ghost a plugin.
    ASSERT_NO_THROW(std::filesystem::rename(
        dataPath / blankMasterDependentEsm,
        dataPath / (blankMasterDependentEsm + ".ghost")));
    ASSERT_FALSE(exists(dataPath / blankMasterDependentEsm));
    ASSERT_TRUE(exists(dataPath / (blankMasterDependentEsm + ".ghost")));

    ASSERT_FALSE(exists(missingPath));
    ASSERT_FALSE(exists(dataPath / missingEsp));
  }

  void TearDown() override {
    // Grant write permissions to everything in rootTestPath
    // in case the test made anything read only.
    for (const auto& path :
         std::filesystem::recursive_directory_iterator(rootTestPath)) {
      std::filesystem::permissions(path,
                                   std::filesystem::perms::owner_write,
                                   std::filesystem::perm_options::add);
    }

    std::filesystem::remove_all(rootTestPath);
  }

  void copyPlugin(const std::filesystem::path& sourceParentPath,
                  const std::string& filename) {
    std::filesystem::copy_file(sourceParentPath / filename,
                               dataPath / filename);
    ASSERT_TRUE(std::filesystem::exists(dataPath / filename));
  }

  std::vector<std::string> readFileLines(const std::filesystem::path& file) {
    std::ifstream in(file);

    std::vector<std::string> lines;
    while (in) {
      std::string line;
      std::getline(in, line);

      if (!line.empty()) {
        lines.push_back(line);
      }
    }

    return lines;
  }

  std::vector<std::string> getLoadOrder() {
    std::vector<std::string> actual;
    if (isLoadOrderTimestampBased(getGameType())) {
      std::map<std::filesystem::file_time_type, std::string> loadOrder;
      for (std::filesystem::directory_iterator it(dataPath);
           it != std::filesystem::directory_iterator();
           ++it) {
        if (std::filesystem::is_regular_file(it->status())) {
          std::string filename = it->path().filename().u8string();
          if (boost::ends_with(filename, ".ghost"))
            filename = it->path().stem().u8string();
          if (boost::ends_with(filename, ".esp") ||
              boost::ends_with(filename, ".esm"))
            loadOrder.emplace(std::filesystem::last_write_time(it->path()),
                              filename);
        }
      }
      for (const auto& plugin : loadOrder) actual.push_back(plugin.second);
    } else if (getGameType() == GameType::tes5) {
      std::ifstream in(localPath / "loadorder.txt");
      while (in) {
        std::string line;
        std::getline(in, line);

        if (!line.empty())
          actual.push_back(line);
      }
    } else {
      actual = readFileLines(localPath / "Plugins.txt");
      for (auto& line : actual) {
        if (line[0] == '*')
          line = line.substr(1);
        line = boost::locale::conv::to_utf<char>(line, "Windows-1252");
      }
    }

    return actual;
  }

  inline std::vector<std::pair<std::string, bool>> getInitialLoadOrder() const {
    return std::vector<std::pair<std::string, bool>>({
        {masterFile, true},
        {blankEsm, true},
        {blankDifferentEsm, false},
        {blankMasterDependentEsm, false},
        {blankDifferentMasterDependentEsm, false},
        {blankEsp, false},
        {blankDifferentEsp, false},
        {blankMasterDependentEsp, false},
        {blankDifferentMasterDependentEsp, true},
        {blankPluginDependentEsp, false},
        {blankDifferentPluginDependentEsp, false},
        {nonAsciiEsp, true},
    });
  }

  GameType getGameType() const {
    switch (gameId_) {
      case GameId::tes3:
        return GameType::tes3;
      case GameId::tes4:
      case GameId::nehrim:
        return GameType::tes4;
      case GameId::tes5:
      case GameId::enderal:
        return GameType::tes5;
      case GameId::tes5se:
      case GameId::enderalse:
        return GameType::tes5se;
      case GameId::tes5vr:
        return GameType::tes5vr;
      case GameId::fo3:
        return GameType::fo3;
      case GameId::fonv:
        return GameType::fonv;
      case GameId::fo4:
        return GameType::fo4;
      case GameId::fo4vr:
        return GameType::fo4vr;
      case GameId::starfield:
        return GameType::starfield;
      default:
        throw std::logic_error("Unrecognised game ID");
    }
  }

private:
  GameId gameId_;
  const std::filesystem::path rootTestPath;

protected:
  const std::filesystem::path missingPath;
  const std::filesystem::path dataPath;
  const std::filesystem::path localPath;
  const std::filesystem::path lootDataPath;

  const std::string masterFile;
  const std::string missingEsp;
  const std::string blankEsm;
  const std::string blankDifferentEsm;
  const std::string blankMasterDependentEsm;
  const std::string blankDifferentMasterDependentEsm;
  const std::string blankEsp;
  const std::string blankDifferentEsp;
  const std::string blankMasterDependentEsp;
  const std::string blankDifferentMasterDependentEsp;
  const std::string blankPluginDependentEsp;
  const std::string blankDifferentPluginDependentEsp;
  const std::string nonAsciiEsp;

  const uint32_t blankEsmCrc;

private:
  std::filesystem::path getSourcePluginsPath() const {
    switch (getGameType()) {
      case GameType::tes3:
        return "./Morrowind/Data Files";
      case GameType::tes4:
        return "./Oblivion/Data";
      default:
        return "./Skyrim/Data";
    }
  }

  std::string getMasterFile() const {
    switch (gameId_) {
      case GameId::tes3:
        return "Morrowind.esm";
      case GameId::tes4:
        return "Oblivion.esm";
      case GameId::nehrim:
        return "Nehrim.esm";
      case GameId::tes5:
      case GameId::tes5se:
      case GameId::tes5vr:
      case GameId::enderal:
      case GameId::enderalse:
        return "Skyrim.esm";
      case GameId::fo3:
        return "Fallout3.esm";
      case GameId::fonv:
        return "FalloutNV.esm";
      case GameId::fo4:
      case GameId::fo4vr:
        return "Fallout4.esm";
      case GameId::starfield:
        return "Starfield.esm";
      default:
        throw std::logic_error("Unrecognised game ID");
    }
  }

  std::string getPluginsFolder() const {
    if (gameId_ == GameId::tes3) {
      return "Data Files";
    } else {
      return "Data";
    }
  }

  inline uint32_t getBlankEsmCrc() const {
    switch (getGameType()) {
      case GameType::tes3:
        return 0x790DC6FB;
      case GameType::tes4:
        return 0x374E2A6F;
      default:
        return 0x6A1273DC;
    }
  }

  void setLoadOrder(
      const std::vector<std::pair<std::string, bool>>& loadOrder) const {
    using std::filesystem::u8path;
    if (getGameType() == GameType::tes3) {
      std::ofstream out(dataPath.parent_path() / "Morrowind.ini");
      for (const auto& plugin : loadOrder) {
        if (plugin.second) {
          out << "GameFile0="
              << boost::locale::conv::from_utf(plugin.first, "Windows-1252")
              << std::endl;
        }
      }
    } else {
      std::ofstream out(localPath / "Plugins.txt");
      for (const auto& plugin : loadOrder) {
        if (getGameType() == GameType::fo4 ||
            getGameType() == GameType::fo4vr ||
            getGameType() == GameType::tes5se ||
            getGameType() == GameType::tes5vr) {
          if (plugin.second)
            out << '*';
        } else if (!plugin.second)
          continue;

        out << boost::locale::conv::from_utf(plugin.first, "Windows-1252")
            << std::endl;
      }
    }

    if (isLoadOrderTimestampBased(getGameType())) {
      std::filesystem::file_time_type modificationTime =
          std::filesystem::file_time_type::clock::now();
      for (const auto& plugin : loadOrder) {
        auto ghostedPath = dataPath / u8path(plugin.first + ".ghost");
        if (std::filesystem::exists(ghostedPath)) {
          std::filesystem::last_write_time(ghostedPath, modificationTime);
        } else {
          std::filesystem::last_write_time(dataPath / u8path(plugin.first),
                                           modificationTime);
        }
        modificationTime += std::chrono::seconds(60);
        ;
      }
    } else if (getGameType() == GameType::tes5) {
      std::ofstream out(localPath / "loadorder.txt");
      for (const auto& plugin : loadOrder) out << plugin.first << std::endl;
    }
  }

  inline static bool isLoadOrderTimestampBased(GameType gameType) {
    return gameType == GameType::tes3 || gameType == GameType::tes4 ||
           gameType == GameType::fo3 || gameType == GameType::fonv;
  }

  bool isExecutableNeeded() {
    const GameType gameType = getGameType();
    return gameType == GameType::tes5 || gameType == GameType::tes5se ||
           gameType == GameType::tes5vr || gameType == GameType::fo4 ||
           gameType == GameType::fo4vr;
  }

  std::string getGameExecutable() {
    switch (getGameType()) {
      case GameType::tes5:
        return "TESV.exe";
      case GameType::tes5se:
        return "SkyrimSE.exe";
      case GameType::tes5vr:
        return "SkyrimVR.exe";
      case GameType::fo4:
        return "Fallout4.exe";
      case GameType::fo4vr:
        return "Fallout4VR.exe";
      default:
        throw std::logic_error("Unexpected game type");
    }
  }
};
}
}
#endif
