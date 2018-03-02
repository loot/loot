
// import {PolymerElement} from '@polymer/polymer/polymer-element.js';
// import {PaperButton} from 'paper-button/paper-button.js">
// import {PaperDialog} from 'paper-dialog/paper-dialog.js">
// import {WebAnimations} from 'neon-animation/web-animations.js">
// import {FadeInAnimation} from 'neon-animation/animations/fade-in-animation.js">
// import {FadeOutAnimation} from 'neon-animation/animations/fade-out-animation.js">
// Also depends on the loot.l10n global.

export class LootMessageDialog extends Polymer.Element {
  static get is() {
    return 'loot-message-dialog';
  }

  constructor() {
    super();

    this.closeCallback = undefined;
  }

  static get template() {
    return Polymer.html`
      <style>
        paper-dialog {
          margin: 24px 40px;
          max-width: 50%;
        }
        [hidden] {
          display: none;
        }
      </style>
      <paper-dialog id="dialog"
                    entry-animation="fade-in-animation"
                    exit-animation="fade-out-animation"
                    modal>
        <slot></slot>
        <div class="buttons">
          <paper-button id="dismiss" dialog-dismiss>[[_localise('Cancel')]]</paper-button>
          <paper-button id="confirm" dialog-confirm autofocus>[[_localise('OK')]]</paper-button>
        </div>
      </paper-dialog>`;
  }

  _localise(text) {
    return loot.l10n.translate(text);
  }

  setConfirmText(confirmText) {
    this.shadowRoot.getElementById('confirm').textContent = confirmText;
  }

  setDismissable(isDialogDismissable) {
    this.shadowRoot.getElementById('dismiss').hidden = !isDialogDismissable;
  }

  onClose(evt) {
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
    this.getElementsByClassName('message')[0].textContent = text;

    this.closeCallback = closeCallback;

    // This is necessary to prevent the dialog position changing when it is
    // displayed.
    Polymer.dom.flush(); // eslint-disable-line new-cap, no-undef

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

    this.$.dialog.addEventListener('iron-overlay-closed', this.onClose);
  }

  disconnectedCallback() {
    super.disconnectedCallback();

    this.$.dialog.removeEventListener('iron-overlay-closed', this.onClose);
  }
}

customElements.define(LootMessageDialog.is, LootMessageDialog);
