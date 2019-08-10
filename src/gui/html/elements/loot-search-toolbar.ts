import { PolymerElement, html } from '@polymer/polymer';
import '@polymer/app-layout/app-toolbar/app-toolbar.js';
import '@polymer/iron-icons/iron-icons.js';
import '@polymer/iron-flex-layout/iron-flex-layout.js';
import '@polymer/paper-tooltip/paper-tooltip.js';
import { PolymerElementProperties } from '@polymer/polymer/interfaces.d';
import { isPaperInput, isPaperIconButton } from '../js/dom/helpers';

function isHTMLElement(element: Element): element is HTMLElement {
  return element instanceof HTMLElement;
}

interface LootSearchToolbarEnterPrevNextEvent extends KeyboardEvent {
  target: Element & {
    parentElement: Element & {
      parentNode: ShadowRoot & { host: LootSearchToolbar };
    };
  };
}

function isLootSearchToolbarEnterPrevNextEvent(
  evt: Event
): evt is LootSearchToolbarEnterPrevNextEvent {
  return (
    evt.target instanceof Element &&
    evt.target.parentElement !== null &&
    evt.target.parentElement.parentNode instanceof ShadowRoot
  );
}

interface LootSearchToolbarSearchEvent extends KeyboardEvent {
  target: Element & {
    value: string;
    parentElement: Element & {
      parentNode: ShadowRoot & { host: LootSearchToolbar };
    };
  };
}

function isLootSearchToolbarSearchEvent(
  evt: Event
): evt is LootSearchToolbarSearchEvent {
  return (
    evt.target instanceof Element &&
    evt.target.parentElement !== null &&
    evt.target.parentElement.parentNode instanceof ShadowRoot
  );
}

export default class LootSearchToolbar extends PolymerElement {
  private _currentResult: number;

  public results: number[];

  public constructor() {
    super();
    this._currentResult = 0;
    this.results = [];
  }

  public static get is(): string {
    return 'loot-search-toolbar';
  }

  public static get properties(): PolymerElementProperties {
    return {
      _currentResult: {
        type: Number,
        value: -1,
        observer: '_currentResultChanged'
      },
      results: {
        type: Array,
        value: () => [],
        observer: '_resultsChanged'
      }
    };
  }

  public static get template(): HTMLTemplateElement {
    return html`
      <style>
        app-toolbar {
          height: 56px;
          background: var(--primary-color);
          color: var(--dark-theme-text-color);
        }

        #search {
          @apply --layout-flex;

          --paper-input-container-input-color: var(--dark-theme-text-color);
          --paper-input-container-color: var(--dark-theme-secondary-color);
          --paper-input-container-underline: {
            border-color: var(--dark-theme-divider-color);
          }

          display: block;
        }
        #count {
          font-weight: 400;
          font-size: 0.857rem;
          color: var(--dark-theme-secondary-color);
        }
      </style>
      <app-toolbar>
        <paper-input
          id="search"
          label="Search cards"
          no-label-float
        ></paper-input>
        <div id="count">
          <span>[[_computeResultNum(_currentResult)]]</span> /
          <span><!-- number of results --></span>
        </div>
        <paper-icon-button
          id="prev"
          icon="expand-less"
          disabled
        ></paper-icon-button>
        <paper-icon-button
          id="next"
          icon="expand-more"
          disabled
        ></paper-icon-button>
        <paper-icon-button id="close" icon="close"></paper-icon-button>
      </app-toolbar>
    `;
  }

  // @ts-ignore used in the Polymer template
  private _currentResultChanged(newValue: number): void {
    if (this.results && this.results.length > 0) {
      if (!isPaperIconButton(this.$.prev)) {
        throw new TypeError(
          "Expected loot-message-dialog's shadow root to contain an HTML element with ID 'prev'"
        );
      }

      if (!isPaperIconButton(this.$.next)) {
        throw new TypeError(
          "Expected loot-message-dialog's shadow root to contain an HTML element with ID 'next'"
        );
      }

      if (this.results.length === 1) {
        this.$.prev.disabled = true;
        this.$.next.disabled = true;
      } else if (newValue === 0) {
        this.$.prev.disabled = true;
        this.$.next.disabled = false;
      } else if (newValue === this.results.length - 1) {
        this.$.prev.disabled = false;
        this.$.next.disabled = true;
      } else {
        this.$.prev.disabled = false;
        this.$.next.disabled = false;
      }
    }
    if (newValue > -1) {
      this.dispatchEvent(
        new CustomEvent('loot-search-change-selection', {
          detail: { selection: this.results[newValue] },
          bubbles: true,
          composed: true
        })
      );
    }
  }

  // @ts-ignore used in the Polymer template
  private _resultsChanged(newValue: number[]): void {
    if (!isHTMLElement(this.$.count)) {
      throw new TypeError(
        "Expected loot-message-dialog's shadow root to contain an HTML element with ID 'prev'"
      );
    }

    if (this.$.count.lastElementChild === null) {
      throw new TypeError(
        "Expected loot-message-dialog's shadow root to contain an HTML element with ID 'prev' and a child element"
      );
    }

    this.$.count.lastElementChild.textContent = newValue.length.toString();
    if (newValue.length > 0) {
      this._currentResult = 0;
    }
  }

  public connectedCallback(): void {
    super.connectedCallback();
    this.$.search.addEventListener('input', LootSearchToolbar._onSearch);
    this.$.search.addEventListener('keyup', LootSearchToolbar._onEnter);
    this.$.prev.addEventListener('click', LootSearchToolbar._onPrev);
    this.$.next.addEventListener('click', LootSearchToolbar._onNext);
    this.$.close.addEventListener('click', LootSearchToolbar._onClose);
  }

  public disconnectedCallback(): void {
    super.disconnectedCallback();
    this.$.search.removeEventListener('input', LootSearchToolbar._onSearch);
    this.$.search.removeEventListener('keyup', LootSearchToolbar._onEnter);
    this.$.prev.removeEventListener('click', LootSearchToolbar._onPrev);
    this.$.next.removeEventListener('click', LootSearchToolbar._onNext);
    this.$.close.removeEventListener('change', LootSearchToolbar._onClose);
  }

  private static _onEnter(evt: Event): void {
    if (!isLootSearchToolbarEnterPrevNextEvent(evt)) {
      throw new TypeError(
        "Expected loot-message-dialog's shadow root to contain an HTML element with ID 'prev'"
      );
    }

    const host = evt.target.parentElement.parentNode.host;
    if (evt.keyCode !== 13 || host.results.length === 0) {
      return;
    }
    if (host._currentResult === host.results.length - 1) {
      host._currentResult = 0;
    } else {
      host._currentResult += 1;
    }
  }

  public focusInput(): void {
    if (!isPaperInput(this.$.search)) {
      throw new TypeError(
        "Expected loot-message-dialog's shadow root to contain an HTML element with ID 'prev'"
      );
    }

    this.$.search.focus();
  }

  private _resetResults(): void {
    this.results = [];
    this._currentResult = -1;
    this.$.prev.setAttribute('disabled', '');
    this.$.next.setAttribute('disabled', '');
  }

  public search(): void {
    this.$.search.dispatchEvent(new Event('input'));
  }

  private static _onSearch(evt: Event): void {
    if (!isLootSearchToolbarSearchEvent(evt)) {
      throw new TypeError(
        "Expected loot-message-dialog's search event to come from a subchild"
      );
    }

    evt.target.parentElement.parentNode.host._resetResults();

    const needle = evt.target.value ? evt.target.value.toLowerCase() : '';
    evt.target.dispatchEvent(
      new CustomEvent('loot-search-begin', {
        detail: { needle },
        bubbles: true,
        composed: true
      })
    );
  }

  private static _onPrev(evt: Event): void {
    if (!isLootSearchToolbarEnterPrevNextEvent(evt)) {
      throw new TypeError(
        "Expected loot-message-dialog's search event to come from a subchild"
      );
    }

    evt.target.parentElement.parentNode.host._currentResult -= 1;
  }

  private static _onNext(evt: Event): void {
    if (!isLootSearchToolbarEnterPrevNextEvent(evt)) {
      throw new TypeError(
        "Expected loot-message-dialog's search event to come from a subchild"
      );
    }

    evt.target.parentElement.parentNode.host._currentResult += 1;
  }

  private static _onClose(evt: Event): void {
    if (!isLootSearchToolbarEnterPrevNextEvent(evt)) {
      throw new TypeError(
        "Expected loot-message-dialog's search event to come from a subchild"
      );
    }

    const host = evt.target.parentElement.parentNode.host;

    if (!isPaperInput(host.$.search)) {
      throw new TypeError(
        "Expected loot-message-dialog's shadow root to contain an HTML element with ID 'prev'"
      );
    }

    host._resetResults();
    host.$.search.value = '';
    host.$.count.classList.toggle('hidden', true);
    evt.target.dispatchEvent(
      new CustomEvent('loot-search-end', {
        bubbles: true,
        composed: true
      })
    );
  }

  /* eslint-disable class-methods-use-this */
  // @ts-ignore used in the Polymer template
  private _computeResultNum(currentResult: number): number {
    return currentResult + 1;
  }
  /* eslint-enable class-methods-use-this */
}

customElements.define(LootSearchToolbar.is, LootSearchToolbar);
