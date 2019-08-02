import * as DOM from '../../../../gui/html/js/dom.js';
import State from '../../../../gui/html/js/state';

jest.mock('../../../../gui/html/js/dom.js');

function clearMocks() {
  DOM.enable.mockClear();
  DOM.setUIState.mockClear();
  DOM.show.mockClear();
}

describe('State', () => {
  beforeEach(clearMocks);

  describe('State()', () => {
    test('should set the current state to DEFAULT_STATE', () => {
      const state = new State();
      expect(state.currentState).toBe(State.DEFAULT_STATE);
    });
  });

  describe('DEFAULT_STATE', () => {
    test('should not equal EDITING_STATE', () => {
      expect(State.DEFAULT_STATE).not.toBe(State.EDITING_STATE);
    });

    test('should not equal SORTING_STATE', () => {
      expect(State.DEFAULT_STATE).not.toBe(State.SORTING_STATE);
    });
  });

  describe('EDITING_STATE', () => {
    test('should not equal SORTING_STATE', () => {
      expect(State.SORTING_STATE).not.toBe(State.EDITING_STATE);
    });
  });

  describe('isInDefaultState()', () => {
    test('should return true if the default state is the current state', () => {
      const state = new State();

      expect(state.isInDefaultState()).toBe(true);
    });

    test('should return false if the editing state is the current state', () => {
      const state = new State();
      state.enterEditingState();

      expect(state.isInDefaultState()).toBe(false);
    });

    test('should return false if the sorting state is the current state', () => {
      const state = new State();
      state.enterSortingState();

      expect(state.isInDefaultState()).toBe(false);
    });
  });

  describe('isInEditingState()', () => {
    test('should return true if the editing state is the current state', () => {
      const state = new State();
      state.enterEditingState();

      expect(state.isInEditingState()).toBe(true);
    });

    test('should return false if the default state is the current state', () => {
      const state = new State();

      expect(state.isInEditingState()).toBe(false);
    });

    test('should return false if the sorting state is the current state', () => {
      const state = new State();
      state.enterSortingState();

      expect(state.isInEditingState()).toBe(false);
    });
  });

  describe('isInSortingState()', () => {
    test('should return true if the sorting state is the current state', () => {
      const state = new State();
      state.enterSortingState();

      expect(state.isInSortingState()).toBe(true);
    });

    test('should return false if the editing state is the current state', () => {
      const state = new State();
      state.enterEditingState();

      expect(state.isInSortingState()).toBe(false);
    });

    test('should return false if the default state is the current state', () => {
      const state = new State();

      expect(state.isInSortingState()).toBe(false);
    });
  });

  describe('enterSortingState()', () => {
    test('should change state to the sorting state from the default state', () => {
      const state = new State();
      state.enterSortingState();

      expect(state.currentState).toBe(State.SORTING_STATE);

      expect(DOM.show.mock.calls.length).toBe(4);
      expect(DOM.show.mock.calls[0]).toEqual(['updateMasterlistButton', false]);
      expect(DOM.show.mock.calls[1]).toEqual(['sortButton', false]);
      expect(DOM.show.mock.calls[2]).toEqual(['applySortButton']);
      expect(DOM.show.mock.calls[3]).toEqual(['cancelSortButton']);

      expect(DOM.enable.mock.calls.length).toBe(2);
      expect(DOM.enable.mock.calls[0]).toEqual(['gameMenu', false]);
      expect(DOM.enable.mock.calls[1]).toEqual(['refreshContentButton', false]);

      expect(DOM.setUIState.mock.calls.length).toBe(1);
      expect(DOM.setUIState.mock.calls[0]).toEqual(['sorting']);
    });

    test('should throw an error if called in the editing state', () => {
      const state = new State();
      state.enterEditingState();

      expect(() => {
        state.enterSortingState();
      }).toThrow(Error);
    });

    test('should have no effect if already in the sorting state', () => {
      const state = new State();
      state.enterSortingState();

      clearMocks();

      state.enterSortingState();

      expect(state.currentState).toBe(State.SORTING_STATE);
      expect(DOM.show.mock.calls.length).toBe(0);
      expect(DOM.enable.mock.calls.length).toBe(0);
      expect(DOM.setUIState.mock.calls.length).toBe(0);
    });
  });

  describe('exitSortingState()', () => {
    test('should change state to the default state from the sorting state', () => {
      const state = new State();
      state.enterSortingState();

      clearMocks();

      state.exitSortingState();

      expect(state.currentState).toBe(State.DEFAULT_STATE);

      expect(DOM.show.mock.calls.length).toBe(4);
      expect(DOM.show.mock.calls[0]).toEqual(['updateMasterlistButton']);
      expect(DOM.show.mock.calls[1]).toEqual(['sortButton']);
      expect(DOM.show.mock.calls[2]).toEqual(['applySortButton', false]);
      expect(DOM.show.mock.calls[3]).toEqual(['cancelSortButton', false]);

      expect(DOM.enable.mock.calls.length).toBe(2);
      expect(DOM.enable.mock.calls[0]).toEqual(['gameMenu']);
      expect(DOM.enable.mock.calls[1]).toEqual(['refreshContentButton']);

      expect(DOM.setUIState.mock.calls.length).toBe(1);
      expect(DOM.setUIState.mock.calls[0]).toEqual(['default']);
    });

    test('should throw an error if called in the editing state', () => {
      const state = new State();
      state.enterEditingState();

      expect(() => {
        state.exitSortingState();
      }).toThrow(Error);
    });

    test('should have no effect if already in the default state', () => {
      const state = new State();
      state.exitSortingState();

      expect(state.currentState).toBe(State.DEFAULT_STATE);
      expect(DOM.show.mock.calls.length).toBe(0);
      expect(DOM.enable.mock.calls.length).toBe(0);
      expect(DOM.setUIState.mock.calls.length).toBe(0);
    });
  });

  describe('enterEditingState()', () => {
    test('should change state to the editing state from the default state', () => {
      const state = new State();
      state.enterEditingState();

      expect(state.currentState).toBe(State.EDITING_STATE);

      expect(DOM.enable.mock.calls.length).toBe(7);
      expect(DOM.enable.mock.calls[0]).toEqual(['wipeUserlistButton', false]);
      expect(DOM.enable.mock.calls[1]).toEqual(['copyContentButton', false]);
      expect(DOM.enable.mock.calls[2]).toEqual(['refreshContentButton', false]);
      expect(DOM.enable.mock.calls[3]).toEqual(['settingsButton', false]);
      expect(DOM.enable.mock.calls[4]).toEqual(['gameMenu', false]);
      expect(DOM.enable.mock.calls[5]).toEqual([
        'updateMasterlistButton',
        false
      ]);
      expect(DOM.enable.mock.calls[6]).toEqual(['sortButton', false]);

      expect(DOM.setUIState.mock.calls.length).toBe(1);
      expect(DOM.setUIState.mock.calls[0]).toEqual(['editing']);
    });

    test('should throw an error if called in the sorting state', () => {
      const state = new State();
      state.enterSortingState();

      expect(() => {
        state.enterEditingState();
      }).toThrow(Error);
    });

    test('should have no effect if already in the editing state', () => {
      const state = new State();
      state.enterEditingState();

      clearMocks();

      state.enterEditingState();

      expect(state.currentState).toBe(State.EDITING_STATE);
      expect(DOM.show.mock.calls.length).toBe(0);
      expect(DOM.enable.mock.calls.length).toBe(0);
      expect(DOM.setUIState.mock.calls.length).toBe(0);
    });
  });

  describe('exitEditingState()', () => {
    test('should change state to the default state from the editing state', () => {
      const state = new State();
      state.enterEditingState();

      clearMocks();

      state.exitEditingState();

      expect(state.currentState).toBe(State.DEFAULT_STATE);

      expect(DOM.enable.mock.calls.length).toBe(7);
      expect(DOM.enable.mock.calls[0]).toEqual(['wipeUserlistButton']);
      expect(DOM.enable.mock.calls[1]).toEqual(['copyContentButton']);
      expect(DOM.enable.mock.calls[2]).toEqual(['refreshContentButton']);
      expect(DOM.enable.mock.calls[3]).toEqual(['settingsButton']);
      expect(DOM.enable.mock.calls[4]).toEqual(['gameMenu']);
      expect(DOM.enable.mock.calls[5]).toEqual(['updateMasterlistButton']);
      expect(DOM.enable.mock.calls[6]).toEqual(['sortButton']);

      expect(DOM.setUIState.mock.calls.length).toBe(1);
      expect(DOM.setUIState.mock.calls[0]).toEqual(['default']);
    });

    test('should throw an error if called in the sorting state', () => {
      const state = new State();
      state.enterSortingState();

      expect(() => {
        state.exitEditingState();
      }).toThrow(Error);
    });

    test('should have no effect if already in the default state', () => {
      const state = new State();

      clearMocks();

      state.exitEditingState();

      expect(state.currentState).toBe(State.DEFAULT_STATE);
      expect(DOM.show.mock.calls.length).toBe(0);
      expect(DOM.enable.mock.calls.length).toBe(0);
      expect(DOM.setUIState.mock.calls.length).toBe(0);
    });
  });
});
