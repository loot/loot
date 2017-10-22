'use strict';

/* eslint-disable no-unused-expressions */
describe('Game', () => {
  /* Mock the Translator class. */
  class Translator {
    translate(text) {  // eslint-disable-line class-methods-use-this
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

    it('should set generalMessages to an empty array by default', () => {
      const game = new loot.Game({}, l10n);
      game.generalMessages.should.deep.equal([]);
    });

    it('should set generalMessages to the object\'s value if defined', () => {
      const game = new loot.Game({ generalMessages: ['test'] }, l10n);
      game.generalMessages.should.deep.equal(['test']);
    });

    it('should set masterlist to an empty object by default', () => {
      const game = new loot.Game({}, l10n);
      game.masterlist.should.deep.equal({});
    });

    it('should set masterlist to the object\'s value if defined', () => {
      const game = new loot.Game({ masterlist: { revision: 0 } }, l10n);
      game.masterlist.should.deep.equal({ revision: 0 });
    });

    it('should set plugins to an empty array by default', () => {
      const game = new loot.Game({}, l10n);
      game.plugins.should.deep.equal([]);
    });

    it('should set plugins to the object\'s value if defined', () => {
      const game = new loot.Game({ plugins: [{ name: 'test' }] }, l10n);
      game.plugins.should.deep.equal([{ name: 'test', cardZIndex: 1 }]);
    });

    it('should set loadOrder to undefined by default', () => {
      const game = new loot.Game({}, l10n);
      should.equal(undefined, game.loadOrder);
    });

    it('should set loadOrder to undefined even if the object\'s value if defined', () => {
      const game = new loot.Game({ loadOrder: ['test'] }, l10n);
      should.equal(undefined, game.loadOrder);
    });

    it('should set oldLoadOrder to undefined by default', () => {
      const game = new loot.Game({}, l10n);
      should.equal(undefined, game.oldLoadOrder);
    });

    it('should set oldLoadOrder to undefined even if the object\'s value if defined', () => {
      const game = new loot.Game({ oldLoadOrder: ['test'] }, l10n);
      should.equal(undefined, game.oldLoadOrder);
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

  describe('#generalMessages', () => {
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

      game.generalMessages = game.generalMessages;
      setTimeout(done, 100);
    });

    it('setting value should dispatch an event if the new value differs from the old one', (done) => {
      const newMessages = [
        { type: 'warn' },
        { type: 'error' },
      ];
      handleEvent = (evt) => {
        evt.detail.messages.should.deep.equal(newMessages);
        evt.detail.totalDiff.should.equal(2);
        evt.detail.errorDiff.should.equal(1);
        evt.detail.warningDiff.should.equal(1);
        done();
      };
      document.addEventListener('loot-game-global-messages-change', handleEvent);

      game.generalMessages = newMessages;
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
        evt.detail.should.deep.equal(newMasterlist);
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
        evt.detail.valuesAreTotals.should.be.true;
        evt.detail.totalMessageNo.should.equal(4);
        evt.detail.warnMessageNo.should.equal(2);
        evt.detail.errorMessageNo.should.equal(1);
        evt.detail.totalPluginNo.should.equal(2);
        evt.detail.activePluginNo.should.equal(1);
        evt.detail.dirtyPluginNo.should.equal(1);
        done();
      };
      document.addEventListener('loot-game-plugins-change', handleEvent);

      /* Set general messages to check they are also counted in the totals. */
      game.generalMessages = [
        { type: 'warn' },
        { type: 'error' },
      ];

      game.plugins = newPlugins;
    });
  });

  describe('#getContent()', () => {
    let game;

    beforeEach(() => {
      game = new loot.Game({}, l10n);
    });

    it('should return an object of two empty arrays if there is no game data', () => {
      game.getContent().should.deep.equal({
        messages: [],
        plugins: [],
      });
    });

    it('should return a structure containing converted plugin and message structures', () => {
      game._generalMessages = [{
        type: 'say',
        condition: 'file("foo.esp")',
        language: 'fr',
        text: 'Bonjour le monde',
      }];
      game._plugins = [{
        name: 'foo',
        crc: 0xDEADBEEF,
        version: '1.0',
        isActive: true,
        isEmpty: true,
        loadsArchive: true,

        masterlist: {},
        userlist: {},

        priority: -100,
        globalPriority: 100,
        messages: [{
          type: 'warn',
          condition: 'file("bar.esp")',
          language: 'en',
          text: 'Hello world',
        }],
        tags: ['invalidStructure'],
        isDirty: true,

        id: '',
        _isEditorOpen: true,
        _isSearchResult: true,
      }];

      game.getContent().should.deep.equal({
        messages: game._generalMessages,
        plugins: [{
          name: game._plugins[0].name,
          crc: game._plugins[0].crc,
          version: game._plugins[0].version,
          isActive: game._plugins[0].isActive,
          isEmpty: game._plugins[0].isEmpty,
          loadsArchive: game._plugins[0].loadsArchive,

          priority: game._plugins[0].priority,
          globalPriority: game._plugins[0].globalPriority,
          messages: game._plugins[0].messages,
          tags: game._plugins[0].tags,
          isDirty: game._plugins[0].isDirty,
        }],
      });
    });
  });

  describe('#getPluginNames()', () => {
    let game;

    beforeEach(() => {
      game = new loot.Game({}, l10n);
    });

    it('should return an empty array if there are no plugins', () => {
      game.getPluginNames().should.be.empty;
    });

    it('should return an array of plugin filenames if there are plugins', () => {
      game._plugins = [{
        name: 'foo',
        isActive: true,
        messages: [{ type: 'warn' }],
      }];

      game.getPluginNames().should.deep.equal(['foo']);
    });
  });

  describe('#setSortedPlugins', () => {
    let game;
    let handleEvent;

    beforeEach(() => {
      game = new loot.Game({}, l10n);
    });

    afterEach(() => {
      document.removeEventListener('loot-game-plugins-change', handleEvent);
    });

    it('should append new plugins to the plugins array', () => {
      game.setSortedPlugins([{
        name: 'foo',
      }]);

      game.plugins[0].name.should.equal('foo');
    });

    it('should update existing plugins with new data', () => {
      game._plugins = [new loot.Plugin({
        name: 'foo',
        isActive: true,
        messages: [{ type: 'warn' }],
      })];

      game.setSortedPlugins([{
        name: 'foo',
        crc: 0xDEADBEEF,
      }]);

      game.plugins[0].crc.should.equal(0xDEADBEEF);
      game.plugins[0].isActive.should.be.true;
    });

    it('should reorder plugins to given order', () => {
      game._plugins = [new loot.Plugin({
        name: 'foo',
      }), new loot.Plugin({
        name: 'bar',
      })];

      game.setSortedPlugins([{
        name: 'bar',
      }, {
        name: 'foo',
      }]);

      game.plugins[0].name.should.equal('bar');
      game.plugins[1].name.should.equal('foo');
    });

    it('should store old load order', () => {
      game._plugins = [new loot.Plugin({
        name: 'foo',
      }), new loot.Plugin({
        name: 'bar',
      })];

      game.setSortedPlugins([{
        name: 'bar',
      }, {
        name: 'foo',
      }]);

      game.oldLoadOrder[0].name.should.equal('foo');
      game.oldLoadOrder[1].name.should.equal('bar');
    });

    it('should dispatch an event', (done) => {
      handleEvent = () => {
        done();
      };
      document.addEventListener('loot-game-plugins-change', handleEvent);

      game.setSortedPlugins([{
        name: 'foo',
      }]);
    });
  });

  describe('#applySort', () => {
    let game;

    beforeEach(() => {
      game = new loot.Game({}, l10n);
    });

    it('should delete the stored old load order', () => {
      game.oldLoadOrder = [0, 1, 2];

      game.applySort();

      should.equal(undefined, game.oldLoadOrder);
    });
  });

  describe('#cancelSort', () => {
    let game;

    beforeEach(() => {
      game = new loot.Game({}, l10n);
    });

    it('should throw if no parameters are supplied', () => {
      (() => { game.cancelSort(); }).should.throw();
    });

    it('should set the current load order to the old load order', () => {
      const oldLoadOrder = [new loot.Plugin({
        name: 'foo',
      }), new loot.Plugin({
        name: 'bar',
      })];
      game.oldLoadOrder = oldLoadOrder;
      game.plugins = [new loot.Plugin({
        name: 'bar',
      }), new loot.Plugin({
        name: 'foo',
      })];

      game.cancelSort([]);

      game.plugins.should.deep.equal(oldLoadOrder);
    });

    it('should delete the stored old load order', () => {
      game.oldLoadOrder = [new loot.Plugin({
        name: 'foo',
      }), new loot.Plugin({
        name: 'bar',
      })];

      game.cancelSort([]);

      should.equal(undefined, game.oldLoadOrder);
    });

    it('should set plugin load order indices using the array passed as the first parameter', () => {
      game.oldLoadOrder = [new loot.Plugin({
        name: 'foo',
        loadOrderIndex: 1,
      }), new loot.Plugin({
        name: 'bar',
        loadOrderIndex: 0,
      })];

      game.cancelSort([new loot.Plugin({
        name: 'bar',
        loadOrderIndex: 1,
      }), new loot.Plugin({
        name: 'foo',
        loadOrderIndex: 0,
      })]);

      game.plugins[0].name.should.equal('foo');
      game.plugins[0].loadOrderIndex.should.equal(0);
      game.plugins[1].name.should.equal('bar');
      game.plugins[1].loadOrderIndex.should.equal(1);
    });

    it('should set the general messages to the second passed parameter', () => {
      game.oldLoadOrder = [new loot.Plugin({
        name: 'foo',
        loadOrderIndex: 1,
      }), new loot.Plugin({
        name: 'bar',
        loadOrderIndex: 0,
      })];

      game.cancelSort([], ['foo']);

      game.generalMessages.should.deep.equal(['foo']);
    });
  });

  describe('#clearMetadata', () => {
    let game;

    beforeEach(() => {
      game = new loot.Game({}, l10n);
    });

    it('should delete stored userlist data for existing plugins', () => {
      game._plugins = [new loot.Plugin({
        name: 'foo',
        userlist: {},
      })];

      game.clearMetadata([{
        name: 'foo',
      }]);

      should.equal(undefined, game.plugins[0].userlist);
    });

    it('should update existing plugin data', () => {
      game._plugins = [new loot.Plugin({
        name: 'foo',
        isActive: true,
      })];

      game.clearMetadata([{
        name: 'foo',
        crc: 0xDEADBEEF,
      }]);

      game.plugins[0].crc.should.equal(0xDEADBEEF);
      game.plugins[0].isActive.should.be.true;
    });
  });
});
