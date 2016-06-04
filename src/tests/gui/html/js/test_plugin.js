'use strict';

/* Mock the filters class. */
class Filters {
  constructor() {
    this.hideVersionNumbers = false;
    this.hideCRCs = false;
    this.hideBashTags = false;
    this.hideAllPluginMessages = false;
    this.hideNotes = false;
    this.hideDoNotCleanMessages = false;
  }

  messageFilter(message, index) {
    if (this.hideAllPluginMessages) {
      return false;
    }

    if (this.hideNotes && index === 0) {
      return false;
    }

    if (this.hideDoNotCleanMessages && index === 1) {
      return false;
    }

    return true;
  }
}

describe('Plugin', () => {
  describe('#Plugin()', () => {
    it('should throw if nothing is passed', () => {
      (() => { new loot.Plugin(); }).should.throw(); // eslint-disable-line no-new
    });

    it('should throw if an object with no name key is passed', () => {
      (() => { new loot.Plugin({}); }).should.throw(); // eslint-disable-line no-new
    });

    it('should not throw if some members are undefined', () => {
      (() => { new loot.Plugin({ name: 'test' }); }).should.not.throw(); // eslint-disable-line no-new
    });

    it('should set name to passed key\'s value', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.name.should.equal('test');
    });

    it('should set crc to zero if no key was passed', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.crc.should.equal(0);
    });

    it('should set crc to passed key\'s value', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        crc: 0xDEADBEEF,
      });

      plugin.crc.should.equal(0xDEADBEEF);
    });

    it('should set version to an empty string if no key was passed', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.version.should.equal('');
    });

    it('should set version to passed key\'s value', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        version: 'foo',
      });

      plugin.version.should.equal('foo');
    });

    it('should set isActive value to false if no key was passed', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.isActive.should.be.false();
    });

    it('should set isActive to passed key\'s value', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        isActive: true,
      });

      plugin.isActive.should.be.true();
    });

    it('should set isEmpty value to false if no key was passed', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.isEmpty.should.be.false();
    });

    it('should set isEmpty to passed key\'s value', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        isEmpty: true,
      });

      plugin.isEmpty.should.be.true();
    });

    it('should set isMaster value to false if no key was passed', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.isMaster.should.be.false();
    });

    it('should set isMaster to passed key\'s value', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        isMaster: true,
      });

      plugin.isMaster.should.be.true();
    });

    it('should set loadsArchive value to false if no key was passed', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.loadsArchive.should.be.false();
    });

    it('should set loadsArchive to passed key\'s value', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        loadsArchive: true,
      });

      plugin.loadsArchive.should.be.true();
    });

    it('should set masterlist value to undefined if no key was passed', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      should(plugin.masterlist).be.undefined();
    });

    it('should set masterlist to passed key\'s value', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        masterlist: {},
      });

      plugin.masterlist.should.be.deepEqual({});
    });

    it('should set userlist value to undefined if no key was passed', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      should(plugin.userlist).be.undefined();
    });

    it('should set userlist to passed key\'s value', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        userlist: {},
      });

      plugin.userlist.should.be.deepEqual({});
    });

    it('should set priority to 0 if no key was passed', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.priority.should.equal(0);
    });

    it('should set priority to passed key\'s value', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        priority: 5,
      });

      plugin.priority.should.equal(5);
    });

    it('should set isPriorityGlobal value to false if no key was passed', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.isPriorityGlobal.should.be.false();
    });

    it('should set isPriorityGlobal to passed key\'s value', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        isPriorityGlobal: true,
      });

      plugin.isPriorityGlobal.should.be.true();
    });

    it('should set messages value to an empty array if no key was passed', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.messages.length.should.equal(0);
    });

    it('should set messages to passed key\'s value', () => {
      const messages = [{
        type: 'say',
        content: 'test message',
      }];
      const plugin = new loot.Plugin({
        name: 'test',
        messages,
      });

      plugin.messages.should.deepEqual(messages);
    });

    it('should set tags value to an empty array if no key was passed', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.tags.length.should.equal(0);
    });

    it('should set tags to passed key\'s value', () => {
      const tags = [{
        name: 'Delev',
      }];
      const plugin = new loot.Plugin({
        name: 'test',
        tags,
      });

      plugin.tags.should.deepEqual(tags);
    });

    it('should set isDirty value to false if no key was passed', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.isDirty.should.be.false();
    });

    it('should set isDirty to passed key\'s value', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        isDirty: true,
      });

      plugin.isDirty.should.be.true();
    });

    it('should set id to the plugins name without spaces', () => {
      const plugin = new loot.Plugin({ name: 'test plugin name' });

      plugin.id.should.equal('testpluginname');
    });

    it('should set isEditorOpen to false', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.isEditorOpen.should.be.false();
    });

    it('should set isConflictFilterChecked to false', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.isConflictFilterChecked.should.be.false();
    });

    it('should set isSearchResult to false', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.isSearchResult.should.be.false();
    });
  });

  describe('#update()', () => {
    let plugin;
    const updatedPlugin = {
      name: 'test',
      foo: 'bar',
      crc: 0xDEADBEEF,
    };

    beforeEach(() => {
      plugin = new loot.Plugin({ name: 'test' });
    });

    it('should do nothing if its argument is undefined', () => {
      plugin.update();

      plugin.should.deepEqual(new loot.Plugin({ name: 'test' }));
    });

    it('should throw if the argument has no name property', () => {
      should(() => { plugin.update({}); }).throw(Error);
    });

    it('should throw if the argument\'s name property doesn\'t match the plugin\'s name', () => {
      should(() => { plugin.update({ name: 'other test' }); }).throw(Error);
    });

    it('should set property values for all the given argument\'s properties', () => {
      plugin.update(updatedPlugin);

      plugin.foo.should.equal(updatedPlugin.foo);
      plugin.crc.should.equal(updatedPlugin.crc);
    });

    it('should not change property values for properties not present in the argument', () => {
      plugin.isActive = true;

      plugin.update(updatedPlugin);

      plugin.foo.should.equal(updatedPlugin.foo);
      plugin.crc.should.equal(updatedPlugin.crc);
      plugin.isActive.should.be.true();
    });

    it('should set explicitly undefined values', () => {
      plugin.isActive = true;

      plugin.update({
        name: plugin.name,
        isActive: undefined,
      });

      should(plugin.isActive).be.undefined();
    });
  });

  describe('#fromJson()', () => {
    it('should return the value object if the JSON is not of the Plugin type', () => {
      const testInputObj = {
        name: 'test',
        crc: 0xDEADBEEF,
      };
      const testInputJson = JSON.stringify(testInputObj);

      JSON.parse(testInputJson, loot.Plugin.fromJson).should.deepEqual(testInputObj);
    });

    it('should return a Plugin object if the JSON is of the Plugin type', () => {
      const testInputObj = {
        name: 'test',
        crc: 0xDEADBEEF,
        __type: 'Plugin',
      };
      const testInputJson = JSON.stringify(testInputObj);

      JSON.parse(testInputJson, loot.Plugin.fromJson).should.be.instanceof(loot.Plugin);
    });
  });

  describe('#tagFromRowData()', () => {
    it('should throw if passed nothing', () => {
      (() => { loot.Plugin.tagFromRowData(); }).should.throw();
    });

    it('should return an empty object if passed nothing', () => {
      (() => { loot.Plugin.tagFromRowData({}); }).should.throw();
    });

    it('should return a raw metadata object if passed a row data object that removes a tag', () => {
      loot.Plugin.tagFromRowData({
        condition: 'foo',
        type: 'remove',
        name: 'bar',
      }).should.deepEqual({
        condition: 'foo',
        name: '-bar',
      });
    });

    it('should return a raw metadata object if passed a row data object that adds a tag', () => {
      loot.Plugin.tagFromRowData({
        condition: 'foo',
        type: 'add',
        name: 'bar',
      }).should.deepEqual({
        condition: 'foo',
        name: 'bar',
      });
    });
  });

  describe('#tagToRowData()', () => {
    it('should throw if passed nothing', () => {
      (() => { loot.Plugin.tagToRowData(); }).should.throw();
    });

    it('should return an empty object if passed nothing', () => {
      (() => { loot.Plugin.tagToRowData({}); }).should.throw();
    });

    it('should return a row data object if passed a raw metadata object that removes a tag', () => {
      loot.Plugin.tagToRowData({
        condition: 'foo',
        name: '-bar',
      }).should.deepEqual({
        condition: 'foo',
        type: 'remove',
        name: 'bar',
      });
    });

    it('should return a row data object if passed a raw metadata object that adds a tag', () => {
      loot.Plugin.tagToRowData({
        condition: 'foo',
        name: 'bar',
      }).should.deepEqual({
        condition: 'foo',
        type: 'add',
        name: 'bar',
      });
    });
  });

  describe('#messages', () => {
    let handleEvent;

    afterEach(() => {
      document.removeEventListener('loot-plugin-message-change', handleEvent);
    });

    it('getting messages if they are undefined should return undefined', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.should.not.have.ownProperty('messages');
    });

    it('getting messages if the array is empty should return an empty array', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        messages: [],
      });

      plugin.messages.should.be.Array();
      plugin.messages.should.be.empty();
    });

    it('getting messages should return any that are set', () => {
      const messages = [{
        type: 'say',
        content: 'test message',
      }];
      const plugin = new loot.Plugin({
        name: 'test',
        messages,
      });

      plugin.messages.should.be.deepEqual(messages);
    });

    it('setting messages should store any set', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        messages: [],
      });
      const messages = [{
        type: 'say',
        content: 'test message',
      }];

      plugin.messages = messages;

      plugin.messages.should.be.deepEqual(messages);
    });

    it('setting messages should not fire an event if no messages were changed', (done) => {
      const plugin = new loot.Plugin({
        name: 'test',
        messages: [{
          type: 'say',
          content: 'test message',
        }],
      });

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener('loot-plugin-message-change', handleEvent);

      plugin.messages = plugin.messages;

      setTimeout(done, 100);
    });

    it('setting messages should fire an event if the messages were changed', (done) => {
      const plugin = new loot.Plugin({
        name: 'test',
        messages: [],
      });
      const messages = [{
        type: 'error',
        content: 'test message',
      }];

      handleEvent = (evt) => {
        evt.detail.pluginId.should.equal(plugin.id);
        evt.detail.totalDiff.should.equal(1);
        evt.detail.warningDiff.should.equal(0);
        evt.detail.errorDiff.should.equal(1);
        done();
      };

      document.addEventListener('loot-plugin-message-change', handleEvent);

      plugin.messages = messages;
    });
  });

  describe('#isDirty', () => {
    let handleEvent;

    afterEach(() => {
      document.removeEventListener('loot-plugin-isdirty-change', handleEvent);
    });

    it('getting value should return false if isDirty has not been set in the constructor', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.isDirty.should.be.false();
    });

    it('getting value should return true if isDirty is set to true in the constructor', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        isDirty: true,
      });

      plugin.isDirty.should.be.true();
    });

    it('setting value should store set value', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.isDirty = true;

      plugin.isDirty.should.be.true();
    });

    it('setting value to the current value should not fire an event', (done) => {
      const plugin = new loot.Plugin({ name: 'test' });

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener('loot-plugin-isdirty-change', handleEvent);

      plugin.isDirty = plugin.isDirty;

      setTimeout(done, 100);
    });

    it('setting value not equal to the current value should fire an event', (done) => {
      const plugin = new loot.Plugin({ name: 'test' });

      handleEvent = (evt) => {
        evt.detail.isDirty.should.be.true();
        done();
      };

      document.addEventListener('loot-plugin-isdirty-change', handleEvent);

      plugin.isDirty = !plugin.isDirty;
    });
  });

  describe('#crc', () => {
    let handleEvent;

    afterEach(() => {
      document.removeEventListener('loot-plugin-card-content-change', handleEvent);
    });

    it('getting value should return 0 if crc has not been set in the constructor', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.isDirty.should.be.false();
    });

    it('getting value should return 0xDEADBEEF if it was set in the constructor', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        crc: 0xDEADBEEF,
      });

      plugin.crc.should.equal(0xDEADBEEF);
    });

    it('setting value should store set value', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.crc = 0xDEADBEEF;

      plugin.crc.should.equal(0xDEADBEEF);
    });

    it('setting value to the current value should not fire an event', (done) => {
      const plugin = new loot.Plugin({ name: 'test' });

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener('loot-plugin-card-content-change', handleEvent);

      plugin.crc = plugin.crc;

      setTimeout(done, 100);
    });

    it('setting value not equal to the current value should fire an event', (done) => {
      const plugin = new loot.Plugin({ name: 'test' });

      handleEvent = (evt) => {
        evt.detail.pluginId.should.equal(plugin.id);
        done();
      };

      document.addEventListener('loot-plugin-card-content-change', handleEvent);

      plugin.crc = 0xDEADBEEF;
    });
  });

  describe('#tags', () => {
    let handleEvent;

    afterEach(() => {
      document.removeEventListener('loot-plugin-card-content-change', handleEvent);
    });

    it('getting value should return an empty array if tags have not been set in the constructor', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.tags.length.should.equal(0);
    });

    it('getting value should return any tags that are set', () => {
      const tags = [{
        name: 'Delev',
      }];
      const plugin = new loot.Plugin({
        name: 'test',
        tags,
      });

      plugin.tags.should.deepEqual(tags);
    });

    it('setting value should store set value', () => {
      const plugin = new loot.Plugin({ name: 'test' });
      const tags = [{
        name: 'Delev',
      }];

      plugin.tags = tags;

      plugin.tags.should.deepEqual(tags);
    });

    it('setting value to the current value should not fire an event', (done) => {
      const plugin = new loot.Plugin({ name: 'test' });

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener('loot-plugin-card-content-change', handleEvent);

      plugin.tags = plugin.tags;

      setTimeout(done, 100);
    });

    it('setting value not equal to the current value should fire an event', (done) => {
      const plugin = new loot.Plugin({ name: 'test' });
      const tags = [{
        name: 'Delev',
      }];

      handleEvent = (evt) => {
        evt.detail.pluginId.should.equal(plugin.id);
        done();
      };

      document.addEventListener('loot-plugin-card-content-change', handleEvent);

      plugin.tags = tags;
    });
  });

  describe('#userlist', () => {
    let handleEvent;

    afterEach(() => {
      document.removeEventListener('loot-plugin-item-content-change', handleEvent);
    });

    it('getting value should return undefined if it has not been set in the constructor', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      should(plugin.userlist).be.undefined();
    });

    it('getting value should return the value that was set', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        userlist: {},
      });

      plugin.userlist.should.deepEqual({});
    });

    it('setting value should store set value', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.userlist = {};

      plugin.userlist.should.deepEqual({});
    });

    it('setting value to the current value should not fire an event', (done) => {
      const plugin = new loot.Plugin({ name: 'test' });

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener('loot-plugin-item-content-change', handleEvent);

      plugin.userlist = plugin.userlist;

      setTimeout(done, 100);
    });

    it('setting value not equal to the current value should fire an event', (done) => {
      const plugin = new loot.Plugin({ name: 'test' });

      handleEvent = (evt) => {
        evt.detail.pluginId.should.equal(plugin.id);
        evt.detail.priority.should.equal(plugin.priority);
        evt.detail.isPriorityGlobal.should.equal(plugin.isPriorityGlobal);
        evt.detail.isEditorOpen.should.equal(plugin.isEditorOpen);
        evt.detail.hasUserEdits.should.equal(plugin.hasUserEdits);
        done();
      };

      document.addEventListener('loot-plugin-item-content-change', handleEvent);

      plugin.userlist = { priority: 1 };
    });
  });

  describe('#priority', () => {
    let handleEvent;

    afterEach(() => {
      document.removeEventListener('loot-plugin-item-content-change', handleEvent);
    });

    it('getting value should return 0 if it has not been set in the constructor', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.priority.should.equal(0);
    });

    it('getting value should return the value that was set', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        priority: 5,
      });

      plugin.priority.should.equal(5);
    });

    it('setting value should store set value', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.priority = 5;

      plugin.priority.should.equal(5);
    });

    it('setting value to the current value should not fire an event', (done) => {
      const plugin = new loot.Plugin({ name: 'test' });

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener('loot-plugin-item-content-change', handleEvent);

      plugin.priority = plugin.priority;

      setTimeout(done, 100);
    });

    it('setting value not equal to the current value should fire an event', (done) => {
      const plugin = new loot.Plugin({ name: 'test' });

      handleEvent = (evt) => {
        evt.detail.pluginId.should.equal(plugin.id);
        evt.detail.priority.should.equal(plugin.priority);
        evt.detail.isPriorityGlobal.should.equal(plugin.isPriorityGlobal);
        evt.detail.isEditorOpen.should.equal(plugin.isEditorOpen);
        evt.detail.hasUserEdits.should.equal(plugin.hasUserEdits);
        done();
      };

      document.addEventListener('loot-plugin-item-content-change', handleEvent);

      plugin.priority = 5;
    });
  });

  describe('#isPriorityGlobal', () => {
    let handleEvent;

    afterEach(() => {
      document.removeEventListener('loot-plugin-item-content-change', handleEvent);
    });

    it('getting value should return false if it has not been set in the constructor', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.isPriorityGlobal.should.be.false();
    });

    it('getting value should return the value that was set', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        isPriorityGlobal: true,
      });

      plugin.isPriorityGlobal.should.be.true();
    });

    it('setting value should store set value', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.isPriorityGlobal = true;

      plugin.isPriorityGlobal.should.be.true();
    });

    it('setting value to the current value should not fire an event', (done) => {
      const plugin = new loot.Plugin({ name: 'test' });

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener('loot-plugin-item-content-change', handleEvent);

      plugin.isPriorityGlobal = plugin.isPriorityGlobal;

      setTimeout(done, 100);
    });

    it('setting value not equal to the current value should fire an event', (done) => {
      const plugin = new loot.Plugin({ name: 'test' });

      handleEvent = (evt) => {
        evt.detail.pluginId.should.equal(plugin.id);
        evt.detail.priority.should.equal(plugin.priority);
        evt.detail.isPriorityGlobal.should.equal(plugin.isPriorityGlobal);
        evt.detail.isEditorOpen.should.equal(plugin.isEditorOpen);
        evt.detail.hasUserEdits.should.equal(plugin.hasUserEdits);
        done();
      };

      document.addEventListener('loot-plugin-item-content-change', handleEvent);

      plugin.isPriorityGlobal = true;
    });
  });

  describe('#isEditorOpen', () => {
    let handleEvent;

    afterEach(() => {
      document.removeEventListener('loot-plugin-item-content-change', handleEvent);
    });

    it('getting value should return false if it has not been set in the constructor', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.isEditorOpen.should.be.false();
    });

    it('setting value should store set value', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.isEditorOpen = true;

      plugin.isEditorOpen.should.be.true();
    });

    it('setting value to the current value should not fire an event', (done) => {
      const plugin = new loot.Plugin({ name: 'test' });

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener('loot-plugin-item-content-change', handleEvent);

      plugin.isEditorOpen = plugin.isEditorOpen;

      setTimeout(done, 100);
    });

    it('setting value not equal to the current value should fire an event', (done) => {
      const plugin = new loot.Plugin({ name: 'test' });

      handleEvent = (evt) => {
        evt.detail.pluginId.should.equal(plugin.id);
        evt.detail.priority.should.equal(plugin.priority);
        evt.detail.isPriorityGlobal.should.equal(plugin.isPriorityGlobal);
        evt.detail.isEditorOpen.should.equal(plugin.isEditorOpen);
        evt.detail.hasUserEdits.should.equal(plugin.hasUserEdits);
        done();
      };

      document.addEventListener('loot-plugin-item-content-change', handleEvent);

      plugin.isEditorOpen = true;
    });
  });

  describe('#isSearchResult', () => {
    let handleEvent;

    afterEach(() => {
      document.removeEventListener('loot-plugin-card-styling-change', handleEvent);
    });

    it('getting value should return false if it has not been set in the constructor', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.isSearchResult.should.be.false();
    });

    it('setting value should store set value', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.isSearchResult = true;

      plugin.isSearchResult.should.be.true();
    });

    it('setting value to the current value should not fire an event', (done) => {
      const plugin = new loot.Plugin({ name: 'test' });

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener('loot-plugin-card-styling-change', handleEvent);

      plugin.isSearchResult = plugin.isSearchResult;

      setTimeout(done, 100);
    });

    it('setting value not equal to the current value should fire an event', (done) => {
      const plugin = new loot.Plugin({ name: 'test' });

      handleEvent = (evt) => {
        evt.detail.pluginId.should.equal(plugin.id);
        done();
      };

      document.addEventListener('loot-plugin-card-styling-change', handleEvent);

      plugin.isSearchResult = true;
    });
  });

  describe('#getCardContent()', () => {
    let plugin;
    beforeEach(() => {
      plugin = new loot.Plugin({ name: 'test' });
    });

    it('should throw if no argument was passed', () => {
      (() => { plugin.getCardContent(); }).should.throw();
    });

    it('should throw with an empty object', () => {
      plugin.messages = [{
        type: 'say',
        content: 'test message',
      }];

      (() => { plugin.getCardContent({}); }).should.throw();
    });

    it('should succeed if passed a filters object', () => {
      (() => { plugin.getCardContent(new Filters()); }).should.not.throw();
    });
  });
});

describe('PluginCardContent', () => {
  let plugin;
  let filters;

  beforeEach(() => {
    plugin = new loot.Plugin({
      name: 'test',
      version: 'foo',
      messages: [
        {
          type: 'say',
          content: [{
            str: 'test message',
            lang: 'en',
          }],
        },
        {
          type: 'warn',
          content: [{
            str: 'do not clean',
            lang: 'en',
          }],
        },
      ],
    });

    filters = new Filters();
  });

  describe('#name', () => {
    it('getting value should return plugin\'s value', () => {
      plugin.getCardContent(filters).name.should.equal(plugin.name);
    });

    it('setting value should throw', () => {
      (() => { plugin.getCardContent(filters).name = ''; }).should.throw();
    });
  });

  describe('#isActive', () => {
    it('getting value should return plugin\'s value', () => {
      plugin.getCardContent(filters).isActive.should.equal(plugin.isActive);
    });

    it('setting value should throw', () => {
      (() => { plugin.getCardContent(filters).isActive = true; }).should.throw();
    });
  });

  describe('#isEmpty', () => {
    it('getting value should return plugin\'s value', () => {
      plugin.getCardContent(filters).isEmpty.should.equal(plugin.isEmpty);
    });

    it('setting value should throw', () => {
      (() => { plugin.getCardContent(filters).isEmpty = true; }).should.throw();
    });
  });

  describe('#isMaster', () => {
    it('getting value should return plugin\'s value', () => {
      plugin.getCardContent(filters).isMaster.should.equal(plugin.isMaster);
    });

    it('setting value should throw', () => {
      (() => { plugin.getCardContent(filters).isMaster = true; }).should.throw();
    });
  });

  describe('#loadsArchive', () => {
    it('getting value should return plugin\'s value', () => {
      plugin.getCardContent(filters).loadsArchive.should.equal(plugin.loadsArchive);
    });

    it('setting value should throw', () => {
      (() => { plugin.getCardContent(filters).loadsArchive = true; }).should.throw();
    });
  });

  describe('#version', () => {
    it('getting value should return plugin\'s value if the version filter is not enabled', () => {
      plugin.getCardContent(filters).version.should.equal(plugin.version);
    });

    it('getting value should return empty string if the version filter is enabled', () => {
      filters.hideVersionNumbers = true;
      plugin.getCardContent(filters).version.should.equal('');
    });

    it('setting value should throw', () => {
      (() => { plugin.getCardContent(filters).version = ''; }).should.throw();
    });
  });

  describe('#tags', () => {
    it('should return an object containing empty strings if no tags are set', () => {
      plugin.getCardContent(filters).tags.should.deepEqual({
        added: '',
        removed: '',
      });
    });

    it('should return an object containing strings of comma-separated tag names if tags are set', () => {
      plugin.tags = [
        { name: 'Relev' },
        { name: 'Delev' },
        { name: 'Names' },
        { name: '-C.Climate' },
        { name: '-Actor.ABCS' },
      ];

      plugin.getCardContent(filters).tags.should.deepEqual({
        added: 'Relev, Delev, Names',
        removed: 'C.Climate, Actor.ABCS',
      });
    });

    it('should return an object containing empty strings if tags are set and the tags filter is enabled', () => {
      plugin.tags = [
        { name: 'Relev' },
        { name: 'Delev' },
        { name: 'Names' },
        { name: '-C.Climate' },
        { name: '-Actor.ABCS' },
      ];
      filters.hideBashTags = true;

      plugin.getCardContent(filters).tags.should.deepEqual({
        added: '',
        removed: '',
      });
    });

    it('should output a tag in the removed string if it appears as both added and removed', () => {
      plugin.tags = [
        { name: 'Relev' },
        { name: '-Relev' },
      ];

      plugin.getCardContent(filters).tags.should.deepEqual({
        added: '',
        removed: 'Relev',
      });
    });

    it('setting value should throw', () => {
      (() => {
        plugin.getCardContent(filters).tags = {
          added: 'Relev',
          removed: 'Delev',
        };
      }).should.throw();
    });
  });

  describe('#crc', () => {
    it('should return an empty string if crc is undefined', () => {
      plugin.getCardContent(filters).crc.should.equal('');
    });

    it('should return an empty string if crc is zero', () => {
      plugin.crc = 0;

      plugin.getCardContent(filters).crc.should.equal('');
    });

    it('should return crc value as string if non zero', () => {
      plugin.crc = 0xDEADBEEF;

      plugin.getCardContent(filters).crc.should.equal('DEADBEEF');
    });

    it('should return an empty string if crc is non-zero and the CRC filter is enabled', () => {
      plugin.crc = 0xDEADBEEF;
      filters.hideCRCs = true;

      plugin.getCardContent(filters).crc.should.equal('');
    });

    it('should pad crc value to eight digits', () => {
      plugin.crc = 0xBEEF;

      plugin.getCardContent(filters).crc.should.equal('0000BEEF');
    });

    it('setting value should throw', () => {
      (() => { plugin.getCardContent(filters).crc = 0xBECADECA; }).should.throw();
    });
  });

  describe('#messages', () => {
    it('should return message objects mapped from the plugin\'s message objects', () => {
      plugin.getCardContent(filters).messages.should.deepEqual([
        {
          type: plugin.messages[0].type,
          content: plugin.messages[0].content[0].str,
        },
        {
          type: plugin.messages[1].type,
          content: plugin.messages[1].content[0].str,
        },
      ]);
    });

    it('should return an array missing the note message when the notes filter is enabled', () => {
      filters.hideNotes = true;
      plugin.getCardContent(filters).messages.should.deepEqual([{
        type: plugin.messages[1].type,
        content: plugin.messages[1].content[0].str,
      }]);
    });

    it('should return an array missing the "do not clean" message when the "do not clean" messages filter is enabled', () => {
      filters.hideDoNotCleanMessages = true;
      plugin.getCardContent(filters).messages.should.deepEqual([{
        type: plugin.messages[0].type,
        content: plugin.messages[0].content[0].str,
      }]);
    });

    it('should return an empty array when the all messages filter is enabled', () => {
      filters.hideAllPluginMessages = true;
      plugin.getCardContent(filters).messages.should.deepEqual([]);
    });

    it('setting value should throw', () => {
      (() => { plugin.getCardContent(filters).messages = []; }).should.throw();
    });
  });

  describe('#containsText()', () => {
    it('should return true if argument is undefined', () => {
      plugin.getCardContent(filters).containsText().should.be.true();
    });

    it('should return true if argument is an empty string', () => {
      plugin.getCardContent(filters).containsText('').should.be.true();
    });

    it('should search name case-insensitively', () => {
      plugin.getCardContent(filters).containsText('Tes').should.be.true();
    });

    it('should search CRC case-insensitively', () => {
      plugin.crc = 0xDEADBEEF;
      plugin.getCardContent(filters).containsText('dead').should.be.true();
    });

    it('should search added tags case-insensitively', () => {
      plugin.tags = [
        { name: 'Relev' },
        { name: 'Delev' },
        { name: 'Names' },
        { name: '-C.Climate' },
        { name: '-Actor.ABCS' },
      ];
      plugin.getCardContent(filters).containsText('climate').should.be.true();
    });

    it('should search removed tags case-insensitively', () => {
      plugin.tags = [
        { name: 'Relev' },
        { name: 'Delev' },
        { name: 'Names' },
        { name: '-C.Climate' },
        { name: '-Actor.ABCS' },
      ];
      plugin.getCardContent(filters).containsText('.abc').should.be.true();
    });

    it('should search message content case-insensitively', () => {
      plugin.getCardContent(filters).containsText('Clean').should.be.true();
    });

    it('should not find text that is not present', () => {
      plugin.getCardContent(filters).containsText('say').should.be.false();
    });
  });
});
