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
}(this, (dom) => {
  return class State {

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
      loot.DOM.show('updateMasterlistButton', false);
      loot.DOM.show('sortButton', false);
      loot.DOM.show('applySortButton');
      loot.DOM.show('cancelSortButton');

      /* Disable changing game. */
      loot.DOM.enable('gameMenu', false);
      loot.DOM.enable('refreshContentButton', false);

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
      loot.DOM.show('updateMasterlistButton');
      loot.DOM.show('sortButton');
      loot.DOM.show('applySortButton', false);
      loot.DOM.show('cancelSortButton', false);

      /* Enable changing game. */
      loot.DOM.enable('gameMenu');
      loot.DOM.enable('refreshContentButton');

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
      loot.DOM.enable('wipeUserlistButton', false);
      loot.DOM.enable('copyContentButton', false);
      loot.DOM.enable('refreshContentButton', false);
      loot.DOM.enable('settingsButton', false);
      loot.DOM.enable('gameMenu', false);
      loot.DOM.enable('updateMasterlistButton', false);
      loot.DOM.enable('sortButton', false);

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
      loot.DOM.enable('wipeUserlistButton');
      loot.DOM.enable('copyContentButton');
      loot.DOM.enable('refreshContentButton');
      loot.DOM.enable('settingsButton');
      loot.DOM.enable('gameMenu');
      loot.DOM.enable('updateMasterlistButton');
      loot.DOM.enable('sortButton');

      this.currentState = State.DEFAULT_STATE;
    }
  };
}));
