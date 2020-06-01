declare module 'cytoscape-dagre' {
  import { Core, CytoscapeOptions, LayoutOptions } from 'cytoscape';

  export interface DagreLayoutOptions {
    name: 'dagre';
    rankDir?: string;
    nodeDimensionsIncludeLabels?: boolean;
    animate?: boolean;
    animationDuration?: number;
  }

  export default function (
    cytoscape: (
      options?: CytoscapeOptions & {
        layout: LayoutOptions | DagreLayoutOptions;
      }
    ) => Core
  ): void;
}
