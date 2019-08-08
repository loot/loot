import { PolymerElement, html } from '@polymer/polymer';
import { flush } from '@polymer/polymer/lib/utils/flush.js';
import '@polymer/paper-button/paper-button.js';
import { PaperDialogElement } from '@polymer/paper-dialog/paper-dialog.js';
import '@polymer/neon-animation/animations/fade-in-animation.js';
import '@polymer/neon-animation/animations/fade-out-animation.js';
import 'web-animations-js/web-animations-next.min.js';
import { getShadowElementById } from '../js/dom/helpers';

// Also depends on the loot.l10n global.

interface ClosingReason {
  confirmed?: boolean;
}

interface LootMessageDialogCloseEvent extends Event {
  target: EventTarget &
    PaperDialogElement & {
      closingReason: ClosingReason | null | undefined;
      parentNode: ShadowRoot & { host: LootMessageDialog };
    };
}

function isLootMessageDialogCloseEvent(
  evt: Event
): evt is LootMessageDialogCloseEvent {
  return (
    evt.type === 'iron-overlay-closed' &&
    evt.target instanceof Element &&
    evt.target.id === 'dialog'
  );
}

function isPaperDialog(
  element: Element
): element is Element & PaperDialogElement {
  return element.tagName === 'PAPER-DIALOG';
}

export default class LootMessageDialog extends PolymerElement {
  private closeCallback?: (confirmed: boolean) => void;

  public static get is(): string {
    return 'loot-message-dialog';
  }

  public constructor() {
    super();

    this.closeCallback = undefined;
  }

  public static get template(): HTMLTemplateElement {
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
        <slot name="heading"></slot>
        <paper-dialog-scrollable>
          <slot name="message"></slot>
        </paper-dialog-scrollable>
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
  // @ts-ignore _localise is called in template bindings.
  private _localise(text: string): string {
    return window.loot.l10n.translate(text);
  }
  /* eslint-enable class-methods-use-this */

  public setConfirmText(confirmText: string): void {
    if (this.shadowRoot === null) {
      throw new Error(
        'Expected loot-message-dialog element to have a shadow root'
      );
    }
    getShadowElementById(this.shadowRoot, 'confirm').textContent = confirmText;
  }

  public setDismissable(isDialogDismissable: boolean): void {
    if (this.shadowRoot === null) {
      throw new Error(
        'Expected loot-message-dialog element to have a shadow root'
      );
    }
    getShadowElementById(
      this.shadowRoot,
      'dismiss'
    ).hidden = !isDialogDismissable;
  }

  public static onClose(evt: Event): void {
    if (!isLootMessageDialogCloseEvent(evt)) {
      throw new TypeError(`Expected a LootMessageDialogCloseEvent, got ${evt}`);
    }

    if (evt.target.parentNode.host.closeCallback) {
      evt.target.parentNode.host.closeCallback(
        !!evt.target.closingReason && !!evt.target.closingReason.confirmed
      );
    }

    if (evt.target.parentNode.host.parentElement === null) {
      throw new Error('Expected loot-message-dialog to have a parent element');
    }

    evt.target.parentNode.host.parentElement.removeChild(
      evt.target.parentNode.host
    );
  }

  public showModal(
    title: string,
    text: string,
    closeCallback?: (confirmed: boolean) => void
  ): void {
    this.getElementsByClassName('heading')[0].textContent = title;
    this.getElementsByClassName('message')[0].innerHTML = text;

    this.closeCallback = closeCallback;

    // This is necessary to prevent the dialog position changing when it is
    // displayed.
    flush();

    if (!isPaperDialog(this.$.dialog)) {
      throw new TypeError(
        "Expected loot-message-dialog's shadow root to contain a paper-dialog with ID 'dialog'"
      );
    }

    this.$.dialog.open();
  }

  public close(): void {
    if (!isPaperDialog(this.$.dialog)) {
      throw new TypeError(
        "Expected loot-message-dialog's shadow root to contain a paper-dialog with ID 'dialog'"
      );
    }

    this.$.dialog.close();
  }

  public connectedCallback(): void {
    super.connectedCallback();

    if (this.children.length === 0) {
      const h2 = document.createElement('h2');
      h2.className = 'heading';
      h2.slot = 'heading';
      this.appendChild(h2);

      const p = document.createElement('p');
      p.className = 'message';
      p.slot = 'message';
      this.appendChild(p);
    }

    this.$.dialog.addEventListener(
      'iron-overlay-closed',
      LootMessageDialog.onClose
    );
  }

  public disconnectedCallback(): void {
    super.disconnectedCallback();

    this.$.dialog.removeEventListener(
      'iron-overlay-closed',
      LootMessageDialog.onClose
    );
  }
}

customElements.define(LootMessageDialog.is, LootMessageDialog);
