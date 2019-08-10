import { PolymerElement, html } from '@polymer/polymer';
import * as Gestures from '@polymer/polymer/lib/utils/gestures.js';

import '@polymer/app-layout/app-toolbar/app-toolbar.js';

import '@polymer/iron-flex-layout/iron-flex-layout.js';
import '@polymer/iron-icon/iron-icon.js';
import '@polymer/iron-icons/iron-icons.js';
import '@polymer/iron-pages/iron-pages.js';

import '@polymer/paper-icon-button/paper-icon-button.js';
import '@polymer/paper-input/paper-input.js';
import '@polymer/paper-tabs/paper-tabs.js';
import { PaperToggleButtonElement } from '@polymer/paper-toggle-button/paper-toggle-button';
import '@polymer/paper-tooltip/paper-tooltip.js';
import { PolymerElementProperties } from '@polymer/polymer/interfaces.d';

import EditableTable from './editable-table';
import './loot-custom-icons';
import { crcToString, Plugin } from '../js/plugin';
import {
  PluginCleaningData,
  PluginMetadata,
  File,
  SimpleMessage,
  TagRowData,
  ModLocation,
  CleaningRowData
} from '../js/interfaces';
import { querySelector } from '../js/dom/helpers';
import LootDropdownMenu from './loot-dropdown-menu';

interface HideEditorEvent extends Event {
  target: EventTarget &
    Element & {
      parentElement: Element & {
        parentElement: Element & {
          parentNode: Node & { host: LootPluginEditor };
        };
      };
    };
}

function isHideEditorEvent(evt: Event): evt is HideEditorEvent {
  return (
    evt.target instanceof Element &&
    evt.target.parentElement !== null &&
    evt.target.parentElement.parentElement !== null &&
    evt.target.parentElement.parentElement.parentNode instanceof ShadowRoot &&
    evt.target.parentElement.parentElement.parentNode.host.tagName ===
      'LOOT-PLUGIN-EDITOR'
  );
}

interface SplitterDragEvent extends Event {
  target: EventTarget &
    Element & { parentNode: Node & { host: LootPluginEditor } };
  detail: {
    state: string;
    ddy: number;
  };
}

function isSplitterDragEvent(evt: Event): evt is SplitterDragEvent {
  // Not ideal, but track isn't a CustomEvent, so a typesafe check for
  // evt.detail.* is tricky.
  return (
    evt instanceof Event &&
    evt.type === 'track' &&
    evt.target instanceof Element &&
    evt.target.parentNode instanceof ShadowRoot &&
    evt.target.parentNode.host.tagName === 'LOOT-PLUGIN-EDITOR' &&
    'detail' in evt
  );
}

function isPaperToggleButton(
  element: Element
): element is PaperToggleButtonElement {
  return element.tagName === 'PAPER-TOGGLE-BUTTON';
}

export default class LootPluginEditor extends PolymerElement {
  public static get is(): string {
    return 'loot-plugin-editor';
  }

  public static get properties(): PolymerElementProperties {
    return {
      selected: {
        type: String,
        value: 'main'
      }
    };
  }

  public static get template(): HTMLTemplateElement {
    return html`
      <style>
        :host {
          position: relative;
          @apply --shadow-elevation-2dp;
          height: 264px;
          min-height: 56px;
          max-height: calc(100% - 56px);
          transition: height var(--state-transition-time),
            min-height var(--state-transition-time);
          background-color: var(--primary-background-color);
        }
        :host(.hidden) {
          height: 0;
          min-height: 0;
        }
        :host(.resizing) {
          transition: unset;
        }

        /* Material Design for non-button icons. */
        iron-icon {
          color: var(--secondary-text-color);
          padding: 8px;
        }

        /* Icon styling. */
        [hidden] {
          display: none;
        }
        #accept {
          color: var(--dark-accent-color);
        }
        #cancel {
          color: red;
        }

        /* Content styling. */
        ::slotted(h1) {
          margin: 0;
          font-size: 1.143rem;
        }

        /* Editor input styling. */
        #main > div {
          height: 48px;
          @apply --layout-horizontal;
          @apply --layout-center;
        }
        #main > div > div:first-child {
          margin-right: 16px;
          @apply --layout-flex;
        }
        iron-pages {
          /* Should be 104px, but that causes scroll bars to appear, probably a
            rounding error. */
          height: calc(100% - 103px);
          overflow: auto;
        }
        iron-pages > slot > div:not(.iron-selected) {
          display: none;
        }

        /* Misc Styling. */
        app-toolbar {
          background: var(
            --loot-plugin-editor-toolbar-background,
            var(--light-primary-color)
          );
          color: var(--primary-text-color);
          height: 104px;
        }
        header[top-item] {
          @apply --layout-horizontal;
          padding: 8px 16px;
          align-items: center;
        }
        #main {
          padding: 8px 16px;
        }
        #title {
          @apply --layout-flex;
          overflow: hidden;
          white-space: nowrap;
        }
        #splitter {
          width: 100%;
          height: 10px;
          position: absolute;
          top: -5px;
          cursor: row-resize;
          z-index: 1;
        }
        #splitter:active {
          background: var(--divider-color);
        }
        editable-table paper-icon-button {
          color: var(--secondary-text-color);
        }
        editable-table paper-icon-button[disabled] {
          color: var(--disabled-text-color);
        }
        editable-table paper-icon-button[icon='delete']:hover {
          color: red;
        }
        editable-table paper-icon-button[icon='add']:hover {
          color: green;
        }
      </style>
      <div id="splitter"></div>
      <app-toolbar>
        <header top-item>
          <div id="title"><slot name="title"></slot></div>
          <paper-icon-button id="accept" icon="save"></paper-icon-button>
          <paper-tooltip for="accept">Save Metadata</paper-tooltip>
          <paper-icon-button id="cancel" icon="close"></paper-icon-button>
          <paper-tooltip for="cancel">Cancel</paper-tooltip>
        </header>
        <paper-tabs
          id="tableTabs"
          selected="{{selected}}"
          attr-for-selected="data-for"
          scrollable
          bottom-item
        >
          <paper-tab data-for="main">Main</paper-tab>
          <paper-tab data-for="after">Load After</paper-tab>
          <paper-tab data-for="req">Requirements</paper-tab>
          <paper-tab data-for="inc">Incompatibilities</paper-tab>
          <paper-tab data-for="message">Messages</paper-tab>
          <paper-tab data-for="tags">Bash Tags</paper-tab>
          <paper-tab data-for="dirty">Dirty Plugin Info</paper-tab>
          <paper-tab data-for="clean">Clean Plugin Info</paper-tab>
          <paper-tab data-for="url">Locations</paper-tab>
        </paper-tabs>
      </app-toolbar>
      <iron-pages
        id="tablePages"
        selected="{{selected}}"
        attr-for-selected="data-page"
      >
        <div id="main" data-page="main">
          <div>
            <div>Enable Edits</div>
            <paper-toggle-button id="enableEdits"></paper-toggle-button>
          </div>
          <div>
            <div>Group</div>
            <loot-dropdown-menu
              id="group"
              no-label-float
              vertical-align="bottom"
            >
              <!-- Group <paper-item> elements go here. -->
            </loot-dropdown-menu>
          </div>
        </div>
        <slot name="after"></slot>
        <slot name="req"></slot>
        <slot name="inc"></slot>
        <slot name="message"></slot>
        <slot name="tags"></slot>
        <slot name="dirty"></slot>
        <slot name="clean"></slot>
        <slot name="url"></slot>
      </iron-pages>
    `;
  }

  public connectedCallback(): void {
    super.connectedCallback();
    this.$.accept.addEventListener('click', LootPluginEditor._onHideEditor);
    this.$.cancel.addEventListener('click', LootPluginEditor._onHideEditor);
    this.$.splitter.addEventListener(
      'mousedown',
      LootPluginEditor._stopPropagation
    );
    Gestures.addListener(
      this.$.splitter,
      'track',
      LootPluginEditor._onSplitterDrag
    );
  }

  public disconnectedCallback(): void {
    super.disconnectedCallback();
    this.$.accept.removeEventListener('click', LootPluginEditor._onHideEditor);
    this.$.cancel.removeEventListener('click', LootPluginEditor._onHideEditor);
    this.$.splitter.removeEventListener(
      'mousedown',
      LootPluginEditor._stopPropagation
    );
    Gestures.removeListener(
      this.$.splitter,
      'track',
      LootPluginEditor._onSplitterDrag
    );
  }

  private static _stopPropagation(evt: Event): void {
    evt.preventDefault();
    evt.stopPropagation();
  }

  private static _onSplitterDrag(evt: Event): void {
    if (!isSplitterDragEvent(evt)) {
      throw new Error(`Expected a SplitterDragEvent, got ${evt}`);
    }

    const editor = evt.target.parentNode.host;

    if (evt.detail.state === 'start') {
      editor.classList.toggle('resizing', true);
      editor.style.height = `${parseInt(
        window.getComputedStyle(editor).height || '0',
        10
      )}px`;
    } else if (evt.detail.state === 'end') {
      editor.classList.toggle('resizing', false);
    }

    const selection = window.getSelection();
    if (selection !== null) {
      selection.removeAllRanges();
    }

    editor.style.height = `${parseInt(editor.style.height || '0', 10) -
      evt.detail.ddy}px`;
    evt.preventDefault();
    evt.stopPropagation();
  }

  private static _rowDataToCrc(rowData: CleaningRowData): PluginCleaningData {
    return {
      crc: parseInt(rowData.crc, 16),
      itm: parseInt(rowData.itm, 10) || 0,
      udr: parseInt(rowData.udr, 10) || 0,
      nav: parseInt(rowData.nav, 10) || 0,
      util: rowData.utility,
      info: []
    };
  }

  public readFromEditor(): PluginMetadata {
    /* Need to turn all the editor controls' values into data to
        process. Determining whether or not the group has changed
        is left to the C++ side of things, and masterlist rows in
        the tables can be ignored because they're immutable. */

    if (!isPaperToggleButton(this.$.enableEdits)) {
      throw new TypeError(
        "Expected loot-groups-editor's shadow root to contain a paper-toggle-button with ID 'enableEdits'"
      );
    }

    if (!(this.$.group instanceof LootDropdownMenu)) {
      throw new TypeError(
        "Expected loot-groups-editor's shadow root to contain a loot-dropdown-menu with ID 'group'"
      );
    }

    const metadata: PluginMetadata = {
      name: querySelector(this, 'h1').textContent || '',
      enabled: !!this.$.enableEdits.checked,
      group: this.$.group.value,
      after: [],
      req: [],
      inc: [],
      msg: [],
      tag: [],
      dirty: [],
      clean: [],
      url: []
    };

    const tables = this.querySelectorAll('editable-table');
    tables.forEach(table => {
      if (!(table instanceof EditableTable)) {
        throw new Error('Expected editable-table to be an editable-table');
      }

      if (table.parentElement === null) {
        throw new Error('Expected table to have a parent element');
      }

      // TODO: Replace the type assertions once EditableTable has been converted to TypeScript.
      const rowsData = table.getRowsData(true);
      const tableType = table.parentElement.getAttribute('data-page');
      if (rowsData.length > 0) {
        if (tableType === 'after') {
          metadata.after = rowsData as File[];
        } else if (tableType === 'req') {
          metadata.req = rowsData as File[];
        } else if (tableType === 'inc') {
          metadata.inc = rowsData as File[];
        } else if (tableType === 'message') {
          metadata.msg = rowsData as SimpleMessage[];
        } else if (tableType === 'tags') {
          metadata.tag = rowsData.map(rowData =>
            Plugin.tagFromRowData(rowData as TagRowData)
          );
        } else if (tableType === 'dirty') {
          metadata.dirty = rowsData.map(rowData =>
            LootPluginEditor._rowDataToCrc(rowData as CleaningRowData)
          );
        } else if (tableType === 'clean') {
          metadata.clean = rowsData.map(rowData =>
            LootPluginEditor._rowDataToCrc(rowData as CleaningRowData)
          );
        } else if (tableType === 'url') {
          metadata.url = rowsData as ModLocation[];
        }
      }
    });

    return metadata;
  }

  private static _onHideEditor(evt: Event): void {
    if (!isHideEditorEvent(evt)) {
      throw new Error(`Expected a HideEditorEvent, got ${evt}`);
    }

    /* First validate table inputs. */
    let isValid = true;
    const tables = evt.target.parentElement.parentElement.parentNode.host.querySelectorAll(
      'editable-table'
    );

    for (const table of tables) {
      if (!(table instanceof EditableTable)) {
        throw new Error('Expected editable-table to be an editable-table');
      }

      if (!table.validate()) {
        isValid = false;
        break;
      }
    }

    if (isValid || evt.target.id !== 'accept') {
      /* Now hide editor. */
      const editor = evt.target.parentElement.parentElement.parentNode.host;
      editor.style.height = '';
      editor.classList.toggle('hidden', true);

      /* Fire the close event, saying whether or not to save data. */
      evt.target.dispatchEvent(
        new CustomEvent('loot-editor-close', {
          detail: evt.target.id === 'accept',
          bubbles: true,
          composed: true
        })
      );
    }
  }

  private static _dirtyInfoToRowData(
    dirtyInfo: PluginCleaningData
  ): CleaningRowData {
    return {
      crc: crcToString(dirtyInfo.crc),
      itm: dirtyInfo.itm.toString(),
      udr: dirtyInfo.udr.toString(),
      nav: dirtyInfo.nav.toString(),
      utility: dirtyInfo.util
    };
  }

  private static _highlightNonUserGroup(
    groupElements: HTMLCollection,
    pluginData: Plugin
  ): void {
    let nonUserGroup;
    if (pluginData.masterlist && pluginData.masterlist.group) {
      nonUserGroup = pluginData.masterlist.group;
    } else {
      nonUserGroup = 'default';
    }

    for (const groupElement of groupElements) {
      if (!(groupElement instanceof HTMLElement)) {
        throw new Error('Expected group element to be a HTML element');
      }

      if (
        groupElement.getAttribute('value') === nonUserGroup &&
        nonUserGroup !== pluginData.group
      ) {
        if (nonUserGroup !== pluginData.group) {
          groupElement.style.fontWeight = 'bold';
          groupElement.style.color = 'var(--primary-color)';
        }
      } else {
        groupElement.style.fontWeight = null;
        groupElement.style.color = null;
      }
    }
  }

  public setEditorData(newData: Plugin): void {
    if (!isPaperToggleButton(this.$.enableEdits)) {
      throw new TypeError(
        "Expected loot-groups-editor's shadow root to contain a paper-toggle-button with ID 'enableEdits'"
      );
    }

    if (!(this.$.group instanceof LootDropdownMenu)) {
      throw new TypeError(
        "Expected loot-groups-editor's shadow root to contain a loot-dropdown-menu with ID 'group'"
      );
    }

    /* newData is a Plugin object reference. */
    querySelector(this, 'h1').textContent = newData.name;

    /* Fill in the editor input values. */
    if (newData.userlist && !newData.userlist.enabled) {
      this.$.enableEdits.checked = false;
    } else {
      this.$.enableEdits.checked = true;
    }
    this.$.group.value = newData.group;

    LootPluginEditor._highlightNonUserGroup(this.$.group.children, newData);

    /* Clear then fill in editor table data. Masterlist-originated
        rows should have their contents made read-only. */
    const tables = this.querySelectorAll('editable-table');
    tables.forEach(table => {
      if (!(table instanceof EditableTable)) {
        throw new Error('Expected editable-table to be an editable-table');
      }

      if (table.parentElement === null) {
        throw new Error('Expected table to have a parent element');
      }

      table.clear();
      const tableType = table.parentElement.getAttribute('data-page');
      if (tableType === 'tags') {
        if (newData.masterlist && newData.masterlist.tag) {
          newData.masterlist.tag
            .map(Plugin.tagToRowData)
            .forEach(table.addReadOnlyRow, table);
        }
        if (newData.userlist && newData.userlist.tag) {
          newData.userlist.tag
            .map(Plugin.tagToRowData)
            .forEach(table.addRow, table);
        }
      } else if (tableType === 'dirty') {
        if (newData.masterlist && newData.masterlist.dirty) {
          newData.masterlist.dirty
            .map(LootPluginEditor._dirtyInfoToRowData)
            .forEach(table.addReadOnlyRow, table);
        }
        if (newData.userlist && newData.userlist.dirty) {
          newData.userlist.dirty
            .map(LootPluginEditor._dirtyInfoToRowData)
            .forEach(table.addRow, table);
        }
      } else if (tableType === 'clean') {
        if (newData.masterlist && newData.masterlist.clean) {
          newData.masterlist.clean
            .map(LootPluginEditor._dirtyInfoToRowData)
            .forEach(table.addReadOnlyRow, table);
        }
        if (newData.userlist && newData.userlist.clean) {
          newData.userlist.clean
            .map(LootPluginEditor._dirtyInfoToRowData)
            .forEach(table.addRow, table);
        }
      } else if (
        tableType === 'msg' ||
        tableType === 'after' ||
        tableType === 'req' ||
        tableType === 'inc' ||
        tableType === 'url'
      ) {
        if (newData.masterlist && newData.masterlist[tableType]) {
          newData.masterlist[tableType].forEach(table.addReadOnlyRow, table);
        }
        if (newData.userlist && newData.userlist[tableType]) {
          newData.userlist[tableType].forEach(table.addRow, table);
        }
      }
    });

    /* Now show editor. */
    this.classList.toggle('hidden', false);
  }
}

customElements.define(LootPluginEditor.is, LootPluginEditor);
