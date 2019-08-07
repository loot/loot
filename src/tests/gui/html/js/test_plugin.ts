/* eslint-disable no-self-assign */
import { Plugin } from '../../../../gui/html/js/plugin';
import Filters from '../../../../gui/html/js/filters';
import Translator from '../../../../gui/html/js/translator';

const defaultDerivedPluginMetadata = {
  name: 'test',
  isActive: false,
  isDirty: false,
  isEmpty: false,
  isMaster: false,
  isLightMaster: false,
  loadsArchive: false,
  messages: [],
  suggestedTags: [],
  currentTags: []
};

const defaultPluginMetadata = {
  name: 'test',
  enabled: false,
  after: [],
  req: [],
  inc: [],
  msg: [],
  tag: [],
  dirty: [],
  clean: [],
  url: []
};

/* eslint-disable no-unused-expressions */
describe('Plugin', () => {
  describe('#Plugin()', () => {
    test('should not throw if some members are undefined', () => {
      expect(() => {
        new Plugin(defaultDerivedPluginMetadata); // eslint-disable-line no-new
      }).not.toThrow();
    });

    test("should set name to passed key's value", () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      expect(plugin.name).toBe('test');
    });

    test('should set crc to zero if no key was passed', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      expect(plugin.crc).toBe(0);
    });

    test("should set crc to passed key's value", () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        crc: 0xdeadbeef
      });

      expect(plugin.crc).toBe(0xdeadbeef);
    });

    test('should set version to an empty string if no key was passed', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      expect(plugin.version).toBe('');
    });

    test("should set version to passed key's value", () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        version: 'foo'
      });

      expect(plugin.version).toBe('foo');
    });

    test("should set isActive to passed key's value", () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        isActive: true
      });

      expect(plugin.isActive).toBe(true);
    });

    test("should set isEmpty to passed key's value", () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        isEmpty: true
      });

      expect(plugin.isEmpty).toBe(true);
    });

    test("should set isMaster to passed key's value", () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        isMaster: true
      });

      expect(plugin.isMaster).toBe(true);
    });

    test("should set loadsArchive to passed key's value", () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        loadsArchive: true
      });

      expect(plugin.loadsArchive).toBe(true);
    });

    test('should set masterlist value to undefined if no key was passed', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      expect(plugin.masterlist).toBe(undefined);
    });

    test("should set masterlist to passed key's value", () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        masterlist: defaultPluginMetadata
      });

      expect(plugin.masterlist).toEqual(defaultPluginMetadata);
    });

    test('should set userlist value to undefined if no key was passed', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      expect(plugin.userlist).toBe(undefined);
    });

    test("should set userlist to passed key's value", () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        userlist: defaultPluginMetadata
      });

      expect(plugin.userlist).toEqual(defaultPluginMetadata);
    });

    test('should set group to default if no key was passed', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      expect(plugin.group).toBe('default');
    });

    test("should set group to passed key's value", () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        group: 'group1'
      });

      expect(plugin.group).toBe('group1');
    });

    test("should set messages to passed key's value", () => {
      const messages = [
        {
          type: 'say',
          text: 'test message',
          language: 'en',
          condition: ''
        }
      ];
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        messages
      });

      expect(plugin.messages).toEqual(messages);
    });

    test("should set currentTags to passed key's value", () => {
      const currentTags = [{ name: 'Delev', isAddition: false, condition: '' }];
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        currentTags
      });

      expect(plugin.currentTags).toEqual(currentTags);
    });

    test("should set suggestedTags to passed key's value", () => {
      const suggestedTags = [
        { name: 'Delev', isAddition: false, condition: '' }
      ];
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        suggestedTags
      });

      expect(plugin.suggestedTags).toEqual(suggestedTags);
    });

    test("should set isDirty to passed key's value", () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        isDirty: true
      });

      expect(plugin.isDirty).toBe(true);
    });

    test('should set cleanedWith value to an empty string if no key was passed', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      expect(plugin.cleanedWith).toBe('');
    });

    test("should set cleanedWith to passed key's value", () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        cleanedWith: 'TES5Edit 3.11'
      });

      expect(plugin.cleanedWith).toBe('TES5Edit 3.11');
    });

    test('should set id to the plugins name without spaces', () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        name: 'test plugin name'
      });

      expect(plugin.id).toBe('testpluginname');
    });

    test('should set isEditorOpen to false', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      expect(plugin.isEditorOpen).toBe(false);
    });

    test('should set isSearchResult to false', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      expect(plugin.isSearchResult).toBe(false);
    });
  });

  describe('#update()', () => {
    let plugin: Plugin;

    beforeEach(() => {
      plugin = new Plugin(defaultDerivedPluginMetadata);
    });

    test("should throw if the argument's name property doesn't match the plugin's name", () => {
      expect(() => {
        plugin.update({ ...defaultDerivedPluginMetadata, name: 'other test' });
      }).toThrow(Error);
    });

    test("should set property values for all the given argument's properties", () => {
      const updatedPlugin = {
        ...defaultDerivedPluginMetadata,
        crc: 0xdeadbeef,
        version: '1.0.0',
        isActive: true,
        masterlist: defaultPluginMetadata,
        userlist: defaultPluginMetadata,
        group: 'default',
        loadOrderIndex: 1,
        cleanedWith: 'xEdit',
        messages: []
      };
      plugin.update(updatedPlugin);

      expect(plugin.crc).toBe(updatedPlugin.crc);
      expect(plugin.version).toBe(updatedPlugin.version);
      expect(plugin.isActive).toBe(updatedPlugin.isActive);
      expect(plugin.masterlist).toBe(updatedPlugin.masterlist);
      expect(plugin.userlist).toBe(updatedPlugin.userlist);
      expect(plugin.group).toBe(updatedPlugin.group);
      expect(plugin.loadOrderIndex).toBe(updatedPlugin.loadOrderIndex);
      expect(plugin.cleanedWith).toBe(updatedPlugin.cleanedWith);
      expect(plugin.messages).toEqual(updatedPlugin.messages);
    });

    test('should set version to an empty string if not given', () => {
      plugin.version = '1.0.0';

      plugin.update(defaultDerivedPluginMetadata);

      expect(plugin.version).toBe('');
    });

    test('should set crc to 0 if not given', () => {
      plugin.crc = 0xdeadbeef;

      plugin.update(defaultDerivedPluginMetadata);

      expect(plugin.crc).toBe(0);
    });

    test('should set group to default if not given', () => {
      plugin.group = 'DLC';

      plugin.update(defaultDerivedPluginMetadata);

      expect(plugin.group).toBe('default');
    });

    test('should set loadOrderIndex to be undefined if not given', () => {
      plugin.loadOrderIndex = 1;

      plugin.update(defaultDerivedPluginMetadata);

      expect(plugin.loadOrderIndex).toBe(undefined);
    });

    test('should set cleanedWith to an empty string if not given', () => {
      plugin.cleanedWith = 'xEdit';

      plugin.update(defaultDerivedPluginMetadata);

      expect(plugin.cleanedWith).toBe('');
    });

    test('should set masterlist to be undefined if not given', () => {
      plugin.masterlist = defaultPluginMetadata;

      plugin.update(defaultDerivedPluginMetadata);

      expect(plugin.masterlist).toBe(undefined);
    });

    test('should set userlist to be undefined if not given', () => {
      plugin.userlist = defaultPluginMetadata;

      plugin.update(defaultDerivedPluginMetadata);

      expect(plugin.userlist).toBe(undefined);
    });
  });

  describe('#tagFromRowData()', () => {
    test('should return a raw metadata object if passed a row data object that removes a tag', () => {
      expect(
        Plugin.tagFromRowData({
          condition: 'foo',
          type: 'remove',
          name: 'bar'
        })
      ).toEqual({
        condition: 'foo',
        name: 'bar',
        isAddition: false
      });
    });

    test('should return a raw metadata object if passed a row data object that adds a tag', () => {
      expect(
        Plugin.tagFromRowData({
          condition: 'foo',
          type: 'add',
          name: 'bar'
        })
      ).toEqual({
        condition: 'foo',
        name: 'bar',
        isAddition: true
      });
    });
  });

  describe('#tagToRowData()', () => {
    test('should return a row data object if passed a raw metadata object that removes a tag', () => {
      expect(
        Plugin.tagToRowData({
          condition: 'foo',
          name: 'bar',
          isAddition: false
        })
      ).toEqual({
        condition: 'foo',
        type: 'remove',
        name: 'bar'
      });
    });

    test('should return a row data object if passed a raw metadata object that adds a tag', () => {
      expect(
        Plugin.tagToRowData({
          condition: 'foo',
          name: 'bar',
          isAddition: true
        })
      ).toEqual({
        condition: 'foo',
        type: 'add',
        name: 'bar'
      });
    });
  });

  describe('#messages', () => {
    // It's not worth the hassle of defining and checking the event type in test
    // code.
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    let handleEvent: (evt: any) => void;

    afterEach(() => {
      document.removeEventListener('loot-plugin-message-change', handleEvent);
    });

    test('getting messages if the array is empty should return an empty array', () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        messages: []
      });

      expect(plugin.messages).toBeInstanceOf(Array);
      expect(plugin.messages.length).toBe(0);
    });

    test('getting messages should return any that are set', () => {
      const messages = [
        {
          type: 'say',
          text: 'test message',
          language: 'en',
          condition: ''
        }
      ];
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        messages
      });

      expect(plugin.messages).toEqual(messages);
    });

    test('setting messages should store any set', () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        messages: []
      });
      const messages = [
        {
          type: 'say',
          text: 'test message',
          language: 'en',
          condition: ''
        }
      ];

      plugin.messages = messages;

      expect(plugin.messages).toEqual(messages);
    });

    test('setting messages should not fire an event if no messages were changed', done => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        messages: [
          {
            type: 'say',
            text: 'test message',
            language: 'en',
            condition: ''
          }
        ]
      });

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener('loot-plugin-message-change', handleEvent);

      plugin.messages = plugin.messages;

      setTimeout(done, 100);
    });

    test('setting messages should fire an event if the messages were changed', done => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        messages: []
      });
      const messages = [
        {
          type: 'error',
          text: 'test message',
          language: 'en',
          condition: ''
        }
      ];

      handleEvent = evt => {
        expect(evt.detail.pluginId).toBe(plugin.id);
        expect(evt.detail.totalDiff).toBe(1);
        expect(evt.detail.warningDiff).toBe(0);
        expect(evt.detail.errorDiff).toBe(1);
        done();
      };

      document.addEventListener('loot-plugin-message-change', handleEvent);

      plugin.messages = messages;
    });
  });

  describe('#isDirty', () => {
    // It's not worth the hassle of defining and checking the event type in test
    // code.
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    let handleEvent: (evt: any) => void;

    afterEach(() => {
      document.removeEventListener(
        'loot-plugin-cleaning-data-change',
        handleEvent
      );
    });

    test('getting value should return false if isDirty has not been set in the constructor', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      expect(plugin.isDirty).toBe(false);
    });

    test('getting value should return true if isDirty is set to true in the constructor', () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        isDirty: true
      });

      expect(plugin.isDirty).toBe(true);
    });

    test('setting value should store set value', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      plugin.isDirty = true;

      expect(plugin.isDirty).toBe(true);
    });

    test('setting value to the current value should not fire an event', done => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener(
        'loot-plugin-cleaning-data-change',
        handleEvent
      );

      plugin.isDirty = plugin.isDirty;

      setTimeout(done, 100);
    });

    test('setting value not equal to the current value should fire an event', done => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      handleEvent = evt => {
        expect(evt.detail.isDirty).toBe(true);
        done();
      };

      document.addEventListener(
        'loot-plugin-cleaning-data-change',
        handleEvent
      );

      plugin.isDirty = !plugin.isDirty;
    });
  });

  describe('#cleanedWith', () => {
    // It's not worth the hassle of defining and checking the event type in test
    // code.
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    let handleEvent: (evt: any) => void;

    afterEach(() => {
      document.removeEventListener(
        'loot-plugin-cleaning-data-change',
        handleEvent
      );
    });

    test('getting value should return an empty string if cleanedWith has not been set in the constructor', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      expect(plugin.cleanedWith).toBe('');
    });

    test('getting value should return a string if cleanedWith is set in the constructor', () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        cleanedWith: 'utility'
      });

      expect(plugin.cleanedWith).toBe('utility');
    });

    test('setting value should store set value', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      plugin.cleanedWith = 'utility';

      expect(plugin.cleanedWith).toBe('utility');
    });

    test('setting value to the current value should not fire an event', done => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener(
        'loot-plugin-cleaning-data-change',
        handleEvent
      );

      plugin.cleanedWith = plugin.cleanedWith;

      setTimeout(done, 100);
    });

    test('setting value not equal to the current value should fire an event', done => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      handleEvent = evt => {
        expect(evt.detail.cleanedWith).toBe('utility');
        done();
      };

      document.addEventListener(
        'loot-plugin-cleaning-data-change',
        handleEvent
      );

      plugin.cleanedWith = 'utility';
    });
  });

  describe('#crc', () => {
    // It's not worth the hassle of defining and checking the event type in test
    // code.
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    let handleEvent: (evt: any) => void;

    afterEach(() => {
      document.removeEventListener(
        'loot-plugin-card-content-change',
        handleEvent
      );
    });

    test('getting value should return 0 if crc has not been set in the constructor', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      expect(plugin.crc).toBe(0);
    });

    test('getting value should return 0xDEADBEEF if it was set in the constructor', () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        crc: 0xdeadbeef
      });

      expect(plugin.crc).toBe(0xdeadbeef);
    });

    test('setting value should store set value', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      plugin.crc = 0xdeadbeef;

      expect(plugin.crc).toBe(0xdeadbeef);
    });

    test('setting value to the current value should not fire an event', done => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener('loot-plugin-card-content-change', handleEvent);

      plugin.crc = plugin.crc;

      setTimeout(done, 100);
    });

    test('setting value not equal to the current value should fire an event', done => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      handleEvent = evt => {
        expect(evt.detail.pluginId).toBe(plugin.id);
        done();
      };

      document.addEventListener('loot-plugin-card-content-change', handleEvent);

      plugin.crc = 0xdeadbeef;
    });
  });

  describe('#currentTags', () => {
    // It's not worth the hassle of defining and checking the event type in test
    // code.
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    let handleEvent: (evt: any) => void;

    afterEach(() => {
      document.removeEventListener(
        'loot-plugin-card-content-change',
        handleEvent
      );
    });

    test('getting value should return an empty array if tags have not been set in the constructor', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      expect(plugin.currentTags.length).toBe(0);
    });

    test('getting value should return any tags that are set', () => {
      const currentTags = [{ name: 'Delev', isAddition: false, condition: '' }];
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        currentTags
      });

      expect(plugin.currentTags).toEqual(currentTags);
    });

    test('setting value should store set value', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);
      const tags = [{ name: 'Delev', isAddition: false, condition: '' }];

      plugin.currentTags = tags;

      expect(plugin.currentTags).toEqual(tags);
    });

    test('setting value to the current value should not fire an event', done => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener('loot-plugin-card-content-change', handleEvent);

      plugin.currentTags = plugin.currentTags;

      setTimeout(done, 100);
    });

    test('setting value not equal to the current value should fire an event', done => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);
      const tags = [{ name: 'Delev', isAddition: false, condition: '' }];

      handleEvent = evt => {
        expect(evt.detail.pluginId).toBe(plugin.id);
        done();
      };

      document.addEventListener('loot-plugin-card-content-change', handleEvent);

      plugin.currentTags = tags;
    });
  });

  describe('#suggestedTags', () => {
    // It's not worth the hassle of defining and checking the event type in test
    // code.
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    let handleEvent: (evt: any) => void;

    afterEach(() => {
      document.removeEventListener(
        'loot-plugin-card-content-change',
        handleEvent
      );
    });

    test('getting value should return an empty array if tags have not been set in the constructor', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      expect(plugin.suggestedTags.length).toBe(0);
    });

    test('getting value should return any tags that are set', () => {
      const suggestedTags = [
        { name: 'Delev', isAddition: false, condition: '' }
      ];
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        suggestedTags
      });

      expect(plugin.suggestedTags).toEqual(suggestedTags);
    });

    test('setting value should store set value', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);
      const tags = [{ name: 'Delev', isAddition: false, condition: '' }];

      plugin.suggestedTags = tags;

      expect(plugin.suggestedTags).toEqual(tags);
    });

    test('setting value to the current value should not fire an event', done => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener('loot-plugin-card-content-change', handleEvent);

      plugin.suggestedTags = plugin.suggestedTags;

      setTimeout(done, 100);
    });

    test('setting value not equal to the current value should fire an event', done => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);
      const tags = [{ name: 'Delev', isAddition: false, condition: '' }];

      handleEvent = evt => {
        expect(evt.detail.pluginId).toBe(plugin.id);
        done();
      };

      document.addEventListener('loot-plugin-card-content-change', handleEvent);

      plugin.suggestedTags = tags;
    });
  });

  describe('#userlist', () => {
    // It's not worth the hassle of defining and checking the event type in test
    // code.
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    let handleEvent: (evt: any) => void;

    afterEach(() => {
      document.removeEventListener(
        'loot-plugin-item-content-change',
        handleEvent
      );
    });

    test('getting value should return undefined if it has not been set in the constructor', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      expect(plugin.userlist).toBe(undefined);
    });

    test('getting value should return the value that was set', () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        userlist: defaultPluginMetadata
      });

      expect(plugin.userlist).toEqual(defaultPluginMetadata);
    });

    test('setting value should store set value', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      plugin.userlist = defaultPluginMetadata;

      expect(plugin.userlist).toEqual(defaultPluginMetadata);
    });

    test('setting value to the current value should not fire an event', done => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener('loot-plugin-item-content-change', handleEvent);

      plugin.userlist = plugin.userlist;

      setTimeout(done, 100);
    });

    test('setting value not equal to the current value should fire an event', done => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      handleEvent = evt => {
        expect(evt.detail.pluginId).toBe(plugin.id);
        expect(evt.detail.group).toBe(plugin.group);
        expect(evt.detail.isEditorOpen).toBe(plugin.isEditorOpen);
        expect(evt.detail.hasUserEdits).toBe(plugin.hasUserEdits);
        done();
      };

      document.addEventListener('loot-plugin-item-content-change', handleEvent);

      plugin.userlist = { ...defaultPluginMetadata, group: 'test' };
    });
  });

  describe('#group', () => {
    // It's not worth the hassle of defining and checking the event type in test
    // code.
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    let handleEvent: (evt: any) => void;

    afterEach(() => {
      document.removeEventListener(
        'loot-plugin-item-content-change',
        handleEvent
      );
    });

    test('getting value should return default if it has not been set in the constructor', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      expect(plugin.group).toBe('default');
    });

    test('getting value should return the value that was set', () => {
      const plugin = new Plugin({
        ...defaultDerivedPluginMetadata,
        group: 'group1'
      });

      expect(plugin.group).toBe('group1');
    });

    test('setting value should store set value', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      plugin.group = 'group1';

      expect(plugin.group).toBe('group1');
    });

    test('setting value to the current value should not fire an event', done => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener('loot-plugin-item-content-change', handleEvent);

      plugin.group = plugin.group;

      setTimeout(done, 100);
    });

    test('setting value not equal to the current value should fire an event', done => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      handleEvent = evt => {
        expect(evt.detail.pluginId).toBe(plugin.id);
        expect(evt.detail.group).toBe(plugin.group);
        expect(evt.detail.isEditorOpen).toBe(plugin.isEditorOpen);
        expect(evt.detail.hasUserEdits).toBe(plugin.hasUserEdits);
        done();
      };

      document.addEventListener('loot-plugin-item-content-change', handleEvent);

      plugin.group = 'group1';
    });
  });

  describe('#isEditorOpen', () => {
    // It's not worth the hassle of defining and checking the event type in test
    // code.
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    let handleEvent: (evt: any) => void;

    afterEach(() => {
      document.removeEventListener(
        'loot-plugin-item-content-change',
        handleEvent
      );
    });

    test('getting value should return false if it has not been set in the constructor', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      expect(plugin.isEditorOpen).toBe(false);
    });

    test('setting value should store set value', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      plugin.isEditorOpen = true;

      expect(plugin.isEditorOpen).toBe(true);
    });

    test('setting value to the current value should not fire an event', done => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener('loot-plugin-item-content-change', handleEvent);

      plugin.isEditorOpen = plugin.isEditorOpen;

      setTimeout(done, 100);
    });

    test('setting value not equal to the current value should fire an event', done => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      handleEvent = evt => {
        expect(evt.detail.pluginId).toBe(plugin.id);
        expect(evt.detail.group).toBe(plugin.group);
        expect(evt.detail.isEditorOpen).toBe(plugin.isEditorOpen);
        expect(evt.detail.hasUserEdits).toBe(plugin.hasUserEdits);
        done();
      };

      document.addEventListener('loot-plugin-item-content-change', handleEvent);

      plugin.isEditorOpen = true;
    });
  });

  describe('#isSearchResult', () => {
    // It's not worth the hassle of defining and checking the event type in test
    // code.
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    let handleEvent: (evt: any) => void;

    afterEach(() => {
      document.removeEventListener(
        'loot-plugin-card-styling-change',
        handleEvent
      );
    });

    test('getting value should return false if it has not been set in the constructor', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      expect(plugin.isSearchResult).toBe(false);
    });

    test('setting value should store set value', () => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      plugin.isSearchResult = true;

      expect(plugin.isSearchResult).toBe(true);
    });

    test('setting value to the current value should not fire an event', done => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener('loot-plugin-card-styling-change', handleEvent);

      plugin.isSearchResult = plugin.isSearchResult;

      setTimeout(done, 100);
    });

    test('setting value not equal to the current value should fire an event', done => {
      const plugin = new Plugin(defaultDerivedPluginMetadata);

      handleEvent = evt => {
        expect(evt.detail.pluginId).toBe(plugin.id);
        done();
      };

      document.addEventListener('loot-plugin-card-styling-change', handleEvent);

      plugin.isSearchResult = true;
    });
  });

  describe('#getCardContent()', () => {
    let plugin: Plugin;
    beforeEach(() => {
      plugin = new Plugin(defaultDerivedPluginMetadata);
    });

    test('should succeed if passed a filters object', () => {
      expect(() => {
        plugin.getCardContent(new Filters(new Translator()));
      }).not.toThrow();
    });
  });
});

describe('PluginCardContent', () => {
  let plugin: Plugin;
  let filters: Filters;

  beforeEach(() => {
    plugin = new Plugin({
      ...defaultDerivedPluginMetadata,
      name: 'test',
      version: 'foo',
      messages: [
        {
          type: 'say',
          text: 'test message',
          language: 'en',
          condition: ''
        },
        {
          type: 'warn',
          text: 'do not clean',
          language: 'en',
          condition: ''
        }
      ]
    });

    filters = new Filters(new Translator());
  });

  describe('#name', () => {
    test("getting value should return plugin's value", () => {
      expect(plugin.getCardContent(filters).name).toBe(plugin.name);
    });
  });

  describe('#isActive', () => {
    test("getting value should return plugin's value", () => {
      expect(plugin.getCardContent(filters).isActive).toBe(plugin.isActive);
    });
  });

  describe('#isEmpty', () => {
    test("getting value should return plugin's value", () => {
      expect(plugin.getCardContent(filters).isEmpty).toBe(plugin.isEmpty);
    });
  });

  describe('#isMaster', () => {
    test("getting value should return plugin's value", () => {
      expect(plugin.getCardContent(filters).isMaster).toBe(plugin.isMaster);
    });
  });

  describe('#loadsArchive', () => {
    test("getting value should return plugin's value", () => {
      expect(plugin.getCardContent(filters).loadsArchive).toBe(
        plugin.loadsArchive
      );
    });
  });

  describe('#version', () => {
    test("getting value should return plugin's value if the version filter is not enabled", () => {
      expect(plugin.getCardContent(filters).version).toBe(plugin.version);
    });

    test('getting value should return empty string if the version filter is enabled', () => {
      filters.hideVersionNumbers = true;
      expect(plugin.getCardContent(filters).version).toBe('');
    });
  });

  describe('#tags', () => {
    test('should return an object containing empty strings if no tags are set', () => {
      expect(plugin.getCardContent(filters).tags).toEqual({
        current: '',
        add: '',
        remove: ''
      });
    });

    test('should return an object containing strings of comma-separated tag names if tags are set', () => {
      plugin.currentTags = [
        { name: 'C.Climate', isAddition: false, condition: '' }
      ];
      plugin.suggestedTags = [
        { name: 'Relev', isAddition: true, condition: '' },
        { name: 'Delev', isAddition: true, condition: '' },
        { name: 'Names', isAddition: true, condition: '' },
        { name: 'C.Climate', isAddition: false, condition: '' },
        { name: 'Actor.ABCS', isAddition: false, condition: '' }
      ];

      expect(plugin.getCardContent(filters).tags).toEqual({
        current: 'C.Climate',
        add: 'Relev, Delev, Names',
        remove: 'C.Climate'
      });
    });

    test('should return an object containing empty strings if tags are set and the tags filter is enabled', () => {
      plugin.suggestedTags = [
        { name: 'Relev', isAddition: true, condition: '' },
        { name: 'Delev', isAddition: true, condition: '' },
        { name: 'Names', isAddition: true, condition: '' },
        { name: 'C.Climate', isAddition: false, condition: '' },
        { name: 'Actor.ABCS', isAddition: false, condition: '' }
      ];
      filters.hideBashTags = true;

      expect(plugin.getCardContent(filters).tags).toEqual({
        current: '',
        add: '',
        remove: ''
      });
    });

    test('should not output a tag if it appears as removed but not current', () => {
      plugin.suggestedTags = [
        { name: 'Relev', isAddition: false, condition: '' }
      ];

      expect(plugin.getCardContent(filters).tags).toEqual({
        current: '',
        add: '',
        remove: ''
      });
    });

    test('should not output a tag if it appears as both added and removed', () => {
      plugin.suggestedTags = [
        { name: 'Relev', isAddition: true, condition: '' },
        { name: 'Relev', isAddition: false, condition: '' }
      ];

      expect(plugin.getCardContent(filters).tags).toEqual({
        current: '',
        add: '',
        remove: ''
      });
    });

    test('should output a tag in the current and removed strings if it appears as both current and removed', () => {
      plugin.currentTags = [
        { name: 'Relev', isAddition: false, condition: '' }
      ];
      plugin.suggestedTags = [
        { name: 'Relev', isAddition: false, condition: '' }
      ];

      expect(plugin.getCardContent(filters).tags).toEqual({
        current: 'Relev',
        add: '',
        remove: 'Relev'
      });
    });

    test('should output a tag in the current string if it appears as both current and added', () => {
      plugin.currentTags = [
        { name: 'Relev', isAddition: false, condition: '' }
      ];
      plugin.suggestedTags = [
        { name: 'Relev', isAddition: true, condition: '' }
      ];

      expect(plugin.getCardContent(filters).tags).toEqual({
        current: 'Relev',
        add: '',
        remove: ''
      });
    });
  });

  describe('#crc', () => {
    test('should return an empty string if crc is undefined', () => {
      expect(plugin.getCardContent(filters).crc).toBe('');
    });

    test('should return an empty string if crc is zero', () => {
      plugin.crc = 0;

      expect(plugin.getCardContent(filters).crc).toBe('');
    });

    test('should return crc value as string if non zero', () => {
      plugin.crc = 0xdeadbeef;

      expect(plugin.getCardContent(filters).crc).toBe('DEADBEEF');
    });

    test('should return an empty string if crc is non-zero and the CRC filter is enabled', () => {
      plugin.crc = 0xdeadbeef;
      filters.hideCRCs = true;

      expect(plugin.getCardContent(filters).crc).toBe('');
    });

    test('should pad crc value to eight digits', () => {
      plugin.crc = 0xbeef;

      expect(plugin.getCardContent(filters).crc).toBe('0000BEEF');
    });
  });

  describe('#messages', () => {
    test("should return message objects mapped from the plugin's message objects", () => {
      expect(plugin.getCardContent(filters).messages).toEqual(plugin.messages);
    });

    test('should return an array missing the note message when the notes filter is enabled', () => {
      filters.hideNotes = true;
      expect(plugin.getCardContent(filters).messages).toEqual([
        plugin.messages[1]
      ]);
    });

    test('should return an array missing the "do not clean" message when the "do not clean" messages filter is enabled', () => {
      filters.hideDoNotCleanMessages = true;
      expect(plugin.getCardContent(filters).messages).toEqual([
        plugin.messages[0]
      ]);
    });

    test('should return an empty array when the all messages filter is enabled', () => {
      filters.hideAllPluginMessages = true;
      expect(plugin.getCardContent(filters).messages).toEqual([]);
    });
  });

  describe('#containsText()', () => {
    test('should return true if argument is an empty string', () => {
      expect(plugin.getCardContent(filters).containsText('')).toBe(true);
    });

    test('should search name case-insensitively', () => {
      expect(plugin.getCardContent(filters).containsText('Tes')).toBe(true);
    });

    test('should search CRC case-insensitively', () => {
      plugin.crc = 0xdeadbeef;
      expect(plugin.getCardContent(filters).containsText('dead')).toBe(true);
    });

    test('should search current tags case-insensitively', () => {
      plugin.currentTags = [
        { name: 'Relev', isAddition: true, condition: '' },
        { name: 'Delev', isAddition: true, condition: '' },
        { name: 'Names', isAddition: true, condition: '' },
        { name: 'C.Climate', isAddition: false, condition: '' },
        { name: 'Actor.ABCS', isAddition: false, condition: '' }
      ];
      expect(plugin.getCardContent(filters).containsText('climate')).toBe(true);
    });

    test('should search added tags case-insensitively', () => {
      plugin.suggestedTags = [
        { name: 'Relev', isAddition: true, condition: '' },
        { name: 'Delev', isAddition: true, condition: '' },
        { name: 'Names', isAddition: true, condition: '' },
        { name: 'C.Climate', isAddition: false, condition: '' },
        { name: 'Actor.ABCS', isAddition: false, condition: '' }
      ];
      expect(plugin.getCardContent(filters).containsText('relev')).toBe(true);
    });

    test('should search removed tags case-insensitively', () => {
      plugin.currentTags = [
        { name: 'Actor.ABCS', isAddition: false, condition: '' }
      ];
      plugin.suggestedTags = [
        { name: 'Relev', isAddition: true, condition: '' },
        { name: 'Delev', isAddition: true, condition: '' },
        { name: 'Names', isAddition: true, condition: '' },
        { name: 'C.Climate', isAddition: false, condition: '' },
        { name: 'Actor.ABCS', isAddition: false, condition: '' }
      ];
      expect(plugin.getCardContent(filters).containsText('.abc')).toBe(true);
    });

    test('should search message content case-insensitively', () => {
      expect(plugin.getCardContent(filters).containsText('Clean')).toBe(true);
    });

    test('should not find text that is not present', () => {
      expect(plugin.getCardContent(filters).containsText('say')).toBe(false);
    });
  });
});
