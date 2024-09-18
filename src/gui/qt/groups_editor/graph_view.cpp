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
#include <QtGui/QGuiApplication>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QStyle>
#include <set>

#include "gui/qt/groups_editor/edge.h"
#include "gui/qt/groups_editor/layout.h"
#include "gui/qt/groups_editor/node.h"
#include "gui/state/logging.h"

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

std::map<std::string, QPointF> convertNodePositions(
    const std::vector<GroupNodePosition> &nodePositions) {
  std::map<std::string, QPointF> map;

  for (const auto &position : nodePositions) {
    map.emplace(position.groupName, QPointF(position.x, position.y));
  }

  return map;
}

void setNodePositions(const std::vector<Node *> &nodes,
                      const std::map<std::string, QPointF> &savedPositions) {
  for (const auto node : nodes) {
    const auto name = node->getName().toStdString();
    const auto positionIt = savedPositions.find(name);

    if (positionIt == savedPositions.end()) {
      throw std::runtime_error(
          "Failed to set node positions, could not find position for " + name);
    } else {
      node->setPosition(positionIt->second);
    }
  }
}

GraphView::GraphView(QWidget *parent) :
    QGraphicsView(parent),
    masterColor(
        QGuiApplication::palette().color(QPalette::Disabled, QPalette::Text)),
    userColor(
        QGuiApplication::palette().color(QPalette::Active, QPalette::Text)),
    backgroundColor(
        QGuiApplication::palette().color(QPalette::Active, QPalette::Base)) {
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
  setBackgroundBrush(QBrush(backgroundColor));
}

void GraphView::setGroups(const std::vector<Group> &masterlistGroups,
                          const std::vector<Group> &userGroups,
                          const std::set<std::string> &installedPluginGroups,
                          const std::vector<GroupNodePosition> &nodePositions) {
  // Remove all existing items.
  scene()->clear();
  hasUnsavedLayoutChanges_ = false;

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
  doLayout(nodePositions);
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

  auto pos = mapToScene(width() / 2, height() / 2);
  while (scene()->itemAt(pos, QTransform())) {
    pos.setY(pos.y() + NODE_SPACING);
  }
  scene()->addItem(node);
  node->setPosition(pos);

  return true;
}

void GraphView::renameGroup(const std::string &oldName,
                            const std::string &newName) {
  auto qOldName = QString::fromStdString(oldName);

  for (const auto item : scene()->items()) {
    auto node = qgraphicsitem_cast<Node *>(item);
    if (node && node->getName() == qOldName) {
      node->setName(QString::fromStdString(newName));
      return;
    }
  }
}

void GraphView::setGroupContainsInstalledPlugins(
    const std::string &name,
    bool containsInstalledPlugins) {
  const auto qName = QString::fromStdString(name);

  for (const auto item : scene()->items()) {
    auto node = qgraphicsitem_cast<Node *>(item);
    if (!node || node->getName() != qName) {
      continue;
    }

    node->setContainsInstalledPlugins(containsInstalledPlugins);
    return;
  }
}

void GraphView::autoLayout() {
  doLayout({});

  // Reset unsaved change tracker because all user customisations have been
  // removed (while the auto layout results can vary, they're all pretty
  // similar and not worth counting as a user customisation).
  hasUnsavedLayoutChanges_ = false;
}

void GraphView::registerUserLayoutChange() { hasUnsavedLayoutChanges_ = true; }

std::vector<Group> GraphView::getUserGroups() const {
  std::vector<Group> userGroups;

  for (const auto item : scene()->items()) {
    auto node = qgraphicsitem_cast<Node *>(item);
    if (!node) {
      continue;
    }

    // A node should be recorded as a user group if it is user metadata itself
    // or if it is not user metadata but has a user metadata edge going to it:
    // in the latter case only user metadata edges should be recorded as load
    // after metadata.

    std::vector<std::string> afterGroups;
    for (const auto edge : node->inEdges()) {
      if (node->isUserMetadata() || edge->isUserMetadata()) {
        afterGroups.push_back(edge->sourceNode()->getName().toStdString());
      }
    }

    if (node->isUserMetadata() || !afterGroups.empty()) {
      userGroups.push_back(Group(node->getName().toStdString(), afterGroups));
    }
  }

  return userGroups;
}

std::vector<GroupNodePosition> GraphView::getNodePositions() const {
  std::vector<GroupNodePosition> nodePositions;

  for (const auto item : scene()->items()) {
    auto node = qgraphicsitem_cast<Node *>(item);
    if (node) {
      const auto name = node->getName().toStdString();
      const auto scenePos = node->scenePos();

      GroupNodePosition position{name, scenePos.x(), scenePos.y()};

      nodePositions.push_back(position);
    }
  }

  return nodePositions;
}

bool GraphView::hasUnsavedLayoutChanges() const {
  return hasUnsavedLayoutChanges_;
}

bool GraphView::isUserGroup(const std::string &name) const {
  auto qName = QString::fromStdString(name);

  for (const auto item : scene()->items()) {
    auto node = qgraphicsitem_cast<Node *>(item);
    if (node && node->getName() == qName) {
      return node->isUserMetadata();
    }
  }

  return false;
}

void GraphView::handleGroupRemoved(const QString &name) {
  emit groupRemoved(name);
}

void GraphView::handleGroupSelected(const QString &name) {
  emit groupSelected(name);
}

QColor GraphView::getMasterColor() const { return masterColor; }

QColor GraphView::getUserColor() const { return userColor; }

QColor GraphView::getBackgroundColor() const { return backgroundColor; }

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

void GraphView::doLayout(const std::vector<GroupNodePosition> &nodePositions) {
  std::vector<Node *> nodes;
  for (const auto item : scene()->items()) {
    auto node = qgraphicsitem_cast<Node *>(item);
    if (node) {
      nodes.push_back(node);
    }
  }

  const auto logger = getLogger();

  if (!nodePositions.empty()) {
    try {
      const auto nodePositionsMap = convertNodePositions(nodePositions);

      setNodePositions(nodes, nodePositionsMap);

      if (logger) {
        logger->debug("Graph layout loaded from saved node positions");
      }

      return;
    } catch (const std::exception &e) {
      if (logger) {
        logger->warn("Failed to set node positions from stored data: {}",
                     e.what());
      }
    }
  }

  if (logger) {
    logger->debug("Calculating new graph layout");
  }

  const auto calculatedNodePositions = calculateGraphLayout(nodes);
  for (const auto &[node, position] : calculatedNodePositions) {
    node->setPosition(position);
  }
}
}
