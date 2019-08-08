import { PolymerElement, html } from '@polymer/polymer';
import '@polymer/paper-dropdown-menu/paper-dropdown-menu.js';
import '@polymer/paper-listbox/paper-listbox.js';
import { PolymerElementProperties } from '@polymer/polymer/interfaces.d';

export default class LootDropdownMenu extends PolymerElement {
  public value: string;

  public label: string;

  public constructor() {
    super();
    this.value = '';
    this.label = '';
  }

  public static get is(): string {
    return 'loot-dropdown-menu';
  }

  public static get properties(): PolymerElementProperties {
    return {
      disabled: {
        type: Boolean,
        value: false,
        reflectToAttribute: true
      },
      value: {
        type: String,
        notify: true,
        reflectToAttribute: true
      },
      label: {
        type: String,
        reflectToAttribute: true
      },
      noLabelFloat: {
        type: Boolean,
        value: false,
        reflectToAttribute: true
      },
      verticalAlign: {
        type: String,
        value: 'top',
        reflectToAttribute: true
      }
    };
  }

  public static get template(): HTMLTemplateElement {
    return html`
      <style>
        paper-dropdown-menu {
          width: 100%;

          --paper-input-container: {
            transition: opacity var(--state-transition-time);
          }
          --paper-dropdown-menu-icon: {
            color: var(--secondary-text-color);
          }
        }
        :host(.dark) paper-dropdown-menu {
          --paper-input-container-underline: {
            border-color: var(--dark-theme-divider-color);
          }
          --paper-input-container-underline-disabled: {
            border-color: var(--dark-theme-divider-color);
          }
          --paper-input-container-input-color: white;
          --paper-dropdown-menu-input: {
            color: var(--dark-theme-text-color);
          }
          --paper-dropdown-menu-icon: {
            color: var(--dark-theme-secondary-color);
          }
        }

        ::slotted(paper-item) {
          white-space: nowrap;
        }
        ::slotted(paper-item:not([disabled])) {
          cursor: pointer;
        }
      </style>
      <paper-dropdown-menu
        disabled$="[[disabled]]"
        no-label-float="[[noLabelFloat]]"
        label="[[label]]"
        vertical-align="[[verticalAlign]]"
      >
        <paper-listbox
          slot="dropdown-content"
          attr-for-selected="value"
          selected="{{value}}"
        >
          <slot></slot>
        </paper-listbox>
      </paper-dropdown-menu>
    `;
  }
}

customElements.define(LootDropdownMenu.is, LootDropdownMenu);
