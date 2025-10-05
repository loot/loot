/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2018    Oliver Hamlet

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

#ifndef LOOT_TESTS_GUI_STATE_GAME_GROUP_NODE_POSITIONS_TEST
#define LOOT_TESTS_GUI_STATE_GAME_GROUP_NODE_POSITIONS_TEST

#include <gtest/gtest.h>

#include "gui/state/game/group_node_positions.h"
#include "tests/gui/test_helpers.h"

namespace loot {
namespace test {
class GroupNodePositionsFixture : public ::testing::Test {
protected:
  GroupNodePositionsFixture() :
      rootPath_(getTempPath()), filePath_(rootPath_ / "positions.bin") {}

  void SetUp() override {
    std::filesystem::create_directories(rootPath_);

    std::ofstream out(filePath_);
    out << "blob_sha1 = \"686d51d2991e7359e636720c5cb04446257a42af\""
        << std::endl;
    out << "update_timestamp = \"2022-01-22\"";
    out.close();
  }

  void TearDown() override { std::filesystem::remove_all(rootPath_); }

  void writeBytes(const std::filesystem::path& path,
                  const std::vector<char>& bytes) {
    std::ofstream out(path, std::ios::binary | std::ios_base::trunc);

    for (const auto byte : bytes) {
      out.put(byte);
    }
  }

  std::vector<char> readBytes(const std::filesystem::path& path) {
    std::vector<char> bytes;

    std::ifstream in(path, std::ios::binary);

    std::copy(std::istreambuf_iterator<char>(in),
              std::istreambuf_iterator<char>(),
              std::back_inserter(bytes));

    return bytes;
  }

  const std::filesystem::path rootPath_;
  const std::filesystem::path filePath_;
};

class LoadGroupNodePositionsTest : public GroupNodePositionsFixture {};

class SaveGroupNodePositionsTest : public GroupNodePositionsFixture {};

TEST_F(LoadGroupNodePositionsTest,
       shouldReturnAnEmptyVectorIfFileDoesNotExist) {
  const auto positions = loadGroupNodePositions(rootPath_ / "missing.bin");

  EXPECT_TRUE(positions.empty());
}

TEST_F(LoadGroupNodePositionsTest, shouldThrowIfFileCannotBeOpened) {
  const auto path = rootPath_ / "missing.dir";

  std::filesystem::create_directory(path);

  EXPECT_THROW(loadGroupNodePositions(path), std::runtime_error);
}

TEST_F(LoadGroupNodePositionsTest, shouldThrowIfFileIsTooShort) {
  const auto path = rootPath_ / "positions.bin";

  touch(path);

  EXPECT_THROW(loadGroupNodePositions(path), std::runtime_error);
}

TEST_F(LoadGroupNodePositionsTest, shouldThrowIfFileMagicNumberIsUnexpected) {
  const auto path = rootPath_ / "positions.bin";

  writeBytes(path, {'\xDE', '\xAD', '\xBE', '\xEF'});

  EXPECT_THROW(loadGroupNodePositions(path), std::runtime_error);
}

TEST_F(LoadGroupNodePositionsTest,
       shouldThrowIfFileFormatVersionIsUnrecognised) {
  const auto path = rootPath_ / "positions.bin";

  writeBytes(path, {'\x4C', '\x47', '\x4E', '\x50', '\x0'});

  EXPECT_THROW(loadGroupNodePositions(path), std::runtime_error);
}

TEST_F(LoadGroupNodePositionsTest,
       shouldAcceptARecognisedMagicNumberAndFormatVersion) {
  const auto path = rootPath_ / "positions.bin";

  writeBytes(path, {'\x4C', '\x47', '\x4E', '\x50', '\x1'});

  const auto positions = loadGroupNodePositions(path);

  EXPECT_TRUE(positions.empty());
}

TEST_F(LoadGroupNodePositionsTest, shouldAcceptDataWrittenBySave) {
  const auto path = rootPath_ / "positions.bin";

  std::vector<GroupNodePosition> originalPositions = {
      GroupNodePosition{"default", 1.1, 2.2},
      GroupNodePosition{"DLC", -3.0, -4.5}};

  saveGroupNodePositions(path, originalPositions);

  const auto positions = loadGroupNodePositions(path);

  ASSERT_EQ(2, positions.size());

  EXPECT_EQ(originalPositions[0].groupName, positions[0].groupName);
  EXPECT_EQ(originalPositions[0].x, positions[0].x);
  EXPECT_EQ(originalPositions[0].y, positions[0].y);

  EXPECT_EQ(originalPositions[1].groupName, positions[1].groupName);
  EXPECT_EQ(originalPositions[1].x, positions[1].x);
  EXPECT_EQ(originalPositions[1].y, positions[1].y);
}

TEST_F(SaveGroupNodePositionsTest, shouldThrowIfFileCannotBeOpened) {
  const auto path = rootPath_ / "missing.dir";

  std::filesystem::create_directory(path);

  EXPECT_THROW(saveGroupNodePositions(path, {}), std::runtime_error);
}

TEST_F(SaveGroupNodePositionsTest, shouldWriteMagicNumberAndFormatVersion) {
  const auto path = rootPath_ / "positions.bin";

  saveGroupNodePositions(path, {});

  const auto bytes = readBytes(path);

  ASSERT_EQ(5, bytes.size());

  EXPECT_EQ(0x4C, bytes[0]);
  EXPECT_EQ(0x47, bytes[1]);
  EXPECT_EQ(0x4E, bytes[2]);
  EXPECT_EQ(0x50, bytes[3]);
  EXPECT_EQ(0x1, bytes[4]);
}

TEST_F(SaveGroupNodePositionsTest, shouldWriteNamesPrefixedWithTheirLengths) {
  const auto path = rootPath_ / "positions.bin";

  std::vector<GroupNodePosition> positions = {
      GroupNodePosition{"default", 1.1, 2.2},
      GroupNodePosition{"DLC", -3.0, -4.5}};

  saveGroupNodePositions(path, positions);

  const auto bytes = readBytes(path);

  ASSERT_EQ(51, bytes.size());

  EXPECT_EQ(0x4C, bytes[0]);
  EXPECT_EQ(0x47, bytes[1]);
  EXPECT_EQ(0x4E, bytes[2]);
  EXPECT_EQ(0x50, bytes[3]);
  EXPECT_EQ(0x1, bytes[4]);

  EXPECT_EQ(0x7, bytes[5]);
  EXPECT_EQ(0, bytes[6]);
  EXPECT_EQ("default", std::string(bytes.begin() + 7, bytes.begin() + 14));

  EXPECT_EQ(1.1, *reinterpret_cast<const double*>(&bytes[14]));
  EXPECT_EQ(2.2, *reinterpret_cast<const double*>(&bytes[22]));

  EXPECT_EQ(0x3, bytes[30]);
  EXPECT_EQ(0, bytes[31]);
  EXPECT_EQ("DLC", std::string(bytes.begin() + 32, bytes.begin() + 35));

  EXPECT_EQ(-3.0, *reinterpret_cast<const double*>(&bytes[35]));
  EXPECT_EQ(-4.5, *reinterpret_cast<const double*>(&bytes[43]));
}
}
}

#endif
