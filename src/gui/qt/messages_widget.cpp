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

#include <QtGui/QTextDocument>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStyle>

#include "gui/qt/helpers.h"
#include "gui/qt/icon_factory.h"
#include "gui/sourced_message.h"
#include "gui/state/logging.h"

namespace {
using loot::BareMessage;
using loot::MessageType;

static constexpr const char* MESSAGE_TYPE_PROPERTY = "messageType";
static constexpr int COLUMN_COUNT = 2;
static constexpr int BULLET_POINT_COLUMN = 0;
static constexpr int MESSAGE_LABEL_COLUMN = 1;

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

QString getHtmlText(const std::string& markdownText) {
  QTextDocument document;

  document.setMarkdown(QString::fromStdString(markdownText),
                       {QTextDocument::MarkdownNoHTML,
                        QTextDocument::MarkdownDialectCommonMark});

  // It's not possible to control how a QLabel styles Markdown text beyond
  // setting the Link color in the palette, and the default style sheet is
  // ignored when setting Markdown (or maybe it's just not included in the
  // HTML returned by toHtml()?), so insert a <style> element into the
  // HTML instead.
  auto styleSheet = QString("a { text-decoration: none; }");

  auto html = document.toHtml();
  html.replace("</head>", QString("<style>%1</style></head>").arg(styleSheet));
  return html;
}

QLabel* createBulletPointLabel() {
  auto label = new QLabel();
  label->setTextFormat(Qt::TextFormat::PlainText);
  label->setText(QString(u8"\u2022"));

  const auto leftMargin =
      label->style()->pixelMetric(QStyle::PM_LayoutLeftMargin);
  const auto topMargin =
      label->style()->pixelMetric(QStyle::PM_LayoutTopMargin);
  const auto rightMargin =
      label->style()->pixelMetric(QStyle::PM_LayoutRightMargin);
  const auto bottomMargin =
      label->style()->pixelMetric(QStyle::PM_LayoutBottomMargin);

  label->setContentsMargins(leftMargin, topMargin, rightMargin, bottomMargin);

  return label;
}

QLabel* createMessageLabel() {
  auto label = new QLabel();
  label->setTextFormat(Qt::TextFormat::RichText);

  label->setWordWrap(true);
  label->setOpenExternalLinks(true);

  const auto leftMargin =
      label->style()->pixelMetric(QStyle::PM_LayoutLeftMargin);
  const auto topMargin =
      label->style()->pixelMetric(QStyle::PM_LayoutTopMargin);
  const auto rightMargin =
      label->style()->pixelMetric(QStyle::PM_LayoutRightMargin);
  const auto bottomMargin =
      label->style()->pixelMetric(QStyle::PM_LayoutBottomMargin);

  label->setContentsMargins(leftMargin, topMargin, rightMargin, bottomMargin);

  return label;
}

void updateMessageLabel(QLabel* label, const BareMessage& message) {
  auto oldPropertyValue = label->property(MESSAGE_TYPE_PROPERTY);
  auto newPropertyValue = getPropertyValue(message.first);
  auto propertyChanged =
      oldPropertyValue.isValid() && oldPropertyValue != newPropertyValue;

  label->setProperty(MESSAGE_TYPE_PROPERTY, newPropertyValue);

  // Set HTML because otherwise it's not possible to interpret the text as
  // CommonMark instead of GitHub Flavored Markdown, or set custom styling
  // beyond setting the link text (which is done by setting the palette Link
  // color).
  label->setText(getHtmlText(message.second));

  if (propertyChanged) {
    // Trigger styling changes.
    label->style()->unpolish(label);
    label->style()->polish(label);
  }
}

std::vector<BareMessage> toBareMessages(
    const std::vector<loot::SourcedMessage>& messages) {
  std::vector<BareMessage> bareMessages;
  for (const auto& message : messages) {
    bareMessages.push_back(BareMessage{message.type, message.text});
  }

  return bareMessages;
}
}

namespace loot {
MessagesWidget::MessagesWidget(QWidget* parent) : QWidget(parent) { setupUi(); }

void MessagesWidget::setMessages(const std::vector<SourcedMessage>& messages) {
  if (!willChangeContent(messages)) {
    // Avoid expensive layout changes.
    return;
  }

  setMessages(toBareMessages(messages));
}

void MessagesWidget::refresh() { setMessages(currentMessages); }

void MessagesWidget::setupUi() {
  hideMessageAction->setIcon(IconFactory::getHideMessagesIcon());

  menuMessage->addAction(hideMessageAction);

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

  setContextMenuPolicy(Qt::CustomContextMenu);

  translateUi();

  connect(hideMessageAction,
          &QAction::triggered,
          this,
          &MessagesWidget::onHideMessageAction);

  connect(this,
          &QWidget::customContextMenuRequested,
          this,
          &MessagesWidget::onCustomContextMenuRequested);
}

void MessagesWidget::translateUi() {
  hideMessageAction->setText(translate("Hide message"));
}

void MessagesWidget::setMessages(const std::vector<BareMessage>& messages) {
  // Don't use rowCount() because that seems to have a starting value of 1
  // even when there's nothing in the layout yet. Instead, use count() /
  // COLUMN_COUNT.

  // Delete any extra QLabels.
  QLayoutItem* child = nullptr;
  const auto pastTheEndIndex = static_cast<int>(messages.size() * COLUMN_COUNT);
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
  while (gridLayout->count() < pastTheEndIndex) {
    auto bulletPointLabel = createBulletPointLabel();
    auto messageLabel = createMessageLabel();

    const auto row = gridLayout->count() / COLUMN_COUNT;
    gridLayout->addWidget(bulletPointLabel,
                          row,
                          BULLET_POINT_COLUMN,
                          Qt::AlignHCenter | Qt::AlignTop);
    gridLayout->addWidget(messageLabel, row, MESSAGE_LABEL_COLUMN);
  }

  // Now update the QLabels.
  for (size_t i = 0; i < messages.size(); i += 1) {
    const auto position = static_cast<int>(i);
    auto label = qobject_cast<QLabel*>(
        gridLayout->itemAtPosition(position, MESSAGE_LABEL_COLUMN)->widget());
    const auto& message = messages.at(i);
    updateMessageLabel(label, message);
  }

  layout()->activate();
  setVisible(!messages.empty());

  // Store the source markdown text because it can't be retrieved from the
  // QLabel text, as that's set using HTML. Also store the message types
  // because it's easier to do that than to derive them from the current
  // layout content.
  currentMessages = messages;
}

bool MessagesWidget::willChangeContent(
    const std::vector<SourcedMessage>& messages) const {
  if (messages.size() != currentMessages.size()) {
    return true;
  }

  return currentMessages != toBareMessages(messages);
}

void MessagesWidget::onCustomContextMenuRequested(const QPoint& pos) {
  try {
    // Find message index that is displayed at pos.
    menuTargetMessageIndex = std::nullopt;

    auto layout = qobject_cast<QGridLayout*>(this->layout());
    for (int i = 0; i < layout->rowCount(); i += 1) {
      auto item = layout->itemAtPosition(i, BULLET_POINT_COLUMN);
      if (item) {
        auto widget = item->widget();
        if (widget && widget->underMouse()) {
          menuTargetMessageIndex = i;
          break;
        }
      }

      item = layout->itemAtPosition(i, MESSAGE_LABEL_COLUMN);
      if (item) {
        auto widget = item->widget();
        if (widget && widget->underMouse()) {
          menuTargetMessageIndex = i;
          break;
        }
      }
    }

    if (menuTargetMessageIndex.has_value()) {
      const auto globalPos = this->mapToGlobal(pos);

      menuMessage->exec(globalPos);
    } else {
      const auto logger = getLogger();
      if (logger) {
        logger->warn("Could not find message under mouse");
      }
    }
  } catch (const std::exception& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error(
          "Caught an exception in "
          "MessagesWidget::onCustomContextMenuRequested(): {}",
          e.what());
    }
  }
}

void MessagesWidget::onHideMessageAction() {
  try {
    if (menuTargetMessageIndex.has_value()) {
      auto index = menuTargetMessageIndex.value();
      if (index < currentMessages.size()) {
        std::string text = currentMessages.at(index).second;

        currentMessages.erase(currentMessages.begin() + index);
        setMessages(currentMessages);

        emit hideMessage(text);
      } else {
        const auto logger = getLogger();
        if (logger) {
          logger->error(
              "Menu target message index is greater than the number of current "
              "messages: index is {}, size is {}",
              index,
              currentMessages.size());
        }
      }
    }
  } catch (const std::exception& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error(
          "Caught an exception in MessagesWidget::onHideMessageAction(): {}",
          e.what());
    }
  }
}
}
