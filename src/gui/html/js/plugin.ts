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

import { isEqual } from 'lodash';
import LootPluginCard from '../elements/loot-plugin-card';
import LootPluginItem from '../elements/loot-plugin-item';
import Filters from './filters';
import {
  SimpleMessage,
  DerivedPluginMetadata,
  Tag,
  PluginMetadata,
  PluginItemContentChangePayload,
  PluginTags,
  TagRowData
} from './interfaces';
import {
  incrementCounterText,
  getElementById,
  querySelector
} from './dom/helpers';

interface PluginMessageChangeEvent extends CustomEvent {
  detail: {
    pluginId: string;
    mayChangeCardHeight: boolean;
    totalDiff: number;
    warningDiff: number;
    errorDiff: number;
  };
}

function isPluginMessageChangeEvent(
  evt: Event
): evt is PluginMessageChangeEvent {
  return (
    evt instanceof CustomEvent &&
    typeof evt.detail.pluginId === 'string' &&
    typeof evt.detail.mayChangeCardHeight === 'boolean' &&
    typeof evt.detail.totalDiff === 'number' &&
    typeof evt.detail.warningDiff === 'number' &&
    typeof evt.detail.errorDiff === 'number'
  );
}

interface PluginCleaningDataChangeEvent extends CustomEvent {
  detail: {
    isDirty?: boolean;
    cleanedWith?: string;
    pluginId?: string;
  };
}

function isPluginCleaningDataChangeEvent(
  evt: Event
): evt is PluginCleaningDataChangeEvent {
  return (
    evt instanceof CustomEvent &&
    (typeof evt.detail.isDirty === 'boolean' ||
      typeof evt.detail.isDirty === 'undefined') &&
    (typeof evt.detail.cleanedWith === 'string' ||
      typeof evt.detail.cleanedWith === 'undefined') &&
    (typeof evt.detail.pluginId === 'string' ||
      typeof evt.detail.pluginId === 'undefined')
  );
}

interface PluginContentChangeEvent extends CustomEvent {
  detail: {
    pluginId: string;
    mayChangeCardHeight: boolean;
    totalDiff?: number;
    warningDiff?: number;
    errorDiff?: number;
  };
}

function isPluginContentChangeEvent(
  evt: Event
): evt is PluginContentChangeEvent {
  return (
    evt instanceof CustomEvent &&
    typeof evt.detail.pluginId === 'string' &&
    typeof evt.detail.mayChangeCardHeight === 'boolean' &&
    (typeof evt.detail.totalDiff === 'number' ||
      typeof evt.detail.totalDiff === 'undefined') &&
    (typeof evt.detail.warningDiff === 'number' ||
      typeof evt.detail.warningDiff === 'undefined') &&
    (typeof evt.detail.errorDiff === 'number' ||
      typeof evt.detail.errorDiff === 'undefined')
  );
}

interface PluginCardStylingChangeEvent extends CustomEvent {
  detail: {
    pluginId: string;
  };
}

function isPluginCardStylingChangeEvent(
  evt: Event
): evt is PluginCardStylingChangeEvent {
  return evt instanceof CustomEvent && typeof evt.detail.pluginId === 'string';
}

interface PluginItemContentChangeEvent extends CustomEvent {
  detail: PluginItemContentChangePayload;
}

function isPluginItemContentChangeEvent(
  evt: Event
): evt is PluginItemContentChangeEvent {
  return (
    evt instanceof CustomEvent &&
    typeof evt.detail.pluginId === 'string' &&
    typeof evt.detail.group === 'string' &&
    typeof evt.detail.isEditorOpen === 'boolean' &&
    typeof evt.detail.hasUserEdits === 'boolean' &&
    (typeof evt.detail.loadOrderIndex === 'number' ||
      typeof evt.detail.loadOrderIndex === 'undefined') &&
    typeof evt.detail.isLightMaster === 'boolean'
  );
}

function deduplicateTags(currentTags: Tag[], suggestedTags: Tag[]): PluginTags {
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

export function crcToString(crc: number): string {
  /* Pad CRC string to 8 characters. */
  return `00000000${crc.toString(16).toUpperCase()}`.slice(-8);
}

/* Messages, tags, CRCs and version strings can all be hidden by filters.
    Use getters with no setters for member variables as data should not be
    written to objects of this class. */
class PluginCardContent {
  private readonly _name: string;

  private _isActive: boolean;

  private _isEmpty: boolean;

  private _isMaster: boolean;

  private _isLightMaster: boolean;

  private _loadsArchive: boolean;

  private _version: string;

  private _crc: number;

  private _tags: PluginTags;

  private _messages: SimpleMessage[];

  public constructor(plugin: Plugin, filters: Filters) {
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

  public get name(): string {
    return this._name;
  }

  public get isActive(): boolean {
    return this._isActive;
  }

  public get isEmpty(): boolean {
    return this._isEmpty;
  }

  public get isMaster(): boolean {
    return this._isMaster;
  }

  public get isLightMaster(): boolean {
    return this._isLightMaster;
  }

  public get loadsArchive(): boolean {
    return this._loadsArchive;
  }

  public get version(): string {
    return this._version;
  }

  public get crc(): string {
    if (this._crc === 0) {
      return '';
    }

    /* Pad CRC string to 8 characters. */
    return crcToString(this._crc);
  }

  public get tags(): PluginTags {
    return this._tags;
  }

  public get messages(): SimpleMessage[] {
    return this._messages;
  }

  public containsText(text: string): boolean {
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

    for (const message of this.messages) {
      if (message.text.toLowerCase().indexOf(needle) !== -1) {
        return true;
      }
    }

    return false;
  }
}

export class Plugin {
  public readonly name: string;

  public isActive: boolean;

  private _isDirty: boolean;

  public isEmpty: boolean;

  public isMaster: boolean;

  public isLightMaster: boolean;

  public loadsArchive: boolean;

  private _messages: SimpleMessage[];

  private _suggestedTags: Tag[];

  private _currentTags: Tag[];

  public version: string;

  private _crc: number;

  private _group: string;

  private _loadOrderIndex?: number;

  private _cleanedWith: string;

  public masterlist?: PluginMetadata;

  private _userlist?: PluginMetadata;

  public id: string;

  private _isEditorOpen: boolean;

  private _isSearchResult: boolean;

  public cardZIndex: number;

  public constructor(obj: DerivedPluginMetadata) {
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
    this._cleanedWith = obj.cleanedWith || '';

    /* UI state variables */
    this.id = this.name.replace(/\s+/g, '');
    this._isEditorOpen = false;
    this._isSearchResult = false;
    this.cardZIndex = 0;
  }

  public update(plugin: DerivedPluginMetadata): void {
    if (!plugin) {
      return;
    }
    if (plugin.name !== this.name) {
      throw new Error(
        `Cannot update data for ${this.name} using data for ${plugin.name}`
      );
    }

    this.isActive = plugin.isActive;
    this.isDirty = plugin.isDirty;
    this.isEmpty = plugin.isEmpty;
    this.isMaster = plugin.isMaster;
    this.isLightMaster = plugin.isLightMaster;
    this.loadsArchive = plugin.loadsArchive;
    this.messages = plugin.messages;
    this.suggestedTags = plugin.suggestedTags;
    this.currentTags = plugin.currentTags;

    /* Set default values for fields that may not be present. */
    if (plugin.version === undefined) {
      this.version = '';
    } else {
      this.version = plugin.version;
    }

    if (plugin.crc === undefined) {
      this.crc = 0;
    } else {
      this.crc = plugin.crc;
    }

    if (plugin.group === undefined) {
      this.group = 'default';
    } else {
      this.group = plugin.group;
    }

    if (plugin.loadOrderIndex === undefined) {
      this.loadOrderIndex = undefined;
    } else {
      this.loadOrderIndex = plugin.loadOrderIndex;
    }

    if (plugin.cleanedWith === undefined) {
      this.cleanedWith = '';
    } else {
      this.cleanedWith = plugin.cleanedWith;
    }

    if (plugin.masterlist === undefined) {
      this.masterlist = undefined;
    } else {
      this.masterlist = plugin.masterlist;
    }
    if (plugin.userlist === undefined) {
      this.userlist = undefined;
    } else {
      this.userlist = plugin.userlist;
    }
  }

  public static tagFromRowData(rowData: TagRowData): Tag {
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

  public static tagToRowData(tag: Tag): TagRowData {
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

  private _dispatchCardContentChangeEvent(mayChangeCardHeight: boolean): void {
    document.dispatchEvent(
      new CustomEvent('loot-plugin-card-content-change', {
        detail: {
          pluginId: this.id,
          mayChangeCardHeight
        }
      })
    );
  }

  private _dispatchCardStylingChangeEvent(): void {
    document.dispatchEvent(
      new CustomEvent('loot-plugin-card-styling-change', {
        detail: { pluginId: this.id }
      })
    );
  }

  private _dispatchItemContentChangeEvent(): void {
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

  public get messages(): SimpleMessage[] {
    return this._messages;
  }

  public set messages(messages) {
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
      !isEqual(this._messages, messages)
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

  public get isDirty(): boolean {
    return this._isDirty;
  }

  public set isDirty(dirty) {
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

  public get cleanedWith(): string {
    return this._cleanedWith;
  }

  public set cleanedWith(cleanedWith) {
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

  public get crc(): number {
    return this._crc;
  }

  public set crc(crc) {
    if (this._crc !== crc) {
      this._crc = crc;

      this._dispatchCardContentChangeEvent(false);
    }
  }

  public get currentTags(): Tag[] {
    return this._currentTags;
  }

  public set currentTags(tags) {
    if (!isEqual(this._currentTags, tags)) {
      this._currentTags = tags;

      this._dispatchCardContentChangeEvent(true);
    }
  }

  public get suggestedTags(): Tag[] {
    return this._suggestedTags;
  }

  public set suggestedTags(tags) {
    if (!isEqual(this._suggestedTags, tags)) {
      this._suggestedTags = tags;

      this._dispatchCardContentChangeEvent(true);
    }
  }

  public get hasUserEdits(): boolean {
    return this.userlist !== undefined && Object.keys(this.userlist).length > 1;
  }

  public get userlist(): PluginMetadata | undefined {
    return this._userlist;
  }

  public set userlist(userlist) {
    if (!isEqual(this._userlist, userlist)) {
      this._userlist = userlist;

      this._dispatchItemContentChangeEvent();
      this._dispatchCardStylingChangeEvent();
    }
  }

  public get group(): string {
    return this._group;
  }

  public set group(group) {
    if (this._group !== group) {
      this._group = group;

      this._dispatchItemContentChangeEvent();
    }
  }

  public get isEditorOpen(): boolean {
    return this._isEditorOpen;
  }

  public set isEditorOpen(isEditorOpen) {
    if (this._isEditorOpen !== isEditorOpen) {
      this._isEditorOpen = isEditorOpen;

      this._dispatchItemContentChangeEvent();
    }
  }

  public get isSearchResult(): boolean {
    return this._isSearchResult;
  }

  public set isSearchResult(isSearchResult) {
    if (this._isSearchResult !== isSearchResult) {
      this._isSearchResult = isSearchResult;

      this._dispatchCardStylingChangeEvent();
    }
  }

  public get loadOrderIndex(): number | undefined {
    return this._loadOrderIndex;
  }

  public set loadOrderIndex(loadOrderIndex) {
    if (this._loadOrderIndex !== loadOrderIndex) {
      this._loadOrderIndex = loadOrderIndex;

      this._dispatchItemContentChangeEvent();
    }
  }

  public getCardContent(filters: Filters): PluginCardContent {
    return new PluginCardContent(this, filters);
  }

  public static onMessageChange(evt: Event): void {
    if (!isPluginMessageChangeEvent(evt)) {
      throw new TypeError(`Expected a PluginMessageChangeEvent, got ${evt}`);
    }

    incrementCounterText('filterTotalMessageNo', evt.detail.totalDiff);
    incrementCounterText('totalMessageNo', evt.detail.totalDiff);
    incrementCounterText('totalWarningNo', evt.detail.warningDiff);
    incrementCounterText('totalErrorNo', evt.detail.errorDiff);
  }

  public static onCleaningDataChange(evt: Event): void {
    if (!isPluginCleaningDataChangeEvent(evt)) {
      throw new TypeError(
        `Expected a PluginCleaningDataChangeEvent, got ${evt}`
      );
    }

    if (evt.detail.isDirty !== undefined) {
      if (evt.detail.isDirty) {
        incrementCounterText('dirtyPluginNo', 1);
      } else {
        incrementCounterText('dirtyPluginNo', -1);
      }
    }
    if (
      evt.detail.cleanedWith !== undefined &&
      evt.detail.pluginId !== undefined
    ) {
      const card = document.getElementById(
        evt.detail.pluginId
      ) as LootPluginCard;
      if (card !== null) {
        card.updateIsCleanIcon();
      }
    }
  }

  public static onContentChange(evt: Event): void {
    if (!isPluginContentChangeEvent(evt)) {
      throw new TypeError(`Expected a PluginContentChangeEvent, got ${evt}`);
    }

    const card = document.getElementById(evt.detail.pluginId) as LootPluginCard;
    if (card !== null) {
      card.updateContent(evt.detail.mayChangeCardHeight, false);
    }
  }

  public static onCardStylingChange(evt: Event): void {
    if (!isPluginCardStylingChangeEvent(evt)) {
      throw new TypeError(
        `Expected a PluginCardStylingChangeEvent, got ${evt}`
      );
    }

    const card = document.getElementById(evt.detail.pluginId) as LootPluginCard;
    if (card !== null) {
      card.updateStyling();
    }
  }

  public static onItemContentChange(evt: Event): void {
    if (!isPluginItemContentChangeEvent(evt)) {
      throw new TypeError(
        `Expected a PluginItemContentChangeEvent, got ${evt}`
      );
    }

    const item = querySelector(
      getElementById('cardsNav'),
      `[data-id="${evt.detail.pluginId}"]`
    ) as LootPluginItem;
    if (item) {
      item.updateContent(evt.detail);
    }
  }
}
