import { PolymerElement, html } from '@polymer/polymer';
import '@polymer/paper-item/paper-icon-item.js';
import '@polymer/paper-material/paper-material.js';
import { PolymerElementProperties } from '@polymer/polymer/interfaces.d';

import { createMessageItem } from '../js/dom/createItem';
import './loot-custom-icons';
import './loot-menu';
import { SimpleMessage, PluginTags } from '../js/interfaces';
import { Plugin } from '../js/plugin';
// Also depends on the loot.l10n and loot.filters globals.

export default class LootPluginCard extends PolymerElement {
  public data?: Plugin;

  public static get is(): string {
    return 'loot-plugin-card';
  }

  public static get properties(): PolymerElementProperties {
    return {
      data: {
        notify: true,
        observer: '_dataChanged'
      }
    };
  }

  public static get template(): HTMLTemplateElement {
    return html`
      <style>
        /* Host styling. */
        #wrapper {
          display: block;
          background: var(--primary-background-color);
          margin: 0 8px 8px 8px;
          position: relative;
        }
        :host(.highlight) #wrapper {
          outline: 4px solid var(--accent-color);
        }
        :host(.search-result) #wrapper > * {
          box-shadow: inset 4px 0 var(--primary-color);
        }

        /* Icon styling. */
        iron-icon {
          color: var(--secondary-text-color);
          padding: 8px;
        }
        #activeTick {
          color: green;
        }
        #activeTick[hidden] {
          display: initial;
          visibility: hidden;
        }

        /* Content styling. */
        ::slotted(.tag) {
          display: inline-block;
          padding: 0 0 16px 16px;
        }
        ::slotted(ul) {
          display: block;
          padding: 0 16px 16px 40px;
          margin: 0;
        }
        ::slotted(h1) {
          font-size: 1.143rem;
          display: inline-block;
          margin: 0;
          color: inherit;
        }
        ::slotted(.version),
        ::slotted(.crc) {
          display: inline-block;
          margin-left: 16px;
          font-weight: 400;
          font-size: 1rem;
        }

        /* Misc Styling. */
        app-toolbar {
          height: 56px;
          color: var(--primary-text-color);
          background: var(--primary-background-color);
        }
        #content {
          overflow: hidden;
        }
        #title {
          @apply --layout-flex;
          overflow: hidden;
          white-space: nowrap;
        }
        :host-context(body[data-state='editing']) #editMetadata,
        :host-context(body[data-state='sorting']) #editMetadata {
          color: #9b9b9b;
          pointer-events: none;
        }
        #editMetadata,
        #copyMetadata,
        #clearMetadata {
          cursor: pointer;
        }
        [hidden],
        ::slotted([hidden]) {
          display: none;
        }
      </style>
      <paper-material id="wrapper" elevation="1">
        <app-toolbar>
          <iron-icon id="activeTick" icon="check"></iron-icon>
          <paper-tooltip for="activeTick"
            >[[_localise('Active Plugin')]]</paper-tooltip
          >
          <div id="title">
            <slot name="name"></slot> <slot name="version"></slot>
            <slot name="crc"></slot>
          </div>
          <iron-icon id="isMaster" icon="loot-custom-icons:crown"></iron-icon>
          <paper-tooltip for="isMaster"
            >[[_localise('Master File')]]</paper-tooltip
          >
          <iron-icon id="isLightMaster" icon="image:flare"></iron-icon>
          <paper-tooltip for="isLightMaster"
            >[[_localise('Light Master File')]]</paper-tooltip
          >
          <iron-icon id="isEmpty" icon="visibility-off"></iron-icon>
          <paper-tooltip for="isEmpty"
            >[[_localise('Empty Plugin')]]</paper-tooltip
          >
          <iron-icon id="loadsArchive" icon="attachment"></iron-icon>
          <paper-tooltip for="loadsArchive"
            >[[_localise('Loads Archive')]]</paper-tooltip
          >
          <iron-icon id="isClean" icon="loot-custom-icons:droplet"></iron-icon>
          <paper-tooltip id="isCleanTooltip" for="isClean"
            >[[_localise('Is Clean')]]</paper-tooltip
          >
          <iron-icon id="hasUserEdits" icon="account-circle"></iron-icon>
          <paper-tooltip for="hasUserEdits"
            >[[_localise('Has User Metadata')]]</paper-tooltip
          >
          <paper-menu-button id="menu" horizontal-align="right">
            <paper-icon-button
              icon="more-vert"
              slot="dropdown-trigger"
            ></paper-icon-button>
            <loot-menu slot="dropdown-content">
              <paper-icon-item id="editMetadata">
                <iron-icon icon="create" slot="item-icon"></iron-icon>
                [[_localise('Edit Metadata')]]
              </paper-icon-item>
              <paper-icon-item id="copyMetadata">
                <iron-icon icon="content-copy" slot="item-icon"></iron-icon>
                [[_localise('Copy Metadata')]]
              </paper-icon-item>
              <paper-icon-item id="clearMetadata">
                <iron-icon icon="delete" slot="item-icon"></iron-icon>
                [[_localise('Clear User Metadata')]]
              </paper-icon-item>
            </loot-menu>
          </paper-menu-button>
        </app-toolbar>
        <div id="content">
          <slot name="tag current"></slot> <slot name="tag add"></slot>
          <slot name="tag remove"></slot> <slot></slot>
        </div>
      </paper-material>
    `;
  }

  public connectedCallback(): void {
    super.connectedCallback();
    this.$.editMetadata.addEventListener('click', this.onShowEditor);
    this.$.copyMetadata.addEventListener(
      'click',
      LootPluginCard._onCopyMetadata
    );
    this.$.clearMetadata.addEventListener(
      'click',
      LootPluginCard._onClearMetadata
    );
  }

  public disconnectedCallback(): void {
    super.disconnectedCallback();
    this.$.editMetadata.removeEventListener('click', this.onShowEditor);
    this.$.copyMetadata.removeEventListener(
      'click',
      LootPluginCard._onCopyMetadata
    );
    this.$.clearMetadata.removeEventListener(
      'click',
      LootPluginCard._onClearMetadata
    );
  }

  /* eslint-disable class-methods-use-this */
  // @ts-ignore _localise is called in template bindings.
  private _localise(text: string): string {
    return window.loot.l10n.translate(text);
  }
  /* eslint-enable class-methods-use-this */

  // @ts-ignore _dataChanged is called in Polymer's data bindings.
  private _dataChanged(newValue: Plugin | undefined /* , oldValue */): void {
    if (newValue) {
      /* Initialise the card content data. */
      this.updateContent(true, true);
      this.updateStyling();

      if (!(this.$.activeTick instanceof HTMLElement)) {
        throw new Error(
          'Expected element with ID "activeTick" to be a HTMLElement'
        );
      }
      if (!(this.$.isMaster instanceof HTMLElement)) {
        throw new Error(
          'Expected element with ID "isMaster" to be a HTMLElement'
        );
      }
      if (!(this.$.isLightMaster instanceof HTMLElement)) {
        throw new Error(
          'Expected element with ID "isLightMaster" to be a HTMLElement'
        );
      }
      if (!(this.$.isEmpty instanceof HTMLElement)) {
        throw new Error(
          'Expected element with ID "isEmpty" to be a HTMLElement'
        );
      }
      if (!(this.$.loadsArchive instanceof HTMLElement)) {
        throw new Error(
          'Expected element with ID "loadsArchive" to be a HTMLElement'
        );
      }

      /* Set icons' visibility */
      this.$.activeTick.hidden = !newValue.isActive;
      this.$.isMaster.hidden = !newValue.isMaster;
      this.$.isLightMaster.hidden = !newValue.isLightMaster;
      this.$.isEmpty.hidden = !newValue.isEmpty;
      this.$.loadsArchive.hidden = !newValue.loadsArchive;
    }
  }

  private _setTagsContent(tags: PluginTags): void {
    if (tags) {
      const currentTags = this.getElementsByClassName('tag current')[0];
      if (!(currentTags instanceof HTMLElement)) {
        throw new Error(
          'Expected element with class "tag current" to be a HTMLElement'
        );
      }

      currentTags.textContent = tags.current;
      currentTags.hidden = tags.current.length === 0;

      const tagsToAdd = this.getElementsByClassName('tag add')[0];
      if (!(tagsToAdd instanceof HTMLElement)) {
        throw new Error(
          'Expected element with class "tag add" to be a HTMLElement'
        );
      }

      tagsToAdd.textContent = tags.add;
      tagsToAdd.hidden = tags.add.length === 0;

      const tagsToRemove = this.getElementsByClassName('tag remove')[0];
      if (!(tagsToRemove instanceof HTMLElement)) {
        throw new Error(
          'Expected element with class "tag remove" to be a HTMLElement'
        );
      }

      tagsToRemove.textContent = tags.remove;
      tagsToRemove.hidden = tags.remove.length === 0;
    }
  }

  private _setMessagesContent(messages: SimpleMessage[]): void {
    /* First clear any existing messages. */
    const messageList = this.getElementsByTagName('ul')[0];
    while (messageList.firstElementChild) {
      messageList.removeChild(messageList.firstElementChild);
    }

    if (messages) {
      /* Now add new messages. */
      messages.forEach(message => {
        messageList.appendChild(createMessageItem(message.type, message.text));
      });
      messageList.hidden = messages.length === 0;
    }
  }

  public updateContent(
    updateBodyContent: boolean,
    suppressResizeEvent: boolean
  ): void {
    if (this.data) {
      const cardContent = this.data.getCardContent(window.loot.filters);

      this.getElementsByClassName('version')[0].textContent =
        cardContent.version;

      this.getElementsByClassName('crc')[0].textContent = cardContent.crc;

      this.updateIsCleanIcon();

      if (updateBodyContent) {
        this._setTagsContent(cardContent.tags);
        this._setMessagesContent(cardContent.messages);

        if (!suppressResizeEvent) {
          /* Notify that this card (may have) changed size. */
          this.dispatchEvent(
            new CustomEvent('iron-resize', {
              bubbles: true,
              composed: true
            })
          );
        }
      }
    }
  }

  public updateIsCleanIcon(): void {
    if (!(this.$.isClean instanceof HTMLElement)) {
      throw new Error('Expected element with ID "isClean" to be a HTMLElement');
    }

    if (this.data === undefined) {
      throw new Error('Expected loot-plugin-card data to be defined');
    }

    this.$.isClean.hidden = !this.data.cleanedWith;
    this.$.isCleanTooltip.textContent = window.loot.l10n.translateFormatted(
      'Verified clean by %s',
      this.data.cleanedWith
    );
  }

  public updateStyling(): void {
    if (this.data) {
      if (!(this.$.hasUserEdits instanceof HTMLElement)) {
        throw new Error(
          'Expected element with ID "hasUserEdits" to be a HTMLElement'
        );
      }

      this.$.hasUserEdits.hidden = !this.data.hasUserEdits;

      /* Set highlight if the plugin is a search result. */
      this.classList.toggle('search-result', this.data.isSearchResult);
    }
  }

  public getName(): string {
    return this.getElementsByTagName('h1')[0].textContent || '';
  }

  public onShowEditor(): void {
    /* Fire an open event, so that the UI can enter edit mode. */
    this.dispatchEvent(
      new CustomEvent('loot-editor-open', {
        bubbles: true,
        composed: true
      })
    );
  }

  private static _onCopyMetadata(evt: Event): void {
    if (evt.target === null) {
      throw new TypeError('Expected event target to be non-null');
    }

    evt.target.dispatchEvent(
      new CustomEvent('loot-copy-metadata', {
        bubbles: true,
        composed: true
      })
    );
  }

  private static _onClearMetadata(evt: Event): void {
    if (evt.target === null) {
      throw new TypeError('Expected event target to be non-null');
    }

    evt.target.dispatchEvent(
      new CustomEvent('loot-clear-metadata', {
        bubbles: true,
        composed: true
      })
    );
  }
}

customElements.define(LootPluginCard.is, LootPluginCard);
