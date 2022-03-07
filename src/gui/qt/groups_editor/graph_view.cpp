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
#include "gui/qt/groups_editor/node.h"

namespace loot {
constexpr qreal NODE_SPACING_HORIZONTAL = 70;
constexpr qreal NODE_SPACING_VERTICAL = 70;

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

void depthFirstSearch(Node *node,
                      int depth,
                      std::set<Node *> &visitedNodes,
                      std::function<void(Node *, int)> visitor) {
  visitor(node, depth);

  visitedNodes.insert(node);

  for (const auto edge : node->outEdges()) {
    auto nextNode = edge->destNode();
    if (visitedNodes.count(nextNode) == 0) {
      depthFirstSearch(nextNode, depth + 1, visitedNodes, visitor);
    }
  }
}

void reverseDepthFirstSearch(Node *node,
                             int depth,
                             std::set<Node *> &visitedNodes,
                             std::function<void(Node *, int)> visitor) {
  visitor(node, depth);

  visitedNodes.insert(node);

  for (const auto edge : node->inEdges()) {
    auto nextNode = edge->sourceNode();
    if (visitedNodes.count(nextNode) == 0) {
      reverseDepthFirstSearch(nextNode, depth - 1, visitedNodes, visitor);
    }
  }

  return;
}

int countMaxPathLength(Node *rootNode) {
  std::set<Node *> visitedNodes;
  static constexpr int INITIAL_DEPTH = 1;
  auto maxDepth = INITIAL_DEPTH;

  depthFirstSearch(rootNode,
                   INITIAL_DEPTH,
                   visitedNodes,
                   [&maxDepth](const Node *, int depth) {
                     if (depth > maxDepth) {
                       maxDepth = depth;
                     }
                   });

  return maxDepth;
};

void visitConnectedSubgraph(Node *topLevelNode,
                            int topLevel,
                            std::function<void(Node *, int)> setNodePosition) {
  std::set<Node *> visitedNodes;

  Node *leafNode = topLevelNode;
  int furthestLevel = topLevel;
  depthFirstSearch(
      topLevelNode, topLevel, visitedNodes, [&](Node *node, int depth) {
        // Set the node position, and record the leaf node of the longest
        // path.
        setNodePosition(node, depth);
        if (depth > furthestLevel) {
          leafNode = node;
          furthestLevel = depth;
        }
      });

  // Now start with the leaf node of the longest path and do a DFS of in
  // edges to find any nodes that are connected but not yet assigned levels.
  visitedNodes.clear();
  reverseDepthFirstSearch(
      leafNode, furthestLevel, visitedNodes, setNodePosition);
};

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
  // This algorithm makes no attempt to avoid intersections between edges,
  // which can cause confusion, but it's good enough as a naive approach.

  // Identify nodes, root nodes and their longest path lengths.
  std::vector<Node *> nodes;
  std::vector<Node *> rootNodes;
  std::map<Node *, int> rootNodeMaxPathLengths;
  for (const auto item : scene()->items()) {
    auto node = qgraphicsitem_cast<Node *>(item);
    if (node) {
      nodes.push_back(node);
      if (node->isRootNode()) {
        rootNodes.push_back(node);
        auto maxPathLength = countMaxPathLength(node);
        rootNodeMaxPathLengths.emplace(node, maxPathLength);
      }
    }
  }

  // Sort the root nodes by their max path length so that the node with the
  // longest path is first. This ensures that the longest path is laid out
  // to be the straightest.
  std::sort(rootNodes.begin(),
            rootNodes.end(),
            [&rootNodeMaxPathLengths](auto lhs, auto rhs) {
              return rootNodeMaxPathLengths.at(lhs) >
                     rootNodeMaxPathLengths.at(rhs);
            });

  // Set the node level to its depth along the current path. To prevent nodes
  // at the same depth from being positioned at the same point, keep a count
  // of how many nodes each level has, and apply an offset based on that count.
  std::set<Node *> visitedNodes;
  std::map<Node *, int> nodeLevels;
  std::map<Node *, size_t> nodeLevelOffsets;
  std::map<int, size_t> levelNodeCounts;

  const auto setNodePosition = [&](Node *node, int depth) {
    if (nodeLevels.count(node) != 0) {
      // Don't set a position if it's already got one.
      return;
    }

    auto it = levelNodeCounts.find(depth);
    if (it == levelNodeCounts.end()) {
      it = levelNodeCounts.emplace(depth, 0).first;
    }

    nodeLevels.emplace(node, depth);
    nodeLevelOffsets.emplace(node, it->second);
    it->second += 1;
  };

  // Now visit each of the root nodes in turn, setting levels and offsets for
  // every connected node.
  static constexpr int TOP_LEVEL = 0;
  for (const auto rootNode : rootNodes) {
    visitConnectedSubgraph(rootNode, TOP_LEVEL, setNodePosition);
  }

  for (const auto node : nodes) {
    if (nodeLevels.count(node) == 0) {
      // This shouldn't happen, but if this node has no assigned level, just
      // put it on the top level.
      setNodePosition(node, TOP_LEVEL);
    }

    auto level = nodeLevels.find(node)->second;
    auto offset = nodeLevelOffsets.find(node)->second;

    node->setPosition(offset * NODE_SPACING_HORIZONTAL,
                      level * NODE_SPACING_VERTICAL);
  }
}
}
