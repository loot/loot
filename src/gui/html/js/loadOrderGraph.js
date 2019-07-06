import ForceGraph from 'force-graph';

const edgeTypes = {
  Hardcoded: { color: '#a6cee3', classes: 'hardcoded' },
  'Master Flag': { color: '#1f78b4', classes: 'masterFlag' },
  Master: { color: '#b2df8a', classes: 'master' },
  'Masterlist Requirement': { color: '#33a02c', classes: 'masterlistReq' },
  'User Requirement': { color: '#fb9a99', classes: 'userReq' },
  'Masterlist Load After': {
    color: '#e31a1c',
    classes: 'masterlistLoadAfter'
  },
  'User Load After': { color: '#fdbf6f', classes: 'userLoadAfter' },
  Group: { color: '#ff7f00', classes: 'group' },
  Overlap: { color: '#cab2d6', classes: 'overlap' },
  'Tie Break': { color: '#6a3d9a', classes: 'tieBreak' },
  Unknown: { color: '#000', classes: 'unknown' }
};

function paintNodeCanvasObject(node, context, globalScale) {
  const label = node.name;
  const fontSize = 12 / globalScale;
  context.font = `${fontSize}px Sans-Serif`;
  const textWidth = context.measureText(label).width;
  const bckgDimensions = [textWidth, fontSize].map(n => n + fontSize * 0.2); // some padding
  context.fillStyle = 'rgba(255, 255, 255, 0.8)';
  context.fillRect(
    node.x - bckgDimensions[0] / 2,
    node.y - bckgDimensions[1] / 2,
    ...bckgDimensions
  );
  context.textAlign = 'center';
  context.textBaseline = 'middle';
  context.fillStyle = 'rgb(0, 0, 0)';
  context.fillText(label, node.x, node.y);
}

export function writeGraphLegend(legendElement) {
  while (legendElement.firstElementChild) {
    legendElement.removeChild(legendElement.firstElementChild);
  }

  Object.keys(edgeTypes).forEach(edgeType => {
    const span = document.createElement('span');
    span.style.color = edgeTypes[edgeType].color;
    span.textContent = `${edgeType}  `;
    legendElement.appendChild(span);
  });
}

export function drawGraph(container, loadOrderGraph) {
  while (container.firstElementChild) {
    container.removeChild(container.firstElementChild);
  }

  const f3d = ForceGraph()(container);

  const nodes = loadOrderGraph.vertices.map((vertex, index) => ({
    id: index,
    name: vertex
  }));
  const links = loadOrderGraph.edges.map(edge => ({
    source: edge.sourceIndex,
    target: edge.targetIndex,
    color: edgeTypes[edge.type].color
  }));

  f3d
    .graphData({
      nodes,
      links
    })
    .dagMode('td')
    .linkDirectionalArrowLength(1)
    .nodeCanvasObject(paintNodeCanvasObject)
    .enablePointerInteraction(false);
}
