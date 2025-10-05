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

#ifndef LOOT_GUI_QT_GROUPS_EDITOR_EDGE
#define LOOT_GUI_QT_GROUPS_EDITOR_EDGE

#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsSceneMouseEvent>

namespace loot {
class Node;
class GraphView;

static constexpr int LINE_WIDTH = 1;
static constexpr qreal ARROW_HYPOTENUSE = 10;

class Edge : public QGraphicsItem {
public:
  Edge(Node *sourceNode, Node *destNode, bool isUserMetadata);

  Node *getSourceNode() const;
  Node *getDestNode() const;
  bool isUserMetadata() const;

  void adjust();

  enum { Type = UserType + 2 };
  int type() const override { return Type; }

protected:
  QRectF boundingRect() const override;
  void paint(QPainter *painter,
             const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;

  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
  Node *source{nullptr};
  Node *dest{nullptr};

  QPointF sourcePoint;
  QPointF destPoint;

  bool isUserMetadata_{false};
};

QPolygonF createLineWithArrow(QPointF startPos, QPointF endPos);

QColor getDefaultColor(const GraphView &graphView, bool isUserMetadata);

QColor toOpaqueColor(const QColor &color, const QColor &backgroundColor);
}

#endif
