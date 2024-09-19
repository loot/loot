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

#include "gui/qt/icon_factory.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QPalette>
#include <QtGui/QScreen>
#include <QtWidgets/QStyle>

namespace loot {
QPixmap changeColor(QPixmap pixmap, QColor color) {
  QImage image = pixmap.toImage();

  for (int y = 0; y < image.height(); y++) {
    for (int x = 0; x < image.width(); x++) {
      color.setAlpha(image.pixelColor(x, y).alpha());
      image.setPixelColor(x, y, color);
    }
  }

  return QPixmap::fromImage(image);
}

QIcon IconFactory::getIsActiveIcon() {
  return getIcon(":/icons/material-icons/check_black_48dp.svg");
}

QIcon IconFactory::getMasterFileIcon() { return getIcon(":/icons/crown.svg"); }

QIcon IconFactory::getBlueprintMasterIcon() { return getIcon(":/icons/crown-blueprint.svg"); }

QIcon IconFactory::getLightPluginIcon() {
  return getIcon(":/icons/feather.svg");
}

QIcon IconFactory::getSmallPluginIcon() {
  return getIcon(":/icons/small.svg");
}

QIcon IconFactory::getMediumPluginIcon() {
  return getIcon(":/icons/medium.svg");
}

QIcon IconFactory::getEmptyPluginIcon() {
  return getIcon(":/icons/material-icons/visibility_off_black_48dp.svg");
}

QIcon IconFactory::getLoadsArchiveIcon() {
  return getIcon(":/icons/material-icons/attachment_black_48dp.svg");
}

QIcon IconFactory::getIsCleanIcon() { return getIcon(":/icons/droplet.svg"); }

QIcon IconFactory::getHasUserMetadataIcon() {
  return getIcon(":/icons/material-icons/account_circle_black_48dp.svg");
}

QIcon IconFactory::getEditIcon() {
  return getIcon(":/icons/material-icons/create_black_48dp.svg");
}

QIcon IconFactory::getSortIcon() {
  return getIcon(":/icons/material-icons/sort_black_48dp.svg");
}

QIcon IconFactory::getApplySortIcon() {
  return getIcon(":/icons/material-icons/check_black_48dp.svg");
}

QIcon IconFactory::getDiscardSortIcon() {
  return getIcon(":/icons/material-icons/close_black_48dp.svg");
}

QIcon IconFactory::getUpdateMasterlistIcon() {
  return getIcon(":/icons/material-icons/file_download_black_48dp.svg");
}

QIcon IconFactory::getSettingsIcon() {
  return getIcon(":/icons/material-icons/settings_black_48dp.svg");
}

QIcon IconFactory::getArchiveIcon() {
  return getIcon(":/icons/material-icons/archive_black_48dp.svg");
}

QIcon IconFactory::getQuitIcon() {
  return getIcon(":/icons/material-icons/close_black_48dp.svg");
}

QIcon IconFactory::getOpenGroupsEditorIcon() {
  return getIcon(":/icons/material-icons/group_work_black_48dp.svg");
}

QIcon IconFactory::getSearchIcon() {
  return getIcon(":/icons/material-icons/search_black_48dp.svg");
}

QIcon IconFactory::getCopyLoadOrderIcon() {
  return getIcon(":/icons/material-icons/receipt_black_48dp.svg");
}

QIcon IconFactory::getCopyContentIcon() {
  return getIcon(":/icons/material-icons/content_copy_black_48dp.svg");
}

QIcon IconFactory::getCopyMetadataIcon() {
  return getIcon(":/icons/material-icons/data_object_black_48dp.svg");
}

QIcon IconFactory::getRefreshIcon() {
  return getIcon(":/icons/material-icons/refresh_black_48dp.svg");
}

QIcon IconFactory::getRedateIcon() {
  return getIcon(":/icons/material-icons/today_black_48dp.svg");
}

QIcon IconFactory::getFixIcon() {
  return getIcon(":/icons/material-icons/build_black_48dp.svg");
}

QIcon IconFactory::getDeleteIcon() {
  return getIcon(":/icons/material-icons/delete_black_48dp.svg");
}

QIcon IconFactory::getViewDocsIcon() {
  return getIcon(":/icons/material-icons/book_black_48dp.svg");
}
QIcon IconFactory::getOpenFAQsIcon() {
  return getIcon(":/icons/material-icons/quiz_black_48dp.svg");
}

QIcon IconFactory::getOpenLOOTDataFolderIcon() {
  return getIcon(":/icons/material-icons/folder_black_48dp.svg");
}

QIcon IconFactory::getJoinDiscordServerIcon() {
  return getIcon(":/icons/material-icons/forum_black_48dp.svg");
}

QIcon IconFactory::getAboutIcon() {
  return getIcon(":/icons/material-icons/help_black_48dp.svg");
}

QPixmap IconFactory::getPixmap(const QIcon& icon,
                               int extent,
                               QIcon::Mode mode,
                               QIcon::State state) {
  // Take the device pixel ratio into account when reading or writing the cache
  // as it may change while the application is running.
  const auto pixelRatio =
      dynamic_cast<QGuiApplication*>(QCoreApplication::instance())
          ->devicePixelRatio();
  auto scaledSize = extent * pixelRatio;
  auto key = std::make_tuple(icon.cacheKey(), scaledSize, mode, state);

  const auto it = pixmaps.find(key);
  if (it != pixmaps.end()) {
    return it->second;
  }

  auto pixmap = icon.pixmap(extent, mode, state);

  pixmaps.emplace(key, pixmap);

  return pixmap;
}

void IconFactory::setColours(QColor normal, QColor disabled, QColor selected) {
  icons.clear();
  pixmaps.clear();

  normalColor = normal;
  disabledColor = disabled;
  selectedColor = selected;
}

std::map<QString, QIcon> IconFactory::icons;

std::map<std::tuple<qint64, double, QIcon::Mode, QIcon::State>, QPixmap>
    IconFactory::pixmaps;

QColor IconFactory::normalColor;

QColor IconFactory::disabledColor;

QColor IconFactory::selectedColor;

QIcon IconFactory::getIcon(QString resourcePath) {
  const auto it = icons.find(resourcePath);
  if (it != icons.end()) {
    return it->second;
  }

  if (!normalColor.isValid()) {
    normalColor = QGuiApplication::palette().color(QPalette::Disabled,
                                                   QPalette::WindowText);
  }

  if (!disabledColor.isValid()) {
    disabledColor = QGuiApplication::palette().color(QPalette::Disabled,
                                                     QPalette::WindowText);
  }

  if (!selectedColor.isValid()) {
    selectedColor = QGuiApplication::palette().color(QPalette::Active,
                                                     QPalette::HighlightedText);
  }

  auto rawPixmap = QPixmap(resourcePath);
  auto normalPixmap = changeColor(rawPixmap, normalColor);
  auto disabledPixmap = changeColor(rawPixmap, disabledColor);
  auto selectedPixmap = changeColor(rawPixmap, selectedColor);

  QIcon icon;
  icon.addPixmap(normalPixmap, QIcon::Normal);
  icon.addPixmap(disabledPixmap, QIcon::Disabled);
  icon.addPixmap(selectedPixmap, QIcon::Selected);

  icons.emplace(resourcePath, icon);

  return icon;
}
}
