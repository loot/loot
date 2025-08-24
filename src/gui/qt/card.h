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

#ifndef LOOT_GUI_QT_CARD
#define LOOT_GUI_QT_CARD

#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>

namespace loot {
class Card : public QFrame {
  Q_OBJECT
  Q_PROPERTY(QColor shadowNearColor MEMBER shadowNearColor_)
  Q_PROPERTY(QColor shadowFarColor MEMBER shadowFarColor_)
public:
  explicit Card(QWidget* parent, bool paintTopShadow);

protected:
  static constexpr int ATTRIBUTE_ICON_HEIGHT = 18;

  static void setIcon(QLabel* label, QIcon icon);

  void paintEvent(QPaintEvent* event) override;

private:
  QColor shadowNearColor_;
  QColor shadowFarColor_;
  bool paintTopShadow_;
};
}

#endif
