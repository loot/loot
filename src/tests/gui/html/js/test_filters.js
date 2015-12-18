'use strict';

describe('Filters', () => {
  /* Mock the Translator class. */
  class Translator {
    translate(text) {
      return text;
    }
  }

  let l10n;

  beforeEach(() => {
    l10n = new Translator();
  });

  describe('#constructor()', () => {
    it('should throw if no parameter is passed', () => {
      (() => { new loot.Filters(); }).should.throw(); // eslint-disable-line no-new
    });

    it('should throw if an empty object is passed', () => {
      (() => { new loot.Filters({}); }).should.throw(); // eslint-disable-line no-new
    });

    it('should not throw if a valid Translator object is passed', () => {
      (() => { new loot.Filters(l10n); }).should.not.throw(); // eslint-disable-line no-new
    });

    it('should initialise filters as not enabled', () => {
      const filters = new loot.Filters(l10n);

      filters.hideMessagelessPlugins.should.be.false();
      filters.hideInactivePlugins.should.be.false();
      filters.conflictingPluginNames.should.deepEqual([]);
      filters.contentSearchString.should.equal('');

      filters.hideVersionNumbers.should.be.false();
      filters.hideCRCs.should.be.false();
      filters.hideBashTags.should.be.false();
      filters.hideAllPluginMessages.should.be.false();
      filters.hideNotes.should.be.false();
      filters.hideDoNotCleanMessages.should.be.false();
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

      containsText(text) {
        return text === 'found text';
      }
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
      filters.pluginFilter(plugin).should.be.true();
    });

    it('should return false if inactive plugins filter is enabled', () => {
      filters.hideInactivePlugins = true;
      filters.pluginFilter(plugin).should.be.false();
    });

    it('should return true if inactive plugins filter is enabled and plugin is active', () => {
      filters.hideInactivePlugins = true;
      plugin.isActive = true;
      filters.pluginFilter(plugin).should.be.true();
    });

    it('should return false if messageless plugins filter is enabled', () => {
      filters.hideMessagelessPlugins = true;
      filters.pluginFilter(plugin).should.be.false();
    });

    it('should return true if messageless plugins filter is enabled and plugin has a non-zero message array', () => {
      filters.hideMessagelessPlugins = true;
      plugin.messages = [0];
      filters.pluginFilter(plugin).should.be.true();
    });

    it('should return false if all plugin messages and messageless plugin filters are enabled and plugin has a non-zero message array', () => {
      filters.hideAllPluginMessages = true;
      filters.hideMessagelessPlugins = true;
      plugin.messages = [0];
      filters.pluginFilter(plugin).should.be.false();
    });

    it('should return false if conflicting plugins filter is enabled', () => {
      filters.conflictingPluginNames = ['conflicting plugin'];
      filters.pluginFilter(plugin).should.be.false();
    });

    it('should return true if conflicting plugins filter is enabled and plugin name is in the conflicting plugins array', () => {
      filters.conflictingPluginNames = [
        'conflicting plugin',
        plugin.name,
      ];
      filters.pluginFilter(plugin).should.be.true();
    });

    it('should return false if plugin content filter is enabled', () => {
      filters.contentSearchString = 'unfound text';
      filters.pluginFilter(plugin).should.be.false();
    });

    it('should return true if plugin content filter is enabled and plugin contains the filter text', () => {
      filters.contentSearchString = 'found text';
      filters.pluginFilter(plugin).should.be.true();
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
        content: 'test message',
      };
      doNotCleanMessage = {
        type: 'warn',
        content: 'do not clean',
      };
    });

    it('should return true for a note message when no filters are enabled', () => {
      filters.messageFilter(note).should.be.true();
    });

    it('should return true for a warning "do not clean" message when no filters are enabled', () => {
      filters.messageFilter(doNotCleanMessage).should.be.true();
    });

    it('should return false for a note message when the notes filter is enabled', () => {
      filters.hideNotes = true;
      filters.messageFilter(note).should.be.false();
    });

    it('should return true for a warning message when the notes filter is enabled', () => {
      filters.hideNotes = true;
      filters.messageFilter(doNotCleanMessage).should.be.true();
    });

    it('should return false for a "do not clean" message when the "do not clean" messages filter is enabled', () => {
      filters.hideDoNotCleanMessages = true;
      filters.messageFilter(doNotCleanMessage).should.be.false();
    });

    it('should return true for a message not containing "do not clean" when the "do not clean" messages filter is enabled', () => {
      filters.hideDoNotCleanMessages = true;
      filters.messageFilter(note).should.be.true();
    });

    it('should return false for a note message when the all messages filter is enabled', () => {
      filters.hideAllPluginMessages = true;
      filters.messageFilter(note).should.be.false();
    });

    it('should return false for a "do not clean" message when the all messages filter is enabled', () => {
      filters.hideAllPluginMessages = true;
      filters.messageFilter(doNotCleanMessage).should.be.false();
    });
  });
});
