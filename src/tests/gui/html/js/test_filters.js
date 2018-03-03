/* eslint-disable no-unused-expressions */

describe('Filters', () => {
  /* Mock the Translator class. */

  class Translator {
    /* eslint-disable class-methods-use-this */
    translate(text) {
      return text;
    }
    /* eslint-enable class-methods-use-this */
  }

  let l10n;

  beforeEach(() => {
    l10n = new Translator();
  });

  describe('#constructor()', () => {
    it('should throw if no parameter is passed', () => {
      (() => {
        new loot.Filters(); // eslint-disable-line no-new
      }).should.throw();
    });

    it('should throw if an empty object is passed', () => {
      (() => {
        new loot.Filters({}); // eslint-disable-line no-new
      }).should.throw();
    });

    it('should not throw if a valid Translator object is passed', () => {
      (() => {
        new loot.Filters(l10n); // eslint-disable-line no-new
      }).should.not.throw();
    });

    it('should initialise filters as not enabled', () => {
      const filters = new loot.Filters(l10n);

      filters.hideMessagelessPlugins.should.be.false;
      filters.hideInactivePlugins.should.be.false;
      filters.conflictingPluginNames.should.deep.equal([]);
      filters.contentSearchString.should.equal('');

      filters.hideVersionNumbers.should.be.false;
      filters.hideCRCs.should.be.false;
      filters.hideBashTags.should.be.false;
      filters.hideAllPluginMessages.should.be.false;
      filters.hideNotes.should.be.false;
      filters.hideDoNotCleanMessages.should.be.false;
    });

    it('should initialise "do not clean" search string', () => {
      const filters = new loot.Filters(l10n);

      filters._doNotCleanString.should.equal('do not clean');
    });
  });

  describe('#pluginFilter()', () => {
    let filters;
    let plugin;

    /* Mock the PluginCardContent class */
    class PluginCardContent {
      constructor(pluginObj, filtersObj) {
        this._messages = pluginObj.messages;
        this._hideMessages = filtersObj.hideAllPluginMessages;
      }

      get messages() {
        if (this._hideMessages) {
          return [];
        }
        return this._messages;
      }

      /* eslint-disable class-methods-use-this */
      containsText(text) {
        return text === 'found text';
      }
      /* eslint-enable class-methods-use-this */
    }

    /* Mock the Plugin class */
    class Plugin {
      constructor() {
        this.name = 'test';
        this.isActive = false;
        this.messages = [];
      }

      getCardContent(filtersObj) {
        return new PluginCardContent(this, filtersObj);
      }
    }

    beforeEach(() => {
      filters = new loot.Filters(l10n);
      plugin = new Plugin();
    });

    it('should return true if no filters are enabled', () => {
      filters.pluginFilter(plugin).should.be.true;
    });

    it('should return false if inactive plugins filter is enabled', () => {
      filters.hideInactivePlugins = true;
      filters.pluginFilter(plugin).should.be.false;
    });

    it('should return true if inactive plugins filter is enabled and plugin is active', () => {
      filters.hideInactivePlugins = true;
      plugin.isActive = true;
      filters.pluginFilter(plugin).should.be.true;
    });

    it('should return false if messageless plugins filter is enabled', () => {
      filters.hideMessagelessPlugins = true;
      filters.pluginFilter(plugin).should.be.false;
    });

    it('should return true if messageless plugins filter is enabled and plugin has a non-zero message array', () => {
      filters.hideMessagelessPlugins = true;
      plugin.messages = [0];
      filters.pluginFilter(plugin).should.be.true;
    });

    it('should return false if all plugin messages and messageless plugin filters are enabled and plugin has a non-zero message array', () => {
      filters.hideAllPluginMessages = true;
      filters.hideMessagelessPlugins = true;
      plugin.messages = [0];
      filters.pluginFilter(plugin).should.be.false;
    });

    it('should return false if conflicting plugins filter is enabled', () => {
      filters.conflictingPluginNames = ['conflicting plugin'];
      filters.pluginFilter(plugin).should.be.false;
    });

    it('should return true if conflicting plugins filter is enabled and plugin name is in the conflicting plugins array', () => {
      filters.conflictingPluginNames = ['conflicting plugin', plugin.name];
      filters.pluginFilter(plugin).should.be.true;
    });

    it('should return false if plugin content filter is enabled', () => {
      filters.contentSearchString = 'unfound text';
      filters.pluginFilter(plugin).should.be.false;
    });

    it('should return true if plugin content filter is enabled and plugin contains the filter text', () => {
      filters.contentSearchString = 'found text';
      filters.pluginFilter(plugin).should.be.true;
    });
  });

  describe('#messageFilter()', () => {
    let filters;
    let note;
    let doNotCleanMessage;

    beforeEach(() => {
      filters = new loot.Filters(l10n);
      note = {
        type: 'say',
        text: 'test message'
      };
      doNotCleanMessage = {
        type: 'warn',
        text: 'do not clean'
      };
    });

    it('should return true for a note message when no filters are enabled', () => {
      filters.messageFilter(note).should.be.true;
    });

    it('should return true for a warning "do not clean" message when no filters are enabled', () => {
      filters.messageFilter(doNotCleanMessage).should.be.true;
    });

    it('should return false for a note message when the notes filter is enabled', () => {
      filters.hideNotes = true;
      filters.messageFilter(note).should.be.false;
    });

    it('should return true for a warning message when the notes filter is enabled', () => {
      filters.hideNotes = true;
      filters.messageFilter(doNotCleanMessage).should.be.true;
    });

    it('should return false for a "do not clean" message when the "do not clean" messages filter is enabled', () => {
      filters.hideDoNotCleanMessages = true;
      filters.messageFilter(doNotCleanMessage).should.be.false;
    });

    it('should return true for a message not containing "do not clean" when the "do not clean" messages filter is enabled', () => {
      filters.hideDoNotCleanMessages = true;
      filters.messageFilter(note).should.be.true;
    });

    it('should return false for a note message when the all messages filter is enabled', () => {
      filters.hideAllPluginMessages = true;
      filters.messageFilter(note).should.be.false;
    });

    it('should return false for a "do not clean" message when the all messages filter is enabled', () => {
      filters.hideAllPluginMessages = true;
      filters.messageFilter(doNotCleanMessage).should.be.false;
    });
  });

  describe('#deactivateConflictsFilter', () => {
    let filters;
    let handleEvent;

    beforeEach(() => {
      filters = new loot.Filters(l10n);
    });

    afterEach(() => {
      document.removeEventListener(
        'loot-filter-conflicts-deactivate',
        handleEvent
      );
    });

    it('should return false if the conflicts filter was not active', () => {
      filters.deactivateConflictsFilter().should.be.false;
    });

    it('should return true if the conflicts filter was active', () => {
      filters.conflictingPluginNames = ['Skyrim.esm'];

      filters.deactivateConflictsFilter().should.be.true;
    });

    it('should empty the conflicting plugin names array', () => {
      filters.conflictingPluginNames = ['Skyrim.esm'];

      filters.deactivateConflictsFilter().should.be.true;
      filters.conflictingPluginNames.should.have.length(0);
    });

    it('should fire an event', done => {
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
    let filters;

    beforeEach(() => {
      filters = new loot.Filters(l10n);
    });

    it('should return a promise that resolves to an empty array if the argument is falsy', () => {
      filters.activateConflictsFilter().then(result => {
        result.should.be.an('array');
        return result.should.be.empty;
      });
    });
  });

  describe('#areAnyFiltersActive', () => {
    let filters;

    beforeEach(() => {
      filters = new loot.Filters(l10n);
    });

    it('should return false if all the boolean filters are false, and the content and conflict filter lengths are zero', () => {
      filters.areAnyFiltersActive().should.be.false;
    });

    it('should return true if hideMessagelessPlugins is true', () => {
      filters.hideMessagelessPlugins = true;

      filters.areAnyFiltersActive().should.be.true;
    });

    it('should return true if hideInactivePlugins is true', () => {
      filters.hideInactivePlugins = true;

      filters.areAnyFiltersActive().should.be.true;
    });

    it('should return true if hideVersionNumbers is true', () => {
      filters.hideVersionNumbers = true;

      filters.areAnyFiltersActive().should.be.true;
    });

    it('should return true if hideCRCs is true', () => {
      filters.hideCRCs = true;

      filters.areAnyFiltersActive().should.be.true;
    });

    it('should return true if hideBashTags is true', () => {
      filters.hideBashTags = true;

      filters.areAnyFiltersActive().should.be.true;
    });

    it('should return true if hideAllPluginMessages is true', () => {
      filters.hideAllPluginMessages = true;

      filters.areAnyFiltersActive().should.be.true;
    });

    it('should return true if hideNotes is true', () => {
      filters.hideNotes = true;

      filters.areAnyFiltersActive().should.be.true;
    });

    it('should return true if hideDoNotCleanMessages is true', () => {
      filters.hideDoNotCleanMessages = true;

      filters.areAnyFiltersActive().should.be.true;
    });

    it('should return true if conflictingPluginNames is a non-empty array', () => {
      filters.conflictingPluginNames = ['foo'];

      filters.areAnyFiltersActive().should.be.true;
    });

    it('should return true if contentSearchString is a non-empty string', () => {
      filters.contentSearchString = 'foo';

      filters.areAnyFiltersActive().should.be.true;
    });
  });
});
