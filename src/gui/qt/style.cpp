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

#include <cpptoml.h>

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
    auto logger = getLogger();
    if (logger) {
      logger->warn("Unable to set stylesheet, {} not found.",
                   resourcePath.toStdString());
    }
    return std::nullopt;
  }

  file.open(QFile::ReadOnly | QFile::Text);
  QTextStream ts(&file);
  return ts.readAll();
}

std::optional<QString> loadStyleSheet(
    const std::filesystem::path& resourcesPath,
    const std::string& themeName) {
  // First try loading the theme from the filesystem, then try loading from
  // built-in resources, then fall back to the default theme (which itself
  // will load from filesystem then built-in resources).

  auto logger = getLogger();
  if (logger) {
    logger->info("Loading style sheet for the \"{}\" theme...", themeName);
  }

  auto filesystemPath = (resourcesPath / "themes" / (themeName + QSS_SUFFIX));
  auto styleSheet =
      loadStyleSheet(QString::fromStdString(filesystemPath.u8string()));
  if (styleSheet.has_value()) {
    return styleSheet.value();
  }

  if (logger) {
    logger->warn(
        "Failed to find the style sheet in the filesystem, attempting to load "
        "from built-in resources...");
  }

  auto builtInPath =
      QString(":/themes/%1.theme.qss").arg(QString::fromStdString(themeName));
  styleSheet = loadStyleSheet(builtInPath);
  if (styleSheet.has_value()) {
    return styleSheet.value();
  }

  if (logger) {
    logger->error("Failed to find the style sheet in the built-in resources.");
  }

  return std::nullopt;
}

std::optional<QPalette> loadPalette(const QString& resourcePath) {
  QFile file(resourcePath);
  if (!file.exists()) {
    auto logger = getLogger();
    if (logger) {
      logger->warn("Unable to set palette, {} not found.",
                   resourcePath.toStdString());
    }
    return std::nullopt;
  }

  file.open(QFile::ReadOnly | QFile::Text);
  QTextStream ts(&file);
  auto content = ts.readAll();

  std::istringstream in(content.toStdString());
  if (!in) {
    return std::nullopt;
  }

  auto config = cpptoml::parser(in).parse();

  static const std::array<QPalette::ColorGroup, 3> COLOR_GROUPS = {
      QPalette::Active, QPalette::Inactive, QPalette::Disabled};

  static const std::map<QPalette::ColorGroup, const char*> COLOR_GROUP_MAP = {
      {QPalette::Active, "active"},
      {QPalette::Inactive, "inactive"},
      {QPalette::Disabled, "disabled"}};

  static const std::array<QPalette::ColorRole, 20> COLOR_ROLES = {
      QPalette::Window,
      QPalette::WindowText,
      QPalette::Base,
      QPalette::AlternateBase,
      QPalette::ToolTipBase,
      QPalette::ToolTipText,
      QPalette::PlaceholderText,
      QPalette::Text,
      QPalette::Button,
      QPalette::ButtonText,
      QPalette::BrightText,
      QPalette::Light,
      QPalette::Midlight,
      QPalette::Dark,
      QPalette::Mid,
      QPalette::Shadow,
      QPalette::Highlight,
      QPalette::HighlightedText,
      QPalette::Link,
      QPalette::LinkVisited};

  static const std::map<QPalette::ColorRole, const char*> COLOR_ROLE_MAP = {
      {QPalette::Window, "window"},
      {QPalette::WindowText, "windowText"},
      {QPalette::Base, "base"},
      {QPalette::AlternateBase, "alternateBase"},
      {QPalette::ToolTipBase, "toolTipBase"},
      {QPalette::ToolTipText, "toolTipText"},
      {QPalette::PlaceholderText, "placeholderText"},
      {QPalette::Text, "text"},
      {QPalette::Button, "button"},
      {QPalette::ButtonText, "buttonText"},
      {QPalette::BrightText, "brightText"},
      {QPalette::Light, "light"},
      {QPalette::Midlight, "midlight"},
      {QPalette::Dark, "dark"},
      {QPalette::Mid, "mid"},
      {QPalette::Shadow, "mid"},
      {QPalette::Highlight, "highlight"},
      {QPalette::HighlightedText, "highlightedText"},
      {QPalette::Link, "link"},
      {QPalette::LinkVisited, "linkVisited"},
  };

  auto activeButton = config->get_qualified_as<std::string>("active.button");
  auto activeWindow = config->get_qualified_as<std::string>("active.window");

  QPalette palette;
  if (activeButton && activeWindow) {
    // If this isn't run, any colours not given aren't calculated.
    palette = QPalette(QColor(QString::fromStdString(*activeButton)),
                       QColor(QString::fromStdString(*activeButton)));
  }

  for (const auto group : COLOR_GROUPS) {
    auto tableName = COLOR_GROUP_MAP.at(group);

    auto table = config->get_table(tableName);
    if (table) {
      for (const auto role : COLOR_ROLES) {
        auto fieldName = COLOR_ROLE_MAP.at(role);
        auto roleValue = table->get_as<std::string>(fieldName);
        if (roleValue) {
          palette.setColor(
              group, role, QColor(QString::fromStdString(*roleValue)));
        }
      }
    }
  }

  return palette;
}

std::optional<QPalette> loadPalette(const std::filesystem::path& resourcesPath,
                                    const std::string& themeName) {
  // First try loading the theme from the filesystem, then try loading from
  // built-in resources.

  auto logger = getLogger();
  if (logger) {
    logger->info("Loading palette for the \"{}\" theme...", themeName);
  }

  auto filesystemPath =
      (resourcesPath / "themes" / (themeName + ".palette.toml"));
  auto palette = loadPalette(QString::fromStdString(filesystemPath.u8string()));
  if (palette.has_value()) {
    return palette.value();
  }

  if (logger) {
    logger->warn(
        "Failed to find the palette in the filesystem, attempting to load "
        "from built-in resources...");
  }

  auto builtInPath = QString(":/themes/%1.palette.toml")
                         .arg(QString::fromStdString(themeName));
  palette = loadPalette(builtInPath);
  if (palette.has_value()) {
    return palette.value();
  }

  if (logger) {
    logger->warn("Failed to find the palette in the built-in resources.");
  }

  return std::nullopt;
}

std::vector<std::string> findThemes(
    const std::filesystem::path& resourcesPath) {
  std::set<std::string> themes({"default", "dark"});

  auto themesPath = resourcesPath / "themes";
  if (!std::filesystem::exists(themesPath)) {
    return {themes.begin(), themes.end()};
  }

  auto logger = getLogger();
  for (std::filesystem::directory_iterator it(themesPath);
       it != std::filesystem::directory_iterator();
       ++it) {
    if (!std::filesystem::is_regular_file(it->status())) {
      continue;
    }

    auto filename = it->path().filename().u8string();
    if (!boost::iends_with(filename, QSS_SUFFIX)) {
      continue;
    }

    if (logger) {
      logger->info("Found theme QSS file: {}", filename);
    }

    auto themeName = filename.substr(0, filename.size() - QSS_SUFFIX_LENGTH);
    if (themes.count(themeName) == 0) {
      themes.insert(themeName);
    }
  }

  return {themes.begin(), themes.end()};
}
}
