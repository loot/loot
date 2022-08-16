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

#ifndef LOOT_GUI_QT_CARD_DELEGATE
#define LOOT_GUI_QT_CARD_DELEGATE

#include <QtGui/QPainter>
#include <QtWidgets/QListView>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QWidget>

#include "gui/qt/general_info_card.h"
#include "gui/qt/plugin_card.h"

namespace loot {
// current, add and remove bash tags, messages, locations, and whether this is
// the general info card or not.
typedef std::tuple<QString,
                   QString,
                   QString,
                   std::vector<std::string>,
                   std::vector<std::string>,
                   bool>
    SizeHintCacheKey;

class CardDelegate : public QStyledItemDelegate {
  Q_OBJECT
public:
  explicit CardDelegate(QListView* parent);

  void setIcons();
  void refreshMessages();

  void paint(QPainter* painter,
             const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;

  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;

  QWidget* createEditor(QWidget* parent,
                        const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override;

  void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  void setModelData(QWidget* editor,
                    QAbstractItemModel* model,
                    const QModelIndex& index) const override;

private:
  GeneralInfoCard* generalInfoCard{nullptr};
  PluginCard* pluginCard{nullptr};
  mutable std::map<SizeHintCacheKey, std::pair<QWidget*, QSize>> sizeHintCache;
};
}

#endif
