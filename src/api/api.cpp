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
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>

#include "api/game/game.h"

namespace fs = boost::filesystem;

namespace loot {
std::string ResolvePath(const std::string& path) {
  if (path.empty() || !fs::is_symlink(path))
    return path;

  return fs::read_symlink(path).string();
}

LOOT_API void SetLoggingVerbosity(LogVerbosity verbosity) {
  switch (verbosity) {
  case LogVerbosity::off:
    boost::log::core::get()->set_logging_enabled(false);
    break;
  case LogVerbosity::warning:
    boost::log::core::get()->set_filter(boost::log::trivial::severity > boost::log::trivial::warning);
    boost::log::core::get()->set_logging_enabled(true);
    break;
  case LogVerbosity::trace:
    boost::log::core::get()->reset_filter();
    boost::log::core::get()->set_logging_enabled(true);
    break;
  }
}

LOOT_API void SetLogFile(const std::string& path) {
  // Set the locale to get UTF-8 conversions working correctly.
  std::locale::global(boost::locale::generator().generate(""));
  boost::filesystem::path::imbue(std::locale());

  boost::log::add_file_log(
    boost::log::keywords::file_name = path,
    boost::log::keywords::auto_flush = true,
    boost::log::keywords::format = (
      boost::log::expressions::stream
      << "[" << boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%H:%M:%S.%f") << "]"
      << " [" << boost::log::trivial::severity << "]: "
      << boost::log::expressions::smessage
      )
  );
  boost::log::add_common_attributes();
}

LOOT_API bool IsCompatible(const unsigned int versionMajor, const unsigned int versionMinor, const unsigned int versionPatch) {
  if (versionMajor > 0)
    return versionMajor == loot::LootVersion::major;
  else
    return versionMinor == loot::LootVersion::minor;
}

LOOT_API std::shared_ptr<GameInterface> CreateGameHandle(const GameType game,
                                                         const std::string& gamePath,
                                                         const std::string& gameLocalPath) {
  // Set the locale to get UTF-8 conversions working correctly.
  std::locale::global(boost::locale::generator().generate(""));
  boost::filesystem::path::imbue(std::locale());

  // Check for valid paths.
  const std::string resolvedGamePath = ResolvePath(gamePath);
  if (!gamePath.empty() && !fs::is_directory(resolvedGamePath))
    throw std::invalid_argument("Given game path \"" + gamePath + "\" does not resolve to a valid directory.");

  const std::string resolvedGameLocalPath = ResolvePath(gameLocalPath);
  if (!gameLocalPath.empty() && !fs::is_directory(resolvedGameLocalPath))
    throw std::invalid_argument("Given game path \"" + gameLocalPath + "\" does not resolve to a valid directory.");

  return std::make_shared<Game>(game, resolvedGamePath, resolvedGameLocalPath);
}
}
