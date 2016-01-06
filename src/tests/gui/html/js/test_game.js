'use strict';

describe('Game', () => {
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
    it('should throw if passed no paramters', () => {
      (() => { new loot.Game(); }).should.throw(); // eslint-disable-line no-new
    });

    it('should throw if passed no l10n parameter', () => {
      (() => { new loot.Game({}); }).should.throw(); // eslint-disable-line no-new
    });

    it('should not throw if passed an empty object as the first parameter', () => {
      (() => { new loot.Game({}, l10n); }).should.not.throw(); // eslint-disable-line no-new
    });

    it('should set folder to a blank string by default', () => {
      const game = new loot.Game({}, l10n);
      game.folder.should.equal('');
    });

    it('should set folder to the object\'s value if defined', () => {
      const game = new loot.Game({ folder: 'test' }, l10n);
      game.folder.should.equal('test');
    });

    it('should set globalMessages to an empty array by default', () => {
      const game = new loot.Game({}, l10n);
      game.globalMessages.should.deepEqual([]);
    });

    it('should set globalMessages to the object\'s value if defined', () => {
      const game = new loot.Game({ globalMessages: ['test'] }, l10n);
      game.globalMessages.should.deepEqual(['test']);
    });

    it('should set masterlist to an empty object by default', () => {
      const game = new loot.Game({}, l10n);
      game.masterlist.should.deepEqual({});
    });

    it('should set masterlist to the object\'s value if defined', () => {
      const game = new loot.Game({ masterlist: { revision: 0 } }, l10n);
      game.masterlist.should.deepEqual({ revision: 0 });
    });

    it('should set plugins to an empty array by default', () => {
      const game = new loot.Game({}, l10n);
      game.plugins.should.deepEqual([]);
    });

    it('should set plugins to the object\'s value if defined', () => {
      const game = new loot.Game({ plugins: ['test'] }, l10n);
      game.plugins.should.deepEqual(['test']);
    });

    it('should set loadOrder to undefined by default', () => {
      const game = new loot.Game({}, l10n);
      should(game.loadOrder).be.undefined();
    });

    it('should set loadOrder to undefined even if the object\'s value if defined', () => {
      const game = new loot.Game({ loadOrder: ['test'] }, l10n);
      should(game.loadOrder).be.undefined();
    });

    it('should set oldLoadOrder to undefined by default', () => {
      const game = new loot.Game({}, l10n);
      should(game.oldLoadOrder).be.undefined();
    });

    it('should set oldLoadOrder to undefined even if the object\'s value if defined', () => {
      const game = new loot.Game({ oldLoadOrder: ['test'] }, l10n);
      should(game.oldLoadOrder).be.undefined();
    });

    it('should initialise _notApplicableString', () => {
      const game = new loot.Game({ oldLoadOrder: ['test'] }, l10n);
      game._notApplicableString.should.equal('N/A');
    });
  });

  describe('#folder', () => {
    let game;
    let handleEvent;

    beforeEach(() => {
      game = new loot.Game({}, l10n);
    });

    afterEach(() => {
      document.removeEventListener('loot-game-folder-change', handleEvent);
    });

    it('setting value should not dispatch an event if the new value is equal to the old one', (done) => {
      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };
      document.addEventListener('loot-game-folder-change', handleEvent);

      game.folder = game.folder;
      setTimeout(done, 100);
    });

    it('setting value should dispatch an event if the new value differs from the old one', (done) => {
      handleEvent = (evt) => {
        evt.detail.folder.should.equal('test');
        done();
      };
      document.addEventListener('loot-game-folder-change', handleEvent);

      game.folder = 'test';
    });
  });

  describe('#globalMessages', () => {
    let game;
    let handleEvent;

    beforeEach(() => {
      game = new loot.Game({}, l10n);
    });

    afterEach(() => {
      document.removeEventListener('loot-game-global-messages-change', handleEvent);
    });

    it('setting value should not dispatch an event if the new value is equal to the old one', (done) => {
      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };
      document.addEventListener('loot-game-global-messages-change', handleEvent);

      game.globalMessages = game.globalMessages;
      setTimeout(done, 100);
    });

    it('setting value should dispatch an event if the new value differs from the old one', (done) => {
      const newMessages = [
        { type: 'warn' },
        { type: 'error' },
      ];
      handleEvent = (evt) => {
        evt.detail.messages.should.deepEqual(newMessages);
        evt.detail.totalDiff.should.equal(2);
        evt.detail.errorDiff.should.equal(1);
        evt.detail.warningDiff.should.equal(1);
        done();
      };
      document.addEventListener('loot-game-global-messages-change', handleEvent);

      game.globalMessages = newMessages;
    });
  });

  describe('#masterlist', () => {
    let game;
    let handleEvent;

    beforeEach(() => {
      game = new loot.Game({}, l10n);
    });

    afterEach(() => {
      document.removeEventListener('loot-game-masterlist-change', handleEvent);
    });

    it('setting value should not dispatch an event if the new value is equal to the old one', (done) => {
      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };
      document.addEventListener('loot-game-masterlist-change', handleEvent);

      game.masterlist = game.masterlist;
      setTimeout(done, 100);
    });

    it('setting value should dispatch an event if the new value differs from the old one', (done) => {
      const newMasterlist = {
        revision: 'foo',
        date: 'bar',
      };
      handleEvent = (evt) => {
        evt.detail.should.deepEqual(newMasterlist);
        done();
      };
      document.addEventListener('loot-game-masterlist-change', handleEvent);

      game.masterlist = newMasterlist;
    });

    it('setting value to undefined should dispatch an event with the details being the not applicable string', (done) => {
      handleEvent = (evt) => {
        evt.detail.revision.should.equal(game._notApplicableString);
        evt.detail.date.should.equal(game._notApplicableString);
        done();
      };
      document.addEventListener('loot-game-masterlist-change', handleEvent);

      game.masterlist = undefined;
    });
  });

  describe('#plugins', () => {
    let game;
    let handleEvent;

    beforeEach(() => {
      game = new loot.Game({}, l10n);
    });

    afterEach(() => {
      document.removeEventListener('loot-game-plugins-change', handleEvent);
    });

    it('setting value should dispatch an event if the new value is equal to the old one', (done) => {
      handleEvent = () => {
        done();
      };
      document.addEventListener('loot-game-plugins-change', handleEvent);

      game.plugins = game.plugins;
    });

    it('setting value should dispatch an event if the new value differs from the old one', (done) => {
      const newPlugins = [
        {
          isActive: true,
          messages: [{ type: 'warn' }],
        },
        {
          isDirty: true,
          messages: [{ type: 'say' }],
        },
      ];
      handleEvent = (evt) => {
        evt.detail.valuesAreTotals.should.be.true();
        evt.detail.totalMessageNo.should.equal(4);
        evt.detail.warnMessageNo.should.equal(2);
        evt.detail.errorMessageNo.should.equal(1);
        evt.detail.totalPluginNo.should.equal(2);
        evt.detail.activePluginNo.should.equal(1);
        evt.detail.dirtyPluginNo.should.equal(1);
        done();
      };
      document.addEventListener('loot-game-plugins-change', handleEvent);

      /* Set global messages to check they are also counted in the totals. */
      game.globalMessages = [
        { type: 'warn' },
        { type: 'error' },
      ];

      game.plugins = newPlugins;
    });
  });

  describe('#appendPlugin()', () => {
    let game;
    let handleEvent;

    beforeEach(() => {
      game = new loot.Game({}, l10n);
    });

    afterEach(() => {
      document.removeEventListener('loot-game-plugins-change', handleEvent);
    });

    it('should append the passed plugin to the plugins array', () => {
      const newPlugin = {
        isActive: true,
        messages: [{ type: 'warn' }],
      };
      game.appendPlugin(newPlugin);

      game.plugins[0].should.deepEqual(newPlugin);
    });

    it('should dispatch an event with the correct counter differences', (done) => {
      handleEvent = (evt) => {
        evt.detail.valuesAreTotals.should.be.false();
        evt.detail.totalMessageNo.should.equal(1);
        evt.detail.warnMessageNo.should.equal(1);
        evt.detail.errorMessageNo.should.equal(0);
        evt.detail.totalPluginNo.should.equal(1);
        evt.detail.activePluginNo.should.equal(1);
        evt.detail.dirtyPluginNo.should.equal(0);
        done();
      };
      document.addEventListener('loot-game-plugins-change', handleEvent);

      /* Set global messages to check they are also counted in the totals. */
      game.globalMessages = [
        { type: 'warn' },
        { type: 'error' },
      ];

      game.appendPlugin({
        isActive: true,
        messages: [{ type: 'warn' }],
      });
    });
  });

  describe('#removePluginAtIndex()', () => {
    let game;
    let handleEvent;

    beforeEach(() => {
      game = new loot.Game({}, l10n);
      game._plugins = [{
        isActive: true,
        messages: [{ type: 'warn' }],
      }];
    });

    afterEach(() => {
      document.removeEventListener('loot-game-plugins-change', handleEvent);
    });

    it('should remove the passed plugin to the plugins array', () => {
      game.removePluginAtIndex(0);
      game.plugins.length.should.equal(0);
    });

    it('should dispatch an event with the correct counter differences', (done) => {
      handleEvent = (evt) => {
        evt.detail.valuesAreTotals.should.be.false();
        evt.detail.totalMessageNo.should.equal(-1);
        evt.detail.warnMessageNo.should.equal(-1);
        evt.detail.errorMessageNo.should.equal(0);
        evt.detail.totalPluginNo.should.equal(-1);
        evt.detail.activePluginNo.should.equal(-1);
        evt.detail.dirtyPluginNo.should.equal(0);
        done();
      };
      document.addEventListener('loot-game-plugins-change', handleEvent);

      /* Set global messages to check they are also counted in the totals. */
      game.globalMessages = [
        { type: 'warn' },
        { type: 'error' },
      ];

      game.removePluginAtIndex(0);
    });
  });
});
