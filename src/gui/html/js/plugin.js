/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013 WrinklyNinja

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

function deduplicateTags(currentTags, suggestedTags) {
  const currentTagNames = currentTags.map(tag => tag.name);
  const removalTagNames = suggestedTags
    .filter(tag => !tag.isAddition)
    .map(tag => tag.name);

  /* Ignore any removal suggestions that aren't in the current list. */
  let tagsToIgnore = currentTagNames.map(tag => tag.toLowerCase());

  const filteredRemovalTagNames = removalTagNames.filter(tag =>
    tagsToIgnore.includes(tag.toLowerCase())
  );

  /* Ignore any addition suggestions that are in the current or remove lists. */
  tagsToIgnore = tagsToIgnore.concat(
    removalTagNames.map(tag => tag.toLowerCase())
  );

  const additionTagNames = suggestedTags
    .filter(
      tag => tag.isAddition && !tagsToIgnore.includes(tag.name.toLowerCase())
    )
    .map(tag => tag.name);

  return {
    current: currentTagNames.join(', '),
    add: additionTagNames.join(', '),
    remove: filteredRemovalTagNames.join(', ')
  };
}

export function crcToString(crc) {
  /* Pad CRC string to 8 characters. */
  return `00000000${crc.toString(16).toUpperCase()}`.slice(-8);
}

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
      this._tags = deduplicateTags(plugin.currentTags, plugin.suggestedTags);
    } else {
      this._tags = deduplicateTags([], []);
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
    return crcToString(this._crc);
  }

  get tags() {
    return this._tags;
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
      this.tags.current.toLowerCase().indexOf(needle) !== -1 ||
      this.tags.add.toLowerCase().indexOf(needle) !== -1 ||
      this.tags.remove.toLowerCase().indexOf(needle) !== -1
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

export class Plugin {
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
    this._messages = obj.messages || [];
    this._suggestedTags = obj.suggestedTags || [];
    this._currentTags = obj.currentTags || [];
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

    /* Set default values for fields that may not be present. */
    if (plugin.version === undefined) {
      this.version = '';
    }
    if (plugin.crc === undefined) {
      this.crc = 0;
    }
    if (plugin.group === undefined) {
      this.group = 'default';
    }
    if (plugin.loadOrderIndex === undefined) {
      this.loadOrderIndex = undefined;
    }
    if (plugin.cleanedWith === undefined) {
      this.cleanedWith = '';
    }
    if (plugin.masterlist === undefined) {
      this.masterlist = undefined;
    }
    if (plugin.userlist === undefined) {
      this.userlist = undefined;
    }
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
    return {
      name: rowData.name,
      isAddition: rowData.type === 'add',
      condition: rowData.condition
    };
  }

  static tagToRowData(tag) {
    if (
      tag.condition === undefined ||
      tag.name === undefined ||
      tag.isAddition === undefined
    ) {
      throw new TypeError('Tag members are undefined');
    }

    return {
      name: tag.name,
      type: tag.isAddition ? 'add' : 'remove',
      condition: tag.condition
    };
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

  get currentTags() {
    return this._currentTags;
  }

  set currentTags(tags) {
    if (!_.isEqual(this._currentTags, tags)) {
      this._currentTags = tags;

      this._dispatchCardContentChangeEvent(true);
    }
  }

  get suggestedTags() {
    return this._suggestedTags;
  }

  set suggestedTags(tags) {
    if (!_.isEqual(this._suggestedTags, tags)) {
      this._suggestedTags = tags;

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
