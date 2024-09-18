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

#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/layered/MedianHeuristic.h>
#include <ogdf/layered/OptimalHierarchyLayout.h>
#include <ogdf/layered/OptimalRanking.h>
#include <ogdf/layered/SugiyamaLayout.h>

#include "gui/qt/groups_editor/edge.h"

namespace loot {
constexpr double LAYER_SPACING = 30.0;

std::map<Node *, QPointF> calculateGraphLayout(
    const std::vector<Node *> &nodes) {
  ogdf::Graph graph;
  ogdf::GraphAttributes graphAttributes(
      graph,
      ogdf::GraphAttributes::nodeGraphics |
          ogdf::GraphAttributes::edgeGraphics |
          ogdf::GraphAttributes::edgeArrow | ogdf::GraphAttributes::nodeLabel |
          ogdf::GraphAttributes::edgeStyle | ogdf::GraphAttributes::nodeStyle |
          ogdf::GraphAttributes::nodeTemplate);

  graphAttributes.directed() = true;

  // Add all nodes to the graph.
  std::map<Node *, ogdf::node> graphNodes;
  std::map<ogdf::node, Node *> sceneNodes;
  for (const auto node : nodes) {
    const auto graphNode = graph.newNode();
    const auto boundingRect =
        node->boundingRect().marginsRemoved(Node::MARGINS);

    // The height and width are transposed because the layout algorithm
    // arranges layers vertically, and the result is then rotated to get a
    // horizontal layout.
    graphAttributes.width(graphNode) = boundingRect.height();
    graphAttributes.height(graphNode) = boundingRect.width();

    graphNodes.emplace(node, graphNode);
    sceneNodes.emplace(graphNode, node);
  }

  // Now add all edges to the graph, without having to worry if the nodes
  // already exist or not.
  for (const auto node : nodes) {
    const auto fromGraphNode = graphNodes.find(node);
    if (fromGraphNode == graphNodes.end()) {
      throw std::logic_error("Node is not in graph");
    }

    for (const auto outEdge : node->outEdges()) {
      const auto toGraphNode = graphNodes.find(outEdge->destNode());
      if (toGraphNode == graphNodes.end()) {
        throw std::logic_error("Node is not in graph");
      }

      graph.newEdge(fromGraphNode->second, toGraphNode->second);
    }
  }

  ogdf::SugiyamaLayout SL;
  SL.setRanking(new ogdf::OptimalRanking);
  SL.setCrossMin(new ogdf::MedianHeuristic);

  ogdf::OptimalHierarchyLayout *ohl = new ogdf::OptimalHierarchyLayout;
  ohl->layerDistance(LAYER_SPACING);
  ohl->nodeDistance(NODE_SPACING);
  SL.setLayout(ohl);

  SL.call(graphAttributes);

  // Now rotate the layout to get a layers arranged horizontally.
  graphAttributes.rotateLeft90();

  std::map<Node *, QPointF> nodePositions;

  for (const auto node : graph.nodes) {
    QPointF position(graphAttributes.x(node), graphAttributes.y(node));

    const auto sceneNode = sceneNodes.find(node);
    if (sceneNode == sceneNodes.end()) {
      throw std::logic_error("Node is not in scene");
    }

    nodePositions.emplace(sceneNode->second, position);
  }

  return nodePositions;
}
}
