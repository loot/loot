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

#ifndef LOOT_GUI_QT_PLUGIN_CARD
#define LOOT_GUI_QT_PLUGIN_CARD

#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

#include "gui/plugin_item.h"
#include "gui/qt/filters_states.h"
#include "gui/qt/general_info_card.h"

namespace loot {
class PluginCard : public QFrame {
  Q_OBJECT
public:
  explicit PluginCard(QWidget* parent);

  void setIcons();

  void setContent(const PluginItem& plugin,
                  const CardContentFiltersState& filters);

  void setSearchResult(bool isSearchResult, bool isCurrentSearchResult);

  void refreshMessages();

private:
  QLabel* nameLabel{new QLabel(this)};
  QLabel* crcLabel{new QLabel(this)};
  QLabel* versionLabel{new QLabel(this)};
  QLabel* isActiveLabel{new QLabel(this)};
  QLabel* masterFileLabel{new QLabel(this)};
  QLabel* lightPluginLabel{new QLabel(this)};
  QLabel* emptyPluginLabel{new QLabel(this)};
  QLabel* loadsArchiveLabel{new QLabel(this)};
  QLabel* isCleanLabel{new QLabel(this)};
  QLabel* hasUserEditsLabel{new QLabel(this)};
  QLabel* currentTagsHeaderLabel{new QLabel(this)};
  QLabel* currentTagsLabel{new QLabel(this)};
  QLabel* addTagsHeaderLabel{new QLabel(this)};
  QLabel* addTagsLabel{new QLabel(this)};
  QLabel* removeTagsHeaderLabel{new QLabel(this)};
  QLabel* removeTagsLabel{new QLabel(this)};
  QGroupBox* tagsGroupBox{new QGroupBox(this)};
  QLabel* locationsLabel{new QLabel(this)};
  MessagesWidget* messagesWidget{new MessagesWidget(this)};

  void setupUi();

  void translateUi();
};

// current, add and remove bash tags, messages, locations, and whether this is
// the general info card or not.
typedef std::tuple<QString,
                   QString,
                   QString,
                   std::vector<std::string>,
                   std::vector<std::string>,
                   bool>
    SizeHintCacheKey;

class PluginCardDelegate : public QStyledItemDelegate {
  Q_OBJECT
public:
  explicit PluginCardDelegate(QListView* parent);

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

  static GeneralInfoCard* setGeneralInfoCardContent(
      GeneralInfoCard* generalInfoCard,
      const QModelIndex& index);
  static PluginCard* setPluginCardContent(PluginCard* card,
                                          const QModelIndex& index);

  static QSize calculateSize(const QWidget* widget,
                             const QStyleOptionViewItem& option);
};
}

#endif
