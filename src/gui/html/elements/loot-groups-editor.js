import cytoscape from 'cytoscape';
import edgehandles from 'cytoscape-edgehandles';
import coseBilkent from 'cytoscape-cose-bilkent';

function graphElements(groups) {
  const nodes = groups.map(group => ({
    data: { id: group.name },
    classes: group.isUserAdded ? 'userlist' : 'masterlist'
  }));
  const edges = groups
    .filter(group => group.after.length > 0)
    .map(group =>
      group.after.map(after => ({
        data: {
          id: group.name + after.name,
          source: after.name,
          target: group.name
        },
        classes: after.isUserAdded ? 'userlist' : 'masterlist'
      }))
    )
    .reduce((accumulator, element) => accumulator.concat(element), []);

  return nodes.concat(edges);
}

function onRemoveGraphElement(evt) {
  if (evt.target === evt.cy || !evt.target.hasClass('userlist')) {
    return;
  }

  evt.target.remove();

  if (evt.target.isEdge()) {
    /* Flash selection on source and target nodes to force them to update styling. */
    evt.target.source().select();
    evt.target.source().unselect();
    evt.target.target().select();
    evt.target.target().unselect();
  }
}

export default class LootGroupsEditor extends Polymer.Element {
  static get is() {
    return 'loot-groups-editor';
  }

  static get template() {
    return Polymer.html`
      <style>
        :host > div {
          display: flex;
          padding: 0 24px;
          height: calc(100vh - 214px);
          width: calc(100% - 48px);
          flex-direction: column;
        }
        #cy {
          flex: 1 0 auto;
          height: calc(100% - 132px);
          width: 100%;
          position: relative;
        }
        .inputContainer {
          display: flex;
          align-items: center;
          flex-direction: row;
        }
        #newGroupInput {
          height: 72px;
          position: relative;
          top: -5px;
        }
        paper-icon-button {
          color: var(--secondary-text-color);
        }
        paper-icon-button[disabled] {
          color: var(--disabled-text-color);
        }
        paper-icon-button[icon=add]:hover {
            color: green;
        }
      </style>
      <div>
        <div id="cy"></div>
        <div class="inputContainer">
          <paper-input id="newGroupInput" label="Add a new group" placeholder="Group name" always-float-label></paper-input>
          <paper-icon-button id="newGroupButton" icon="add" disabled></paper-icon-button>
        </div>
        <div id="addEdgeText">Add new ordering rules by clicking on one group and dragging a line to another.</div>
        <div id="removeEdgeText">Remove existing ordering rules by right-clicking the lines between groups. Lightly shaded lines are for masterlist metadata and cannot be removed.</div>
        <div id="removeNodeText">Remove existing groups by right-clicking them. Lightly shaded groups are for masterlist metadata and cannot be removed.</div>
      </div>
    `;
  }

  constructor() {
    super();
    this.cy = undefined;
    this.cyLayoutOptions = {};
    this.l10n = {
      translate: text => text,
      translateFormatted: text => text
    };

    this.messages = {
      groupAlreadyExists: 'Group already exists!'
    };
  }

  connectedCallback() {
    super.connectedCallback();

    this.$.newGroupButton.addEventListener('click', evt =>
      this._onAddGroup(evt)
    );

    this.$.newGroupInput.addEventListener('input', evt =>
      this._onNewGroupInput(evt)
    );
    this.$.newGroupInput.addEventListener('keyup', evt =>
      this._onAddGroup(evt)
    );

    cytoscape.use(edgehandles);
    cytoscape.use(coseBilkent);
  }

  disconnectedCallback() {
    super.disconnectedCallback();
  }

  setGroups(groups) {
    this.cy = cytoscape({
      container: this.$.cy,
      elements: graphElements(groups),
      minZoom: 0.33,
      maxZoom: 3,
      wheelSensitivity: 0.25,
      style: [
        {
          selector: 'node',
          style: {
            'background-color': '#666',
            label: 'data(id)',
            'text-background-color': 'white',
            'text-background-opacity': 1
          }
        },
        {
          selector: 'node.masterlist',
          style: {
            'background-blacken': -0.5
          }
        },
        {
          selector: 'node[[indegree = 0]][[outdegree > 0]]',
          style: {
            'background-color': '#64b5f6'
          }
        },
        {
          selector: 'node[[outdegree = 0]][[indegree > 0]]',
          style: {
            'background-color': '#8BC34A'
          }
        },
        {
          selector: 'edge',
          style: {
            width: 2,
            'curve-style': 'bezier',
            'mid-target-arrow-shape': 'triangle',
            'arrow-scale': 1.25,
            'target-endpoint': 'inside-to-node'
          }
        },
        {
          selector: 'edge.masterlist',
          style: {
            'line-color': '#ccc',
            'mid-target-arrow-color': '#ccc'
          }
        },
        {
          selector: 'edge.userlist',
          style: {
            'line-color': '#666',
            'mid-target-arrow-color': '#666'
          }
        },
        {
          selector: 'node.eh-handle',
          style: {
            label: '',
            'background-color': 'white',
            height: 15,
            width: 15
          }
        }
      ]
    });

    this.cyLayoutOptions = {
      name: 'cose-bilkent',
      nodeDimensionsIncludeLabels: true,
      gravityRange: 1
    };

    this.cy.addListener('cxttap', onRemoveGraphElement);

    this.cy.edgehandles({
      handlePosition: () => 'middle middle',
      edgeParams: () => ({ classes: 'userlist' })
    });
  }

  render() {
    this.cy.resize();
    this.cy.center();
    this.cy.layout(this.cyLayoutOptions).run();
  }

  getUserGroups() {
    const userGroups = this.cy
      .nodes()
      .filter(
        node =>
          node.hasClass('userlist') ||
          node.incomers('edge').some(edge => edge.hasClass('userlist'))
      )
      .map(node => {
        const afterNames = node
          .incomers('edge')
          .filter(edge => edge.hasClass('userlist'))
          .map(edge => edge.source().id());

        return {
          name: node.id(),
          after: afterNames
        };
      });

    return userGroups;
  }

  localise(l10n) {
    this.l10n = l10n;

    this.$.newGroupInput.label = l10n.translate('Add a new group');
    this.$.newGroupInput.placeholder = l10n.translate('Group name');
    this.$.addEdgeText.textContent = l10n.translate(
      'Add new ordering rules by clicking on one group and dragging a line to another.'
    );
    this.$.removeEdgeText.textContent = l10n.translate(
      'Remove existing ordering rules by right-clicking the lines between groups. Lightly shaded lines are for masterlist metadata and cannot be removed.'
    );
    this.$.removeNodeText.textContent = l10n.translate(
      'Remove existing groups by right-clicking them. Lightly shaded groups are for masterlist metadata and cannot be removed.'
    );

    this.messages.groupAlreadyExists = l10n.translate('Group already exists!');
  }

  _onAddGroup(evt) {
    let input;
    let button;
    if (evt.type === 'keyup') {
      if (evt.key !== 'Enter') {
        return;
      }

      input = evt.currentTarget;
      button = evt.currentTarget.nextElementSibling;
    } else {
      input = evt.currentTarget.previousElementSibling;
      button = evt.currentTarget;
    }

    if (input.value === '' || input.invalid) {
      return;
    }

    this.cy.elements().lock();

    this.cy.add({
      group: 'nodes',
      data: { id: input.value },
      classes: 'userlist'
    });

    this.cy.layout(this.cyLayoutOptions).run();
    this.cy.elements().unlock();

    input.value = '';
    button.disabled = true;
  }

  _onNewGroupInput(evt) {
    const newGroupName = evt.currentTarget.value;

    if (newGroupName === '') {
      this.$.newGroupButton.disabled = true;
      evt.currentTarget.invalid = false;
      return;
    }

    // Don't use cy.getElementById() as it's case-insensitive.
    if (this.cy.getElementById(newGroupName).nonempty()) {
      this.$.newGroupButton.disabled = true;
      evt.currentTarget.errorMessage = this.messages.groupAlreadyExists;
      evt.currentTarget.invalid = true;
      return;
    }

    this.$.newGroupButton.disabled = false;
    evt.currentTarget.invalid = false;
  }
}

customElements.define(LootGroupsEditor.is, LootGroupsEditor);
