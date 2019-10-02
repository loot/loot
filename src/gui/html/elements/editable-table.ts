import { PolymerElement, html } from '@polymer/polymer';
import { beforeNextRender } from '@polymer/polymer/lib/utils/render-status';
import './editable-table-rows';
import { PaperButtonElement } from '@polymer/paper-button';
import {
  getElementById,
  querySelector,
  isPaperIconButton
} from '../js/dom/helpers';
import {
  SimpleMessage,
  TagRowData,
  CleaningRowData,
  File,
  ModLocation,
  GameSettings
} from '../js/interfaces';
import Translator from '../js/translator';

type RowData =
  | File
  | SimpleMessage
  | ModLocation
  | TagRowData
  | CleaningRowData
  | GameSettings;

function getRowTemplate(templateId: string): HTMLTemplateElement {
  const element = getElementById(templateId);
  if (!(element instanceof HTMLTemplateElement)) {
    throw new Error(
      `Expected the element with ID "${templateId} to be a template`
    );
  }

  return element;
}

interface EditableTableDragEvent extends DragEvent {
  currentTarget: EventTarget & EditableTable;
}

function isDragEvent(evt: Event): evt is EditableTableDragEvent {
  return (
    evt instanceof DragEvent &&
    evt.currentTarget instanceof Element &&
    evt.currentTarget.tagName === 'EDITABLE-TABLE'
  );
}

interface RemoveRowEvent extends Event {
  target: EventTarget &
    PaperButtonElement & {
      parentElement: Element & {
        parentElement: Element & {
          parentElement: HTMLTableRowElement;
        };
      };
    };
}

function isRemoveRowEvent(evt: Event): evt is RemoveRowEvent {
  return (
    evt.target instanceof Element &&
    isPaperIconButton(evt.target) &&
    evt.target.parentElement !== null &&
    evt.target.parentElement.parentElement !== null &&
    evt.target.parentElement.parentElement.parentElement instanceof
      HTMLTableRowElement
  );
}

interface AddEmptyRowEvent extends MouseEvent {
  target: EventTarget &
    PaperButtonElement & {
      parentNode: ShadowRoot & {
        host: EditableTable;
      };
    };
}

function isAddEmptyRowEvent(evt: Event): evt is AddEmptyRowEvent {
  return (
    evt.target instanceof Element &&
    evt.target.parentNode instanceof ShadowRoot &&
    evt.target.parentNode.host !== null &&
    evt.target.parentNode.host.tagName === 'EDITABLE-TABLE'
  );
}

interface DisableableElement extends Element {
  disabled: boolean;
}

function isDisableableElement(element: Element): element is DisableableElement {
  return 'disabled' in element;
}

interface ValidateableElement extends Element {
  validate: () => boolean;
}

function isValidateableElement(
  element: Element
): element is ValidateableElement {
  return 'validate' in element;
}

interface ValuedElement extends Element {
  value: string;
}

function isValuedElement(element: Element): element is ValuedElement {
  return 'value' in element;
}

export default class EditableTable extends PolymerElement {
  public static get is(): string {
    return 'editable-table';
  }

  public static get template(): HTMLTemplateElement {
    return html`
      <style>
        ::slotted(table) {
          background-color: inherit;
          border-collapse: collapse;
          width: 100%;
        }
        #addRowButton {
          margin-left: 8px;
        }
      </style>
      <slot></slot>
      <paper-button id="addRowButton">Add New Row</paper-button>
    `;
  }

  public connectedCallback(): void {
    super.connectedCallback();

    beforeNextRender(this, () => {
      /* Add new row listener. */
      this.$.addRowButton.addEventListener('click', this.onAddEmptyRow);

      /* Add drag 'n' drop listeners.
         Drag 'n' drop should only be enabled for file-row tables.
      */
      if (this.getAttribute('data-template') === 'fileRow') {
        this.addEventListener('drop', EditableTable.onDrop);
        this.addEventListener('dragover', EditableTable.onDragOver);
      }
    });
  }

  public disconnectedCallback(): void {
    super.disconnectedCallback();

    /* Remove deletion listener. */
    const icons = querySelector(this, 'tbody').getElementsByClassName('delete');
    for (const icon of icons) {
      icon.removeEventListener('click', this.onRemoveRow);
    }

    /* Remove new row listener. */
    this.$.addRowButton.removeEventListener('click', this.onAddEmptyRow);

    /* Remove drag 'n' drop listeners. */
    this.removeEventListener('drop', EditableTable.onDrop);
    this.removeEventListener('dragover', EditableTable.onDragOver);
  }

  public static onDrop(evt: Event): boolean {
    if (!isDragEvent(evt)) {
      throw new Error(
        `Expected event to be a EditableTableDragEvent, got ${evt}`
      );
    }
    evt.stopPropagation();

    if (
      evt.currentTarget.tagName === 'EDITABLE-TABLE' &&
      evt.currentTarget.getAttribute('data-template') === 'fileRow' &&
      evt.dataTransfer !== null
    ) {
      const data = evt.dataTransfer.getData('text/plain');
      if (typeof data !== 'string') {
        throw new Error("Expected drag 'n' drop data to be a string");
      }

      evt.currentTarget.addRow({
        name: data,
        display: '',
        condition: ''
      });
    }

    return false;
  }

  public static onDragOver(evt: Event): void {
    if (!isDragEvent(evt)) {
      throw new Error(
        `Expected event to be a EditableTableDragEvent, got ${evt}`
      );
    }

    evt.preventDefault();
    if (evt.dataTransfer) {
      evt.dataTransfer.dropEffect = 'copy';
    }
  }

  public getRowsData(writableOnly: boolean): RowData[] {
    const writableRows: RowData[] = [];

    const tbody = querySelector(this, 'tbody');
    if (!(tbody instanceof HTMLTableSectionElement)) {
      throw new Error('Expected tbody element to be a HTMLTableSectionElement');
    }
    const rows = tbody.rows;

    for (const row of rows) {
      const trashElements = row.getElementsByClassName('delete');
      if (trashElements.length === 0) {
        continue; // eslint-disable-line no-continue
      }

      const trash = trashElements[0];
      if (!isPaperIconButton(trash)) {
        throw new Error(
          'Expected element with class "delete" to be a paper-icon-button'
        );
      }

      if (!writableOnly || !trash.disabled) {
        // It's probably not worth trying to handle all the type variations...
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        const rowData: any = {};

        const inputs = row.querySelectorAll(
          'paper-autocomplete, paper-input, paper-textarea, loot-dropdown-menu'
        );
        for (const input of inputs) {
          if (!isValuedElement(input)) {
            throw new Error('Expected element to have value property');
          }

          rowData[input.className] = (input.value || '').trim();
        }

        writableRows.push(rowData);
      }
    }

    return writableRows;
  }

  /* eslint-disable class-methods-use-this */
  public setReadOnly(
    row: Element,
    classMask: string[] = [],
    readOnly = true
  ): void {
    const trash = row.getElementsByClassName('delete')[0];
    if (!isPaperIconButton(trash)) {
      throw new Error(
        'Expected element with class "delete" to be a paper-icon-button'
      );
    }
    if (classMask.length > 0) {
      if (classMask.indexOf('delete') !== -1) {
        trash.disabled = readOnly;
      }
    } else {
      trash.disabled = readOnly;
    }

    const inputs = row.querySelectorAll(
      'paper-autocomplete, paper-input, paper-textarea, loot-dropdown-menu'
    );
    for (const input of inputs) {
      if (!isDisableableElement(input)) {
        throw new Error('Expected row input to have a "disabled" property');
      }

      if (classMask.length > 0) {
        for (const className of classMask) {
          if (input.classList.contains(className)) {
            input.disabled = readOnly;
            break;
          }
        }
      } else {
        input.disabled = readOnly;
      }
    }
  }
  /* eslint-enable class-methods-use-this */

  public clear(): void {
    const rowDeletes = querySelector(this, 'tbody').getElementsByClassName(
      'delete'
    );
    while (rowDeletes.length > 0) {
      const rowDelete = rowDeletes[0];
      if (!isPaperIconButton(rowDelete)) {
        throw new Error(
          'Expected element with class "delete" to be a paper-icon-button'
        );
      }
      rowDelete.click();
    }
  }

  public onRemoveRow(evt: Event): void {
    if (!isRemoveRowEvent(evt)) {
      throw new Error(`Expected event to be a RemoveRowEvent, got ${evt}`);
    }

    const tr = evt.target.parentElement.parentElement.parentElement;
    const tbody = tr.parentElement;
    if (tbody === null) {
      throw new Error('Expected table row to be a child of a tbody element');
    }

    const table = tbody.parentElement;
    if (table === null) {
      throw new Error('Expected tbody to be a child of a table element');
    }

    const editableTable = table.parentElement;
    if (!(editableTable instanceof EditableTable)) {
      throw new Error(
        'Expected table element to be a child of an editable-table'
      );
    }

    /* Remove deletion listener. */
    evt.target.removeEventListener('click', editableTable.onRemoveRow);

    /* Now remove row. */
    tbody.removeChild(tr);

    /* Notify that the table size has changed. */
    this.dispatchEvent(
      new CustomEvent('iron-resize', {
        bubbles: true,
        composed: true
      })
    );
  }

  public onAddEmptyRow(evt: Event): void {
    if (!isAddEmptyRowEvent(evt)) {
      throw new Error(`Expected event to be a AddEmptyRowEvent, got ${evt}`);
    }

    evt.target.parentNode.host.addRow({});

    /* Notify that the table size has changed. */
    this.dispatchEvent(
      new CustomEvent('iron-resize', {
        bubbles: true,
        composed: true
      })
    );
  }

  public addRow(tableData: RowData | {}): Element {
    const rowTemplateId = this.getAttribute('data-template');
    if (rowTemplateId === null) {
      throw new Error('Expected template ID for new row to be non-null');
    }

    const content = getRowTemplate(rowTemplateId).content;
    const rowDocFragment = document.importNode(content, true);

    const tbody = querySelector(this, 'tbody');
    tbody.appendChild(rowDocFragment);
    const row = tbody.lastElementChild;

    if (row === null) {
      throw new Error('Expected table body to contain at least two rows');
    }

    /* Data is an object with keys that match element class names. */
    Object.entries(tableData).forEach(([key, value]) => {
      const elems = row.getElementsByClassName(key);
      if (value !== undefined && elems.length === 1) {
        const element = elems[0];
        if (!isValuedElement(element)) {
          throw new Error('Expected element to have value property');
        }

        element.value = value;
      }
    });

    /* Add deletion listener. */
    row
      .getElementsByClassName('delete')[0]
      .addEventListener('click', this.onRemoveRow);

    /* Notify that the table size has changed. */
    this.dispatchEvent(
      new CustomEvent('iron-resize', {
        bubbles: true,
        composed: true
      })
    );

    return row;
  }

  public addReadOnlyRow(tableData: RowData): void {
    this.setReadOnly(this.addRow(tableData));
  }

  public validate(): boolean {
    const inputs = this.querySelectorAll(
      'paper-autocomplete, paper-input, paper-textarea'
    );
    for (const input of inputs) {
      if (!isValidateableElement(input)) {
        throw new Error('Expected row input to have a "validate" method');
      }
      if (!input.validate()) {
        return false;
      }
    }
    return true;
  }

  public localise(l10n: Translator): void {
    this.$.addRowButton.textContent = l10n.translate('Add New Row');
  }
}

customElements.define(EditableTable.is, EditableTable);
