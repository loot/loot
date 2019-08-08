import { PolymerElement, html } from '@polymer/polymer';
import { IronSelectableBehavior } from '@polymer/iron-selector/iron-selectable';

interface LootMenuSelectEvent extends Event {
  currentTarget: EventTarget & IronSelectableBehavior;
  detail: { item: Element };
}

function isLootMenuSelectEvent(evt: Event): evt is LootMenuSelectEvent {
  // Not ideal, but iron-select isn't a CustomEvent, so a typesafe check for
  // evt.detail.item is tricky.
  return (
    evt.type === 'iron-select' &&
    evt.currentTarget instanceof Element &&
    evt.currentTarget.tagName === 'PAPER-LISTBOX'
  );
}

export default class LootMenu extends PolymerElement {
  public static get is(): string {
    return 'loot-menu';
  }

  public static get template(): HTMLTemplateElement {
    return html`
      <style>
        ::slotted(*:not([disabled])) {
          cursor: pointer;
        }
      </style>
      <paper-listbox id="menu" multi> <slot></slot> </paper-listbox>
    `;
  }

  public connectedCallback(): void {
    super.connectedCallback();

    this.$.menu.addEventListener('iron-select', LootMenu.onSelect);
  }

  public disconnectedCallback(): void {
    super.disconnectedCallback();

    this.$.menu.removeEventListener('iron-select', LootMenu.onSelect);
  }

  public static onSelect(evt: Event): void {
    if (!isLootMenuSelectEvent(evt)) {
      throw new TypeError(`Expected a LootMenuSelectEvent, got ${evt}`);
    }
    evt.currentTarget.select(evt.currentTarget.indexOf(evt.detail.item));
  }
}

customElements.define(LootMenu.is, LootMenu);
