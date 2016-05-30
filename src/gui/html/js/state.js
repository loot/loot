'use strict';
(function exportModule(root, factory) {
  if (typeof define === 'function' && define.amd) {
    // AMD. Register as an anonymous module.
    define([], factory);
  } else {
    // Browser globals
    root.loot = root.loot || {};
    root.loot.State = factory(root.loot.DOM);
    root.loot.state = new root.loot.State();
  }
}(this, dom => class State {
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
    dom.show('updateMasterlistButton', false);
    dom.show('sortButton', false);
    dom.show('applySortButton');
    dom.show('cancelSortButton');

    /* Disable changing game. */
    dom.enable('gameMenu', false);
    dom.enable('refreshContentButton', false);

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
    dom.show('updateMasterlistButton');
    dom.show('sortButton');
    dom.show('applySortButton', false);
    dom.show('cancelSortButton', false);

    /* Enable changing game. */
    dom.enable('gameMenu');
    dom.enable('refreshContentButton');

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
    dom.enable('wipeUserlistButton', false);
    dom.enable('copyContentButton', false);
    dom.enable('refreshContentButton', false);
    dom.enable('settingsButton', false);
    dom.enable('gameMenu', false);
    dom.enable('updateMasterlistButton', false);
    dom.enable('sortButton', false);

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
    dom.enable('wipeUserlistButton');
    dom.enable('copyContentButton');
    dom.enable('refreshContentButton');
    dom.enable('settingsButton');
    dom.enable('gameMenu');
    dom.enable('updateMasterlistButton');
    dom.enable('sortButton');

    this.currentState = State.DEFAULT_STATE;
  }
}));
