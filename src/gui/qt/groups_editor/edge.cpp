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

#include "gui/qt/groups_editor/edge.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QPainter>
#include <QtMath>
#include <QtWidgets/QGraphicsScene>

#include "gui/qt/groups_editor/graph_view.h"
#include "gui/qt/groups_editor/node.h"

namespace loot {
Edge::Edge(Node *sourceNode, Node *destNode, bool isUserMetadata) :
    source(sourceNode), dest(destNode), isUserMetadata_(isUserMetadata) {
  setAcceptedMouseButtons(Qt::RightButton);

  source->addEdge(this);
  source->update();

  dest->addEdge(this);
  dest->update();

  adjust();
}

Node *Edge::sourceNode() const { return source; }

Node *Edge::destNode() const { return dest; }

bool Edge::isUserMetadata() const { return isUserMetadata_; }

void Edge::adjust() {
  if (!source || !dest) {
    return;
  }

  const QLineF line(mapFromItem(source, 0, 0), mapFromItem(dest, 0, 0));
  const auto length = line.length();

  prepareGeometryChange();

  if (length > qreal{Node::DIAMETER}) {
    const QPointF edgeOffset((line.dx() * Node::RADIUS) / length,
                             (line.dy() * Node::RADIUS) / length);
    sourcePoint = line.p1() + edgeOffset;
    destPoint = line.p2() - edgeOffset;
  } else {
    sourcePoint = line.p1();
    destPoint = line.p1();
  }
}

QRectF Edge::boundingRect() const {
  if (!source || !dest) {
    return QRectF();
  }

  static constexpr auto PADDING = (LINE_WIDTH + ARROW_HYPOTENUSE) / 2.0;

  return QRectF(sourcePoint,
                QSizeF(destPoint.x() - sourcePoint.x(),
                       destPoint.y() - sourcePoint.y()))
      .normalized()
      .adjusted(-PADDING, -PADDING, PADDING, PADDING);
}

void Edge::paint(QPainter *painter,
                 const QStyleOptionGraphicsItem *,
                 QWidget *) {
  if (!source || !dest) {
    return;
  }

  auto polygon = createLineWithArrow(sourcePoint, destPoint);

  if (polygon.isEmpty()) {
    return;
  }

  const auto graphView = qobject_cast<GraphView *>(scene()->parent());
  const auto color = getDefaultColor(*graphView, isUserMetadata_);

  painter->setPen(
      QPen(color, LINE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  painter->setBrush(color);
  painter->drawPolygon(polygon);
}

void Edge::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  if (!isUserMetadata_ || event->button() != Qt::RightButton) {
    event->ignore();
    return;
  }

  // Remove this edge: remove it from its source and dest nodes, then remove
  // it from the scene, then delete the object.
  source->removeEdge(this);
  source->update();

  dest->removeEdge(this);
  dest->update();

  scene()->removeItem(this);
  delete this;
  // DO NOT use "this" past this line.
}

QPolygonF createLineWithArrow(QPointF startPos, QPointF endPos) {
  const QLineF line(startPos, endPos);

  if (qFuzzyCompare(line.length(), qreal{0.0})) {
    return QPolygonF();
  }

  const auto angleOfLine = std::atan2(-line.dy(), line.dx());
  const auto angleOfArrowEdge1 = angleOfLine - M_PI / 3;
  const auto angleOfArrowEdge2 = angleOfArrowEdge1 - M_PI / 3;

  const auto destArrowP1 =
      endPos + QPointF(sin(angleOfArrowEdge1) * ARROW_HYPOTENUSE,
                       cos(angleOfArrowEdge1) * ARROW_HYPOTENUSE);
  const auto destArrowP2 =
      endPos + QPointF(sin(angleOfArrowEdge2) * ARROW_HYPOTENUSE,
                       cos(angleOfArrowEdge2) * ARROW_HYPOTENUSE);

  return QPolygonF() << line.p1() << line.p2() << destArrowP1 << destArrowP2
                     << line.p2();
}

QColor getDefaultColor(const GraphView &graphView, bool isUserMetadata) {
  static constexpr int MAX_ALPHA = 255;

  const auto color =
      isUserMetadata ? graphView.getUserColor() : graphView.getMasterColor();

  // The color may use the alpha channel to appear darken its RGB color, but
  // that means that overlapping shapes using this color will appear lighter,
  // which we don't want. Use the alpha channel and the background color
  // to work out what the equivalent opaque color is and use that instead.
  const auto alpha = color.alpha();
  if (alpha == MAX_ALPHA) {
    return color;
  }

  const auto backgroundColor = graphView.getBackgroundColor();

  const auto red = backgroundColor.red() +
                   (color.red() - backgroundColor.red()) * alpha / MAX_ALPHA;
  const auto green =
      backgroundColor.green() +
      (color.green() - backgroundColor.green()) * alpha / MAX_ALPHA;
  const auto blue = backgroundColor.blue() +
                    (color.blue() - backgroundColor.blue()) * alpha / MAX_ALPHA;

  return QColor(red, green, blue);
}
}
