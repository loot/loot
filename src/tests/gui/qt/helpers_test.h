/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2022    Oliver Hamlet

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

#ifndef LOOT_TESTS_GUI_QT_HELPERS_TEST
#define LOOT_TESTS_GUI_QT_HELPERS_TEST

#include <gtest/gtest.h>
#include <loot/exception/file_access_error.h>

#include "gui/qt/helpers.h"
#include "tests/gui/test_helpers.h"

namespace loot {
namespace test {
class QtHelpersFixture : public ::testing::Test {
protected:
  QtHelpersFixture() :
      rootPath_(getTempPath()),
      filePath_(rootPath_ / "Blank.esm"),
      fileMetadataPath_(rootPath_ / "Blank.esm.metadata.toml") {}

  void SetUp() override {
    std::filesystem::create_directories(rootPath_);

    std::filesystem::copy_file(getSourcePluginsPath(GameId::tes5) / "Blank.esm", filePath_);

    std::ofstream out(fileMetadataPath_);
    out << "blob_sha1 = \"686d51d2991e7359e636720c5cb04446257a42af\""
        << std::endl;
    out << "update_timestamp = \"2022-01-22\"";
    out.close();
  }

  void TearDown() override { std::filesystem::remove_all(rootPath_); }

  const std::filesystem::path rootPath_;
  const std::filesystem::path filePath_;
  const std::filesystem::path fileMetadataPath_;
};

class CalculateGitBlobHashTest : public QtHelpersFixture {};

class GetFileRevisionTest : public QtHelpersFixture {};

class GetFileRevisionSummaryTest : public QtHelpersFixture {};

class UpdateFileWithDataTest : public QtHelpersFixture {};

class UpdateFileTest : public QtHelpersFixture {};

TEST(calculateGitBlobHash, shouldCalculateTheSameHashAsGitDoesForABlob) {
  auto data = QByteArray("some text to hash");
  auto hash = calculateGitBlobHash(data);

  EXPECT_EQ("f0cdcd7eb1d4118095dc0fd8bf4dd448a73354c0", hash);
}

TEST_F(CalculateGitBlobHashTest, shouldCalculateTheSameHashForAFileAsGitDoes) {
  auto file = getSourcePluginsPath(GameId::tes5) / "Blank.esm";
  auto hash = calculateGitBlobHash(file);

  EXPECT_EQ("686d51d2991e7359e636720c5cb04446257a42af", hash);
}

TEST_F(CalculateGitBlobHashTest, shouldReplaceCRLFWithLFBeforeCalculatingHash) {
  auto file = rootPath_ / "text.txt";

  // On Windows the \n is written out as \r\n.
  std::ofstream out(file);
  out << "First line\n";
  out << "Second line\n";
  out.close();

  auto hash = calculateGitBlobHash(file);

  EXPECT_EQ("7d91453217afc429984c4706e8df22aaac47c9ce", hash);
}

TEST_F(GetFileRevisionTest, shouldThrowIfGivenPathIsNotARegularFile) {
  EXPECT_THROW(getFileRevision(rootPath_), FileAccessError);
}

TEST_F(GetFileRevisionTest,
       shouldThrowIfGivenPathHasNoCorrespondingMetadataFile) {
  std::filesystem::remove(fileMetadataPath_);
  EXPECT_THROW(getFileRevision(filePath_), std::runtime_error);
}

TEST_F(GetFileRevisionTest, shouldThrowIfMetadataIsMissingHashProperty) {
  std::ofstream out(fileMetadataPath_);
  out << "update_timestamp = \"2022-01-22\"";
  out.close();

  EXPECT_THROW(getFileRevision(filePath_), std::runtime_error);
}

TEST_F(GetFileRevisionTest, shouldThrowIfMetadataIsMissingTimestampProperty) {
  std::ofstream out(fileMetadataPath_);
  out << "blob_sha1 = \"686d51d2991e7359e636720c5cb04446257a42af\"";
  out.close();

  EXPECT_THROW(getFileRevision(filePath_), std::runtime_error);
}

TEST_F(GetFileRevisionTest,
       shouldReturnIsModifiedFalseIfFileHashEqualsRecordedHash) {
  auto revision = getFileRevision(filePath_);

  EXPECT_EQ("686d51d2991e7359e636720c5cb04446257a42af", revision.id);
  EXPECT_EQ("2022-01-22", revision.date);
  EXPECT_FALSE(revision.is_modified);
}

TEST_F(GetFileRevisionTest,
       shouldReturnIsModifiedTrueIfFileHashDoesNotEqualRecordedHash) {
  std::ofstream out(filePath_);
  out << "";
  out.close();

  auto revision = getFileRevision(filePath_);

  EXPECT_EQ("e69de29bb2d1d6434b8b29ae775ad8c2e48c5391", revision.id);
  EXPECT_EQ("2022-01-22", revision.date);
  EXPECT_TRUE(revision.is_modified);
}

TEST_F(GetFileRevisionSummaryTest, shouldReturnTheFileRevisionIfItCanBeRead) {
  auto summary = getFileRevisionSummary(filePath_, FileType::Masterlist);

  EXPECT_EQ("686d51d", summary.id);
  EXPECT_EQ("2022-01-22", summary.date);
}

TEST_F(GetFileRevisionSummaryTest, shouldIndicateIfTheFileHasBeenEdited) {
  std::ofstream out(filePath_);
  out << "";
  out.close();

  auto summary = getFileRevisionSummary(filePath_, FileType::Masterlist);

  EXPECT_EQ("e69de29 (edited)", summary.id);
  EXPECT_EQ("2022-01-22 (edited)", summary.date);
}

TEST_F(GetFileRevisionSummaryTest,
       shouldDisplayErrorsIfTheMasterlistCannotBeRead) {
  auto summary = getFileRevisionSummary(rootPath_, FileType::Masterlist);

  EXPECT_EQ("N/A: No masterlist present", summary.id);
  EXPECT_EQ("N/A: No masterlist present", summary.date);
}

TEST_F(GetFileRevisionSummaryTest,
       shouldDisplayErrorsIfThePreludeCannotBeRead) {
  auto summary = getFileRevisionSummary(rootPath_, FileType::MasterlistPrelude);

  EXPECT_EQ("N/A: No masterlist prelude present", summary.id);
  EXPECT_EQ("N/A: No masterlist prelude present", summary.date);
}

TEST_F(GetFileRevisionSummaryTest,
       shouldDisplayErrorsIfTheMetadataCannotBeRead) {
  std::filesystem::remove(fileMetadataPath_);
  auto summary = getFileRevisionSummary(filePath_, FileType::Masterlist);

  EXPECT_EQ("Unknown: No revision metadata found", summary.id);
  EXPECT_EQ("Unknown: No revision metadata found", summary.date);
}

TEST_F(UpdateFileWithDataTest, shouldWriteToFileIfHashesAreDifferent) {
  auto originalHash = calculateGitBlobHash(filePath_);

  auto data = QByteArray("new data");
  auto dataHash = calculateGitBlobHash(data);

  auto result = updateFileWithData(filePath_, data);
  auto newHash = calculateGitBlobHash(filePath_);

  EXPECT_TRUE(result);
  EXPECT_NE(originalHash, newHash);
  EXPECT_EQ(dataHash, newHash);
}

TEST_F(UpdateFileWithDataTest, shouldWriteMetadataFileIfHashesAreDifferent) {
  std::filesystem::remove(fileMetadataPath_);
  ASSERT_FALSE(std::filesystem::exists(fileMetadataPath_));

  auto data = QByteArray("new data");
  auto dataHash = calculateGitBlobHash(data);

  auto result = updateFileWithData(filePath_, data);

  EXPECT_TRUE(result);
  EXPECT_TRUE(std::filesystem::exists(fileMetadataPath_));

  auto revision = getFileRevision(filePath_);
  auto expectedDate = QDate::currentDate().toString(Qt::ISODate).toStdString();

  EXPECT_EQ(dataHash, revision.id);
  EXPECT_EQ(expectedDate, revision.date);
}

TEST_F(UpdateFileWithDataTest, shouldWriteMetadataFileEvenIfHashIsUnchanged) {
  std::filesystem::remove(fileMetadataPath_);
  ASSERT_FALSE(std::filesystem::exists(fileMetadataPath_));

  auto data = QByteArray("new data");
  auto dataHash = calculateGitBlobHash(data);

  std::ofstream out(filePath_);
  out << data.data();
  out.close();

  auto result = updateFileWithData(filePath_, data);

  EXPECT_FALSE(result);
  EXPECT_TRUE(std::filesystem::exists(fileMetadataPath_));

  auto revision = getFileRevision(filePath_);
  auto expectedDate = QDate::currentDate().toString(Qt::ISODate).toStdString();

  EXPECT_EQ(dataHash, revision.id);
  EXPECT_EQ(expectedDate, revision.date);
}

TEST_F(UpdateFileTest,
       shouldOverwriteDestinationWithSourceIfHashesAreDifferent) {
  auto originalHash = calculateGitBlobHash(filePath_);

  auto sourceFilePath = rootPath_ / "source";

  std::ofstream out(sourceFilePath);
  out << "new data";
  out.close();

  auto dataHash = calculateGitBlobHash(sourceFilePath);

  auto result = updateFile(sourceFilePath, filePath_);
  auto newHash = calculateGitBlobHash(filePath_);

  EXPECT_TRUE(result);
  EXPECT_NE(originalHash, newHash);
  EXPECT_EQ(dataHash, newHash);
}

TEST_F(UpdateFileTest, shouldWriteMetadataFileIfHashesAreDifferent) {
  std::filesystem::remove(fileMetadataPath_);
  ASSERT_FALSE(std::filesystem::exists(fileMetadataPath_));

  auto sourceFilePath = rootPath_ / "source";

  std::ofstream out(sourceFilePath);
  out << "new data";
  out.close();

  auto dataHash = calculateGitBlobHash(sourceFilePath);

  auto result = updateFile(sourceFilePath, filePath_);

  EXPECT_TRUE(result);
  EXPECT_TRUE(std::filesystem::exists(fileMetadataPath_));

  auto revision = getFileRevision(filePath_);
  auto expectedDate = QDate::currentDate().toString(Qt::ISODate).toStdString();

  EXPECT_EQ(dataHash, revision.id);
  EXPECT_EQ(expectedDate, revision.date);
}

TEST_F(UpdateFileTest, shouldWriteMetadataFileEvenIfHashIsUnchanged) {
  std::filesystem::remove(fileMetadataPath_);
  ASSERT_FALSE(std::filesystem::exists(fileMetadataPath_));

  auto sourceFilePath = rootPath_ / "source";

  std::ofstream out(sourceFilePath);
  out << "new data";
  out.close();

  auto dataHash = calculateGitBlobHash(sourceFilePath);

  std::filesystem::copy_file(sourceFilePath,
                             filePath_,
                             std::filesystem::copy_options::overwrite_existing);

  auto result = updateFile(sourceFilePath, filePath_);

  EXPECT_FALSE(result);
  EXPECT_TRUE(std::filesystem::exists(fileMetadataPath_));

  auto revision = getFileRevision(filePath_);
  auto expectedDate = QDate::currentDate().toString(Qt::ISODate).toStdString();

  EXPECT_EQ(dataHash, revision.id);
  EXPECT_EQ(expectedDate, revision.date);
}

TEST(isValidUrl, shouldBeFalseForALocalWindowsPath) {
  auto result = isValidUrl("C:\\Users\\user\\file");

  EXPECT_FALSE(result);
}

TEST(isValidUrl, shouldBeFalseForALocalLinuxPath) {
  auto result = isValidUrl("/home/user/file");

  EXPECT_FALSE(result);
}

TEST(isValidUrl, shouldBeTrueForAHttpUrl) {
  auto result = isValidUrl("http://example.com/file");

  EXPECT_TRUE(result);
}

TEST(isValidUrl, shouldBeTrueForAHttpsUrl) {
  auto result = isValidUrl("https://example.com/file");

  EXPECT_TRUE(result);
}
}
}

#endif
