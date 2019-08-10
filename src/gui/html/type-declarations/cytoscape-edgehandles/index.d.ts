declare module 'cytoscape-edgehandles' {
  import { Core, CytoscapeOptions } from 'cytoscape';

  export default function(
    cytoscape: (options?: CytoscapeOptions) => Core
  ): void;
}
