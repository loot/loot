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

#ifndef LOOT_GUI_QT_PLUGIN_EDITOR_DELEGATES
#define LOOT_GUI_QT_PLUGIN_EDITOR_DELEGATES

#include <QtWidgets/QStyledItemDelegate>

#include "gui/state/loot_settings.h"

namespace loot {
class ComboBoxDelegate : public QStyledItemDelegate {
  Q_OBJECT
public:
  ComboBoxDelegate(QObject* parent,
                   const std::vector<std::pair<QString, QVariant>>& textAndData);

  QWidget* createEditor(QWidget* parent,
                        const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override;

  void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  void setModelData(QWidget* editor,
                    QAbstractItemModel* model,
                    const QModelIndex& index) const override;

private:
  std::vector<std::pair<QString, QVariant>> textAndData;
};

class CrcLineEditDelegate : public QStyledItemDelegate {
public:
  using QStyledItemDelegate::QStyledItemDelegate;

  QWidget* createEditor(QWidget* parent,
                        const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override;

  void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  void setModelData(QWidget* editor,
                    QAbstractItemModel* model,
                    const QModelIndex& index) const override;
};

class MessageContentDelegate : public QStyledItemDelegate {
public:
  MessageContentDelegate(QObject* parent,
                         const std::vector<LootSettings::Language>& languages);

  QWidget* createEditor(QWidget* parent,
                        const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override;

  void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  void setModelData(QWidget* editor,
                    QAbstractItemModel* model,
                    const QModelIndex& index) const override;

  void updateEditorGeometry(QWidget* editor,
                            const QStyleOptionViewItem& option,
                            const QModelIndex& index) const override;

  bool eventFilter(QObject* editor, QEvent* event) override;

private:
  const std::vector<LootSettings::Language>* languages;
};

class AutocompletingLineEditDelegate : public QStyledItemDelegate {
public:
  AutocompletingLineEditDelegate(QObject* parent,
                                 const QStringList& completions);

  QWidget* createEditor(QWidget* parent,
                        const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override;

  void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  void setModelData(QWidget* editor,
                    QAbstractItemModel* model,
                    const QModelIndex& index) const override;

private:
  const QStringList* completions;
};
}

#endif
