import { mocked } from 'ts-jest/utils';
import * as DOM from '../../../../gui/html/js/dom';
import State from '../../../../gui/html/js/state';

jest.mock('../../../../gui/html/js/dom');

function clearMocks(): void {
  mocked(DOM.enable).mockClear();
  mocked(DOM.setUIState).mockClear();
  mocked(DOM.show).mockClear();
}

describe('State', () => {
  beforeEach(clearMocks);

  describe('enterSortingState()', () => {
    test('should change state to the sorting state from the default state', () => {
      const state = new State();

      expect(state.isInDefaultState()).toBe(true);
      expect(state.isInEditingState()).toBe(false);
      expect(state.isInSortingState()).toBe(false);

      state.enterSortingState();

      expect(state.isInDefaultState()).toBe(false);
      expect(state.isInEditingState()).toBe(false);
      expect(state.isInSortingState()).toBe(true);

      expect(mocked(DOM.show).mock.calls.length).toBe(4);
      expect(mocked(DOM.show).mock.calls[0]).toEqual([
        'updateMasterlistButton',
        false
      ]);
      expect(mocked(DOM.show).mock.calls[1]).toEqual(['sortButton', false]);
      expect(mocked(DOM.show).mock.calls[2]).toEqual(['applySortButton']);
      expect(mocked(DOM.show).mock.calls[3]).toEqual(['cancelSortButton']);

      expect(mocked(DOM.enable).mock.calls.length).toBe(2);
      expect(mocked(DOM.enable).mock.calls[0]).toEqual(['gameMenu', false]);
      expect(mocked(DOM.enable).mock.calls[1]).toEqual([
        'refreshContentButton',
        false
      ]);

      expect(mocked(DOM.setUIState).mock.calls.length).toBe(1);
      expect(mocked(DOM.setUIState).mock.calls[0]).toEqual(['sorting']);
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

      expect(state.isInDefaultState()).toBe(false);
      expect(state.isInEditingState()).toBe(false);
      expect(state.isInSortingState()).toBe(true);

      clearMocks();

      state.enterSortingState();

      expect(state.isInDefaultState()).toBe(false);
      expect(state.isInEditingState()).toBe(false);
      expect(state.isInSortingState()).toBe(true);

      expect(mocked(DOM.show).mock.calls.length).toBe(0);
      expect(mocked(DOM.enable).mock.calls.length).toBe(0);
      expect(mocked(DOM.setUIState).mock.calls.length).toBe(0);
    });
  });

  describe('exitSortingState()', () => {
    test('should change state to the default state from the sorting state', () => {
      const state = new State();
      state.enterSortingState();

      clearMocks();

      expect(state.isInDefaultState()).toBe(false);
      expect(state.isInEditingState()).toBe(false);
      expect(state.isInSortingState()).toBe(true);

      state.exitSortingState();

      expect(state.isInDefaultState()).toBe(true);
      expect(state.isInEditingState()).toBe(false);
      expect(state.isInSortingState()).toBe(false);

      expect(mocked(DOM.show).mock.calls.length).toBe(4);
      expect(mocked(DOM.show).mock.calls[0]).toEqual([
        'updateMasterlistButton'
      ]);
      expect(mocked(DOM.show).mock.calls[1]).toEqual(['sortButton']);
      expect(mocked(DOM.show).mock.calls[2]).toEqual([
        'applySortButton',
        false
      ]);
      expect(mocked(DOM.show).mock.calls[3]).toEqual([
        'cancelSortButton',
        false
      ]);

      expect(mocked(DOM.enable).mock.calls.length).toBe(2);
      expect(mocked(DOM.enable).mock.calls[0]).toEqual(['gameMenu']);
      expect(mocked(DOM.enable).mock.calls[1]).toEqual([
        'refreshContentButton'
      ]);

      expect(mocked(DOM.setUIState).mock.calls.length).toBe(1);
      expect(mocked(DOM.setUIState).mock.calls[0]).toEqual(['default']);
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

      expect(mocked(DOM.show).mock.calls.length).toBe(0);
      expect(mocked(DOM.enable).mock.calls.length).toBe(0);
      expect(mocked(DOM.setUIState).mock.calls.length).toBe(0);
    });
  });

  describe('enterEditingState()', () => {
    test('should change state to the editing state from the default state', () => {
      const state = new State();

      expect(state.isInDefaultState()).toBe(true);
      expect(state.isInEditingState()).toBe(false);
      expect(state.isInSortingState()).toBe(false);

      state.enterEditingState();

      expect(state.isInDefaultState()).toBe(false);
      expect(state.isInEditingState()).toBe(true);
      expect(state.isInSortingState()).toBe(false);

      expect(mocked(DOM.enable).mock.calls.length).toBe(7);
      expect(mocked(DOM.enable).mock.calls[0]).toEqual([
        'wipeUserlistButton',
        false
      ]);
      expect(mocked(DOM.enable).mock.calls[1]).toEqual([
        'copyContentButton',
        false
      ]);
      expect(mocked(DOM.enable).mock.calls[2]).toEqual([
        'refreshContentButton',
        false
      ]);
      expect(mocked(DOM.enable).mock.calls[3]).toEqual([
        'settingsButton',
        false
      ]);
      expect(mocked(DOM.enable).mock.calls[4]).toEqual(['gameMenu', false]);
      expect(mocked(DOM.enable).mock.calls[5]).toEqual([
        'updateMasterlistButton',
        false
      ]);
      expect(mocked(DOM.enable).mock.calls[6]).toEqual(['sortButton', false]);

      expect(mocked(DOM.setUIState).mock.calls.length).toBe(1);
      expect(mocked(DOM.setUIState).mock.calls[0]).toEqual(['editing']);
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

      expect(state.isInDefaultState()).toBe(false);
      expect(state.isInEditingState()).toBe(true);
      expect(state.isInSortingState()).toBe(false);

      state.enterEditingState();

      expect(state.isInDefaultState()).toBe(false);
      expect(state.isInEditingState()).toBe(true);
      expect(state.isInSortingState()).toBe(false);

      expect(mocked(DOM.show).mock.calls.length).toBe(0);
      expect(mocked(DOM.enable).mock.calls.length).toBe(0);
      expect(mocked(DOM.setUIState).mock.calls.length).toBe(0);
    });
  });

  describe('exitEditingState()', () => {
    test('should change state to the default state from the editing state', () => {
      const state = new State();
      state.enterEditingState();

      clearMocks();

      expect(state.isInDefaultState()).toBe(false);
      expect(state.isInEditingState()).toBe(true);
      expect(state.isInSortingState()).toBe(false);

      state.exitEditingState();

      expect(state.isInDefaultState()).toBe(true);
      expect(state.isInEditingState()).toBe(false);
      expect(state.isInSortingState()).toBe(false);

      expect(mocked(DOM.enable).mock.calls.length).toBe(7);
      expect(mocked(DOM.enable).mock.calls[0]).toEqual(['wipeUserlistButton']);
      expect(mocked(DOM.enable).mock.calls[1]).toEqual(['copyContentButton']);
      expect(mocked(DOM.enable).mock.calls[2]).toEqual([
        'refreshContentButton'
      ]);
      expect(mocked(DOM.enable).mock.calls[3]).toEqual(['settingsButton']);
      expect(mocked(DOM.enable).mock.calls[4]).toEqual(['gameMenu']);
      expect(mocked(DOM.enable).mock.calls[5]).toEqual([
        'updateMasterlistButton'
      ]);
      expect(mocked(DOM.enable).mock.calls[6]).toEqual(['sortButton']);

      expect(mocked(DOM.setUIState).mock.calls.length).toBe(1);
      expect(mocked(DOM.setUIState).mock.calls[0]).toEqual(['default']);
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

      expect(state.isInDefaultState()).toBe(true);
      expect(state.isInEditingState()).toBe(false);
      expect(state.isInSortingState()).toBe(false);

      state.exitEditingState();

      expect(state.isInDefaultState()).toBe(true);
      expect(state.isInEditingState()).toBe(false);
      expect(state.isInSortingState()).toBe(false);

      expect(mocked(DOM.show).mock.calls.length).toBe(0);
      expect(mocked(DOM.enable).mock.calls.length).toBe(0);
      expect(mocked(DOM.setUIState).mock.calls.length).toBe(0);
    });
  });
});
