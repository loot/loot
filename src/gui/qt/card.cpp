/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2025    Oliver Hamlet

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

#include "gui/qt/card.h"

#include <QtGui/QPainter>

namespace {
constexpr auto CARD_TOP_SHADOW_HEIGHT = 3;
constexpr auto CARD_BOTTOM_SHADOW_HEIGHT = 3;

QBrush GetCardTopBorderShadowBrush(QColor near, QColor far) {
  auto gradient = QLinearGradient(QPointF(0, 2), QPointF(0, 26));
  gradient.setColorAt(0, far);
  gradient.setColorAt(1, near);
  return QBrush(gradient);
}

QBrush GetCardBottomBorderShadowBrush(QColor near, QColor far) {
  auto gradient = QLinearGradient(QPointF(0, -2), QPointF(0, 3));
  gradient.setColorAt(0, near);
  gradient.setColorAt(1, far);
  return QBrush(gradient);
}
}

namespace loot {
Card::Card(QWidget* parent, bool paintTopShadow) :
    QFrame(parent), paintTopShadow_(paintTopShadow) {}

void Card::paintEvent(QPaintEvent* event) {
  QFrame::paintEvent(event);

  QPainter painter(this);

  const auto cardRect = rect();

  if (paintTopShadow_) {
    // Draw top border shadow.
    painter.fillRect(
        0,
        0,
        cardRect.width(),
        CARD_TOP_SHADOW_HEIGHT,
        GetCardTopBorderShadowBrush(shadowNearColor_, shadowFarColor_));
  }

  // Draw bottom border shadow.
  painter.translate(0, cardRect.height() - CARD_BOTTOM_SHADOW_HEIGHT);
  painter.fillRect(
      0,
      0,
      cardRect.width(),
      CARD_BOTTOM_SHADOW_HEIGHT,
      GetCardBottomBorderShadowBrush(shadowNearColor_, shadowFarColor_));
}
}