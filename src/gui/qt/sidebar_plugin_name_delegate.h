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

#ifndef LOOT_GUI_QT_SIDEBAR_PLUGIN_NAME_DELEGATE
#define LOOT_GUI_QT_SIDEBAR_PLUGIN_NAME_DELEGATE

#include <QtGui/QPainter>
#include <QtWidgets/QStyledItemDelegate>

namespace loot {
qreal getSidebarRowHeight(bool inEditMode);

class SidebarPluginNameDelegate : public QStyledItemDelegate {
  Q_OBJECT
public:
  explicit SidebarPluginNameDelegate(QObject* parent);

  void setColors(QColor selectedName, QColor selectedGroup);

  void paint(QPainter* painter,
             const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;

private:
  QColor selectedTextColor;
  QColor unselectedGroupColor;
};
}

#endif
