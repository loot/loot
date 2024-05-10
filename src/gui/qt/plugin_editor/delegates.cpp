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

#include "gui/qt/plugin_editor/delegates.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QLineEdit>

#include "gui/qt/helpers.h"
#include "gui/qt/plugin_editor/message_content_editor.h"

namespace loot {
ComboBoxDelegate::ComboBoxDelegate(
    QObject* parent,
    std::vector<std::pair<QString, QVariant>> textAndData) :
    QStyledItemDelegate(parent), textAndData(textAndData) {}

QWidget* ComboBoxDelegate::createEditor(QWidget* parent,
                                        const QStyleOptionViewItem&,
                                        const QModelIndex&) const {
  QComboBox* comboBox = new QComboBox(parent);

  for (const auto& entry : textAndData) {
    comboBox->addItem(entry.first, entry.second);
  }

  return comboBox;
}

void ComboBoxDelegate::setEditorData(QWidget* editor,
                                     const QModelIndex& index) const {
  QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
  Q_ASSERT(comboBox);

  const auto editRoleData = index.data(Qt::EditRole);

  const int cbIndex = comboBox->findData(editRoleData);
  if (cbIndex >= 0) {
    comboBox->setCurrentIndex(cbIndex);
  }
}

void ComboBoxDelegate::setModelData(QWidget* editor,
                                    QAbstractItemModel* model,
                                    const QModelIndex& index) const {
  const QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
  Q_ASSERT(comboBox);

  model->setData(index, comboBox->currentData(), Qt::EditRole);
}

QWidget* CrcLineEditDelegate::createEditor(QWidget* parent,
                                           const QStyleOptionViewItem&,
                                           const QModelIndex&) const {
  QLineEdit* lineEdit = new QLineEdit(parent);
  lineEdit->setInputMask("HHHHHHHH");

  return lineEdit;
}

void CrcLineEditDelegate::setEditorData(QWidget* editor,
                                        const QModelIndex& index) const {
  QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
  Q_ASSERT(lineEdit);

  auto data = index.data(Qt::EditRole);

  lineEdit->setText(data.toString());
}

void CrcLineEditDelegate::setModelData(QWidget* editor,
                                       QAbstractItemModel* model,
                                       const QModelIndex& index) const {
  const QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
  Q_ASSERT(lineEdit);

  model->setData(index, lineEdit->text(), Qt::EditRole);
}

MessageContentDelegate::MessageContentDelegate(
    QObject* parent,
    const std::vector<LootSettings::Language>& languages) :
    QStyledItemDelegate(parent), languages(languages) {}

QWidget* MessageContentDelegate::createEditor(QWidget* parent,
                                              const QStyleOptionViewItem&,
                                              const QModelIndex&) const {
  auto editor = new MessageContentEditor(parent, languages);
  editor->setFocusPolicy(Qt::StrongFocus);

  return editor;
}
void MessageContentDelegate::setEditorData(QWidget* editor,
                                           const QModelIndex& index) const {
  auto dialog = qobject_cast<MessageContentEditor*>(editor);
  Q_ASSERT(dialog);

  auto data = index.data(Qt::EditRole);

  dialog->initialiseInputs(data.value<std::vector<MessageContent>>());
}
void MessageContentDelegate::setModelData(QWidget* editor,
                                          QAbstractItemModel* model,
                                          const QModelIndex& index) const {
  auto dialog = qobject_cast<MessageContentEditor*>(editor);
  Q_ASSERT(dialog);

  if (dialog->result() == QDialog::Accepted) {
    model->setData(
        index, QVariant::fromValue(dialog->getMetadata()), Qt::EditRole);
  }
}

void MessageContentDelegate::updateEditorGeometry(QWidget* editor,
                                                  const QStyleOptionViewItem&,
                                                  const QModelIndex&) const {
  const auto parentGeometry = editor->parentWidget()->geometry();
  const auto mappedGeometry =
      editor->parentWidget()->parentWidget()->mapToGlobal(
          parentGeometry.topLeft());

  editor->move(mappedGeometry);
}

bool MessageContentDelegate::eventFilter(QObject* editor, QEvent* event) {
  const auto keyEvent = static_cast<QKeyEvent*>(event);

  switch (keyEvent->key()) {
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
      // Ignore tabs and back-tabs so that keyboard navigation works within the
      // editor (which is a dialog).
      return false;
    default:
      return QStyledItemDelegate::eventFilter(editor, event);
  }
}

AutocompletingLineEditDelegate::AutocompletingLineEditDelegate(
    QObject* parent,
    const QStringList& completions) :
    QStyledItemDelegate(parent), completions(completions) {}

QWidget* AutocompletingLineEditDelegate::createEditor(
    QWidget* parent,
    const QStyleOptionViewItem&,
    const QModelIndex&) const {
  auto completer = new QCompleter(completions, parent);
  completer->setCaseSensitivity(Qt::CaseInsensitive);

  QLineEdit* lineEdit = new QLineEdit(parent);
  lineEdit->setCompleter(completer);

  return lineEdit;
}

void AutocompletingLineEditDelegate::setEditorData(
    QWidget* editor,
    const QModelIndex& index) const {
  QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
  Q_ASSERT(lineEdit);

  auto data = index.data(Qt::EditRole);

  lineEdit->setText(data.toString());
}

void AutocompletingLineEditDelegate::setModelData(
    QWidget* editor,
    QAbstractItemModel* model,
    const QModelIndex& index) const {
  const QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
  Q_ASSERT(lineEdit);

  model->setData(index, lineEdit->text(), Qt::EditRole);
}
}
