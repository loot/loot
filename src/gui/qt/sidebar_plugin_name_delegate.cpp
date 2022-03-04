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

#include "gui/qt/sidebar_plugin_name_delegate.h"

#include "gui/qt/plugin_item_model.h"

namespace loot {
SidebarPluginNameDelegate::SidebarPluginNameDelegate(QObject* parent) :
    QStyledItemDelegate(parent) {}

void SidebarPluginNameDelegate::paint(QPainter* painter,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const {
  auto styleOption = QStyleOptionViewItem(option);
  initStyleOption(&styleOption, index);

  if (index.row() == 0) {
    QStyledItemDelegate::paint(painter, styleOption, index);
    return;
  }

  // This drawControl is needed to draw the styling that's used to
  // indicate when an item is hovered over or selected.
  option.widget->style()->drawControl(
      QStyle::CE_ItemViewItem, &option, painter, styleOption.widget);

  painter->save();

  auto pluginItem = index.data(RawDataRole).value<PluginItem>();
  auto isEditorOpen = index.data(EditorStateRole).toBool();

  const auto isSelected = styleOption.state.testFlag(QStyle::State_Selected) &&
                          styleOption.state.testFlag(QStyle::State_Active);
  if (isSelected) {
    painter->setPen(styleOption.palette.highlightedText().color());
  }

  auto name = QFontMetricsF(painter->font())
                  .elidedText(QString::fromStdString(pluginItem.name),
                              Qt::ElideRight,
                              styleOption.rect.width());
  painter->drawText(styleOption.rect, Qt::AlignLeft, name);

  if (isEditorOpen && pluginItem.group.has_value() &&
      pluginItem.group.value() != Group::DEFAULT_NAME) {
    auto groupRect = styleOption.rect;
    groupRect.translate(0, SIDEBAR_EDIT_MODE_ROW_HEIGHT / 2);

    if (!isSelected) {
      painter->setPen(
          styleOption.palette.color(QPalette::Disabled, QPalette::Text));
    }

    auto group = painter->fontMetrics().elidedText(
        QString::fromStdString(pluginItem.group.value()),
        Qt::ElideRight,
        groupRect.width());
    painter->drawText(groupRect, Qt::AlignLeft, group);
  }

  painter->restore();
}
}
