/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2018    WrinklyNinja

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

#include <filesystem>
#include <fstream>
#include <map>
#include <unordered_set>

#include <gtest/gtest.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "loot/enum/game_type.h"

namespace loot {
namespace test {

std::filesystem::path getRootTestPath() {
  auto directoryName = "LOOT-" + boost::lexical_cast<std::string>(
                                      (boost::uuids::random_generator())());
  return std::filesystem::absolute(std::filesystem::temp_directory_path() /
                                    directoryName);
}

class CommonGameTestFixture : public ::testing::TestWithParam<GameType> {
protected:
  CommonGameTestFixture() :
      rootTestPath(getRootTestPath()),
      missingPath(rootTestPath / "missing"),
      dataPath(rootTestPath / "game" / "Data"),
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

    // Make sure the game master file exists.
    ASSERT_NO_THROW(
        std::filesystem::copy_file(dataPath / blankEsm, dataPath / masterFile));
    ASSERT_TRUE(exists(dataPath / masterFile));

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

  void TearDown() {
    // Grant write permissions to everything in rootTestPath
    // in case the test made anything read only.
    for (const auto& path : std::filesystem::recursive_directory_iterator(rootTestPath)) {
      std::filesystem::permissions(path, std::filesystem::perms::owner_write, std::filesystem::perm_options::add);
    }

    std::filesystem::remove_all(rootTestPath);
  }

  void copyPlugin(const std::filesystem::path& sourceParentPath, const std::string& filename) {
    std::filesystem::copy_file(sourceParentPath / filename, dataPath / filename);
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
    if (isLoadOrderTimestampBased(GetParam())) {
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
    } else if (GetParam() == GameType::tes5) {
      std::ifstream in(localPath / "loadorder.txt");
      while (in) {
        std::string line;
        std::getline(in, line);

        if (!line.empty())
          actual.push_back(line);
      }
    } else {
      actual = readFileLines(localPath / "plugins.txt");
      for (auto& line : actual) {
        if (line[0] == '*')
          line = line.substr(1);
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
    });
  }

private:
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

  const uint32_t blankEsmCrc;

private:
  std::filesystem::path getSourcePluginsPath() const {
    if (GetParam() == GameType::tes4)
      return "./Oblivion/Data";
    else
      return "./Skyrim/Data";
  }

  inline std::string getMasterFile() const {
    if (GetParam() == GameType::tes4)
      return "Oblivion.esm";
    else if (GetParam() == GameType::tes5 || GetParam() == GameType::tes5se)
      return "Skyrim.esm";
    else if (GetParam() == GameType::fo3)
      return "Fallout3.esm";
    else if (GetParam() == GameType::fonv)
      return "FalloutNV.esm";
    else
      return "Fallout4.esm";
  }

  inline uint32_t getBlankEsmCrc() const {
    if (GetParam() == GameType::tes4)
      return 0x374E2A6F;
    else
      return 0x187BE342;
  }

  void setLoadOrder(
      const std::vector<std::pair<std::string, bool>>& loadOrder) const {
    std::ofstream out(localPath / "plugins.txt");
    for (const auto& plugin : loadOrder) {
      if (GetParam() == GameType::fo4 || GetParam() == GameType::tes5se) {
        if (plugin.second)
          out << '*';
      } else if (!plugin.second)
        continue;

      out << plugin.first << std::endl;
    }

    if (isLoadOrderTimestampBased(GetParam())) {
      std::filesystem::file_time_type modificationTime =
        std::filesystem::file_time_type::clock::now();
      for (const auto& plugin : loadOrder) {
        if (std::filesystem::exists(
                dataPath / std::filesystem::path(plugin.first + ".ghost"))) {
          std::filesystem::last_write_time(
              dataPath / std::filesystem::path(plugin.first + ".ghost"),
              modificationTime);
        } else {
          std::filesystem::last_write_time(dataPath / plugin.first,
                                             modificationTime);
        }
        modificationTime += std::chrono::seconds(60);;
      }
    } else if (GetParam() == GameType::tes5) {
      std::ofstream out(localPath / "loadorder.txt");
      for (const auto& plugin : loadOrder) out << plugin.first << std::endl;
    }
  }

  inline static bool isLoadOrderTimestampBased(GameType gameType) {
    return gameType == GameType::tes4 || gameType == GameType::fo3 ||
           gameType == GameType::fonv;
  }
};
}
}
#endif
