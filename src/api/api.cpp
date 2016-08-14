/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013-2016    WrinklyNinja

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

#include "loot/api.h"

#include <boost/filesystem.hpp>
#include <boost/log/core.hpp>

#include "api/api_database.h"
#include "loot/error.h"
#include "backend/app/loot_paths.h"

namespace loot {
LOOT_API bool IsCompatible(const unsigned int versionMajor, const unsigned int versionMinor, const unsigned int versionPatch) {
  if (versionMajor > 0)
    return versionMajor == loot::LootVersion::major;
  else
    return versionMinor == loot::LootVersion::minor;
}

LOOT_API std::shared_ptr<DatabaseInterface> CreateDatabase(const GameType game,
                                                           const std::string& gamePath,
                                                           const std::string& gameLocalPath) {
  loot::LootPaths::initialise();

  //Disable logging or else stdout will get overrun.
  boost::log::core::get()->set_logging_enabled(false);

  // Check for valid paths.
  if (!gamePath.empty() && !boost::filesystem::is_directory(gamePath))
    throw loot::Error(Error::Code::invalid_args, "Given game path \"" + std::string(gamePath) + "\" is not a valid directory.");

  if (!gameLocalPath.empty() && !boost::filesystem::is_directory(gameLocalPath))
    throw loot::Error(Error::Code::invalid_args, "Given local data path \"" + std::string(gameLocalPath) + "\" is not a valid directory.");

  return std::make_shared<ApiDatabase>(game, gamePath, gameLocalPath);
}
}
