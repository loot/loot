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

#ifndef LOOT_GUI_QT_ICON_FACTORY
#define LOOT_GUI_QT_ICON_FACTORY

#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>
#include <map>

namespace loot {
class IconFactory {
public:
  static QIcon getIsActiveIcon();
  static QIcon getMasterFileIcon();
  static QIcon getBlueprintMasterIcon();
  static QIcon getLightPluginIcon();
  static QIcon getSmallPluginIcon();
  static QIcon getMediumPluginIcon();
  static QIcon getEmptyPluginIcon();
  static QIcon getLoadsArchiveIcon();
  static QIcon getIsCleanIcon();
  static QIcon getHasUserMetadataIcon();

  static QIcon getEditIcon();

  static QIcon getSortIcon();
  static QIcon getApplySortIcon();
  static QIcon getDiscardSortIcon();
  static QIcon getUpdateMasterlistIcon();

  static QIcon getSettingsIcon();
  static QIcon getArchiveIcon();
  static QIcon getQuitIcon();
  static QIcon getOpenGroupsEditorIcon();
  static QIcon getSearchIcon();
  static QIcon getCopyLoadOrderIcon();
  static QIcon getCopyContentIcon();
  static QIcon getCopyMetadataIcon();
  static QIcon getRefreshIcon();
  static QIcon getRedateIcon();
  static QIcon getFixIcon();
  static QIcon getDeleteIcon();
  static QIcon getViewDocsIcon();
  static QIcon getOpenFAQsIcon();
  static QIcon getOpenLOOTDataFolderIcon();
  static QIcon getJoinDiscordServerIcon();
  static QIcon getAboutIcon();

  // Caches resized pixmaps so that the transformation only needs to be done
  // once per (icon, extent, mode, state) tuple of argument values.
  static QPixmap getPixmap(const QIcon& icon,
                           int extent,
                           QIcon::Mode mode = QIcon::Normal,
                           QIcon::State state = QIcon::Off);

  static void setColours(QColor normal, QColor disabled, QColor selected);

private:
  static std::map<QString, QIcon> icons;
  static std::map<std::tuple<qint64, double, QIcon::Mode, QIcon::State>,
                  QPixmap>
      pixmaps;
  static QColor normalColor;
  static QColor disabledColor;
  static QColor selectedColor;

  static QIcon getIcon(QString resourcePath);
};
}

#endif
