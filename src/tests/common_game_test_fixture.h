/*  LOOT

    A modding utility for Starfield and some Elder Scrolls and Fallout games.

    Copyright (C) 2013-2026 Oliver Hamlet

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
#include <boost/algorithm/string/predicate.hpp>
#include <boost/locale/encoding.hpp>
#include <filesystem>
#include <fstream>
#include <map>

#include "gui/state/game/game_id.h"
#include "loot/enum/game_type.h"
#include "tests/gui/test_helpers.h"

namespace loot {
namespace test {
inline constexpr const char* MISSING_ESP{"Blank.missing.esp"};
inline constexpr const char* BLANK_ESM{"Blank.esm"};
inline constexpr const char* BLANK_DIFFERENT_ESM{"Blank - Different.esm"};
inline constexpr const char* BLANK_MASTER_DEPENDENT_ESM{
    "Blank - Master Dependent.esm"};
inline constexpr const char* BLANK_DIFFERENT_MASTER_DEPENDENT_ESM{
    "Blank - Different Master Dependent.esm"};
inline constexpr const char* BLANK_ESP{"Blank.esp"};
inline constexpr const char* BLANK_DIFFERENT_ESP{"Blank - Different.esp"};
inline constexpr const char* BLANK_MASTER_DEPENDENT_ESP{
    "Blank - Master Dependent.esp"};
inline constexpr const char* BLANK_DIFFERENT_MASTER_DEPENDENT_ESP{
    "Blank - Different Master Dependent.esp"};
inline constexpr const char* BLANK_PLUGIN_DEPENDENT_ESP{
    "Blank - Plugin Dependent.esp"};
inline constexpr const char* BLANK_DIFFERENT_PLUGIN_DEPENDENT_ESP{
    "Blank - Different Plugin Dependent.esp"};
inline constexpr const char* NON_ASCII_ESP{u8"non\u00C1scii.esp"};

std::string getPluginsFolder(GameId gameId) {
  if (gameId == GameId::tes3) {
    return "Data Files";
  } else if (gameId == GameId::openmw) {
    return "resources/vfs";
  } else if (gameId == GameId::oblivionRemastered) {
    return "OblivionRemastered/Content/Dev/ObvData/Data";
  } else {
    return "Data";
  }
}

std::string getMasterFile(GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
      return "Morrowind.esm";
    case GameId::tes4:
    case GameId::oblivionRemastered:
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
    case GameId::openmw:
      return "builtin.omwscripts";
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

bool isExecutableNeeded(GameId gameId) {
  return gameId == GameId::tes5 || gameId == GameId::enderal ||
         gameId == GameId::tes5se || gameId == GameId::enderalse ||
         gameId == GameId::tes5vr || gameId == GameId::fo4 ||
         gameId == GameId::fo4vr || gameId == GameId::tes3 ||
         gameId == GameId::openmw;
}

std::string getGameExecutable(GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
      return "Morrowind.exe";
    case GameId::tes5:
    case GameId::enderal:
      return "TESV.exe";
    case GameId::tes5se:
    case GameId::enderalse:
      return "SkyrimSE.exe";
    case GameId::tes5vr:
      return "SkyrimVR.exe";
    case GameId::fo4:
      return "Fallout4.exe";
    case GameId::fo4vr:
      return "Fallout4VR.exe";
    case GameId::openmw:
#ifdef _WIN32
      return "openmw.exe";
#else
      return "openmw";
#endif
    default:
      throw std::logic_error("Unexpected game type");
  }
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

bool isLoadOrderTimestampBased(GameId gameId) {
  return gameId == GameId::tes3 || gameId == GameId::tes4 ||
         gameId == GameId::nehrim || gameId == GameId::fo3 ||
         gameId == GameId::fonv;
}

class FilesystemTest : public ::testing::Test {
protected:
  FilesystemTest() : rootPath_(getTempPath()) {
    std::filesystem::create_directories(rootPath_);
  }

  void TearDown() override { std::filesystem::remove_all(rootPath_); }

  std::filesystem::path rootPath_;
};

class BaseGameDetectionTest : public FilesystemTest {
protected:
  explicit BaseGameDetectionTest(GameId gameId) :
      gameId_(gameId),
      gamePath(rootPath_ / "game"),
      dataPath(gamePath / getPluginsFolder(gameId_)) {
    touch(dataPath / getMasterFile(gameId_));
    if (isExecutableNeeded(gameId_)) {
      touch(gamePath / getGameExecutable(gameId_));
    }
  }

  void TearDown() override {
    // Grant write permissions to everything in rootTestPath
    // in case the test made anything read only.
    for (const auto& entry :
         std::filesystem::recursive_directory_iterator(rootPath_)) {
      if (!entry.is_symlink()) {
        std::filesystem::permissions(entry,
                                     std::filesystem::perms::owner_write,
                                     std::filesystem::perm_options::add);
      }
    }

    FilesystemTest::TearDown();
  }

  GameId gameId_;
  std::filesystem::path gamePath;
  std::filesystem::path dataPath;
};

class CommonGameTestFixture : public FilesystemTest {
protected:
  explicit CommonGameTestFixture(const GameId gameId) :
      gameId_(gameId),
      gamePath(rootPath_ / "games" / "game"),
      dataPath(gamePath / getPluginsFolder(gameId)),
      localPath(rootPath_ / "local" / "game"),
      lootDataPath(rootPath_ / "local" / "LOOT") {
    assertInitialState();
  }

  void assertInitialState() {
    using std::filesystem::create_directories;

    create_directories(dataPath);
    create_directories(localPath);
    create_directories(lootDataPath);
  }

  void copyPlugin(std::string_view filename) { copyPlugin(filename, filename); }

  void copyPlugin(std::string_view sourceFilename,
                  std::string_view destinationFilename) {
    const auto destinationPath =
        dataPath / std::filesystem::u8path(destinationFilename);

    std::filesystem::copy_file(
        getSourcePluginsPath() / std::filesystem::u8path(sourceFilename),
        destinationPath);

    ASSERT_TRUE(std::filesystem::exists(destinationPath));
  }

  std::vector<std::string> getLoadOrder() const {
    std::vector<std::string> actual;
    if (isLoadOrderTimestampBased(gameId_)) {
      std::map<std::filesystem::file_time_type, std::string> loadOrder;
      for (const auto& entry : std::filesystem::directory_iterator(dataPath)) {
        if (entry.is_regular_file()) {
          std::string filename = entry.path().filename().u8string();
          if (boost::ends_with(filename, ".ghost")) {
            filename = entry.path().stem().u8string();
          }
          if (boost::ends_with(filename, ".esp") ||
              boost::ends_with(filename, ".esm")) {
            loadOrder.emplace(std::filesystem::last_write_time(entry.path()),
                              filename);
          }
        }
      }
      for (const auto& plugin : loadOrder) actual.push_back(plugin.second);
    } else if (gameId_ == GameId::tes5 || gameId_ == GameId::enderal ||
               gameId_ == GameId::oblivionRemastered) {
      const auto& parentPath =
          gameId_ == GameId::oblivionRemastered ? dataPath : localPath;
      std::ifstream in(parentPath / "loadorder.txt");
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

  std::filesystem::path getSourcePluginsPath() const {
    return loot::test::getSourcePluginsPath(gameId_);
  }

private:
  GameId gameId_;

protected:
  std::filesystem::path gamePath;
  std::filesystem::path dataPath;
  std::filesystem::path localPath;
  std::filesystem::path lootDataPath;

  void setLoadOrder(
      const std::vector<std::pair<std::string, bool>>& loadOrder) const {
    using std::filesystem::u8path;
    if (gameId_ == GameId::tes3) {
      std::ofstream out(gamePath / "Morrowind.ini");
      out << "[Game Files]" << std::endl;

      size_t activeIndex = 0;
      for (const auto& plugin : loadOrder) {
        if (plugin.second) {
          out << "GameFile" << activeIndex << "="
              << boost::locale::conv::from_utf(plugin.first, "Windows-1252")
              << std::endl;
          activeIndex += 1;
        }
      }
    } else {
      const auto& parentPath =
          gameId_ == GameId::oblivionRemastered ? dataPath : localPath;
      std::ofstream out(parentPath / "Plugins.txt");
      for (const auto& plugin : loadOrder) {
        if (gameId_ == GameId::fo4 || gameId_ == GameId::fo4vr ||
            gameId_ == GameId::tes5se || gameId_ == GameId::enderalse ||
            gameId_ == GameId::tes5vr) {
          if (plugin.second)
            out << '*';
        } else if (!plugin.second)
          continue;

        out << boost::locale::conv::from_utf(plugin.first, "Windows-1252")
            << std::endl;
      }
    }

    if (isLoadOrderTimestampBased(gameId_)) {
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
    } else if (gameId_ == GameId::tes5 || gameId_ == GameId::enderal ||
               gameId_ == GameId::oblivionRemastered) {
      const auto& parentPath =
          gameId_ == GameId::oblivionRemastered ? dataPath : localPath;
      std::ofstream out(parentPath / "loadorder.txt");
      for (const auto& plugin : loadOrder) out << plugin.first << std::endl;
    }
  }
};
}
}
#endif
