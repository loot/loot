import { PolymerElement, html } from '@polymer/polymer';
import '@polymer/paper-listbox/paper-listbox.js';

export default class LootMenu extends PolymerElement {
  static get is() {
    return 'loot-menu';
  }

  static get template() {
    return html`
      <style>
        ::slotted(*:not([disabled])) {
          cursor: pointer;
        }
      </style>
      <paper-listbox id="menu" multi> <slot></slot> </paper-listbox>
    `;
  }

  connectedCallback() {
    super.connectedCallback();

    this.$.menu.addEventListener('iron-select', LootMenu.onSelect);
  }

  disconnectedCallback() {
    super.disconnectedCallback();

    this.$.menu.removeEventListener('iron-select', LootMenu.onSelect);
  }

  static onSelect(evt) {
    evt.currentTarget.select(evt.currentTarget.indexOf(evt.detail.item));
  }
}

customElements.define(LootMenu.is, LootMenu);
