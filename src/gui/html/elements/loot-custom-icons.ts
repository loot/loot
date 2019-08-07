import '@polymer/polymer';
import '@polymer/iron-iconset-svg/iron-iconset-svg.js';

const $documentContainer = document.createElement('div');
$documentContainer.setAttribute('style', 'display: none;');

$documentContainer.innerHTML = `<iron-iconset-svg name="loot-custom-icons">
  <svg viewBox="0 0 24 24" height="100%" width="100%" preserveAspectRatio="xMidYMid meet" fit="" style="pointer-events: none; display: block;">
    <defs>
      <g id="m-circle">
        <circle cx="12" cy="12" r="10"></circle>
        <path d="m 8,17 0,-9 0.4,0 3.2,5 0.8,0 3.2,-5 0.4,0 0,9" style="stroke: white; stroke-width: 2; fill: none;"></path>
      </g>
      <g id="crown">
        <path d="m 5,16 -1,-9 5,3 3,-7 3,7 5,-3 -1,9 z m 0,1 0,2 14,0 0,-2 z"></path>
      </g>
      <g id="brush">
        <path d="m 5,15 -1,0 0,-1 7,0 0,-10 2,0 0,10 7,0 0,1 -2,0 2,5 -9,0 -2,-2 -2,2 -3,0 2,-5 12,0" style="stroke:currentColor;stroke-width: 2;fill:none;"></path>
      </g>
      <g id="brush-checked">
        <path d="m 3.75,11 -0.75,0 0,-0.75 5.25,0 0,-7 1.5,0 0,7 5.25,0 0,0.75 -1.5,0 1.5,4 -6,0 -1.5,-1.5 -1.5,1.5 -3,0 1.5,-4 9,0" style="stroke:currentColor;stroke-width:2;fill:none;"></path>
        <path d="m 9,17 4,4 9,-9" style="stroke:currentColor;stroke-width:2;fill:none;"></path>
      </g>
      <g id="droplet">
        <path d="M17.66 8L12 2.35 6.34 8C4.78 9.56 4 11.64 4 13.64s.78 4.11 2.34 5.67 3.61 2.35 5.66 2.35 4.1-.79 5.66-2.35S20 15.64 20 13.64 19.22 9.56 17.66 8z" style="stroke:currentColor;stroke-width:2;fill:none;"></path>
      </g>
    </defs>
  </svg>
</iron-iconset-svg>`;

document.head.appendChild($documentContainer);
