/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

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

#include "gui/state/game/group_node_positions.h"

#include <fstream>

namespace {
constexpr uint32_t LGNP_MAGIC_NUMBER = 0x504E474C;
constexpr uint8_t LGNP_FORMAT_VERSION = 1;

size_t readStringLength(std::istream& in) {
  uint16_t length{0};
  in.read(reinterpret_cast<char*>(&length), sizeof length);

  return length;
}

// Write string length as a 16-bit unsigned integer.
// Throw if the string length doesn't fit in 16 bits, because this should only
// be used for group names and no group name should be that long.
void writeStringLength(std::ostream& out, size_t length) {
  // Don't care about endianness because the files don't need to be portable.
  if (length <= UINT16_MAX) {
    const auto castLength = static_cast<uint16_t>(length);

    out.write(reinterpret_cast<const char*>(&castLength), sizeof castLength);
  } else {
    throw std::runtime_error("Cannot write length of string longer than " +
                             std::to_string(UINT16_MAX) + " bytes");
  }
}
}

namespace loot {
std::vector<GroupNodePosition> loadGroupNodePositions(
    const std ::filesystem::path& filePath) {
  if (!std::filesystem::exists(filePath)) {
    return {};
  }

  std::ifstream in(filePath, std::ios_base::in | std::ios_base::binary);
  if (!in.is_open()) {
    throw std::runtime_error(filePath.u8string() +
                             " could not be opened for parsing");
  }

  uint32_t magicNumber{0};
  in.read(reinterpret_cast<char*>(&magicNumber), sizeof magicNumber);

  if (magicNumber != LGNP_MAGIC_NUMBER) {
    throw std::runtime_error("Failed to parse " + filePath.u8string() +
                             ": wrong magic number");
  }

  uint8_t formatVersion{0};
  in.read(reinterpret_cast<char*>(&formatVersion), sizeof formatVersion);

  if (formatVersion != LGNP_FORMAT_VERSION) {
    throw std::runtime_error("Failed to parse " + filePath.u8string() +
                             ": unrecognised format version");
  }

  std::vector<GroupNodePosition> nodePositions;
  while (in.good()) {
    const auto stringLength = readStringLength(in);

    if (!in.good()) {
      // Handle reaching end of file.
      break;
    }

    std::string name(stringLength, '\0');
    in.read(name.data(), static_cast<std::streamsize>(stringLength));

    double x{0.0};
    in.read(reinterpret_cast<char*>(&x), sizeof x);

    double y{0.0};
    in.read(reinterpret_cast<char*>(&y), sizeof y);

    nodePositions.push_back(GroupNodePosition{name, x, y});
  }

  return nodePositions;
}

void saveGroupNodePositions(const std ::filesystem::path& filePath,
                            const std::vector<GroupNodePosition>& positions) {
  // Don't care about endianness because the files don't need to be portable.

  std::ofstream out(
      filePath,
      std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  if (!out.is_open()) {
    throw std::runtime_error(filePath.u8string() +
                             " could not be opened for writing");
  }

  out.write(reinterpret_cast<const char*>(&LGNP_MAGIC_NUMBER),
            sizeof LGNP_MAGIC_NUMBER);
  out.write(reinterpret_cast<const char*>(&LGNP_FORMAT_VERSION),
            sizeof LGNP_FORMAT_VERSION);

  for (const auto& nodePosition : positions) {
    writeStringLength(out, nodePosition.groupName.size());

    // Don't write the null terminator as it's unnecessary.
    out.write(nodePosition.groupName.c_str(),
              static_cast<std::streamsize>(nodePosition.groupName.size()));

    out.write(reinterpret_cast<const char*>(&nodePosition.x),
              sizeof nodePosition.x);
    out.write(reinterpret_cast<const char*>(&nodePosition.y),
              sizeof nodePosition.y);
  }
}
}
