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

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <filesystem>
#include <fstream>
#include <string>

namespace loot {
namespace test {
std::filesystem::path getTempPath() {
  auto directoryName =
      u8"LOOT-t\u00E9st-" +
      boost::lexical_cast<std::string>((boost::uuids::random_generator())());

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
