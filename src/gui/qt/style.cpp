/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2021    Oliver Hamlet

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

#include "gui/qt/style.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <boost/algorithm/string.hpp>
#include <set>

#include "gui/state/logging.h"
#include "gui/state/loot_paths.h"

namespace loot {
static constexpr const char* QSS_SUFFIX = ".theme.qss";
static constexpr size_t QSS_SUFFIX_LENGTH =
    std::char_traits<char>::length(QSS_SUFFIX);

std::optional<QString> loadStyleSheet(const QString& resourcePath) {
  QFile file(resourcePath);
  if (!file.exists()) {
    return std::nullopt;
  }

  file.open(QFile::ReadOnly | QFile::Text);
  QTextStream ts(&file);
  return ts.readAll();
}

std::optional<QString> loadStyleSheet(const std::filesystem::path& themesPath,
                                      const std::string& themeName) {
  // First try loading the theme from the filesystem, then try loading from
  // built-in resources, then fall back to the default theme (which itself
  // will load from filesystem then built-in resources).

  const auto logger = getLogger();
  if (logger) {
    logger->debug("Loading style sheet for the \"{}\" theme...", themeName);
  }

  const auto filesystemPath = (themesPath / (themeName + QSS_SUFFIX));
  auto styleSheet =
      loadStyleSheet(QString::fromStdString(filesystemPath.u8string()));
  if (styleSheet.has_value()) {
    return styleSheet.value();
  }

  if (logger) {
    logger->warn(
        "Failed to find the style sheet for the \"{}\" theme in the "
        "filesystem, attempting to load from built-in resources...",
        themeName);
  }

  const auto builtInPath =
      QString(":/themes/%1.theme.qss").arg(QString::fromStdString(themeName));
  styleSheet = loadStyleSheet(builtInPath);
  if (styleSheet.has_value()) {
    return styleSheet.value();
  }

  if (logger) {
    logger->error(
        "Failed to find the style sheet for the \"{}\" theme in the filesystem "
        "or built-in resources.",
        themeName);
  }

  return std::nullopt;
}

std::vector<std::string> findThemes(const std::filesystem::path& themesPath) {
  std::set<std::string> themes({"default", "dark"});

  if (!std::filesystem::exists(themesPath)) {
    return {themes.begin(), themes.end()};
  }

  const auto logger = getLogger();
  for (std::filesystem::directory_iterator it(themesPath);
       it != std::filesystem::directory_iterator();
       ++it) {
    if (!std::filesystem::is_regular_file(it->status())) {
      continue;
    }

    const auto filename = it->path().filename().u8string();
    if (!boost::iends_with(filename, QSS_SUFFIX)) {
      continue;
    }

    if (logger) {
      logger->debug("Found theme QSS file: {}", filename);
    }

    const auto themeName =
        filename.substr(0, filename.size() - QSS_SUFFIX_LENGTH);
    if (themes.count(themeName) == 0) {
      themes.insert(themeName);
    }
  }

  return {themes.begin(), themes.end()};
}
}
