/* eslint-disable no-self-assign */

import Game from '../../../../gui/html/js/game';
import { Plugin } from '../../../../gui/html/js/plugin';
import Translator from '../../../../gui/html/js/translator';

jest.mock('../../../../gui/html/js/dom');
jest.mock('../../../../gui/html/js/filters');

describe('Game', () => {
  const l10n = new Translator();
  const defaultDerivedPluginMetadata = {
    name: 'test',
    isActive: false,
    isDirty: false,
    isEmpty: false,
    isMaster: false,
    isLightPlugin: false,
    loadsArchive: false,
    messages: [],
    suggestedTags: [],
    currentTags: []
  };
  const gameData = {
    folder: 'test',
    generalMessages: [
      {
        type: 'say',
        text: 'test',
        language: 'en',
        condition: ''
      }
    ],
    masterlist: { revision: '0', date: '' },
    groups: { masterlist: [], userlist: [] },
    plugins: [defaultDerivedPluginMetadata],
    bashTags: []
  };

  describe('#constructor()', () => {
    test("should set folder to the object's value", () => {
      const game = new Game(gameData, l10n);
      expect(game.folder).toBe('test');
    });

    test("should set generalMessages to the object's value", () => {
      const game = new Game(gameData, l10n);
      expect(game.generalMessages).toEqual([
        {
          type: 'say',
          text: 'test',
          language: 'en',
          condition: ''
        }
      ]);
    });

    test("should set masterlist to the object's value", () => {
      const game = new Game(gameData, l10n);
      expect(game.masterlist).toEqual({ revision: '0', date: '' });
    });

    test("should construct plugins from the object's plugins value", () => {
      const game = new Game(gameData, l10n);
      expect(game.plugins.length).toBe(1);
      expect(game.plugins[0]).toHaveProperty('update');
      expect(game.plugins[0].name).toBe('test');
      expect(game.plugins[0].cardZIndex).toBe(1);
    });

    test("should set oldLoadOrder to an empty array even if the object's value if defined", () => {
      const game = new Game(gameData, l10n);
      expect(game.oldLoadOrder).toEqual([]);
    });
  });

  describe('#folder', () => {
    let game: Game;
    // It's not worth the hassle of defining and checking the event type in test
    // code.
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    let handleEvent: (evt: any) => void;

    beforeEach(() => {
      game = new Game(gameData, l10n);
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
        expect(evt.detail.folder).toBe('other test');
        done();
      };
      document.addEventListener('loot-game-folder-change', handleEvent);

      game.folder = 'other test';
    });
  });

  describe('#generalMessages', () => {
    let game: Game;
    // It's not worth the hassle of defining and checking the event type in test
    // code.
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    let handleEvent: (evt: any) => void;

    beforeEach(() => {
      game = new Game(gameData, l10n);
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
      const newMessages = [
        {
          type: 'say',
          text: 'bar',
          language: 'en',
          condition: ''
        }
      ];
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
      const newMessages = [
        {
          type: 'warn',
          text: 'foo',
          language: 'en',
          condition: ''
        },
        {
          type: 'error',
          text: 'bar',
          language: 'en',
          condition: ''
        }
      ];
      game.generalMessages = [];
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
    let game: Game;
    // It's not worth the hassle of defining and checking the event type in test
    // code.
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    let handleEvent: (evt: any) => void;

    beforeEach(() => {
      game = new Game(gameData, l10n);
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
  });

  describe('#plugins', () => {
    let game: Game;
    // It's not worth the hassle of defining and checking the event type in test
    // code.
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    let handleEvent: (evt: any) => void;

    beforeEach(() => {
      game = new Game(gameData, l10n);
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
      const newMessages = [
        {
          type: 'warn',
          text: 'foo',
          language: 'en',
          condition: ''
        },
        {
          type: 'error',
          text: 'bar',
          language: 'en',
          condition: ''
        }
      ];
      const newPlugins = [
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'a',
          isActive: true,
          messages: [newMessages[0]]
        }),
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'b',
          isDirty: true,
          messages: [{ ...newMessages[0], type: 'say' }]
        })
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

      game.generalMessages = newMessages;
      game.plugins = newPlugins;
    });
  });

  describe('groups', () => {
    test("get should return the game's groups", () => {
      const groups = {
        masterlist: [{ name: 'a', after: [] }],
        userlist: [{ name: 'b', after: [] }]
      };
      const game = new Game({ ...gameData, groups }, l10n);

      expect(game.groups).toStrictEqual([
        { name: 'a', after: [], isUserAdded: false },
        { name: 'b', after: [], isUserAdded: true }
      ]);
    });
  });

  describe('#setGroups()', () => {
    // It's not worth the hassle of defining and checking the event type in test
    // code.
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    let handleEvent: (evt: any) => void;

    afterEach(() => {
      document.removeEventListener('loot-game-groups-change', handleEvent);
    });

    test('should merge the given masterlist and userlist groups arrays', () => {
      const game = new Game(gameData, l10n);

      const groups = {
        masterlist: [{ name: 'a', after: [] }],
        userlist: [{ name: 'b', after: [] }]
      };

      game.setGroups(groups);

      expect(game.groups).toStrictEqual([
        { name: 'a', after: [], isUserAdded: false },
        { name: 'b', after: [], isUserAdded: true }
      ]);
    });

    test('should dispatch an event', done => {
      handleEvent = () => {
        done();
      };
      document.addEventListener('loot-game-groups-change', handleEvent);

      const game = new Game(gameData, l10n);

      const groups = {
        masterlist: [{ name: 'a', after: [] }],
        userlist: [{ name: 'b', after: [] }]
      };

      game.setGroups(groups);
    });
  });

  describe('#getContent()', () => {
    let game: Game;

    beforeEach(() => {
      game = new Game(gameData, l10n);
    });

    test('should return an object of two empty arrays if there is no game data', () => {
      game.plugins = [];
      game.generalMessages = [];

      expect(game.getContent()).toEqual({
        messages: [],
        plugins: []
      });
    });

    test('should return a structure containing converted plugin and message structures', () => {
      game.generalMessages = [
        {
          type: 'say',
          condition: 'file("foo.esp")',
          language: 'fr',
          text: 'Bonjour le monde'
        }
      ];
      game.plugins = [
        new Plugin({
          name: 'foo',
          crc: 0xdeadbeef,
          version: '1.0',
          isActive: true,
          isEmpty: true,
          isMaster: false,
          isLightPlugin: false,
          loadsArchive: true,

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
          isDirty: true
        })
      ];

      expect(game.getContent()).toEqual({
        messages: game.generalMessages,
        plugins: [
          {
            name: game.plugins[0].name,
            crc: game.plugins[0].crc,
            version: game.plugins[0].version,
            isActive: game.plugins[0].isActive,
            isEmpty: game.plugins[0].isEmpty,
            loadsArchive: game.plugins[0].loadsArchive,

            group: game.plugins[0].group,
            messages: game.plugins[0].messages,
            currentTags: game.plugins[0].currentTags,
            suggestedTags: game.plugins[0].suggestedTags,
            isDirty: game.plugins[0].isDirty
          }
        ]
      });
    });
  });

  describe('#getPluginNames()', () => {
    let game: Game;

    beforeEach(() => {
      game = new Game(gameData, l10n);
    });

    test('should return an empty array if there are no plugins', () => {
      game.plugins = [];
      expect(game.getPluginNames().length).toBe(0);
    });

    test('should return an array of plugin filenames if there are plugins', () => {
      game.plugins = [
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foo',
          isActive: true,
          messages: [{ type: 'warn', text: '', language: 'en', condition: '' }]
        })
      ];

      expect(game.getPluginNames()).toEqual(['foo']);
    });
  });

  describe('#getGroupPluginNames()', () => {
    let game: Game;

    beforeEach(() => {
      const plugins = [
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foo',
          group: 'test group'
        }),
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'bar',
          group: 'other group'
        }),
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foobar',
          group: 'test group'
        })
      ];
      game = new Game({ ...gameData, plugins }, l10n);
    });

    test('should return an empty array if there are no plugins in the given group', () => {
      expect(game.getGroupPluginNames('empty group').length).toBe(0);
    });

    test('should return an array of filenames of plugins in the given group', () => {
      expect(game.getGroupPluginNames('test group')).toEqual(['foo', 'foobar']);
    });
  });

  describe('#setSortedPlugins', () => {
    let game: Game;
    // It's not worth the hassle of defining and checking the event type in test
    // code.
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    let handleEvent: (evt: any) => void;

    beforeEach(() => {
      game = new Game(gameData, l10n);
    });

    afterEach(() => {
      document.removeEventListener('loot-game-plugins-change', handleEvent);
    });

    test('should append new plugins to the plugins array', () => {
      game.setSortedPlugins([
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foo'
        })
      ]);

      expect(game.plugins[0].name).toBe('foo');
    });

    test('should update existing plugins with new data', () => {
      game.plugins = [
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foo',
          isActive: true,
          messages: [{ type: 'warn', text: '', language: 'en', condition: '' }]
        })
      ];

      game.setSortedPlugins([
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foo',
          crc: 0xdeadbeef
        })
      ]);

      expect(game.plugins[0].crc).toBe(0xdeadbeef);
      expect(game.plugins[0].isActive).toBe(false);
    });

    test('should reorder plugins to given order', () => {
      game.plugins = [
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foo'
        }),
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'bar'
        })
      ];

      game.setSortedPlugins([
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'bar'
        }),
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foo'
        })
      ]);

      expect(game.plugins[0].name).toBe('bar');
      expect(game.plugins[1].name).toBe('foo');
    });

    test('should store old load order', () => {
      game.plugins = [
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foo'
        }),
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'bar'
        })
      ];

      game.setSortedPlugins([
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'bar'
        }),
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foo'
        })
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
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foo'
        })
      ]);
    });
  });

  describe('#applySort', () => {
    let game: Game;

    beforeEach(() => {
      game = new Game(gameData, l10n);
    });

    test('should delete the stored old load order', () => {
      game.oldLoadOrder = gameData.plugins.map(p => new Plugin(p));

      game.applySort();

      expect(game.oldLoadOrder).toEqual([]);
    });
  });

  describe('#cancelSort', () => {
    let game: Game;

    beforeEach(() => {
      game = new Game(gameData, l10n);
    });

    test('should set the current load order to the given plugins', () => {
      const oldLoadOrder = [
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foo'
        }),
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'bar'
        }),
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'baz'
        })
      ];
      game.oldLoadOrder = oldLoadOrder;
      game.plugins = [oldLoadOrder[2], oldLoadOrder[1], oldLoadOrder[0]];

      game.cancelSort(
        [{ name: 'baz', loadOrderIndex: 0 }, { name: 'foo' }],
        []
      );

      expect(game.plugins).toEqual([oldLoadOrder[2], oldLoadOrder[0]]);
    });

    test('should delete the stored old load order', () => {
      game.oldLoadOrder = [
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foo'
        }),
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'bar'
        })
      ];

      game.cancelSort([], []);

      expect(game.oldLoadOrder).toEqual([]);
    });

    test('should set plugin load order indices using the array passed as the first parameter', () => {
      game.oldLoadOrder = [
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foo',
          loadOrderIndex: 1
        }),
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'bar',
          loadOrderIndex: 0
        })
      ];

      game.cancelSort(
        [
          { name: 'bar', loadOrderIndex: 0 },
          { name: 'foo', loadOrderIndex: 2 }
        ],
        []
      );

      expect(game.plugins[0].name).toBe('bar');
      expect(game.plugins[0].loadOrderIndex).toBe(0);
      expect(game.plugins[1].name).toBe('foo');
      expect(game.plugins[1].loadOrderIndex).toBe(2);
    });

    test('should set the general messages to the second passed parameter', () => {
      game.oldLoadOrder = [
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foo',
          loadOrderIndex: 1
        }),
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'bar',
          loadOrderIndex: 0
        })
      ];

      const messages = [
        {
          type: 'say',
          text: 'foo',
          language: 'en',
          condition: ''
        }
      ];

      game.cancelSort([], messages);

      expect(game.generalMessages).toEqual(messages);
    });
  });

  describe('#clearMetadata', () => {
    let game: Game;

    beforeEach(() => {
      game = new Game(gameData, l10n);
    });

    test('should delete stored userlist data for existing plugins', () => {
      game.plugins = [
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foo',
          userlist: {
            name: '',
            after: [],
            req: [],
            inc: [],
            msg: [],
            tag: [],
            dirty: [],
            clean: [],
            url: []
          }
        })
      ];

      game.clearMetadata([
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foo'
        })
      ]);

      expect(game.plugins[0].userlist).toBe(undefined);
    });

    test('should update existing plugin data', () => {
      game.plugins = [
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foo',
          isActive: true
        })
      ];

      game.clearMetadata([
        new Plugin({
          ...defaultDerivedPluginMetadata,
          name: 'foo',
          crc: 0xdeadbeef
        })
      ]);

      expect(game.plugins[0].crc).toBe(0xdeadbeef);
      expect(game.plugins[0].isActive).toBe(false);
    });
  });
});
