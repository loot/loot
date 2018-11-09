import { PolymerElement, html } from '@polymer/polymer';
import { flush } from '@polymer/polymer/lib/utils/flush.js';
import '@polymer/paper-button/paper-button.js';
import '@polymer/paper-dialog/paper-dialog.js';
import '@polymer/neon-animation/animations/fade-in-animation.js';
import '@polymer/neon-animation/animations/fade-out-animation.js';
import 'web-animations-js/web-animations-next.min.js';
// Also depends on the loot.l10n global.

export default class LootMessageDialog extends PolymerElement {
  static get is() {
    return 'loot-message-dialog';
  }

  constructor() {
    super();

    this.closeCallback = undefined;
  }

  static get template() {
    return html`
      <style>
        paper-dialog {
          margin: 24px 40px;
          max-width: 50%;
        }
        [hidden] {
          display: none;
        }
      </style>
      <paper-dialog
        id="dialog"
        entry-animation="fade-in-animation"
        exit-animation="fade-out-animation"
        modal
      >
        <slot></slot>
        <div class="buttons">
          <paper-button id="dismiss" dialog-dismiss
            >[[_localise('Cancel')]]</paper-button
          >
          <paper-button id="confirm" dialog-confirm autofocus
            >[[_localise('OK')]]</paper-button
          >
        </div>
      </paper-dialog>
    `;
  }

  /* eslint-disable class-methods-use-this */
  _localise(text) {
    return loot.l10n.translate(text);
  }
  /* eslint-enable class-methods-use-this */

  setConfirmText(confirmText) {
    this.shadowRoot.getElementById('confirm').textContent = confirmText;
  }

  setDismissable(isDialogDismissable) {
    this.shadowRoot.getElementById('dismiss').hidden = !isDialogDismissable;
  }

  static onClose(evt) {
    if (evt.target.parentNode.host.closeCallback) {
      evt.target.parentNode.host.closeCallback(
        evt.target.closingReason.confirmed
      );
    }
    evt.target.parentNode.host.parentElement.removeChild(
      evt.target.parentNode.host
    );
  }

  showModal(title, text, closeCallback) {
    this.getElementsByClassName('heading')[0].textContent = title;
    this.getElementsByClassName('message')[0].innerHTML = text;

    this.closeCallback = closeCallback;

    // This is necessary to prevent the dialog position changing when it is
    // displayed.
    flush();

    this.$.dialog.open();
  }

  close() {
    this.$.dialog.close();
  }

  connectedCallback() {
    super.connectedCallback();

    if (this.children.length === 0) {
      const h2 = document.createElement('h2');
      h2.className = 'heading';
      this.appendChild(h2);

      const p = document.createElement('p');
      p.className = 'message';
      this.appendChild(p);
    }

    this.$.dialog.addEventListener(
      'iron-overlay-closed',
      LootMessageDialog.onClose
    );
  }

  disconnectedCallback() {
    super.disconnectedCallback();

    this.$.dialog.removeEventListener(
      'iron-overlay-closed',
      LootMessageDialog.onClose
    );
  }
}

customElements.define(LootMessageDialog.is, LootMessageDialog);
