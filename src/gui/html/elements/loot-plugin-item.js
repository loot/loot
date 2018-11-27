import { PolymerElement, html } from '@polymer/polymer';

import '@polymer/iron-icon/iron-icon.js';
import '@polymer/iron-icons/iron-icons.js';

import '@polymer/paper-item/paper-item.js';
import '@polymer/paper-item/paper-item-body.js';
import '@polymer/paper-ripple/paper-ripple.js';
import '@polymer/paper-tooltip/paper-tooltip.js';
// Also depends on the loot.l10n global.

export default class LootPluginItem extends PolymerElement {
  static get is() {
    return 'loot-plugin-item';
  }

  static get properties() {
    return {
      loadOrderIndex: {
        type: Number,
        value: undefined
      },
      group: {
        type: String,
        value: 'default'
      },
      isEditorOpen: {
        type: Boolean,
        value: false
      },
      hasUserEdits: {
        type: Boolean,
        value: false
      },
      isLightMaster: {
        type: Boolean,
        value: false
      }
    };
  }

  static get template() {
    return html`
      <style>
        /* Flip effect for user metadata / editor open icons. */
        #flipper {
          transform-style: preserve-3d;
          transition: var(--state-transition-time);
          overflow: visible;
        }
        #flipper.flipped {
          transform: rotateY(180deg);
        }
        #hasUserEdits,
        #editorIsOpen {
          display: block;
          margin-right: 0;
          backface-visibility: hidden;
        }
        #hasUserEdits {
          transform: translateZ(1px);
          position: absolute;
          top: 0;
          left: 0;
        }
        #editorIsOpen {
          transform: rotateY(180deg) translateZ(-1px);
        }

        /* paper-item (two-line) */
        paper-icon-item {
          position: relative;
          min-height: 32px;
          line-height: 32px;
          font-size: 1rem;

          --paper-item-focused-before: {
            opacity: 0;
          }

          --paper-item-icon-width: 32px;
        }
        paper-item-body[two-line] {
          min-height: 32px;
          height: 32px;
        }
        :host-context(body[data-state='editing']) paper-item-body[two-line] {
          min-height: 40px;
        }
        #icon {
          border: 1px solid
            var(--loot-plugin-item-index-border-color, var(--paper-grey-400));
          border-radius: 50%;
          color: var(--secondary-text-color);
          height: 24px;
          width: 24px;
          line-height: 24px;
          text-align: center;
          font-family: monospace;
          text-transform: uppercase;
        }
        #primary,
        #secondary,
        #secondary iron-icon {
          transition: height var(--state-transition-time);
        }
        :host-context(body[data-state='editing']) #primary {
          line-height: normal;
        }
        #secondary > span {
          font-size: 0.857rem;
          color: var(--secondary-text-color);
          height: 13px;
          overflow: visible;
        }
        #secondary iron-icon {
          height: 13px;
          vertical-align: text-bottom;
        }
        iron-icon {
          color: var(--secondary-text-color);
        }
        /* When not in edit mode, hide secondary text. */
        :host-context(body:not([data-state='editing'])) #secondary {
          height: 0;
          overflow: hidden;
        }
        [hidden],
        #hasUserEdits[hidden] {
          display: none;
        }
        :host {
          cursor: pointer;
        }
        paper-ripple {
          color: var(--accent-color);
        }
        #icon.lightMaster {
          white-space: pre;
          line-height: 12px;
          width: 32px;
          position: relative;
          left: -3px;
          border: none;
        }
      </style>
      <paper-icon-item>
        <paper-ripple></paper-ripple>
        <div
          id="icon"
          slot="item-icon"
          class$="[[computeLoadOrderIndexClass(isLightMaster)]]"
          hidden$="[[!computeLoadOrderIndexText(loadOrderIndex, isLightMaster)]]"
          text-content="[[computeLoadOrderIndexText(loadOrderIndex, isLightMaster)]]"
        ></div>
        <paper-item-body two-line>
          <div id="primary"><slot></slot></div>
          <div id="secondary" secondary>
            <span id="groupSpan" hidden$="[[computeIsGroupHidden(group)]]">
              <span>[[group]]</span>
              <paper-tooltip id="groupTooltip" position="right"
                >Group</paper-tooltip
              >
            </span>
          </div>
        </paper-item-body>
        <paper-tooltip for="editorIsOpen" position="left"
          >[[_localise('Editor Is Open')]]</paper-tooltip
        >
        <paper-tooltip for="hasUserEdits" position="left"
          >[[_localise('Has User Metadata')]]</paper-tooltip
        >
        <div id="flipper" class$="[[computeFlipperClass(isEditorOpen)]]">
          <iron-icon id="editorIsOpen" icon="create"></iron-icon>
          <iron-icon
            id="hasUserEdits"
            icon="account-circle"
            hidden$="[[!hasUserEdits]]"
          ></iron-icon>
        </div>
      </paper-icon-item>
    `;
  }

  static _asHexString(number, numberOfDigits) {
    const text = number.toString(16);
    return '0'.repeat(numberOfDigits - text.length) + text;
  }

  /* eslint-disable class-methods-use-this */
  _localise(text) {
    return loot.l10n.translate(text);
  }

  computeFlipperClass(isEditorOpen) {
    if (isEditorOpen) {
      return 'flipped';
    }
    return '';
  }

  computeIsGroupHidden(group) {
    return group === 'default';
  }

  computeLoadOrderIndexClass(isLightMaster) {
    if (isLightMaster) {
      return 'lightMaster';
    }
    return '';
  }

  computeLoadOrderIndexText(loadOrderIndex, isLightMaster) {
    if (loadOrderIndex > -1) {
      if (isLightMaster) {
        return `FE\n${LootPluginItem._asHexString(loadOrderIndex, 3)}`;
      }

      return LootPluginItem._asHexString(loadOrderIndex, 2);
    }

    return '';
  }

  onDragStart(evt) {
    evt.dataTransfer.effectAllowed = 'copy';
    evt.dataTransfer.setData('text/plain', evt.currentTarget.getName());
    evt.dataTransfer.setDragImage(evt.currentTarget, 325, 175);
  }
  /* eslint-enable class-methods-use-this */

  getName() {
    return this.textContent.trim();
  }

  updateContent(pluginData) {
    this.loadOrderIndex = pluginData.loadOrderIndex;
    this.group = pluginData.group;
    this.isEditorOpen = pluginData.isEditorOpen;
    this.hasUserEdits = pluginData.hasUserEdits;
    this.isLightMaster = pluginData.isLightMaster;

    const indexText = this.computeLoadOrderIndexText(
      this.loadOrderIndex,
      this.isLightMaster
    );
    this.$.icon.className = this.computeLoadOrderIndexClass(this.isLightMaster);
    this.$.icon.hidden = !indexText;
    this.$.icon.textContent = indexText;

    this.$.groupSpan.hidden = this.computeIsGroupHidden(this.group);
  }
}

customElements.define(LootPluginItem.is, LootPluginItem);
