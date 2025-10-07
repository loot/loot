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
#ifndef LOOT_TESTS_GUI_BACKUP_TEST
#define LOOT_TESTS_GUI_BACKUP_TEST

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "gui/backup.h"
#include "tests/gui/test_helpers.h"

namespace loot {
namespace test {
class BackupTest : public ::testing::Test {
public:
  BackupTest() : sourceRoot(getTempPath()), destRoot(getTempPath()) {}

protected:
  static constexpr const char* EMPTY_FOLDER = "emptyFolder";
  static constexpr const char* GIT_FOLDER = ".git";
  static constexpr const char* DEBUG_LOG = "LOOTDebugLog.txt";
  static constexpr const char* BACKUPS_FOLDER = "backups";
  static constexpr const char* ROOT_DIR_FILE = "rootFile.txt";
  static constexpr const char* SUB_FOLDER = "subFolder";
  static constexpr const char* SUB_FOLDER_FILE = "subFolderFile.txt";

  void SetUp() override {
    static constexpr const char* GIT_CONFIG = "config";
    static constexpr const char* BACKUP_FILE =
        "LOOT-backup-19700101T000000.zip";

    std::filesystem::create_directories(destRoot);
    std::filesystem::create_directories(sourceRoot / EMPTY_FOLDER);
    std::filesystem::create_directories(sourceRoot / BACKUPS_FOLDER);
    std::filesystem::create_directories(sourceRoot / SUB_FOLDER / GIT_FOLDER);

    touch(sourceRoot / DEBUG_LOG);
    touch(sourceRoot / ROOT_DIR_FILE);
    touch(sourceRoot / BACKUPS_FOLDER / BACKUP_FILE);
    touch(sourceRoot / SUB_FOLDER / SUB_FOLDER_FILE);
    touch(sourceRoot / SUB_FOLDER / GIT_FOLDER / GIT_CONFIG);
  }

  void TearDown() override {
    std::filesystem::remove_all(sourceRoot);
    std::filesystem::remove_all(destRoot);
  }

  std::filesystem::path sourceRoot;
  std::filesystem::path destRoot;
};

class CompressDirectoryTest : public BackupTest {
protected:
  static size_t getStreamSize(std::istream& stream) {
    std::streampos startingPosition = stream.tellg();

    stream.seekg(0, std::ios_base::end);
    size_t streamSize = stream.tellg();
    stream.seekg(startingPosition, std::ios_base::beg);

    return streamSize;
  }
};

class CreateBackupTest : public BackupTest {};

TEST_F(CompressDirectoryTest, shouldReturnThePathToAZipOfTheInput) {
  createBackup(sourceRoot, destRoot);

  auto archivePath = compressDirectory(destRoot);

  EXPECT_TRUE(std::filesystem::exists(archivePath));

  // Just check the file size because minizip-ng is not compiled with support
  // for decompression. The size is different on Windows and Linux due to
  // different amounts of filesystem metadata being stored.
#ifdef _WIN32
  auto expectedSize = 656;
#else
  auto expectedSize = 440;
#endif

  auto archiveSize = std::filesystem::file_size(archivePath);
  EXPECT_EQ(expectedSize, archiveSize);

  std::filesystem::remove(archivePath);
}

TEST_F(CreateBackupTest, shouldRecursivelyCopyFilesInSourceDirToDestDir) {
  createBackup(sourceRoot, destRoot);

  EXPECT_TRUE(std::filesystem::exists(destRoot / ROOT_DIR_FILE));
  EXPECT_TRUE(std::filesystem::exists(destRoot / SUB_FOLDER / SUB_FOLDER_FILE));
}

TEST_F(CreateBackupTest, shouldSkipDebugLogInRootDir) {
  createBackup(sourceRoot, destRoot);

  ASSERT_TRUE(std::filesystem::exists(destRoot / ROOT_DIR_FILE));
  ASSERT_TRUE(std::filesystem::exists(destRoot / SUB_FOLDER / SUB_FOLDER_FILE));

  EXPECT_FALSE(std::filesystem::exists(destRoot / DEBUG_LOG));
}

TEST_F(CreateBackupTest, shouldSkipBackupsDirectoryInRootDir) {
  createBackup(sourceRoot, destRoot);

  ASSERT_TRUE(std::filesystem::exists(destRoot / ROOT_DIR_FILE));
  ASSERT_TRUE(std::filesystem::exists(destRoot / SUB_FOLDER / SUB_FOLDER_FILE));

  EXPECT_FALSE(std::filesystem::exists(destRoot / BACKUPS_FOLDER));
}

TEST_F(CreateBackupTest, shouldSkipDotGitFolderInAnyDirectory) {
  createBackup(sourceRoot, destRoot);

  ASSERT_TRUE(std::filesystem::exists(destRoot / ROOT_DIR_FILE));
  ASSERT_TRUE(std::filesystem::exists(destRoot / SUB_FOLDER / SUB_FOLDER_FILE));

  EXPECT_FALSE(std::filesystem::exists(destRoot / SUB_FOLDER / GIT_FOLDER));
}

TEST_F(CreateBackupTest, shouldSkipEmptyDirectories) {
  createBackup(sourceRoot, destRoot);

  ASSERT_TRUE(std::filesystem::exists(destRoot / ROOT_DIR_FILE));
  ASSERT_TRUE(std::filesystem::exists(destRoot / SUB_FOLDER / SUB_FOLDER_FILE));

  EXPECT_FALSE(std::filesystem::exists(destRoot / EMPTY_FOLDER));
}
}
}

#endif
