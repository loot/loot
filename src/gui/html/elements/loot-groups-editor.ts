import * as cytoscape from 'cytoscape';
import edgehandles from 'cytoscape-edgehandles';
import dagre from 'cytoscape-dagre';
import { PolymerElement, html } from '@polymer/polymer';
import { PaperInputElement } from '@polymer/paper-input/paper-input.js';
import { PaperIconButtonElement } from '@polymer/paper-icon-button/paper-icon-button.js';
import { SourcedGroup, RawGroup } from '../js/interfaces';
import Translator from '../js/translator';

import { isPaperInput, isPaperIconButton } from '../js/dom/helpers';

interface AddGroupClickEvent extends Event {
  currentTarget: EventTarget &
    PaperIconButtonElement & { parentElement: PaperInputElement };
}

function isAddGroupClickEvent(evt: Event): evt is AddGroupClickEvent {
  return (
    evt.currentTarget instanceof Element &&
    isPaperIconButton(evt.currentTarget) &&
    evt.currentTarget.parentElement !== null &&
    isPaperInput(evt.currentTarget.parentElement)
  );
}

interface AddGroupKeyboardEvent extends Event {
  currentTarget: EventTarget &
    PaperInputElement & { firstElementChild: PaperIconButtonElement };
  key: string;
}

function isAddGroupKeyboardEvent(evt: Event): evt is AddGroupKeyboardEvent {
  return (
    evt instanceof KeyboardEvent &&
    evt.currentTarget instanceof Element &&
    isPaperInput(evt.currentTarget) &&
    evt.currentTarget.firstElementChild !== null &&
    isPaperIconButton(evt.currentTarget.firstElementChild)
  );
}

interface NewGroupInputEvent extends Event {
  currentTarget: EventTarget & PaperInputElement;
}

function isNewGroupInputEvent(evt: Event): evt is NewGroupInputEvent {
  return (
    evt.currentTarget instanceof Element && isPaperInput(evt.currentTarget)
  );
}

function graphElements(groups: SourcedGroup[]): cytoscape.ElementDefinition[] {
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

function removeGraphElement(
  element: cytoscape.SingularData & cytoscape.CollectionGraphManipulation
): void {
  element.remove();

  if (element.isEdge()) {
    /* Flash selection on source and target nodes to force them to update styling. */
    element.source().select();
    element.source().unselect();
    element.target().select();
    element.target().unselect();
  }
}

export default class LootGroupsEditor extends PolymerElement {
  public static get is(): string {
    return 'loot-groups-editor';
  }

  public static get template(): HTMLTemplateElement {
    return html`
      <style>
        :host > div {
          margin: 0 24px;
          height: calc(100vh - 214px);
          width: calc(100% - 48px);
          position: relative;
          display: flex;
          border: var(--divider-color) solid 1px;
        }
        #sidebar {
          display: flex;
          flex-direction: column;
          background: var(--primary-background-color);
          border-left: var(--divider-color) solid 1px;
          overflow: hidden;
          padding: 0 8px 8px 8px;
          min-width: 240px;
        }
        #cy {
          flex: auto;
          position: relative;
        }
        .inputContainer {
          height: 120px;
        }
        #groupsHelpText {
          display: block;
          margin: 16px 0 12px 0;
        }
        #newGroupButton {
          padding: 0;
          height: 24px;
          width: 24px;
        }
        #pluginList {
          background-color: var(--primary-background-color);
          flex: auto;
          overflow: auto;
          white-space: pre;
        }
        paper-icon-button {
          color: var(--secondary-text-color);
        }
        paper-icon-button[disabled] {
          color: var(--disabled-text-color);
        }
        paper-icon-button[icon='add']:hover {
          color: green;
        }
        a {
          color: var(--dark-accent-color);
          text-decoration: none;
        }
      </style>
      <div>
        <div id="cy"></div>
        <div id="sidebar">
          <div class="inputContainer">
            <a id="groupsHelpText" href="#">View Documentation</a>
            <paper-input
              id="newGroupInput"
              label="Add a new group"
              placeholder="Group name"
              always-float-label
            >
              <paper-icon-button
                id="newGroupButton"
                icon="add"
                slot="suffix"
                disabled
              ></paper-icon-button>
            </paper-input>
          </div>
          <h3 id="groupSubtitle"></h3>
          <div id="pluginList"></div>
        </div>
      </div>
    `;
  }

  private cy?: cytoscape.Core & { edgehandles: (config: object) => void };

  private cyLayoutOptions?: cytoscape.LayoutOptions & { rankDir: string };

  private getGroupPluginNames: (groupName: string) => string[];

  private l10n: Translator;

  private messages: {
    groupAlreadyExists: string;
    noPluginsInGroup: string;
  };

  public constructor() {
    super();

    this.getGroupPluginNames = () => [];
    this.l10n = new Translator();

    this.messages = {
      groupAlreadyExists: 'Group already exists!',
      noPluginsInGroup: 'No plugins are in this group.'
    };
  }

  public connectedCallback(): void {
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

    this.$.groupsHelpText.addEventListener('click', () => {
      this.dispatchEvent(
        new CustomEvent('loot-open-readme', {
          detail: { relativeFilePath: 'app/usage/groups_editor.html' },
          bubbles: true,
          composed: true
        })
      );
    });

    cytoscape.use(edgehandles);
    cytoscape.use(dagre);
  }

  public disconnectedCallback(): void {
    super.disconnectedCallback();
  }

  public setGroupPluginNamesGetter(
    getGroupPluginNames: (groupName: string) => string[]
  ): void {
    this.getGroupPluginNames = getGroupPluginNames;
  }

  public setGroups(groups: SourcedGroup[]): void {
    if (!(this.$.cy instanceof HTMLElement)) {
      throw new Error(
        'Expected loot-groups-editor shadow child with ID "cy" to be a HTML element'
      );
    }

    const bodyStyle = getComputedStyle(document.body);
    const textColor = bodyStyle.getPropertyValue('--primary-text-color');
    const textBackgroundColor = bodyStyle.getPropertyValue(
      '--primary-background-color'
    );

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
            color: textColor,
            'text-background-color': textBackgroundColor,
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
          selector: 'node#default',
          style: {
            'background-color': 'orange'
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
            // @ts-ignore arrow-scale is a valid style property, the types are
            // incomplete.
            'arrow-scale': 2,
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
      name: 'dagre',
      nodeDimensionsIncludeLabels: true,
      rankDir: 'LR',
      animate: true,
      animationDuration: 200
    };

    if (this.cy === undefined) {
      throw new Error('Expected cy field to be defined');
    }

    this.cy.addListener('cxttap', evt => this._onRemoveGraphElement(evt));
    this.cy.addListener('select', 'node', evt => this._onSelectNode(evt));

    this.cy.edgehandles({
      handlePosition: () => 'middle middle',
      edgeParams: () => ({ classes: 'userlist' })
    });

    this.$.groupSubtitle.textContent = '';
    this.$.pluginList.textContent = '';
  }

  public render(): void {
    if (this.cy === undefined) {
      throw new Error('Expected cy field to be defined');
    }

    if (this.cyLayoutOptions === undefined) {
      throw new Error('Expected cyLayoutOptions field to be defined');
    }

    this.cy.resize();
    this.cy.center();
    this.cy.layout(this.cyLayoutOptions).run();
  }

  public getUserGroups(): RawGroup[] {
    if (this.cy === undefined) {
      throw new Error('Expected cy field to be defined');
    }

    const userGroups = this.cy
      .nodes()
      .filter(
        node =>
          node.hasClass('userlist') ||
          node
            .incomers('edge')
            .some(edge => (edge as cytoscape.EdgeSingular).hasClass('userlist'))
      )
      .map((node: cytoscape.NodeSingular) => {
        const afterNames = node
          .incomers('edge')
          .filter(edge => edge.hasClass('userlist'))
          .map((edge: cytoscape.EdgeSingular) => edge.source().id());

        return {
          name: node.id(),
          after: afterNames
        };
      });

    return userGroups;
  }

  public localise(l10n: Translator): void {
    this.l10n = l10n;

    if (!isPaperInput(this.$.newGroupInput)) {
      throw new TypeError(
        "Expected loot-groups-editor's shadow root to contain a paper-input with ID 'newGroupInput'"
      );
    }

    this.$.newGroupInput.label = l10n.translate('Add a new group');
    this.$.newGroupInput.placeholder = l10n.translate('Group name');
    this.messages.groupAlreadyExists = l10n.translate('Group already exists!');
    this.messages.noPluginsInGroup = l10n.translate(
      'No plugins are in this group.'
    );
  }

  private _onSelectNode(evt: cytoscape.EventObject): void {
    if (!evt.target.isNode()) {
      return;
    }

    this.$.groupSubtitle.textContent = this.l10n.translateFormatted(
      'Plugins in %s',
      evt.target.id()
    );

    const pluginNames = this.getGroupPluginNames(evt.target.id());

    this.$.pluginList.textContent = '';
    pluginNames.forEach(pluginName => {
      this.$.pluginList.textContent += `${pluginName}\n`;
    });

    if (pluginNames.length === 0) {
      this.$.pluginList.innerHTML = `<i>${this.messages.noPluginsInGroup}</i>`;
    }
  }

  private _onRemoveGraphElement(evt: cytoscape.EventObject): void {
    if (!(this.$.groupSubtitle instanceof HTMLHeadingElement)) {
      throw new TypeError(
        "Expected loot-groups-editor's shadow root to contain a h3 with ID 'groupSubtitle'"
      );
    }

    if (evt.target === evt.cy || !evt.target.hasClass('userlist')) {
      return;
    }

    if (
      evt.target.isNode() &&
      this.getGroupPluginNames(evt.target.id()).length !== 0
    ) {
      return;
    }

    removeGraphElement(evt.target);

    if (
      this.$.groupSubtitle.textContent !== null &&
      this.$.groupSubtitle.textContent.includes(evt.target.id())
    ) {
      this.$.groupSubtitle.textContent = '';
      this.$.pluginList.textContent = '';
    }
  }

  private _onAddGroup(evt: Event): void {
    if (this.cy === undefined) {
      throw new Error('Expected cy field to be defined');
    }

    if (this.cyLayoutOptions === undefined) {
      throw new Error('Expected cyLayoutOptions field to be defined');
    }

    let input: PaperInputElement;
    let button: PaperIconButtonElement;
    if (isAddGroupKeyboardEvent(evt) && evt.type === 'keyup') {
      if (evt.key !== 'Enter') {
        return;
      }

      input = evt.currentTarget;
      button = evt.currentTarget.firstElementChild;
    } else if (isAddGroupClickEvent(evt)) {
      input = evt.currentTarget.parentElement;
      button = evt.currentTarget;
    } else {
      throw new Error(
        `Expected an AddGroupClickEvent or AddGroupKeyboardEvent, got ${evt}`
      );
    }

    if (input.value === '' || input.invalid) {
      return;
    }

    this.cy.elements().lock();

    this.cy.add({
      group: 'nodes',
      data: { id: input.value || undefined },
      classes: 'userlist'
    });

    this.cy.layout(this.cyLayoutOptions).run();
    this.cy.elements().unlock();

    input.value = '';
    button.disabled = true;
  }

  private _onNewGroupInput(evt: Event): void {
    if (this.cy === undefined) {
      throw new Error('Expected cy field to be defined');
    }

    if (!isPaperIconButton(this.$.newGroupButton)) {
      throw new TypeError(
        "Expected loot-groups-editor's shadow root to contain a paper-input with ID 'newGroupInput'"
      );
    }

    if (!isNewGroupInputEvent(evt)) {
      throw new Error(`Expected an NewGroupInputEvent, got ${evt}`);
    }

    const newGroupName = evt.currentTarget.value;

    if (
      newGroupName === '' ||
      newGroupName === undefined ||
      newGroupName === null
    ) {
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
