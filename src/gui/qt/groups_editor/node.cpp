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

#include "gui/qt/groups_editor/node.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStyleOption>

#include "gui/qt/groups_editor/edge.h"
#include "gui/qt/groups_editor/graph_view.h"
#include "gui/qt/helpers.h"
#include "gui/state/logging.h"

namespace loot {
NodeLabel::NodeLabel(const GraphView &graphView,
                     const QString &text,
                     bool isUserMetadata) :
    QGraphicsSimpleTextItem(text) {
  setZValue(2);

  auto brush = this->brush();
  brush.setColor(getDefaultColor(graphView, isUserMetadata));

  setBrush(brush);
}

void NodeLabel::paint(QPainter *painter,
                      const QStyleOptionGraphicsItem *option,
                      QWidget *widget) {
  QColor windowBackgroundColor =
      QGuiApplication::palette().color(QPalette::Active, QPalette::Window);
  const auto viewBackgroundColor =
      qobject_cast<GraphView *>(scene()->parent())->getBackgroundColor();

  painter->setPen(Qt::NoPen);
  painter->setBrush(toOpaqueColor(viewBackgroundColor, windowBackgroundColor));
  painter->drawRect(boundingRect());

  QGraphicsSimpleTextItem::paint(painter, option, widget);
}

Node::Node(GraphView *graphView,
           const QString &name,
           bool isUserMetadata,
           bool containsInstalledPlugins) :
    textItem(new NodeLabel(*graphView, name, isUserMetadata)),
    isUserMetadata_(isUserMetadata),
    containsInstalledPlugins(containsInstalledPlugins) {
  setFlag(ItemIsMovable);
  setFlag(ItemSendsGeometryChanges);
  setCacheMode(DeviceCoordinateCache);
  setZValue(1);

  graphView->scene()->addItem(textItem);
}

Node::~Node() { removeEdgeToCursor(); }

QString Node::getName() const { return textItem->text(); }

bool Node::isUserMetadata() const { return isUserMetadata_; }

void Node::setName(const QString &name) {
  textItem->setText(name);
  updateTextPos();
}

void Node::setContainsInstalledPlugins(bool contains) {
  this->containsInstalledPlugins = contains;
}

void Node::addEdge(Edge *edge) {
  edgeList.append(edge);
  edge->adjust();
}

void Node::removeEdge(Edge *edge) { edgeList.removeOne(edge); }

QList<Edge *> Node::getEdges() const { return edgeList; }

QList<Edge *> Node::getInEdges() const {
  QList<Edge *> inEdges;

  for (const auto edge : edgeList) {
    if (edge->getSourceNode() != this) {
      inEdges.append(edge);
    }
  }

  return inEdges;
}

QList<Edge *> Node::getOutEdges() const {
  QList<Edge *> outEdges;

  for (const auto edge : edgeList) {
    if (edge->getDestNode() != this) {
      outEdges.append(edge);
    }
  }

  return outEdges;
}

bool Node::isRootNode() const {
  for (const auto edge : edgeList) {
    if (edge->getDestNode() == this) {
      return false;
    }
  }

  return true;
}

QRectF Node::boundingRect() const {
  const auto textRect = textItem->boundingRect();

  auto width = std::max(double{DIAMETER}, textRect.width());
  const auto height = RADIUS + TEXT_Y_POS + textRect.height();
  auto leftPos = -1 * width / 2;

  return QRectF(leftPos, -RADIUS, width, height).marginsAdded(MARGINS);
}

QPainterPath Node::shape() const {
  QPainterPath path;
  // Draw circle with a little bit of padding (radius / 2) to make it easier
  // to grab nodes.
  path.addEllipse(-RADIUS - RADIUS / 2,
                  -RADIUS - RADIUS / 2,
                  DIAMETER + RADIUS,
                  DIAMETER + RADIUS);

  const auto textRect = textItem->boundingRect();
  const auto width = textRect.width();
  const auto height = textRect.height();
  const auto leftPos = -1 * width / 2;

  path.addRect(QRectF(leftPos, TEXT_Y_POS, width, height));

  return path;
}

void Node::paint(QPainter *painter,
                 const QStyleOptionGraphicsItem *,
                 QWidget *) {
  painter->setPen(Qt::NoPen);
  painter->setBrush(getNodeColor());
  painter->drawEllipse(-RADIUS, -RADIUS, DIAMETER, DIAMETER);
}

void Node::setPosition(const QPointF &pos) {
  QGraphicsItem::setPos(pos);

  updateTextPos();
}

void Node::setPosition(qreal x, qreal y) { setPosition(QPointF(x, y)); }

QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value) {
  if (change == ItemPositionHasChanged) {
    for (Edge *edge : edgeList) {
      edge->adjust();
    }
  }

  return QGraphicsItem::itemChange(change, value);
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  update();
  QGraphicsItem::mousePressEvent(event);

  auto graphicsWidget = qobject_cast<GraphView *>(scene()->parent());

  if (isUserMetadata_ && !containsInstalledPlugins &&
      event->button() == Qt::RightButton) {
    auto logger = getLogger();
    if (logger) {
      logger->info("Removing node for group {}",
                   textItem->text().toStdString());
    }

    // Remove the node. This will involve remove the item from the
    // scene and also removing any edges that hold this node's pointer.
    // The removed edges will also need to be removed from their other
    // nodes' edge lists.
    // The removed edges and this node should also be freed from memory.
    // They can all be deleted from here, so long as "delete this" is
    // not followed by any further use of "this".
    while (!edgeList.isEmpty()) {
      auto edge = edgeList[0];
      auto otherNode =
          edge->getSourceNode() == this ? edge->getDestNode() : edge->getSourceNode();

      removeEdge(edge);
      otherNode->removeEdge(edge);
      scene()->removeItem(edge);
      delete edge;
    }

    graphicsWidget->handleGroupRemoved(textItem->text());

    scene()->removeItem(this->textItem);
    scene()->removeItem(this);

    delete this->textItem;
    delete this;
    // DO NOT use "this" past this line, except in the "else" block below.
  } else if (event->button() == Qt::RightButton) {
    // Not user metadata, ignore the event.
    event->ignore();

    // Inform the user why the event was ignored.
    if (!isUserMetadata_) {
      QMessageBox::critical(graphicsWidget,
                            "LOOT",
                            translate("Only user groups can be removed!"));
    } else if (containsInstalledPlugins) {
      QMessageBox::critical(
          graphicsWidget,
          "LOOT",
          translate("A group that is not empty cannot be removed!"));
    }
  } else {
    graphicsWidget->handleGroupSelected(textItem->text());
  }
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  update();
  QGraphicsItem::mouseReleaseEvent(event);

  // Make the item moveable in case it was made immovable in a previous double
  // click.
  setFlag(ItemIsMovable);

  const auto mousePos = event->scenePos();
  auto itemsUnderMouse = scene()->items(mousePos);

  auto logger = getLogger();
  for (const auto item : itemsUnderMouse) {
    auto node = qgraphicsitem_cast<Node *>(item);
    if (!node || node == this) {
      continue;
    }

    if (logger) {
      logger->info("Adding edge from {} to {}",
                   textItem->text().toStdString(),
                   node->textItem->text().toStdString());
    }

    auto edge = new Edge(this, node, true);
    scene()->addItem(edge);
  }

  drawEdgeToCursor = false;
  removeEdgeToCursor();
}

void Node::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
  update();
  QGraphicsItem::mouseDoubleClickEvent(event);

  if (event->button() != Qt::LeftButton) {
    return;
  }

  // Disable moving the item so that the item cannot be moved between
  // the mouse being double-clicked and released.
  setFlag(ItemIsMovable, false);

  drawEdgeToCursor = true;
}

void Node::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  QGraphicsItem::mouseMoveEvent(event);

  updateTextPos();

  qobject_cast<GraphView *>(scene()->parent())->registerUserLayoutChange();

  if (drawEdgeToCursor) {
    removeEdgeToCursor();

    const auto graphView = qobject_cast<GraphView *>(scene()->parent());
    const auto color = getDefaultColor(*graphView, true);
    auto pen =
        QPen(color, LINE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    auto polygon = createLineWithArrow(scenePos(), event->scenePos());

    if (polygon.isEmpty()) {
      return;
    }

    edgeToCursor = scene()->addPolygon(polygon, pen, color);
  }
}

QColor Node::getNodeColor() const {
  static const QString DEFAULT_GROUP_NAME_QT =
      QString::fromStdString(std::string(Group::DEFAULT_NAME));

  if (textItem->text() == DEFAULT_GROUP_NAME_QT) {
    return QColor("orange");
  }

  bool hasOutgoingEdges = false;
  bool hasIncomingEdges = false;

  for (const auto edge : edgeList) {
    if (edge->getSourceNode() == this) {
      hasOutgoingEdges = true;
    }
    if (edge->getDestNode() == this) {
      hasIncomingEdges = true;
    }
  }

  if (hasOutgoingEdges && !hasIncomingEdges) {
    return QColor("#64B5F6");
  }

  if (!hasOutgoingEdges && hasIncomingEdges) {
    return QColor("#8BC34A");
  }

  const auto graphView = qobject_cast<GraphView *>(scene()->parent());
  return getDefaultColor(*graphView, isUserMetadata_);
}

void Node::removeEdgeToCursor() {
  if (edgeToCursor) {
    scene()->removeItem(edgeToCursor);
    delete edgeToCursor;
    edgeToCursor = nullptr;
  }
}

void Node::updateTextPos() {
  const auto textWidth = textItem->boundingRect().width();
  textItem->setPos(x() - textWidth / 2, y() + TEXT_Y_POS);
}
}
