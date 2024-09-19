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
#include "gui/qt/plugin_item_model.h"

namespace loot {
// SizeHintCacheKey contains all the data that the card size could depend on,
// aside from the available width, and so it means that different plugins with
// cards of the same size can share cached size data.
//
// In order of appearance, the tuple fields are:
//
//   General info card:
//
//   1. Longest text in second table column
//   2. Longest text in fourth table column (total messages count)
//   3. Longest text in sixth table column (total plugins count)
//   4. Message texts
//   5. How many types of plugins supported by the game have count rows
//   6. true
//
//   Plugin card:
//
//   1. Current bash tags
//   2. Add bash tags
//   3. Remove bash tags
//   4. Message texts
//   5. Location info
//   6. false
//
typedef std::tuple<QString,
                   QString,
                   QString,
                   std::vector<std::string>,
                   std::vector<std::string>,
                   bool>
    SizeHintCacheKey;

/**
 * Whenever the model's raw data changes, this cache needs to be updated for the
 * affected indexes. This update needs to happen before the delegate's paint or
 * size hint methods are called so that they are given the correct largest min
 * width value.
 */
class CardSizingCache {
public:
  explicit CardSizingCache(QWidget* cardParentWidget);

  void update(const QAbstractItemModel* model);
  void update(const QModelIndex& topLeft, const QModelIndex& bottomRight);
  void update(const QAbstractItemModel*, int firstRow, int lastRow);
  QWidget* update(const QModelIndex& index);

  QWidget* getCard(const SizeHintCacheKey& key) const;

  int getLargestMinWidth() const;

private:
  QWidget* cardParentWidget{nullptr};
  std::map<int, const SizeHintCacheKey*> keyCache;
  std::map<SizeHintCacheKey, std::pair<QWidget*, unsigned int>> cardCache;
};

class CardDelegate : public QStyledItemDelegate {
  Q_OBJECT
public:
  explicit CardDelegate(QListView* parent, CardSizingCache& cardSizingCache);

  void setIcons();
  void refreshMessages();
  void refreshStyling();

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
  CardSizingCache* cardSizingCache;
  mutable std::map<SizeHintCacheKey, QSize> sizeHintCache;
};
}

#endif
