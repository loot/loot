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
#ifndef LOOT_TESTS_GUI_TEST_HELPERS
#define LOOT_TESTS_GUI_TEST_HELPERS

#include <filesystem>
#include <fstream>
#include <random>
#include <string>

#include "gui/state/game/detection/game_install.h"

namespace loot {
namespace test {
std::filesystem::path getSourcePluginsPath(GameId gameType) {
  switch (gameType) {
    case GameId::tes3:
      return "./testing-plugins/Morrowind/Data Files";
    case GameId::tes4:
    case GameId::nehrim:
      return "./testing-plugins/Oblivion/Data";
    default:
      return "./testing-plugins/Skyrim/Data";
  }
}

std::filesystem::path getTempPath() {
  std::random_device randomDevice;
  std::default_random_engine prng(randomDevice());
  std::uniform_int_distribution dist(0x61,
                                     0x7A);  // values of a-z in ASCII/UTF-8.

  // The non-ASCII character is there to ensure test coverage of non-ASCII path
  // handling.
  std::string directoryName = u8"LOOT-t\u00E9st-";

  for (int i = 0; i < 16; i += 1) {
    directoryName.push_back(static_cast<char>(dist(prng)));
  }

  return std::filesystem::absolute(std::filesystem::temp_directory_path() /
                                   std::filesystem::u8path(directoryName));
}

void touch(const std::filesystem::path& path) {
  std::filesystem::create_directories(path.parent_path());
  std::ofstream out(path);
  out.close();
}
}
}

#endif
