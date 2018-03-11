/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013-2018    WrinklyNinja

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
    <https://www.gnu.org/licenses/>.
*/

import * as _ from 'lodash/core.min';

/* Messages, tags, CRCs and version strings can all be hidden by filters.
    Use getters with no setters for member variables as data should not be
    written to objects of this class. */
class PluginCardContent {
  constructor(plugin, filters) {
    this._name = plugin.name;
    this._isActive = plugin.isActive || false;
    this._isEmpty = plugin.isEmpty;
    this._isMaster = plugin.isMaster;
    this._isLightMaster = plugin.isLightMaster;
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

    this._messages = plugin.messages.filter(filters.messageFilter, filters);
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

  get isLightMaster() {
    return this._isLightMaster;
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
    return `00000000${this._crc.toString(16).toUpperCase()}`.slice(-8);
  }

  get tags() {
    const tagsAdded = [];
    const tagsRemoved = [];

    if (this._tags) {
      for (let i = 0; i < this._tags.length; i += 1) {
        if (this._tags[i].name[0] === '-') {
          tagsRemoved.push(this._tags[i].name.substr(1));
        } else {
          tagsAdded.push(this._tags[i].name);
        }
      }
    }
    /* Now make sure that the same tag doesn't appear in both arrays.
        Prefer the removed list. */
    for (let i = 0; i < tagsAdded.length; i += 1) {
      for (let j = 0; j < tagsRemoved.length; j += 1) {
        if (tagsRemoved[j].toLowerCase() === tagsAdded[i].toLowerCase()) {
          /* Remove tag from the tagsAdded array. */
          tagsAdded.splice(i, 1);
          i -= 1;
        }
      }
    }

    return {
      added: tagsAdded.join(', '),
      removed: tagsRemoved.join(', ')
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

    if (
      this.name.toLowerCase().indexOf(needle) !== -1 ||
      this.crc.toLowerCase().indexOf(needle) !== -1 ||
      this.version.toLowerCase().indexOf(needle) !== -1
    ) {
      return true;
    }

    if (
      this.tags.added.toLowerCase().indexOf(needle) !== -1 ||
      this.tags.removed.toLowerCase().indexOf(needle) !== -1
    ) {
      return true;
    }

    for (let i = 0; i < this.messages.length; i += 1) {
      if (this.messages[i].text.toLowerCase().indexOf(needle) !== -1) {
        return true;
      }
    }

    return false;
  }
}

export default class Plugin {
  constructor(obj) {
    /* Plugin data */
    this.name = obj.name;
    this._crc = obj.crc || 0;
    this.version = obj.version || '';
    this.isActive = obj.isActive || false;
    this.isEmpty = obj.isEmpty || false;
    this.isMaster = obj.isMaster || false;
    this.isLightMaster = obj.isLightMaster || false;
    this.loadsArchive = obj.loadsArchive || false;

    this.masterlist = obj.masterlist;
    this._userlist = obj.userlist;

    this._group = obj.group || 'default';
    this._priority = obj.priority || 0;
    this._globalPriority = obj.globalPriority || 0;
    this._messages = obj.messages || [];
    this._tags = obj.tags || [];
    this._isDirty = obj.isDirty || false;
    this._loadOrderIndex = obj.loadOrderIndex;
    this.cleanedWith = obj.cleanedWith || '';

    /* UI state variables */
    this.id = this.name.replace(/\s+/g, '');
    this._isEditorOpen = false;
    this._isSearchResult = false;
    this.cardZIndex = 0;
  }

  update(plugin) {
    if (!plugin) {
      return;
    }
    if (plugin.name !== this.name) {
      throw new Error(
        `Cannot update data for ${this.name} using data for ${plugin.name}`
      );
    }

    Object.getOwnPropertyNames(plugin).forEach(property => {
      this[property] = plugin[property];
    });
  }

  static fromJson(key, value) {
    if (
      value !== null &&
      Object.prototype.hasOwnProperty.call(value, 'name') &&
      Object.prototype.hasOwnProperty.call(value, 'isEmpty')
    ) {
      return new Plugin(value);
    }
    return value;
  }

  static tagFromRowData(rowData) {
    if (
      rowData.condition === undefined ||
      rowData.name === undefined ||
      rowData.type === undefined
    ) {
      throw new TypeError('Row data members are undefined');
    }
    const tag = {
      condition: rowData.condition,
      name: ''
    };

    if (rowData.type === 'remove') {
      tag.name = '-';
    }
    tag.name += rowData.name;

    return tag;
  }

  static tagToRowData(tag) {
    const rowData = {
      condition: tag.condition
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

  _dispatchCardContentChangeEvent(mayChangeCardHeight) {
    document.dispatchEvent(
      new CustomEvent('loot-plugin-card-content-change', {
        detail: {
          pluginId: this.id,
          mayChangeCardHeight
        }
      })
    );
  }

  _dispatchCardStylingChangeEvent() {
    document.dispatchEvent(
      new CustomEvent('loot-plugin-card-styling-change', {
        detail: { pluginId: this.id }
      })
    );
  }

  _dispatchItemContentChangeEvent() {
    document.dispatchEvent(
      new CustomEvent('loot-plugin-item-content-change', {
        detail: {
          pluginId: this.id,
          group: this.group,
          priority: this.priority,
          globalPriority: this.globalPriority,
          isEditorOpen: this.isEditorOpen,
          hasUserEdits: this.hasUserEdits,
          loadOrderIndex: this.loadOrderIndex,
          isLightMaster: this.isLightMaster
        }
      })
    );
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

    this._messages.forEach(message => {
      if (message.type === 'warn') {
        oldWarns += 1;
      } else if (message.type === 'error') {
        oldErrs += 1;
      }
    });

    newTotal = messages.length;

    messages.forEach(message => {
      if (message.type === 'warn') {
        newWarns += 1;
      } else if (message.type === 'error') {
        newErrs += 1;
      }
    });

    if (
      newTotal !== oldTotal ||
      newWarns !== oldWarns ||
      newErrs !== oldErrs ||
      !_.isEqual(this._messages, messages)
    ) {
      this._messages = messages;

      document.dispatchEvent(
        new CustomEvent('loot-plugin-message-change', {
          detail: {
            pluginId: this.id,
            mayChangeCardHeight: true,
            totalDiff: newTotal - oldTotal,
            warningDiff: newWarns - oldWarns,
            errorDiff: newErrs - oldErrs
          }
        })
      );
    }
  }

  get isDirty() {
    return this._isDirty;
  }

  set isDirty(dirty) {
    /* Update dirty counts. */
    if (dirty !== this._isDirty) {
      this._isDirty = dirty;

      document.dispatchEvent(
        new CustomEvent('loot-plugin-cleaning-data-change', {
          detail: {
            isDirty: this.isDirty
          }
        })
      );
    }
  }

  get cleanedWith() {
    return this._cleanedWith;
  }

  set cleanedWith(cleanedWith) {
    if (cleanedWith !== this._cleanedWith) {
      this._cleanedWith = cleanedWith;

      document.dispatchEvent(
        new CustomEvent('loot-plugin-cleaning-data-change', {
          detail: {
            cleanedWith: this.cleanedWith,
            pluginId: this.id
          }
        })
      );
    }
  }

  get crc() {
    return this._crc;
  }

  set crc(crc) {
    if (this._crc !== crc) {
      this._crc = crc;

      this._dispatchCardContentChangeEvent(false);
    }
  }

  get tags() {
    return this._tags;
  }

  set tags(tags) {
    if (!_.isEqual(this._tags, tags)) {
      this._tags = tags;

      this._dispatchCardContentChangeEvent(true);
    }
  }

  get hasUserEdits() {
    return this.userlist !== undefined && Object.keys(this.userlist).length > 1;
  }

  get userlist() {
    return this._userlist;
  }

  set userlist(userlist) {
    if (!_.isEqual(this._userlist, userlist)) {
      this._userlist = userlist;

      this._dispatchItemContentChangeEvent();
      this._dispatchCardStylingChangeEvent();
    }
  }

  get group() {
    return this._group;
  }

  set group(group) {
    if (this._group !== group) {
      this._group = group;

      this._dispatchItemContentChangeEvent();
    }
  }

  get priority() {
    return this._priority;
  }

  set priority(priority) {
    if (this._priority !== priority) {
      this._priority = priority;

      this._dispatchItemContentChangeEvent();
    }
  }

  get globalPriority() {
    return this._globalPriority;
  }

  set globalPriority(globalPriority) {
    if (this._globalPriority !== globalPriority) {
      this._globalPriority = globalPriority;

      this._dispatchItemContentChangeEvent();
    }
  }

  get isEditorOpen() {
    return this._isEditorOpen;
  }

  set isEditorOpen(isEditorOpen) {
    if (this._isEditorOpen !== isEditorOpen) {
      this._isEditorOpen = isEditorOpen;

      this._dispatchItemContentChangeEvent();
    }
  }

  get isSearchResult() {
    return this._isSearchResult;
  }

  set isSearchResult(isSearchResult) {
    if (this._isSearchResult !== isSearchResult) {
      this._isSearchResult = isSearchResult;

      this._dispatchCardStylingChangeEvent();
    }
  }

  get loadOrderIndex() {
    return this._loadOrderIndex;
  }

  set loadOrderIndex(loadOrderIndex) {
    if (this._loadOrderIndex !== loadOrderIndex) {
      this._loadOrderIndex = loadOrderIndex;

      this._dispatchItemContentChangeEvent();
    }
  }

  getCardContent(filters) {
    return new PluginCardContent(this, filters);
  }

  static onMessageChange(evt) {
    document.getElementById('filterTotalMessageNo').textContent =
      parseInt(
        document.getElementById('filterTotalMessageNo').textContent,
        10
      ) + evt.detail.totalDiff;
    document.getElementById('totalMessageNo').textContent =
      parseInt(document.getElementById('totalMessageNo').textContent, 10) +
      evt.detail.totalDiff;
    document.getElementById('totalWarningNo').textContent =
      parseInt(document.getElementById('totalWarningNo').textContent, 10) +
      evt.detail.warningDiff;
    document.getElementById('totalErrorNo').textContent =
      parseInt(document.getElementById('totalErrorNo').textContent, 10) +
      evt.detail.errorDiff;
  }

  static onCleaningDataChange(evt) {
    if (evt.detail.isDirty !== undefined) {
      if (evt.detail.isDirty) {
        document.getElementById('dirtyPluginNo').textContent =
          parseInt(document.getElementById('dirtyPluginNo').textContent, 10) +
          1;
      } else {
        document.getElementById('dirtyPluginNo').textContent =
          parseInt(document.getElementById('dirtyPluginNo').textContent, 10) -
          1;
      }
    }
    if (evt.detail.cleanedWith !== undefined) {
      const card = document.getElementById(evt.detail.pluginId);
      if (card) {
        card.updateIsCleanIcon();
      }
    }
  }

  static onContentChange(evt) {
    const card = document.getElementById(evt.detail.pluginId);
    if (card) {
      card.updateContent(evt.detail.mayChangeCardHeight);
    }
  }

  static onCardStylingChange(evt) {
    const card = document.getElementById(evt.detail.pluginId);
    if (card) {
      card.updateStyling();
    }
  }

  static onItemContentChange(evt) {
    const item = document
      .getElementById('cardsNav')
      .querySelector(`[data-id="${evt.detail.pluginId}"]`);
    if (item) {
      item.updateContent(evt.detail);
    }
  }
}
