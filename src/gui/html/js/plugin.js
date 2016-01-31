/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013-2016    WrinklyNinja

    This file is part of LOOT.

    LOOT is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    LOOT is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with LOOT.  If not, see
    <http://www.gnu.org/licenses/>.
*/
'use strict';
(function exportModule(root, factory) {
  if (typeof define === 'function' && define.amd) {
    // AMD. Register as an anonymous module.
    define([], factory);
  } else {
    // Browser globals
    root.loot = root.loot || {};
    root.loot.Plugin = factory();
  }
}(this, () => {
  /* Messages, tags, CRCs and version strings can all be hidden by filters.
     Use getters with no setters for member variables as data should not be
     written to objects of this class. */
  class PluginCardContent {
    constructor(plugin, filters) {
      this._name = plugin.name;
      this._isActive = plugin.isActive || false;
      this._isEmpty = plugin.isEmpty;
      this._isMaster = plugin.isMaster;
      this._loadsArchive = plugin.loadsArchive;

      if (!filters.hideVersionNumbers) {
        this._version = plugin.version;
      } else {
        this._version = '';
      }

      if (!filters.hideCRCs) {
        this._crc = plugin.crc;
      } else {
        this._crc = 0;
      }

      if (!filters.hideBashTags) {
        this._tags = plugin.tags;
      } else {
        this._tags = [];
      }

      this._messages = plugin.messages.map((message) => {
        return {
          type: message.type,
          content: message.content[0].str,
        };
      }).filter(filters.messageFilter, filters);
    }

    get name() {
      return this._name;
    }

    get isActive() {
      return this._isActive;
    }

    get isEmpty() {
      return this._isEmpty;
    }

    get isMaster() {
      return this._isMaster;
    }

    get loadsArchive() {
      return this._loadsArchive;
    }

    get version() {
      return this._version;
    }

    get crc() {
      if (this._crc === 0) {
        return '';
      }

      /* Pad CRC string to 8 characters. */
      return (`00000000${this._crc.toString(16).toUpperCase()}`).slice(-8);
    }

    get tags() {
      const tagsAdded = [];
      const tagsRemoved = [];

      if (this._tags) {
        for (let i = 0; i < this._tags.length; ++i) {
          if (this._tags[i].name[0] === '-') {
            tagsRemoved.push(this._tags[i].name.substr(1));
          } else {
            tagsAdded.push(this._tags[i].name);
          }
        }
      }
      /* Now make sure that the same tag doesn't appear in both arrays.
         Prefer the removed list. */
      for (let i = 0; i < tagsAdded.length; ++i) {
        for (let j = 0; j < tagsRemoved.length; ++j) {
          if (tagsRemoved[j].toLowerCase() === tagsAdded[i].toLowerCase()) {
            /* Remove tag from the tagsAdded array. */
            tagsAdded.splice(i, 1);
            --i;
          }
        }
      }

      return {
        added: tagsAdded.join(', '),
        removed: tagsRemoved.join(', '),
      };
    }

    get messages() {
      return this._messages;
    }

    containsText(text) {
      if (text === undefined || text.length === 0) {
        return true;
      }
      const needle = text.toLowerCase();

      if (this.name.toLowerCase().indexOf(needle) !== -1
          || this.crc.toLowerCase().indexOf(needle) !== -1
          || this.version.toLowerCase().indexOf(needle) !== -1) {
        return true;
      }

      if (this.tags.added.toLowerCase().indexOf(needle) !== -1
          || this.tags.removed.toLowerCase().indexOf(needle) !== -1) {
        return true;
      }

      for (let i = 0; i < this.messages.length; ++i) {
        if (this.messages[i].content.toLowerCase().indexOf(needle) !== -1) {
          return true;
        }
      }

      return false;
    }
  }

  return class Plugin {
    constructor(obj) {
      /* Plugin data */
      this.name = obj.name;
      this.crc = obj.crc || 0;
      this.version = obj.version || '';
      this.isActive = obj.isActive || false;
      this.isEmpty = obj.isEmpty || false;
      this.isMaster = obj.isMaster || false;
      this.loadsArchive = obj.loadsArchive || false;

      this.masterlist = obj.masterlist;
      this.userlist = obj.userlist;

      this.priority = obj.priority || 0;
      this.isPriorityGlobal = obj.isPriorityGlobal || false;
      this._messages = obj.messages || [];
      this.tags = obj.tags;
      this._isDirty = obj.isDirty || false;

      /* UI state variables */
      this.id = this.name.replace(/\s+/g, '');
      this.isMenuOpen = false;
      this.isEditorOpen = false;
      this.isConflictFilterChecked = false;
      this.isSearchResult = false;
    }

    static fromJson(key, value) {
      if (value !== null && value.__type === 'Plugin') {
        return new Plugin(value);
      }
      return value;
    }

    static tagFromRowData(rowData) {
      if (rowData.condition === undefined || rowData.name === undefined || rowData.type === undefined) {
        throw new TypeError('Row data members are undefined');
      }
      const tag = {
        condition: rowData.condition,
        name: '',
      };

      if (rowData.type === 'remove') {
        tag.name = '-';
      }
      tag.name += rowData.name;

      return tag;
    }

    static tagToRowData(tag) {
      const rowData = {
        condition: tag.condition,
      };

      if (tag.name[0] === '-') {
        rowData.type = 'remove';
        rowData.name = tag.name.substr(1);
      } else {
        rowData.type = 'add';
        rowData.name = tag.name;
      }

      return rowData;
    }

    get priorityString() {
      if (this.priority === 0) {
        return '';
      }

      return this.priority.toString();
    }

    get messages() {
      return this._messages;
    }

    set messages(messages) {
      /* Update the message counts. */
      let oldTotal = 0;
      let newTotal = 0;
      let oldWarns = 0;
      let newWarns = 0;
      let oldErrs = 0;
      let newErrs = 0;

      oldTotal = this._messages.length;

      this._messages.forEach((message) => {
        if (message.type === 'warn') {
          ++oldWarns;
        } else if (message.type === 'error') {
          ++oldErrs;
        }
      });

      newTotal = messages.length;

      messages.forEach((message) => {
        if (message.type === 'warn') {
          ++newWarns;
        } else if (message.type === 'error') {
          ++newErrs;
        }
      });

      if (newTotal !== oldTotal || newWarns !== oldWarns || newErrs !== oldErrs) {
        document.dispatchEvent(new CustomEvent('loot-plugin-message-change', {
          detail: {
            totalDiff: newTotal - oldTotal,
            warningDiff: newWarns - oldWarns,
            errorDiff: newErrs - oldErrs,
          },
        }));
      }

      this._messages = messages;
    }

    get isDirty() {
      return this._isDirty;
    }

    set isDirty(dirty) {
      /* Update dirty counts. */
      if (dirty !== this._isDirty) {
        document.dispatchEvent(new CustomEvent('loot-plugin-isdirty-change', {
          detail: {
            isDirty: dirty,
          },
        }));
      }

      this._isDirty = dirty;
    }

    getCardContent(filters) {
      return new PluginCardContent(this, filters);
    }

    static onMessageChange(evt) {
      document.getElementById('filterTotalMessageNo').textContent = parseInt(document.getElementById('filterTotalMessageNo').textContent, 10) + evt.detail.totalDiff;
      document.getElementById('totalMessageNo').textContent = parseInt(document.getElementById('totalMessageNo').textContent, 10) + evt.detail.totalDiff;
      document.getElementById('totalWarningNo').textContent = parseInt(document.getElementById('totalWarningNo').textContent, 10) + evt.detail.warningDiff;
      document.getElementById('totalErrorNo').textContent = parseInt(document.getElementById('totalErrorNo').textContent, 10) + evt.detail.errorDiff;
    }
    static onIsDirtyChange(evt) {
      if (evt.detail.isDirty) {
        document.getElementById('dirtyPluginNo').textContent = parseInt(document.getElementById('dirtyPluginNo').textContent, 10) + 1;
      } else {
        document.getElementById('dirtyPluginNo').textContent = parseInt(document.getElementById('dirtyPluginNo').textContent, 10) - 1;
      }
    }
  };
}));
