import { show, enable, setUIState } from './dom.js';

export default class State {
  static get DEFAULT_STATE() {
    return 0;
  }

  static get SORTING_STATE() {
    return 1;
  }

  static get EDITING_STATE() {
    return 2;
  }

  constructor() {
    this.currentState = State.DEFAULT_STATE;
  }

  isInDefaultState() {
    return this.currentState === State.DEFAULT_STATE;
  }

  isInEditingState() {
    return this.currentState === State.EDITING_STATE;
  }

  isInSortingState() {
    return this.currentState === State.SORTING_STATE;
  }

  enterSortingState() {
    if (this.isInSortingState()) {
      return;
    }
    if (this.isInEditingState()) {
      throw new Error('Cannot enter sorting state from editing state');
    }

    /* Hide the masterlist update buttons, and display the accept and
    cancel sort buttons. */
    show('updateMasterlistButton', false);
    show('sortButton', false);
    show('applySortButton');
    show('cancelSortButton');

    /* Disable changing game. */
    enable('gameMenu', false);
    enable('refreshContentButton', false);

    setUIState('sorting');

    this.currentState = State.SORTING_STATE;
  }

  exitSortingState() {
    if (this.isInDefaultState()) {
      return;
    }
    if (this.isInEditingState()) {
      throw new Error('Cannot exit sorting state from editing state');
    }

    /* Show the masterlist update buttons, and hide the accept and
    cancel sort buttons. */
    show('updateMasterlistButton');
    show('sortButton');
    show('applySortButton', false);
    show('cancelSortButton', false);

    /* Enable changing game. */
    enable('gameMenu');
    enable('refreshContentButton');

    setUIState('default');

    this.currentState = State.DEFAULT_STATE;
  }

  enterEditingState() {
    if (this.isInEditingState()) {
      return;
    }
    if (this.isInSortingState()) {
      throw new Error('Cannot enter editing state from sorting state');
    }

    /* Disable the toolbar elements. */
    enable('wipeUserlistButton', false);
    enable('copyContentButton', false);
    enable('refreshContentButton', false);
    enable('settingsButton', false);
    enable('gameMenu', false);
    enable('updateMasterlistButton', false);
    enable('sortButton', false);

    setUIState('editing');

    this.currentState = State.EDITING_STATE;
  }

  exitEditingState() {
    if (this.isInDefaultState()) {
      return;
    }
    if (this.isInSortingState()) {
      throw new Error('Cannot exit editing state from sorting state');
    }

    /* Re-enable toolbar elements. */
    enable('wipeUserlistButton');
    enable('copyContentButton');
    enable('refreshContentButton');
    enable('settingsButton');
    enable('gameMenu');
    enable('updateMasterlistButton');
    enable('sortButton');

    setUIState('default');

    this.currentState = State.DEFAULT_STATE;
  }
}
