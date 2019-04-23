/* eslint-disable no-self-assign */

import Game from '../../../../gui/html/js/game.js';
import { Plugin } from '../../../../gui/html/js/plugin.js';

jest.mock('../../../../gui/html/js/dom.js');
jest.mock('../../../../gui/html/js/filters.js');
jest.mock('../../../../gui/html/js/plugin.js', () => ({
  crcToString: jest.fn(),
  Plugin: jest
    .fn()
    .mockImplementation(({ name, crc, isActive, loadOrderIndex }) => ({
      name,
      crc,
      isActive,
      loadOrderIndex,
      update(other) {
        Object.assign(this, other);
      }
    }))
}));

describe('Game', () => {
  const l10n = {
    translate: jest.fn().mockImplementation(text => text)
  };

  describe('#constructor()', () => {
    test('should throw if passed no paramters', () => {
      expect(() => {
        new Game(); // eslint-disable-line no-new
      }).toThrow();
    });

    test('should throw if passed no l10n parameter', () => {
      expect(() => {
        new Game({}); // eslint-disable-line no-new
      }).toThrow();
    });

    test('should not throw if passed an empty object as the first parameter', () => {
      expect(() => {
        new Game({}, l10n); // eslint-disable-line no-new
      }).not.toThrow();
    });

    test('should set folder to a blank string by default', () => {
      const game = new Game({}, l10n);
      expect(game.folder).toBe('');
    });

    test("should set folder to the object's value if defined", () => {
      const game = new Game({ folder: 'test' }, l10n);
      expect(game.folder).toBe('test');
    });

    test('should set generalMessages to an empty array by default', () => {
      const game = new Game({}, l10n);
      expect(game.generalMessages).toEqual([]);
    });

    test("should set generalMessages to the object's value if defined", () => {
      const game = new Game({ generalMessages: ['test'] }, l10n);
      expect(game.generalMessages).toEqual(['test']);
    });

    test('should set masterlist to an empty object by default', () => {
      const game = new Game({}, l10n);
      expect(game.masterlist).toEqual({});
    });

    test("should set masterlist to the object's value if defined", () => {
      const game = new Game({ masterlist: { revision: 0 } }, l10n);
      expect(game.masterlist).toEqual({ revision: 0 });
    });

    test('should set plugins to an empty array by default', () => {
      const game = new Game({}, l10n);
      expect(game.plugins).toEqual([]);
    });

    test("should set plugins to the object's value if defined", () => {
      const game = new Game({ plugins: [{ name: 'test' }] }, l10n);
      expect(game.plugins).toEqual([{ name: 'test', cardZIndex: 1 }]);
    });

    test('should set loadOrder to undefined by default', () => {
      const game = new Game({}, l10n);
      expect(game.loadOrder).toBe(undefined);
    });

    test("should set loadOrder to undefined even if the object's value if defined", () => {
      const game = new Game({ loadOrder: ['test'] }, l10n);
      expect(game.loadOrder).toBe(undefined);
    });

    test('should set oldLoadOrder to undefined by default', () => {
      const game = new Game({}, l10n);
      expect(game.oldLoadOrder).toBe(undefined);
    });

    test("should set oldLoadOrder to undefined even if the object's value if defined", () => {
      const game = new Game({ oldLoadOrder: ['test'] }, l10n);
      expect(game.oldLoadOrder).toBe(undefined);
    });

    test('should initialise _notApplicableString', () => {
      const game = new Game({ oldLoadOrder: ['test'] }, l10n);
      expect(game._notApplicableString).toBe('N/A');
    });
  });

  describe('#folder', () => {
    let game;
    let handleEvent;

    beforeEach(() => {
      game = new Game({}, l10n);
    });

    afterEach(() => {
      document.removeEventListener('loot-game-folder-change', handleEvent);
    });

    test('setting value should not dispatch an event if the new value is equal to the old one', done => {
      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };
      document.addEventListener('loot-game-folder-change', handleEvent);

      game.folder = game.folder;
      setTimeout(done, 100);
    });

    test('setting value should dispatch an event if the new value differs from the old one', done => {
      handleEvent = evt => {
        expect(evt.detail.folder).toBe('test');
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
      game = new Game({}, l10n);
    });

    afterEach(() => {
      document.removeEventListener(
        'loot-game-global-messages-change',
        handleEvent
      );
    });

    test('setting value should not dispatch an event if the new value is equal to the old one', done => {
      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };
      document.addEventListener(
        'loot-game-global-messages-change',
        handleEvent
      );

      game.generalMessages = game.generalMessages;
      setTimeout(done, 100);
    });

    test('setting value should dispatch an event if the new and old message strings differ', done => {
      game._generalMessages = [{ type: 'warn', content: 'foo' }];
      const newMessages = [{ type: 'warn', content: 'bar' }];
      handleEvent = evt => {
        expect(evt.detail.messages).toEqual(newMessages);
        expect(evt.detail.totalDiff).toBe(0);
        expect(evt.detail.errorDiff).toBe(0);
        expect(evt.detail.warningDiff).toBe(0);
        done();
      };
      document.addEventListener(
        'loot-game-global-messages-change',
        handleEvent
      );

      game.generalMessages = newMessages;
    });

    test('setting value should dispatch an event if the new and old message type counts differ', done => {
      const newMessages = [{ type: 'warn' }, { type: 'error' }];
      handleEvent = evt => {
        expect(evt.detail.messages).toEqual(newMessages);
        expect(evt.detail.totalDiff).toBe(2);
        expect(evt.detail.errorDiff).toBe(1);
        expect(evt.detail.warningDiff).toBe(1);
        done();
      };
      document.addEventListener(
        'loot-game-global-messages-change',
        handleEvent
      );

      game.generalMessages = newMessages;
    });
  });

  describe('#masterlist', () => {
    let game;
    let handleEvent;

    beforeEach(() => {
      game = new Game({}, l10n);
    });

    afterEach(() => {
      document.removeEventListener('loot-game-masterlist-change', handleEvent);
    });

    test('setting value should not dispatch an event if the new value is equal to the old one', done => {
      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };
      document.addEventListener('loot-game-masterlist-change', handleEvent);

      game.masterlist = game.masterlist;
      setTimeout(done, 100);
    });

    test('setting value should dispatch an event if the new value differs from the old one', done => {
      const newMasterlist = {
        revision: 'foo',
        date: 'bar'
      };
      handleEvent = evt => {
        expect(evt.detail).toEqual(newMasterlist);
        done();
      };
      document.addEventListener('loot-game-masterlist-change', handleEvent);

      game.masterlist = newMasterlist;
    });

    test('setting value to undefined should dispatch an event with the details being the not applicable string', done => {
      handleEvent = evt => {
        expect(evt.detail.revision).toBe(game._notApplicableString);
        expect(evt.detail.date).toBe(game._notApplicableString);
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
      game = new Game({}, l10n);
    });

    afterEach(() => {
      document.removeEventListener('loot-game-plugins-change', handleEvent);
    });

    test('setting value should dispatch an event if the new value is equal to the old one', done => {
      handleEvent = () => {
        done();
      };
      document.addEventListener('loot-game-plugins-change', handleEvent);

      game.plugins = game.plugins;
    });

    test('setting value should dispatch an event if the new value differs from the old one', done => {
      const newPlugins = [
        {
          isActive: true,
          messages: [{ type: 'warn' }]
        },
        {
          isDirty: true,
          messages: [{ type: 'say' }]
        }
      ];
      handleEvent = evt => {
        expect(evt.detail.valuesAreTotals).toBe(true);
        expect(evt.detail.totalMessageNo).toBe(4);
        expect(evt.detail.warnMessageNo).toBe(2);
        expect(evt.detail.errorMessageNo).toBe(1);
        expect(evt.detail.totalPluginNo).toBe(2);
        expect(evt.detail.activePluginNo).toBe(1);
        expect(evt.detail.dirtyPluginNo).toBe(1);
        done();
      };
      document.addEventListener('loot-game-plugins-change', handleEvent);

      /* Set general messages to check they are also counted in the totals. */
      game.generalMessages = [{ type: 'warn' }, { type: 'error' }];

      game.plugins = newPlugins;
    });
  });

  describe('#getContent()', () => {
    let game;

    beforeEach(() => {
      game = new Game({}, l10n);
    });

    test('should return an object of two empty arrays if there is no game data', () => {
      expect(game.getContent()).toEqual({
        messages: [],
        plugins: []
      });
    });

    test('should return a structure containing converted plugin and message structures', () => {
      game._generalMessages = [
        {
          type: 'say',
          condition: 'file("foo.esp")',
          language: 'fr',
          text: 'Bonjour le monde'
        }
      ];
      game._plugins = [
        {
          name: 'foo',
          crc: 0xdeadbeef,
          version: '1.0',
          isActive: true,
          isEmpty: true,
          loadsArchive: true,

          masterlist: {},
          userlist: {},

          group: 'group1',
          messages: [
            {
              type: 'warn',
              condition: 'file("bar.esp")',
              language: 'en',
              text: 'Hello world'
            }
          ],
          currentTags: [
            {
              name: 'Relev',
              isAddition: true,
              condition: ''
            }
          ],
          suggestedTags: [
            {
              name: 'Delev',
              isAddition: true,
              condition: ''
            }
          ],
          isDirty: true,

          id: '',
          _isEditorOpen: true,
          _isSearchResult: true
        }
      ];

      expect(game.getContent()).toEqual({
        messages: game._generalMessages,
        plugins: [
          {
            name: game._plugins[0].name,
            crc: game._plugins[0].crc,
            version: game._plugins[0].version,
            isActive: game._plugins[0].isActive,
            isEmpty: game._plugins[0].isEmpty,
            loadsArchive: game._plugins[0].loadsArchive,

            group: game._plugins[0].group,
            messages: game._plugins[0].messages,
            currentTags: game._plugins[0].currentTags,
            suggestedTags: game._plugins[0].suggestedTags,
            isDirty: game._plugins[0].isDirty
          }
        ]
      });
    });
  });

  describe('#getPluginNames()', () => {
    let game;

    beforeEach(() => {
      game = new Game({}, l10n);
    });

    test('should return an empty array if there are no plugins', () => {
      expect(game.getPluginNames().length).toBe(0);
    });

    test('should return an array of plugin filenames if there are plugins', () => {
      game._plugins = [
        {
          name: 'foo',
          isActive: true,
          messages: [{ type: 'warn' }]
        }
      ];

      expect(game.getPluginNames()).toEqual(['foo']);
    });
  });

  describe('#getGroupPluginNames()', () => {
    let game;

    beforeEach(() => {
      const plugins = [
        {
          name: 'foo',
          group: 'test group'
        },
        {
          name: 'bar',
          group: 'other group'
        },
        {
          name: 'foobar',
          group: 'test group'
        }
      ];
      game = new Game({ plugins }, l10n);
    });

    test('should return an empty array if there are no plugins in the given group', () => {
      expect(game.getGroupPluginNames('empty group').length).toBe(0);
    });

    test('should return an array of filenames of plugins in the given group', () => {
      expect(game.getGroupPluginNames('test group')).toEqual(['foo', 'foobar']);
    });
  });

  describe('#setSortedPlugins', () => {
    let game;
    let handleEvent;

    beforeEach(() => {
      game = new Game({}, l10n);
    });

    afterEach(() => {
      document.removeEventListener('loot-game-plugins-change', handleEvent);
    });

    test('should append new plugins to the plugins array', () => {
      game.setSortedPlugins([
        {
          name: 'foo'
        }
      ]);

      expect(game.plugins[0].name).toBe('foo');
    });

    test('should update existing plugins with new data', () => {
      game._plugins = [
        new Plugin({
          name: 'foo',
          isActive: true,
          messages: [{ type: 'warn' }]
        })
      ];

      game.setSortedPlugins([
        {
          name: 'foo',
          crc: 0xdeadbeef
        }
      ]);

      expect(game.plugins[0].crc).toBe(0xdeadbeef);
      expect(game.plugins[0].isActive).toBe(true);
    });

    test('should reorder plugins to given order', () => {
      game._plugins = [
        new Plugin({
          name: 'foo'
        }),
        new Plugin({
          name: 'bar'
        })
      ];

      game.setSortedPlugins([
        {
          name: 'bar'
        },
        {
          name: 'foo'
        }
      ]);

      expect(game.plugins[0].name).toBe('bar');
      expect(game.plugins[1].name).toBe('foo');
    });

    test('should store old load order', () => {
      game._plugins = [
        new Plugin({
          name: 'foo'
        }),
        new Plugin({
          name: 'bar'
        })
      ];

      game.setSortedPlugins([
        {
          name: 'bar'
        },
        {
          name: 'foo'
        }
      ]);

      expect(game.oldLoadOrder[0].name).toBe('foo');
      expect(game.oldLoadOrder[1].name).toBe('bar');
    });

    test('should dispatch an event', done => {
      handleEvent = () => {
        done();
      };
      document.addEventListener('loot-game-plugins-change', handleEvent);

      game.setSortedPlugins([
        {
          name: 'foo'
        }
      ]);
    });
  });

  describe('#applySort', () => {
    let game;

    beforeEach(() => {
      game = new Game({}, l10n);
    });

    test('should delete the stored old load order', () => {
      game.oldLoadOrder = [0, 1, 2];

      game.applySort();

      expect(game.oldLoadOrder).toBe(undefined);
    });
  });

  describe('#cancelSort', () => {
    let game;

    beforeEach(() => {
      game = new Game({}, l10n);
    });

    test('should throw if no parameters are supplied', () => {
      expect(() => {
        game.cancelSort();
      }).toThrow();
    });

    test('should set the current load order to the given plugins', () => {
      const oldLoadOrder = [
        new Plugin({
          name: 'foo'
        }),
        new Plugin({
          name: 'bar'
        }),
        new Plugin({
          name: 'baz'
        })
      ];
      game.oldLoadOrder = oldLoadOrder;
      game.plugins = [oldLoadOrder[2], oldLoadOrder[1], oldLoadOrder[0]];

      game.cancelSort([
        {
          name: 'baz',
          isActive: true
        },
        {
          name: 'foo',
          isMaster: true
        }
      ]);

      expect(game.plugins).toEqual([
        Object.assign({}, oldLoadOrder[2], { isActive: true }),
        Object.assign({}, oldLoadOrder[0], { isMaster: true })
      ]);
    });

    test('should delete the stored old load order', () => {
      game.oldLoadOrder = [
        new Plugin({
          name: 'foo'
        }),
        new Plugin({
          name: 'bar'
        })
      ];

      game.cancelSort([]);

      expect(game.oldLoadOrder).toBe(undefined);
    });

    test('should set plugin load order indices using the array passed as the first parameter', () => {
      game.oldLoadOrder = [
        new Plugin({
          name: 'foo',
          loadOrderIndex: 1
        }),
        new Plugin({
          name: 'bar',
          loadOrderIndex: 0
        })
      ];

      game.cancelSort([
        new Plugin({
          name: 'bar',
          loadOrderIndex: 0
        }),
        new Plugin({
          name: 'foo',
          loadOrderIndex: 2
        })
      ]);

      expect(game.plugins[0].name).toBe('bar');
      expect(game.plugins[0].loadOrderIndex).toBe(0);
      expect(game.plugins[1].name).toBe('foo');
      expect(game.plugins[1].loadOrderIndex).toBe(2);
    });

    test('should set the general messages to the second passed parameter', () => {
      game.oldLoadOrder = [
        new Plugin({
          name: 'foo',
          loadOrderIndex: 1
        }),
        new Plugin({
          name: 'bar',
          loadOrderIndex: 0
        })
      ];

      game.cancelSort([], ['foo']);

      expect(game.generalMessages).toEqual(['foo']);
    });
  });

  describe('#clearMetadata', () => {
    let game;

    beforeEach(() => {
      game = new Game({}, l10n);
    });

    test('should delete stored userlist data for existing plugins', () => {
      game._plugins = [
        new Plugin({
          name: 'foo',
          userlist: {}
        })
      ];

      game.clearMetadata([
        {
          name: 'foo'
        }
      ]);

      expect(game.plugins[0].userlist).toBe(undefined);
    });

    test('should update existing plugin data', () => {
      game._plugins = [
        new Plugin({
          name: 'foo',
          isActive: true
        })
      ];

      game.clearMetadata([
        {
          name: 'foo',
          crc: 0xdeadbeef
        }
      ]);

      expect(game.plugins[0].crc).toBe(0xdeadbeef);
      expect(game.plugins[0].isActive).toBe(true);
    });
  });
});
