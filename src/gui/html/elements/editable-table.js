import { PolymerElement, html } from '@polymer/polymer';
import { beforeNextRender } from '@polymer/polymer/lib/utils/render-status';
import './editable-table-rows.js';

function getRowTemplate(templateId) {
  return document.getElementById(templateId);
}

export default class EditableTable extends PolymerElement {
  static get is() {
    return 'editable-table';
  }

  static get template() {
    return html`
      <style>
        ::slotted(table) {
          background-color: inherit;
          border-collapse: collapse;
          width: 100%;
        }
      </style>
      <slot></slot>
    `;
  }

  connectedCallback() {
    super.connectedCallback();

    beforeNextRender(this, () => {
      /* Add "add new row" row. */
      const content = getRowTemplate('newRow').content;
      let row = document.importNode(content, true);
      this.querySelector('tbody').appendChild(row);
      row = this.querySelector('tbody').lastElementChild;

      /* Add new row listener. */
      row
        .querySelector('paper-icon-button')
        .addEventListener('click', this.onAddEmptyRow);

      /* Add drag 'n' drop listeners.
            Drag 'n' drop should only be enabled for file-row tables.
        */
      if (this.getAttribute('data-template') === 'fileRow') {
        this.addEventListener('drop', EditableTable.onDrop);
        this.addEventListener('dragover', EditableTable.onDragOver);
      }
    });
  }

  disconnectedCallback() {
    super.disconnectedCallback();

    /* Remove deletion listener. */
    const icons = this.querySelector('tbody').getElementsByClassName('delete');
    for (let i = 0; i < icons.length; i += 1) {
      icons[i].removeEventListener('click', this.onRemoveRow);
    }

    /* Remove new row listener. */
    this.querySelector(
      'tbody tr:last-child paper-icon-button'
    ).removeEventListener('click', this.onAddEmptyRow);

    /* Remove drag 'n' drop listeners. */
    this.removeEventListener('drop', EditableTable.onDrop);
    this.removeEventListener('dragover', EditableTable.onDragOver);
  }

  static onDrop(evt) {
    evt.stopPropagation();

    if (
      evt.currentTarget.tagName === 'EDITABLE-TABLE' &&
      evt.currentTarget.getAttribute('data-template') === 'fileRow'
    ) {
      evt.currentTarget.addRow({
        name: evt.dataTransfer.getData('text/plain')
      });
    }

    return false;
  }

  static onDragOver(evt) {
    evt.preventDefault();
    evt.dataTransfer.dropEffect = 'copy';
  }

  getRowsData(writableOnly) {
    const writableRows = [];
    const rows = this.querySelector('tbody').rows;

    for (let i = 0; i < rows.length; i += 1) {
      const trash = rows[i].getElementsByClassName('delete');
      if (trash.length > 0 && (!writableOnly || !trash[0].disabled)) {
        const rowData = {};

        const inputs = rows[i].querySelectorAll(
          'paper-autocomplete, paper-input, paper-textarea, loot-dropdown-menu'
        );
        for (let j = 0; j < inputs.length; j += 1) {
          rowData[inputs[j].className] = (inputs[j].value || '').trim();
        }

        writableRows.push(rowData);
      }
    }

    return writableRows;
  }

  /* eslint-disable class-methods-use-this */
  setReadOnly(row, classMask = [], readOnly = true) {
    const trash = row.getElementsByClassName('delete')[0];
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
    for (let i = 0; i < inputs.length; i += 1) {
      if (classMask.length > 0) {
        for (let j = 0; j < classMask.length; j += 1) {
          if (inputs[i].classList.contains(classMask[j])) {
            inputs[i].disabled = readOnly;
            break;
          }
        }
      } else {
        inputs[i].disabled = readOnly;
      }
    }
  }
  /* eslint-enable class-methods-use-this */

  clear() {
    const rowDeletes = this.querySelector('tbody').getElementsByClassName(
      'delete'
    );
    while (rowDeletes.length > 0) {
      rowDeletes[0].click();
    }
  }

  onRemoveRow(evt) {
    const tr = evt.target.parentElement.parentElement.parentElement;
    const tbody = tr.parentElement;
    const table = tbody.parentElement;

    /* Remove deletion listener. */
    evt.target.removeEventListener('click', table.parentElement.onRemoveRow);

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

  onAddEmptyRow(evt) {
    evt.currentTarget.parentElement.parentElement.parentElement.parentElement.parentElement.parentElement.addRow(
      {}
    );

    /* Notify that the table size has changed. */
    this.dispatchEvent(
      new CustomEvent('iron-resize', {
        bubbles: true,
        composed: true
      })
    );
  }

  addRow(tableData) {
    const rowTemplateId = this.getAttribute('data-template');
    const content = getRowTemplate(rowTemplateId).content;
    let row = document.importNode(content, true);
    this.querySelector('tbody').insertBefore(
      row,
      this.querySelector('tbody').lastElementChild
    );
    row = this.querySelector('tbody').lastElementChild.previousElementSibling;

    /* Data is an object with keys that match element class names. */
    Object.entries(tableData).forEach(([key, value]) => {
      const elems = row.getElementsByClassName(key);
      if (value !== undefined && elems.length === 1) {
        elems[0].value = value;
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

  addReadOnlyRow(tableData) {
    this.setReadOnly(this.addRow(tableData));
  }

  validate() {
    const inputs = this.querySelectorAll(
      'paper-autocomplete, paper-input, paper-textarea'
    );
    for (let i = 0; i < inputs.length; i += 1) {
      if (!inputs[i].validate()) {
        return false;
      }
    }
    return true;
  }
}

customElements.define(EditableTable.is, EditableTable);
