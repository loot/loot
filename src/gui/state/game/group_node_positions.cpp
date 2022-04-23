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

#include <cpptoml.h>

#include <fstream>

#include "gui/state/logging.h"

namespace loot {
std::vector<GroupNodePosition> LoadGroupNodePositions(
    const std ::filesystem::path& filePath) {
  if (!std::filesystem::exists(filePath)) {
    return {};
  }

  std::ifstream in(filePath);
  if (!in.is_open()) {
    throw cpptoml::parse_exception(filePath.u8string() +
                                   " could not be opened for parsing");
  }

  const auto settings = cpptoml::parser(in).parse();

  const auto positionsTable = settings->get_table_array("positions");
  if (!positionsTable) {
    return {};
  }

  const auto logger = getLogger();

  std::vector<GroupNodePosition> nodePositions;
  for (const auto& positionTable : *positionsTable) {
    const auto name = positionTable->get_as<std::string>("name");
    const auto x = positionTable->get_as<double>("x");
    const auto y = positionTable->get_as<double>("y");

    if (name && x && y) {
      nodePositions.push_back(GroupNodePosition{*name, *x, *y});
    } else {
      logger->error(
          "Failed to read position for node, name or x or y are "
          "missing.");
    }
  }

  return nodePositions;
}

void SaveGroupNodePositions(const std ::filesystem::path& filePath,
                            const std::vector<GroupNodePosition>& positions) {
  std::ofstream out(filePath);
  if (!out.is_open()) {
    throw cpptoml::parse_exception(filePath.u8string() +
                                   " could not be opened for writing");
  }

  auto root = cpptoml::make_table();

  if (!positions.empty()) {
    auto positionTables = cpptoml::make_table_array();

    for (const auto& nodePosition : positions) {
      auto positionTable = cpptoml::make_table();
      positionTable->insert("name", nodePosition.groupName);
      positionTable->insert("x", nodePosition.x);
      positionTable->insert("y", nodePosition.y);

      positionTables->push_back(positionTable);
    }

    root->insert("positions", positionTables);
  }

  out << *root;
}
}
