import { isEqual } from 'lodash';
import { IronListElement } from '@polymer/iron-list';
import mergeGroups from './group';

import {
  createMessageItem,
  fillGroupsList,
  initialiseAutocompleteBashTags,
  initialiseAutocompleteFilenames,
  initialiseGroupsEditor,
  updateGroupsEditorState,
  initialiseVirtualLists,
  updateSelectedGame,
  enableGameOperations
} from './dom';
import Filters from './filters';
import { Plugin } from './plugin';
import Translator from './translator';
import {
  GameContent,
  GameData,
  PluginContent,
  SimpleMessage,
  SourcedGroup,
  Masterlist,
  GameGroups,
  DerivedPluginMetadata,
  PluginLoadOrderIndex
} from './interfaces';
import {
  getTextAsInt,
  setTextContent,
  incrementCounterText,
  getElementById
} from './dom/helpers';

interface GamePluginsChangeEvent extends CustomEvent {
  detail: {
    valuesAreTotals: boolean;
    totalMessageNo: number;
    warnMessageNo: number;
    errorMessageNo: number;
    totalPluginNo: number;
    activePluginNo: number;
    dirtyPluginNo: number;
  };
}

function isPluginsChangeEvent(evt: Event): evt is GamePluginsChangeEvent {
  return (
    evt instanceof CustomEvent &&
    typeof evt.detail.valuesAreTotals === 'boolean' &&
    typeof evt.detail.totalMessageNo === 'number' &&
    typeof evt.detail.warnMessageNo === 'number' &&
    typeof evt.detail.errorMessageNo === 'number' &&
    typeof evt.detail.totalPluginNo === 'number' &&
    typeof evt.detail.activePluginNo === 'number' &&
    typeof evt.detail.dirtyPluginNo === 'number'
  );
}

interface GameGeneralMessagesChangeEvent extends CustomEvent {
  detail: {
    totalDiff: number;
    warningDiff: number;
    errorDiff: number;
    messages: SimpleMessage[];
  };
}

function isGeneralMessagesChangeEvent(
  evt: Event
): evt is GameGeneralMessagesChangeEvent {
  return (
    evt instanceof CustomEvent &&
    typeof evt.detail.totalDiff === 'number' &&
    typeof evt.detail.warningDiff === 'number' &&
    typeof evt.detail.errorDiff === 'number' &&
    Array.isArray(evt.detail.messages)
  );
}

interface GameMasterlistChangeEvent extends CustomEvent {
  detail: {
    revision: string;
    date: string;
  };
}

function isMasterlistChangeEvent(evt: Event): evt is GameMasterlistChangeEvent {
  return (
    evt instanceof CustomEvent &&
    typeof evt.detail.revision === 'string' &&
    typeof evt.detail.date === 'string'
  );
}

interface GameGroupsChangeEvent extends CustomEvent {
  detail: { groups: SourcedGroup[] };
}

function isGroupsChangeEvent(evt: Event): evt is GameGroupsChangeEvent {
  return evt instanceof CustomEvent && Array.isArray(evt.detail.groups);
}

export default class Game {
  private _folder: string;

  private _generalMessages: SimpleMessage[];

  private _masterlist: Masterlist;

  private _plugins: Plugin[];

  private _groups: SourcedGroup[];

  private bashTags: string[];

  public oldLoadOrder: Plugin[];

  private _notApplicableString: string;

  public constructor(obj: GameData, l10n: Translator) {
    this._folder = '';
    this._generalMessages = [];
    this._masterlist = { revision: '', date: '' };
    this._plugins = [];
    this._groups = [];

    this.folder = obj.folder || '';
    this.generalMessages = obj.generalMessages || [];
    this.masterlist = obj.masterlist || { revision: '', date: '' };
    this.plugins = obj.plugins ? obj.plugins.map(p => new Plugin(p)) : [];
    this.bashTags = obj.bashTags || [];
    this.setGroups(obj.groups);

    this.oldLoadOrder = [];

    this._notApplicableString = l10n.translate('N/A');
  }

  public get folder(): string {
    return this._folder;
  }

  public set folder(folder) {
    if (folder !== this._folder) {
      document.dispatchEvent(
        new CustomEvent('loot-game-folder-change', {
          detail: { folder }
        })
      );
    }

    this._folder = folder;
  }

  public get generalMessages(): SimpleMessage[] {
    return this._generalMessages;
  }

  public set generalMessages(generalMessages) {
    /* Update the message counts. */
    let oldTotal = 0;
    let newTotal = 0;
    let oldWarns = 0;
    let newWarns = 0;
    let oldErrs = 0;
    let newErrs = 0;

    if (this._generalMessages) {
      oldTotal = this._generalMessages.length;
      this._generalMessages.forEach(message => {
        if (message.type === 'warn') {
          oldWarns += 1;
        } else if (message.type === 'error') {
          oldErrs += 1;
        }
      });
    }

    if (generalMessages) {
      newTotal = generalMessages.length;

      generalMessages.forEach(message => {
        if (message.type === 'warn') {
          newWarns += 1;
        } else if (message.type === 'error') {
          newErrs += 1;
        }
      });
    }

    if (
      newTotal !== oldTotal ||
      newWarns !== oldWarns ||
      newErrs !== oldErrs ||
      !isEqual(this._generalMessages, generalMessages)
    ) {
      document.dispatchEvent(
        new CustomEvent('loot-game-global-messages-change', {
          detail: {
            totalDiff: newTotal - oldTotal,
            warningDiff: newWarns - oldWarns,
            errorDiff: newErrs - oldErrs,
            messages: generalMessages
          }
        })
      );
    }

    this._generalMessages = generalMessages;
  }

  public get masterlist(): Masterlist {
    return this._masterlist;
  }

  public set masterlist(masterlist) {
    if (
      masterlist !== this._masterlist &&
      (masterlist === undefined ||
        this._masterlist === undefined ||
        masterlist.revision !== this._masterlist.revision ||
        masterlist.date !== this._masterlist.date)
    ) {
      const {
        revision = this._notApplicableString,
        date = this._notApplicableString
      } = masterlist || {};

      document.dispatchEvent(
        new CustomEvent('loot-game-masterlist-change', {
          detail: {
            revision,
            date
          }
        })
      );
    }

    this._masterlist = masterlist;
  }

  public get plugins(): Plugin[] {
    return this._plugins;
  }

  public set plugins(plugins) {
    /* Update plugin and message counts. Unlike for general messages
    it's not worth calculating the count differences, just count
    from zero. */
    let totalMessageNo = 0;
    let warnMessageNo = 0;
    let errorMessageNo = 0;
    let activePluginNo = 0;
    let dirtyPluginNo = 0;

    /* Include general messages in the count. */
    if (this.generalMessages) {
      totalMessageNo = this.generalMessages.length;
      this.generalMessages.forEach(message => {
        if (message.type === 'warn') {
          warnMessageNo += 1;
        } else if (message.type === 'error') {
          errorMessageNo += 1;
        }
      });
    }

    plugins.forEach((plugin, index) => {
      /* Recalculate each plugin card's z-index value. */
      plugin.cardZIndex = plugins.length - index;

      if (plugin.isActive) {
        activePluginNo += 1;
      }
      if (plugin.isDirty) {
        dirtyPluginNo += 1;
      }
      if (plugin.messages) {
        totalMessageNo += plugin.messages.length;
        plugin.messages.forEach(message => {
          if (message.type === 'warn') {
            warnMessageNo += 1;
          } else if (message.type === 'error') {
            errorMessageNo += 1;
          }
        });
      }
    });

    document.dispatchEvent(
      new CustomEvent('loot-game-plugins-change', {
        detail: {
          valuesAreTotals: true,
          totalMessageNo,
          warnMessageNo,
          errorMessageNo,
          totalPluginNo: plugins.length,
          activePluginNo,
          dirtyPluginNo
        }
      })
    );

    this._plugins = plugins;
  }

  public get groups(): SourcedGroup[] {
    return this._groups;
  }

  public setGroups(groups: GameGroups): void {
    if (groups) {
      this._groups = mergeGroups(groups.masterlist, groups.userlist);
    } else {
      this._groups = [
        {
          name: 'default',
          isUserAdded: false,
          after: []
        }
      ];
    }

    document.dispatchEvent(
      new CustomEvent('loot-game-groups-change', {
        detail: { groups: this._groups }
      })
    );
  }

  public getContent(): GameContent {
    let messages: SimpleMessage[] = [];
    let plugins: PluginContent[] = [];

    if (this.generalMessages) {
      messages = this.generalMessages;
    }
    if (this.plugins) {
      plugins = this.plugins.map(plugin => ({
        name: plugin.name,
        crc: plugin.crc,
        version: plugin.version,
        isActive: plugin.isActive,
        isEmpty: plugin.isEmpty,
        loadsArchive: plugin.loadsArchive,

        group: plugin.group,
        messages: plugin.messages,
        currentTags: plugin.currentTags,
        suggestedTags: plugin.suggestedTags,
        isDirty: plugin.isDirty
      }));
    }

    return {
      messages,
      plugins
    };
  }

  public getPluginNames(): string[] {
    return this.plugins.map(plugin => plugin.name);
  }

  public getGroupPluginNames(groupName: string): string[] {
    return this.plugins
      .filter(plugin => plugin.group === groupName)
      .map(plugin => plugin.name);
  }

  public setSortedPlugins(plugins: DerivedPluginMetadata[]): void {
    this.oldLoadOrder = this.plugins;

    this.plugins = plugins.map(plugin => {
      const existingPlugin = this.oldLoadOrder.find(
        item => item.name === plugin.name
      );
      if (existingPlugin) {
        existingPlugin.update(plugin);
        return existingPlugin;
      }
      return new Plugin(plugin);
    });
  }

  public applySort(): void {
    this.oldLoadOrder = [];
  }

  public cancelSort(
    plugins: PluginLoadOrderIndex[],
    generalMessages: SimpleMessage[]
  ): void {
    this.plugins = plugins.reduce((existingPlugins: Plugin[], plugin) => {
      const existingPlugin = this.oldLoadOrder.find(
        item => item.name === plugin.name
      );
      if (existingPlugin) {
        existingPlugin.loadOrderIndex = plugin.loadOrderIndex;
        existingPlugins.push(existingPlugin);
      }

      return existingPlugins;
    }, []);
    this.oldLoadOrder = [];

    /* Update general messages */
    this.generalMessages = generalMessages;
  }

  public clearMetadata(plugins: DerivedPluginMetadata[]): void {
    /* Need to empty the UI-side user metadata. */
    plugins.forEach(plugin => {
      const existingPlugin = this.plugins.find(
        item => item.name === plugin.name
      );
      if (existingPlugin) {
        existingPlugin.update(plugin);
      }
    });
  }

  public initialiseUI(filters: Filters): void {
    /* Enable game operations if they were disabled. */
    enableGameOperations(true);

    /* Re-initialise autocomplete suggestions. */
    initialiseAutocompleteFilenames(this.getPluginNames());
    initialiseAutocompleteBashTags(this.bashTags);

    /* Re-initialise conflicts filter plugin list. */
    Filters.fillConflictsFilterList(this.plugins);

    initialiseGroupsEditor(groupName => this.getGroupPluginNames(groupName));

    updateSelectedGame(this.folder);

    /* Now update virtual lists. */
    if (filters.areAnyFiltersActive()) {
      /* Schedule applying the filters instead of applying them immediately.
        This improves the UI initialisation speed, and is quick enough that
        the lists aren't visible pre-filtration. */
      setTimeout(() => filters.apply(this.plugins), 0);
    } else {
      initialiseVirtualLists(this.plugins);
    }
  }

  public static onPluginsChange(evt: Event): void {
    if (!isPluginsChangeEvent(evt)) {
      throw new TypeError(`Expected a GamePluginsChangeEvent, got ${evt}`);
    }

    if (!evt.detail.valuesAreTotals) {
      evt.detail.totalMessageNo += getTextAsInt('totalMessageNo');
      evt.detail.warnMessageNo += getTextAsInt('totalWarningNo');
      evt.detail.errorMessageNo += getTextAsInt('totalErrorNo');
      evt.detail.totalPluginNo += getTextAsInt('totalPluginNo');
      evt.detail.activePluginNo += getTextAsInt('activePluginNo');
      evt.detail.dirtyPluginNo += getTextAsInt('dirtyPluginNo');
    }

    setTextContent('filterTotalMessageNo', evt.detail.totalMessageNo);
    setTextContent('totalMessageNo', evt.detail.totalMessageNo);
    setTextContent('totalWarningNo', evt.detail.warnMessageNo);
    setTextContent('totalErrorNo', evt.detail.errorMessageNo);

    setTextContent('filterTotalPluginNo', evt.detail.totalPluginNo);
    setTextContent('totalPluginNo', evt.detail.totalPluginNo);
    setTextContent('activePluginNo', evt.detail.activePluginNo);
    setTextContent('dirtyPluginNo', evt.detail.dirtyPluginNo);
  }

  public static onGeneralMessagesChange(evt: Event): void {
    if (!isGeneralMessagesChangeEvent(evt)) {
      throw new TypeError(
        `Expected a GameGeneralMessagesChangeEvent, got ${evt}`
      );
    }

    incrementCounterText('filterTotalMessageNo', evt.detail.totalDiff);
    incrementCounterText('totalMessageNo', evt.detail.totalDiff);
    incrementCounterText('totalWarningNo', evt.detail.warningDiff);
    incrementCounterText('totalErrorNo', evt.detail.errorDiff);

    /* Remove old messages from UI. */
    const summary = getElementById('summary');

    const generalMessagesList = summary.getElementsByTagName('ul')[0];
    while (generalMessagesList.firstElementChild) {
      generalMessagesList.removeChild(generalMessagesList.firstElementChild);
    }

    /* Add new messages. */
    if (evt.detail.messages) {
      evt.detail.messages.forEach(message => {
        generalMessagesList.appendChild(
          createMessageItem(message.type, message.text)
        );
      });
    }

    /* Update the plugin card list's configured offset. */
    const cardList = getElementById('pluginCardList') as IronListElement;

    const summaryStyle = getComputedStyle(summary);

    cardList.scrollOffset =
      summary.offsetHeight +
      parseInt(summaryStyle.marginTop || '0', 10) +
      parseInt(summaryStyle.marginBottom || '0', 10);
  }

  public static onMasterlistChange(evt: Event): void {
    if (!isMasterlistChangeEvent(evt)) {
      throw new TypeError(`Expected a GameMasterlistChangeEvent, got ${evt}`);
    }

    getElementById('masterlistRevision').textContent = evt.detail.revision;
    getElementById('masterlistDate').textContent = evt.detail.date;
  }

  public static onGroupsChange(evt: Event): void {
    if (!isGroupsChangeEvent(evt)) {
      throw new TypeError(`Expected a GameGroupsChangeEvent, got ${evt}`);
    }
    fillGroupsList(evt.detail.groups);
    updateGroupsEditorState(evt.detail.groups);
  }
}
