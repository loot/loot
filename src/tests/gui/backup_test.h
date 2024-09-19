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

#include <boost/crc.hpp>
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
  static constexpr const char* emptyFolder = "emptyFolder";
  static constexpr const char* gitFolder = ".git";
  static constexpr const char* gitConfig = "config";
  static constexpr const char* debugLog = "LOOTDebugLog.txt";
  static constexpr const char* backupsFolder = "backups";
  static constexpr const char* backupFile = "LOOT-backup-19700101T000000.zip";
  static constexpr const char* rootDirFile = "rootFile.txt";
  static constexpr const char* subFolder = "subFolder";
  static constexpr const char* subFolderFile = "subFolderFile.txt";

  void SetUp() override {
    std::filesystem::create_directories(destRoot);
    std::filesystem::create_directories(sourceRoot / emptyFolder);
    std::filesystem::create_directories(sourceRoot / backupsFolder);
    std::filesystem::create_directories(sourceRoot / subFolder / gitFolder);

    touch(sourceRoot / debugLog);
    touch(sourceRoot / rootDirFile);
    touch(sourceRoot / backupsFolder / backupFile);
    touch(sourceRoot / subFolder / subFolderFile);
    touch(sourceRoot / subFolder / gitFolder / gitConfig);
  }

  void TearDown() override {
    std::filesystem::remove_all(sourceRoot);
    std::filesystem::remove_all(destRoot);
  }

  const std::filesystem::path sourceRoot;
  const std::filesystem::path destRoot;
};

class CompressDirectoryTest : public BackupTest {
protected:
  static size_t GetStreamSize(std::istream& stream) {
    std::streampos startingPosition = stream.tellg();

    stream.seekg(0, std::ios_base::end);
    size_t streamSize = stream.tellg();
    stream.seekg(startingPosition, std::ios_base::beg);

    return streamSize;
  }

  static uint32_t GetCrc32(const std::filesystem::path& file) {
    std::ifstream ifile(file, std::ios::binary);
    ifile.exceptions(std::ios_base::badbit | std::ios_base::failbit);

    std::array<char, 8192> buffer{};
    boost::crc_32_type result;

    size_t bytesLeft = GetStreamSize(ifile);
    while (bytesLeft > 0) {
      if (bytesLeft > buffer.size())
        ifile.read(buffer.data(), buffer.size());
      else
        ifile.read(buffer.data(), bytesLeft);

      result.process_bytes(buffer.data(), ifile.gcount());
      bytesLeft -= ifile.gcount();
    }

    return result.checksum();
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

  EXPECT_TRUE(std::filesystem::exists(destRoot / rootDirFile));
  EXPECT_TRUE(std::filesystem::exists(destRoot / subFolder / subFolderFile));
}

TEST_F(CreateBackupTest, shouldSkipDebugLogInRootDir) {
  createBackup(sourceRoot, destRoot);

  ASSERT_TRUE(std::filesystem::exists(destRoot / rootDirFile));
  ASSERT_TRUE(std::filesystem::exists(destRoot / subFolder / subFolderFile));

  EXPECT_FALSE(std::filesystem::exists(destRoot / debugLog));
}

TEST_F(CreateBackupTest, shouldSkipBackupsDirectoryInRootDir) {
  createBackup(sourceRoot, destRoot);

  ASSERT_TRUE(std::filesystem::exists(destRoot / rootDirFile));
  ASSERT_TRUE(std::filesystem::exists(destRoot / subFolder / subFolderFile));

  EXPECT_FALSE(std::filesystem::exists(destRoot / backupsFolder));
}

TEST_F(CreateBackupTest, shouldSkipDotGitFolderInAnyDirectory) {
  createBackup(sourceRoot, destRoot);

  ASSERT_TRUE(std::filesystem::exists(destRoot / rootDirFile));
  ASSERT_TRUE(std::filesystem::exists(destRoot / subFolder / subFolderFile));

  EXPECT_FALSE(std::filesystem::exists(destRoot / subFolder / gitFolder));
}

TEST_F(CreateBackupTest, shouldSkipEmptyDirectories) {
  createBackup(sourceRoot, destRoot);

  ASSERT_TRUE(std::filesystem::exists(destRoot / rootDirFile));
  ASSERT_TRUE(std::filesystem::exists(destRoot / subFolder / subFolderFile));

  EXPECT_FALSE(std::filesystem::exists(destRoot / emptyFolder));
}
}
}

#endif