/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013-2015    WrinklyNinja

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
  return class Plugin {
    constructor(obj) {
      /* Plugin data */
      this.name = obj.name;
      this.crc = obj.crc;
      this.version = obj.version;
      this.isActive = obj.isActive;
      this.isEmpty = obj.isEmpty;
      this.isMaster = obj.isMaster;
      this.loadsArchive = obj.loadsArchive;

      this.masterlist = obj.masterlist;
      this.userlist = obj.userlist;

      this.modPriority = obj.modPriority;
      this.isGlobalPriority = obj.isGlobalPriority;
      this._messages = obj.messages;
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

    get tagStrings() {
      const tagsAdded = [];
      const tagsRemoved = [];

      if (this.tags) {
        for (let i = 0; i < this.tags.length; ++i) {
          if (this.tags[i].name[0] === '-') {
            tagsRemoved.push(this.tags[i].name.substr(1));
          } else {
            tagsAdded.push(this.tags[i].name);
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

    get priorityString() {
      if (this.modPriority === undefined || this.modPriority === 0) {
        return '';
      }

      return this.modPriority.toString();
    }

    get crcString() {
      if (this.crc === undefined || this.crc === 0) {
        return '';
      }

      /* Pad CRC string to 8 characters. */
      return ('00000000' + this.crc.toString(16).toUpperCase()).slice(-8);
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

      if (this._messages) {
        oldTotal = this._messages.length;

        this._messages.forEach((message) => {
          if (message.type === 'warn') {
            ++oldWarns;
          } else if (message.type === 'error') {
            ++oldErrs;
          }
        });
      }

      if (messages) {
        newTotal = messages.length;

        messages.forEach((message) => {
          if (message.type === 'warn') {
            ++newWarns;
          } else if (message.type === 'error') {
            ++newErrs;
          }
        });
      }

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
  };
}));
