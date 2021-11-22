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

#include "gui/qt/messages_widget.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStyle>

namespace loot {
static constexpr const char* MESSAGE_TYPE_PROPERTY = "messageType";
static const int COLUMN_COUNT = 2;
static const int BULLET_POINT_COLUMN = 0;
static const int MESSAGE_LABEL_COLUMN = 1;

QString getPropertyValue(MessageType messageType) {
  switch (messageType) {
    case MessageType::warn:
      return "warn";
    case MessageType::error:
      return "error";
    default:
      return "say";
  }
}

QLabel* createBulletPointLabel() {
  auto label = new QLabel();
  label->setTextFormat(Qt::TextFormat::PlainText);
  label->setText(QString(u8"\u2022"));

  auto leftMargin = label->style()->pixelMetric(QStyle::PM_LayoutLeftMargin);
  auto topMargin = label->style()->pixelMetric(QStyle::PM_LayoutTopMargin);
  auto rightMargin = label->style()->pixelMetric(QStyle::PM_LayoutRightMargin);
  auto bottomMargin =
      label->style()->pixelMetric(QStyle::PM_LayoutBottomMargin);

  label->setContentsMargins(leftMargin, topMargin, rightMargin, bottomMargin);

  return label;
}

QLabel* createMessageLabel() {
  auto label = new QLabel();
  label->setTextFormat(Qt::TextFormat::MarkdownText);

  label->setWordWrap(true);
  label->setOpenExternalLinks(true);

  auto leftMargin = label->style()->pixelMetric(QStyle::PM_LayoutLeftMargin);
  auto topMargin = label->style()->pixelMetric(QStyle::PM_LayoutTopMargin);
  auto rightMargin = label->style()->pixelMetric(QStyle::PM_LayoutRightMargin);
  auto bottomMargin =
      label->style()->pixelMetric(QStyle::PM_LayoutBottomMargin);

  label->setContentsMargins(leftMargin, topMargin, rightMargin, bottomMargin);

  return label;
}

void updateMessageLabel(QLabel* label, const SimpleMessage& message) {
  auto oldPropertyValue = label->property(MESSAGE_TYPE_PROPERTY);
  auto newPropertyValue = getPropertyValue(message.type);
  auto propertyChanged =
      oldPropertyValue.isValid() && oldPropertyValue != newPropertyValue;

  label->setProperty(MESSAGE_TYPE_PROPERTY, newPropertyValue);
  label->setText(QString::fromStdString(message.text));

  if (propertyChanged) {
    // Trigger styling changes.
    label->style()->unpolish(label);
    label->style()->polish(label);
  }
}

MessagesWidget::MessagesWidget(QWidget* parent) : QWidget(parent) { setupUi(); }

bool MessagesWidget::willChangeContent(
    const std::vector<SimpleMessage>& messages) const {
  if (messages.size() * COLUMN_COUNT != layout()->count()) {
    return true;
  }

  if (messages.empty() && layout()->count() == 0) {
    return false;
  }

  // Start at the second item and increment by two because we're not interested
  // in the bullet points.
  std::vector<std::pair<QVariant, std::string>> currentMessages;
  for (int i = MESSAGE_LABEL_COLUMN; i < layout()->count(); i += COLUMN_COUNT) {
    auto label = qobject_cast<QLabel*>(layout()->itemAt(i)->widget());
    auto messageType = label->property(MESSAGE_TYPE_PROPERTY);
    auto text = label->text().toStdString();
    currentMessages.push_back(std::make_pair(messageType, text));
  }

  std::vector<std::pair<QVariant, std::string>> newMessages;
  for (const auto& message : messages) {
    auto messageType = getPropertyValue(message.type);
    newMessages.push_back(std::make_pair(messageType, message.text));
  }

  return currentMessages != newMessages;
}

void MessagesWidget::setMessages(const std::vector<SimpleMessage>& messages) {
  if (!willChangeContent(messages)) {
    // Avoid expensive layout changes.
    return;
  }

  // Don't use rowCount() because that seems to have a starting value of 1
  // even when there's nothing in the layout yet. Instead, use count() /
  // COLUMN_COUNT.

  // Delete any extra QLabels.
  QLayoutItem* child;
  auto pastTheEndIndex = messages.size() * COLUMN_COUNT;
  auto itemRemoved = false;
  while ((child = layout()->takeAt(pastTheEndIndex)) != nullptr) {
    delete child->widget();
    delete child;
    itemRemoved = true;
  }

  // For some reason the layout doesn't automatically resize if only some
  // children are removed, but it does when all children are removed.
  if (itemRemoved && layout()->count() != 0) {
    layout()->invalidate();
  }

  // Add any missing QLabels.
  auto gridLayout = qobject_cast<QGridLayout*>(layout());
  while (gridLayout->count() < messages.size() * COLUMN_COUNT) {
    auto bulletPointLabel = createBulletPointLabel();
    auto messageLabel = createMessageLabel();

    auto row = gridLayout->count() / COLUMN_COUNT;
    gridLayout->addWidget(bulletPointLabel,
                          row,
                          BULLET_POINT_COLUMN,
                          Qt::AlignHCenter | Qt::AlignTop);
    gridLayout->addWidget(messageLabel, row, MESSAGE_LABEL_COLUMN);
  }

  // Now update the QLabels.
  for (size_t i = 0; i < messages.size(); i += 1) {
    auto label = qobject_cast<QLabel*>(
        gridLayout->itemAtPosition(i, MESSAGE_LABEL_COLUMN)->widget());
    updateMessageLabel(label, messages[i]);
  }

  layout()->activate();
}

void MessagesWidget::setupUi() {
  // Bullet points rendered using rich text are positioned uncomfortably close
  // to the text following them, and there's no way to change that. Use a
  // QGridLayout instead of a QVBoxLayout to fake a bullet point list by putting
  // bullet point characters in the first column and the message texts in the
  // second column.
  auto layout = new QGridLayout(this);
  layout->setSizeConstraint(QLayout::SetMinimumSize);

  layout->setColumnStretch(MESSAGE_LABEL_COLUMN, 1);

  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
}
}
