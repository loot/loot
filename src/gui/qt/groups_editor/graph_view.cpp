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

#include "gui/qt/groups_editor/graph_view.h"

#include <math.h>

#include <QtCore/QRandomGenerator>
#include <QtGui/QKeyEvent>
#include <set>

#include "gui/qt/groups_editor/edge.h"
#include "gui/qt/groups_editor/layout.h"
#include "gui/qt/groups_editor/node.h"

namespace loot {
std::map<std::string, Node *>::iterator insertNode(
    GraphView *graphView,
    std::map<std::string, Node *> &map,
    const std::string &name,
    bool isUserMetadata,
    bool containsInstalledPlugins) {
  auto it = map.find(name);
  if (it != map.end()) {
    return it;
  }

  auto node = new Node(graphView,
                       QString::fromStdString(name),
                       isUserMetadata,
                       containsInstalledPlugins);
  graphView->scene()->addItem(node);

  return map.emplace(name, node).first;
}

GraphView::GraphView(QWidget *parent) : QGraphicsView(parent) {
  static constexpr qreal INITIAL_SCALING_FACTOR = 0.8;
  static constexpr int MIN_VIEW_SIZE = 400;

  QGraphicsScene *scene = new QGraphicsScene(this);
  scene->setItemIndexMethod(QGraphicsScene::BspTreeIndex);
  setScene(scene);
  setCacheMode(CacheBackground);
  setDragMode(QGraphicsView::ScrollHandDrag);
  setViewportUpdateMode(BoundingRectViewportUpdate);
  setRenderHint(QPainter::Antialiasing);
  setTransformationAnchor(AnchorUnderMouse);
  scale(INITIAL_SCALING_FACTOR, INITIAL_SCALING_FACTOR);
  setMinimumSize(MIN_VIEW_SIZE, MIN_VIEW_SIZE);
}

void GraphView::setGroups(const std::vector<Group> &masterlistGroups,
                          const std::vector<Group> &userGroups,
                          const std::set<std::string> &installedPluginGroups) {
  // Remove all existing items.
  scene()->clear();

  // Now add the given groups.
  std::map<std::string, Node *> groupNameNodeMap;

  for (const auto &group : masterlistGroups) {
    auto name = group.GetName();
    const auto containsInstalledPlugins = installedPluginGroups.count(name) > 0;
    insertNode(this, groupNameNodeMap, name, false, containsInstalledPlugins);
  }

  for (const auto &group : userGroups) {
    auto name = group.GetName();
    const auto containsInstalledPlugins = installedPluginGroups.count(name) > 0;
    insertNode(this, groupNameNodeMap, name, true, containsInstalledPlugins);
  }

  // Now add edges to represent all the dependencies between groups.
  // A group named in a group's "after" metadata may not exist: if
  // not, add it as a user group.
  for (const auto &group : masterlistGroups) {
    for (const auto &groupName : group.GetAfterGroups()) {
      auto node = groupNameNodeMap.at(group.GetName());
      const auto containsInstalledPlugins =
          installedPluginGroups.count(groupName) > 0;
      const auto it = insertNode(
          this, groupNameNodeMap, groupName, true, containsInstalledPlugins);

      auto edge = new Edge(it->second, node, false);
      scene()->addItem(edge);
    }
  }

  for (const auto &group : userGroups) {
    for (const auto &groupName : group.GetAfterGroups()) {
      auto node = groupNameNodeMap.at(group.GetName());
      const auto containsInstalledPlugins =
          installedPluginGroups.count(groupName) > 0;
      const auto it = insertNode(
          this, groupNameNodeMap, groupName, true, containsInstalledPlugins);

      auto edge = new Edge(it->second, node, true);
      scene()->addItem(edge);
    }
  }

  // Now position the new nodes.
  doLayout();
}

bool GraphView::addGroup(const std::string &name) {
  auto qName = QString::fromStdString(name);

  for (const auto item : scene()->items()) {
    auto node = qgraphicsitem_cast<Node *>(item);
    if (node && node->getName() == qName) {
      return false;
    }
  }

  auto node = new Node(this, qName, true, false);

  auto pos = QPointF(0, 0);
  while (scene()->itemAt(pos, QTransform())) {
    pos.setX(pos.x() + NODE_SPACING_HORIZONTAL);
  }
  scene()->addItem(node);
  node->setPosition(pos);

  return true;
}

std::vector<Group> GraphView::getUserGroups() const {
  std::vector<Group> userGroups;

  for (const auto item : scene()->items()) {
    auto node = qgraphicsitem_cast<Node *>(item);
    if (!node) {
      continue;
    }

    std::vector<std::string> afterGroups;
    for (const auto edge : node->edges()) {
      if (edge->destNode() == node &&
          (node->isUserMetadata() || edge->isUserMetadata())) {
        // The edge is going to this node, i.e. this node is after
        // sourceNode().
        afterGroups.push_back(edge->sourceNode()->getName().toStdString());
      }
    }

    if (!afterGroups.empty()) {
      userGroups.push_back(Group(node->getName().toStdString(), afterGroups));
    }
  }

  return userGroups;
}

void GraphView::handleGroupSelected(const QString &name) {
  emit groupSelected(name);
}

#if QT_CONFIG(wheelevent)
void GraphView::wheelEvent(QWheelEvent *event) {
  static constexpr double ROTATION_SCALING_FACTOR = 30.0 * 8.0;
  static constexpr double BASE_SCALE_FACTOR = 2.0;
  static constexpr double MINIMUM_SCALE = 0.07;
  static constexpr double MAXIMUM_SCALE = 100;

  const auto mouseWheelRotationAngle =
      event->angleDelta().y() / ROTATION_SCALING_FACTOR;

  const auto scaleFactor = pow(BASE_SCALE_FACTOR, mouseWheelRotationAngle);

  const auto factor = transform()
                          .scale(scaleFactor, scaleFactor)
                          .mapRect(QRectF(0, 0, 1, 1))
                          .width();

  if (factor < MINIMUM_SCALE || factor > MAXIMUM_SCALE) {
    // Don't allow infinite zoom in or out.
    return;
  }

  scale(scaleFactor, scaleFactor);
}
#endif

void GraphView::doLayout() {
  std::vector<Node *> nodes;
  for (const auto item : scene()->items()) {
    auto node = qgraphicsitem_cast<Node *>(item);
    if (node) {
      nodes.push_back(node);
    }
  }

  const auto nodePositions = calculateGraphLayout(nodes);
  for (const auto &[node, position] : nodePositions) {
    node->setPosition(position);
  }
}
}
