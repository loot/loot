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

#ifndef LOOT_GUI_QT_GROUPS_EDITOR_NODE
#define LOOT_GUI_QT_GROUPS_EDITOR_NODE

#include <QtCore/QList>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsSimpleTextItem>

#include "gui/plugin_item.h"

namespace loot {
class Edge;
class GraphView;

inline static const QString DEFAULT_GROUP_NAME_QT =
    QString::fromStdString(DEFAULT_GROUP_NAME);

class NodeLabel : public QGraphicsSimpleTextItem {
public:
  NodeLabel(const QString &text, bool isUserMetadata);

  void paint(QPainter *painter,
             const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;
};

class Node : public QGraphicsItem {
public:
  Node(GraphView *graphView,
       const QString &name,
       bool isUserMetadata,
       bool containsInstalledPlugins);
  Node(const Node &) = delete;
  Node(Node &&) = delete;
  ~Node();

  Node &operator=(const Node &) = delete;
  Node &operator=(Node &&) = delete;

  QString getName() const;
  bool isUserMetadata() const;

  void addEdge(Edge *edge);
  void removeEdge(Edge *edge);
  QList<Edge *> edges() const;
  QList<Edge *> inEdges() const;
  QList<Edge *> outEdges() const;

  bool isRootNode() const;

  enum { Type = UserType + 1 };
  int type() const override { return Type; }

  QRectF boundingRect() const override;
  QPainterPath shape() const override;
  void paint(QPainter *painter,
             const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;

  void setPosition(const QPointF &pos);
  void setPosition(qreal x, qreal y);

  static constexpr int RADIUS = 10;
  static constexpr int DIAMETER = RADIUS * 2;
  static constexpr double TEXT_Y_POS = RADIUS * 1.5;

protected:
  QVariant itemChange(GraphicsItemChange change,
                      const QVariant &value) override;

  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
  QList<Edge *> edgeList;
  QGraphicsItem *edgeToCursor;
  NodeLabel *textItem;
  bool isUserMetadata_;
  bool containsInstalledPlugins;
  bool drawEdgeToCursor;

  QColor getNodeColor() const;
  void removeEdgeToCursor();
  void updateTextPos();
};
}

#endif
