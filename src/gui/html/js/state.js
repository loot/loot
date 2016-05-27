'use strict';
(function exportModule(root, factory) {
  if (typeof define === 'function' && define.amd) {
    // AMD. Register as an anonymous module.
    define([], factory);
  } else {
    // Browser globals
    root.loot = root.loot || {};
    root.loot.State = factory(root.loot.dom);
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
      loot.dom.show('updateMasterlistButton', false);
      loot.dom.show('sortButton', false);
      loot.dom.show('applySortButton');
      loot.dom.show('cancelSortButton');

      /* Disable changing game. */
      loot.dom.enable('gameMenu', false);

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
      loot.dom.show('updateMasterlistButton');
      loot.dom.show('sortButton');
      loot.dom.show('applySortButton', false);
      loot.dom.show('cancelSortButton', false);

      /* Enable changing game. */
      loot.dom.enable('gameMenu');

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
      loot.dom.enable('wipeUserlistButton', false);
      loot.dom.enable('copyContentButton', false);
      loot.dom.enable('refreshContentButton', false);
      loot.dom.enable('settingsButton', false);
      loot.dom.enable('gameMenu', false);
      loot.dom.enable('updateMasterlistButton', false);
      loot.dom.enable('sortButton', false);

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
      loot.dom.enable('wipeUserlistButton');
      loot.dom.enable('copyContentButton');
      loot.dom.enable('refreshContentButton');
      loot.dom.enable('settingsButton');
      loot.dom.enable('gameMenu');
      loot.dom.enable('updateMasterlistButton');
      loot.dom.enable('sortButton');

      this.currentState = State.DEFAULT_STATE;
    }
  };
}));
