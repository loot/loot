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

#include "gui/qt/groups_editor/layout.h"

#include "gui/qt/groups_editor/edge.h"

namespace loot {
constexpr qreal NODE_SPACING_VERTICAL = 70;

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

std::map<Node *, QPointF> calculateGraphLayout(
    const std::vector<Node *> &nodes) {
  // This algorithm makes no attempt to avoid intersections between edges,
  // which can cause confusion, but it's good enough as a naive approach.

  // Identify nodes, root nodes and their longest path lengths.
  std::vector<Node *> rootNodes;
  std::map<Node *, int> rootNodeMaxPathLengths;
  for (const auto node : nodes) {
    if (node->isRootNode()) {
      rootNodes.push_back(node);
      auto maxPathLength = countMaxPathLength(node);
      rootNodeMaxPathLengths.emplace(node, maxPathLength);
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

  std::map<Node *, QPointF> nodePositions;
  for (const auto node : nodes) {
    if (nodeLevels.count(node) == 0) {
      // This shouldn't happen, but if this node has no assigned level, just
      // put it on the top level.
      setNodePosition(node, TOP_LEVEL);
    }

    auto level = nodeLevels.find(node)->second;
    auto offset = nodeLevelOffsets.find(node)->second;

    const auto position = QPointF(offset * NODE_SPACING_HORIZONTAL,
                                  level * NODE_SPACING_VERTICAL);

    nodePositions.emplace(node, position);
  }

  return nodePositions;
}
}
