import Filters from '../../../../gui/html/js/filters';
import Translator from '../../../../gui/html/js/translator';
import { SimpleMessage } from '../../../../gui/html/js/interfaces';
import { Plugin } from '../../../../gui/html/js/plugin';

jest.mock('../../../../gui/html/js/handlePromiseError');
jest.mock('../../../../gui/html/js/query');

describe('Filters', () => {
  const l10n = new Translator();

  describe('#constructor()', () => {
    test('should not throw if a valid Translator object is passed', () => {
      expect(() => {
        new Filters(l10n); // eslint-disable-line no-new
      }).not.toThrow();
    });

    test('should initialise filters as not enabled', () => {
      const filters = new Filters(l10n);

      expect(filters.hideMessagelessPlugins).toBe(false);
      expect(filters.hideInactivePlugins).toBe(false);
      expect(filters.conflictingPluginNames).toEqual([]);
      expect(filters.contentSearchString).toBe('');

      expect(filters.hideVersionNumbers).toBe(false);
      expect(filters.hideCRCs).toBe(false);
      expect(filters.hideBashTags).toBe(false);
      expect(filters.hideAllPluginMessages).toBe(false);
      expect(filters.hideNotes).toBe(false);
      expect(filters.hideDoNotCleanMessages).toBe(false);
    });
  });

  describe('#pluginFilter()', () => {
    let filters: Filters;
    let plugin: Plugin;

    beforeEach(() => {
      filters = new Filters(l10n);
      plugin = new Plugin({
        name: 'found text',
        isActive: false,
        isDirty: false,
        isEmpty: false,
        isMaster: false,
        isLightMaster: false,
        loadsArchive: false,
        messages: [],
        suggestedTags: [],
        currentTags: []
      });
    });

    test('should return true if no filters are enabled', () => {
      expect(filters.pluginFilter(plugin)).toBe(true);
    });

    test('should return false if inactive plugins filter is enabled', () => {
      filters.hideInactivePlugins = true;
      expect(filters.pluginFilter(plugin)).toBe(false);
    });

    test('should return true if inactive plugins filter is enabled and plugin is active', () => {
      filters.hideInactivePlugins = true;
      plugin.isActive = true;
      expect(filters.pluginFilter(plugin)).toBe(true);
    });

    test('should return false if messageless plugins filter is enabled', () => {
      filters.hideMessagelessPlugins = true;
      expect(filters.pluginFilter(plugin)).toBe(false);
    });

    test('should return true if messageless plugins filter is enabled and plugin has a non-zero message array', () => {
      filters.hideMessagelessPlugins = true;
      plugin.messages = [
        {
          type: 'say',
          text: 'test',
          language: 'en',
          condition: ''
        }
      ];
      expect(filters.pluginFilter(plugin)).toBe(true);
    });

    test('should return false if all plugin messages and messageless plugin filters are enabled and plugin has a non-zero message array', () => {
      filters.hideAllPluginMessages = true;
      filters.hideMessagelessPlugins = true;
      plugin.messages = [
        {
          type: 'say',
          text: 'test',
          language: 'en',
          condition: ''
        }
      ];
      expect(filters.pluginFilter(plugin)).toBe(false);
    });

    test('should return false if conflicting plugins filter is enabled', () => {
      filters.conflictingPluginNames = ['conflicting plugin'];
      expect(filters.pluginFilter(plugin)).toBe(false);
    });

    test('should return true if conflicting plugins filter is enabled and plugin name is in the conflicting plugins array', () => {
      filters.conflictingPluginNames = ['conflicting plugin', plugin.name];
      expect(filters.pluginFilter(plugin)).toBe(true);
    });

    test('should return false if plugin content filter is enabled', () => {
      filters.contentSearchString = 'unfound text';
      expect(filters.pluginFilter(plugin)).toBe(false);
    });

    test('should return true if plugin content filter is enabled and plugin contains the filter text', () => {
      filters.contentSearchString = 'found text';
      expect(filters.pluginFilter(plugin)).toBe(true);
    });
  });

  describe('#messageFilter()', () => {
    let filters: Filters;
    let note: SimpleMessage;
    let doNotCleanMessage: SimpleMessage;

    beforeEach(() => {
      filters = new Filters(l10n);
      note = {
        type: 'say',
        text: 'test message',
        language: 'en',
        condition: ''
      };
      doNotCleanMessage = {
        type: 'warn',
        text: 'do not clean',
        language: 'en',
        condition: ''
      };
    });

    test('should return true for a note message when no filters are enabled', () => {
      expect(filters.messageFilter(note)).toBe(true);
    });

    test('should return true for a warning "do not clean" message when no filters are enabled', () => {
      expect(filters.messageFilter(doNotCleanMessage)).toBe(true);
    });

    test('should return false for a note message when the notes filter is enabled', () => {
      filters.hideNotes = true;
      expect(filters.messageFilter(note)).toBe(false);
    });

    test('should return true for a warning message when the notes filter is enabled', () => {
      filters.hideNotes = true;
      expect(filters.messageFilter(doNotCleanMessage)).toBe(true);
    });

    test('should return false for a "do not clean" message when the "do not clean" messages filter is enabled', () => {
      filters.hideDoNotCleanMessages = true;
      expect(filters.messageFilter(doNotCleanMessage)).toBe(false);
    });

    test('should return true for a message not containing "do not clean" when the "do not clean" messages filter is enabled', () => {
      filters.hideDoNotCleanMessages = true;
      expect(filters.messageFilter(note)).toBe(true);
    });

    test('should return false for a note message when the all messages filter is enabled', () => {
      filters.hideAllPluginMessages = true;
      expect(filters.messageFilter(note)).toBe(false);
    });

    test('should return false for a "do not clean" message when the all messages filter is enabled', () => {
      filters.hideAllPluginMessages = true;
      expect(filters.messageFilter(doNotCleanMessage)).toBe(false);
    });
  });

  describe('#deactivateConflictsFilter', () => {
    let filters: Filters;
    let handleEvent: () => void;

    beforeEach(() => {
      filters = new Filters(l10n);
    });

    afterEach(() => {
      document.removeEventListener(
        'loot-filter-conflicts-deactivate',
        handleEvent
      );
    });

    test('should return false if the conflicts filter was not active', () => {
      expect(filters.deactivateConflictsFilter()).toBe(false);
    });

    test('should return true if the conflicts filter was active', () => {
      filters.conflictingPluginNames = ['Skyrim.esm'];

      expect(filters.deactivateConflictsFilter()).toBe(true);
    });

    test('should empty the conflicting plugin names array', () => {
      filters.conflictingPluginNames = ['Skyrim.esm'];

      expect(filters.deactivateConflictsFilter()).toBe(true);
      expect(filters.conflictingPluginNames.length).toBe(0);
    });

    test('should fire an event', done => {
      handleEvent = () => {
        done();
      };

      document.addEventListener(
        'loot-filter-conflicts-deactivate',
        handleEvent
      );

      filters.deactivateConflictsFilter();
    });
  });

  describe('#activateConflictsFilter', () => {
    let filters: Filters;

    beforeEach(() => {
      filters = new Filters(l10n);
    });

    test('should return a promise that resolves to an object with empty arrays if the argument is falsy', () =>
      filters.activateConflictsFilter('').then(result => {
        expect(result.generalMessages.length).toBe(0);
        expect(result.plugins.length).toBe(0);
      }));
  });

  describe('#areAnyFiltersActive', () => {
    let filters: Filters;

    beforeEach(() => {
      filters = new Filters(l10n);
    });

    test('should return false if all the boolean filters are false, and the content and conflict filter lengths are zero', () => {
      expect(filters.areAnyFiltersActive()).toBe(false);
    });

    test('should return true if hideMessagelessPlugins is true', () => {
      filters.hideMessagelessPlugins = true;

      expect(filters.areAnyFiltersActive()).toBe(true);
    });

    test('should return true if hideInactivePlugins is true', () => {
      filters.hideInactivePlugins = true;

      expect(filters.areAnyFiltersActive()).toBe(true);
    });

    test('should return true if hideVersionNumbers is true', () => {
      filters.hideVersionNumbers = true;

      expect(filters.areAnyFiltersActive()).toBe(true);
    });

    test('should return true if hideCRCs is true', () => {
      filters.hideCRCs = true;

      expect(filters.areAnyFiltersActive()).toBe(true);
    });

    test('should return true if hideBashTags is true', () => {
      filters.hideBashTags = true;

      expect(filters.areAnyFiltersActive()).toBe(true);
    });

    test('should return true if hideAllPluginMessages is true', () => {
      filters.hideAllPluginMessages = true;

      expect(filters.areAnyFiltersActive()).toBe(true);
    });

    test('should return true if hideNotes is true', () => {
      filters.hideNotes = true;

      expect(filters.areAnyFiltersActive()).toBe(true);
    });

    test('should return true if hideDoNotCleanMessages is true', () => {
      filters.hideDoNotCleanMessages = true;

      expect(filters.areAnyFiltersActive()).toBe(true);
    });

    test('should return true if conflictingPluginNames is a non-empty array', () => {
      filters.conflictingPluginNames = ['foo'];

      expect(filters.areAnyFiltersActive()).toBe(true);
    });

    test('should return true if contentSearchString is a non-empty string', () => {
      filters.contentSearchString = 'foo';

      expect(filters.areAnyFiltersActive()).toBe(true);
    });
  });
});
