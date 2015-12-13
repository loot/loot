'use strict';

describe('Plugin', () => {
  describe('#Plugin()', () => {
    it('should throw if nothing is passed', () => {
      (() => { new loot.Plugin(); }).should.throw();
    });

    it('should throw if an object with no name key is passed', () => {
      (() => { new loot.Plugin({}); }).should.throw();
    });

    it('should not throw if some members are undefined', () => {
      (() => { new loot.Plugin({ name: 'test' }); }).should.not.throw();
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

  describe('#tagStrings', () => {
    it('should return an object containing empty strings if no tags are set', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.tagStrings.should.deepEqual({
        added: '',
        removed: '',
      });
    });

    it('should return an object containing strings of comma-separated tag names if tags are set', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        tags: [
          { name: 'Relev' },
          { name: 'Delev' },
          { name: 'Names' },
          { name: '-C.Climate' },
          { name: '-Actor.ABCS' },
        ],
      });

      plugin.tagStrings.should.deepEqual({
        added: 'Relev, Delev, Names',
        removed: 'C.Climate, Actor.ABCS',
      });
    });

    it('should output a tag in the removed string if it appears as both added and removed', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        tags: [
          { name: 'Relev' },
          { name: '-Relev' },
        ],
      });

      plugin.tagStrings.should.deepEqual({
        added: '',
        removed: 'Relev',
      });
    });
  });

  describe('#priorityString', () => {
    it('should return an empty string if priority is undefined', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.priorityString.should.equal('');
    });

    it('should return an empty string if priority is zero', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        modPriority: 0,
      });

      plugin.priorityString.should.equal('');
    });

    it('should return priority valueAs string if non zero', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        modPriority: -50,
      });

      plugin.priorityString.should.equal('-50');
    });
  });

  describe('#crcString', () => {
    it('should return an empty string if crc is undefined', () => {
      const plugin = new loot.Plugin({ name: 'test' });

      plugin.crcString.should.equal('');
    });

    it('should return an empty string if crc is zero', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        crc: 0,
      });

      plugin.crcString.should.equal('');
    });

    it('should return crc value as string if non zero', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        crc: 0xDEADBEEF,
      });

      plugin.crcString.should.equal('DEADBEEF');
    });

    it('should pad crc value to eight digits', () => {
      const plugin = new loot.Plugin({
        name: 'test',
        crc: 0xBEEF,
      });

      plugin.crcString.should.equal('0000BEEF');
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
        messages: messages,
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

    it('setting messages should not fire an event if no message counts were changed', (done) => {
      const plugin = new loot.Plugin({
        name: 'test',
        messages: [{
          type: 'say',
          content: 'test message',
        }],
      });
      const messages = [{
        type: 'say',
        content: 'another test message',
      }];

      handleEvent = () => {
        done(new Error('Should not have fired an event'));
      };

      document.addEventListener('loot-plugin-message-change', handleEvent);

      plugin.messages = messages;

      setTimeout(done, 100);
    });

    it('setting messages should fire an event if message counts were changed', (done) => {
      const plugin = new loot.Plugin({
        name: 'test',
        messages: [],
      });
      const messages = [{
        type: 'error',
        content: 'test message',
      }];

      handleEvent = (evt) => {
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
});
