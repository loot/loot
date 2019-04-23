import * as _ from 'lodash/core.min';
import mergeGroups from './group.js';

import {
  createMessageItem,
  fillGroupsList,
  initialiseAutocompleteBashTags,
  initialiseAutocompleteFilenames,
  initialiseGroupsEditor,
  updateGroupsEditorState
} from './dom.js';
import Filters from './filters.js';
import { Plugin } from './plugin.js';

export default class Game {
  constructor(obj, l10n) {
    this.folder = obj.folder || '';
    this.generalMessages = obj.generalMessages || [];
    this.masterlist = obj.masterlist || {};
    this.plugins = obj.plugins || [];
    this.bashTags = obj.bashTags || [];
    this.groups = obj.groups;

    this.oldLoadOrder = undefined;

    this._notApplicableString = l10n.translate('N/A');
  }

  get folder() {
    return this._folder;
  }

  set folder(folder) {
    if (folder !== this._folder) {
      document.dispatchEvent(
        new CustomEvent('loot-game-folder-change', {
          detail: { folder }
        })
      );
    }

    this._folder = folder;
  }

  get generalMessages() {
    return this._generalMessages;
  }

  set generalMessages(generalMessages) {
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
      !_.isEqual(this._generalMessages, generalMessages)
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

  get masterlist() {
    return this._masterlist;
  }

  set masterlist(masterlist) {
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

  get plugins() {
    return this._plugins;
  }

  set plugins(plugins) {
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

  get groups() {
    return this._groups;
  }

  set groups(groups) {
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

  getContent() {
    let messages = [];
    let plugins = [];

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

  getPluginNames() {
    return this.plugins.map(plugin => plugin.name);
  }

  getGroupPluginNames(groupName) {
    return this.plugins
      .filter(plugin => plugin.group === groupName)
      .map(plugin => plugin.name);
  }

  setSortedPlugins(plugins) {
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

  applySort() {
    this.oldLoadOrder = undefined;
  }

  cancelSort(plugins, generalMessages) {
    this.plugins = plugins.reduce((existingPlugins, plugin) => {
      const existingPlugin = this.oldLoadOrder.find(
        item => item.name === plugin.name
      );
      if (existingPlugin) {
        existingPlugin.update(plugin);
        existingPlugins.push(existingPlugin);
      }

      return existingPlugins;
    }, []);
    this.oldLoadOrder = undefined;

    /* Update general messages */
    this.generalMessages = generalMessages;
  }

  clearMetadata(plugins) {
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

  initialiseUI() {
    /* Re-initialise autocomplete suggestions. */
    initialiseAutocompleteFilenames(this.getPluginNames());
    initialiseAutocompleteBashTags(this.bashTags);

    /* Re-initialise conflicts filter plugin list. */
    Filters.fillConflictsFilterList(this.plugins);

    initialiseGroupsEditor(groupName => this.getGroupPluginNames(groupName));
  }

  static onPluginsChange(evt) {
    if (!evt.detail.valuesAreTotals) {
      evt.detail.totalMessageNo += parseInt(
        document.getElementById('totalMessageNo').textContent,
        10
      );
      evt.detail.warnMessageNo += parseInt(
        document.getElementById('totalWarningNo').textContent,
        10
      );
      evt.detail.errorMessageNo += parseInt(
        document.getElementById('totalErrorNo').textContent,
        10
      );
      evt.detail.totalPluginNo += parseInt(
        document.getElementById('totalPluginNo').textContent,
        10
      );
      evt.detail.activePluginNo += parseInt(
        document.getElementById('activePluginNo').textContent,
        10
      );
      evt.detail.dirtyPluginNo += parseInt(
        document.getElementById('dirtyPluginNo').textContent,
        10
      );
    }

    document.getElementById('filterTotalMessageNo').textContent =
      evt.detail.totalMessageNo;
    document.getElementById('totalMessageNo').textContent =
      evt.detail.totalMessageNo;
    document.getElementById('totalWarningNo').textContent =
      evt.detail.warnMessageNo;
    document.getElementById('totalErrorNo').textContent =
      evt.detail.errorMessageNo;

    document.getElementById('filterTotalPluginNo').textContent =
      evt.detail.totalPluginNo;
    document.getElementById('totalPluginNo').textContent =
      evt.detail.totalPluginNo;
    document.getElementById('activePluginNo').textContent =
      evt.detail.activePluginNo;
    document.getElementById('dirtyPluginNo').textContent =
      evt.detail.dirtyPluginNo;
  }

  static ongeneralMessagesChange(evt) {
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

    /* Remove old messages from UI. */
    const generalMessagesList = document
      .getElementById('summary')
      .getElementsByTagName('ul')[0];
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
    const summary = document.getElementById('summary');
    const summaryStyle = getComputedStyle(summary);
    document.getElementById('pluginCardList').scrollOffset =
      summary.offsetHeight +
      parseInt(summaryStyle.marginTop, 10) +
      parseInt(summaryStyle.marginBottom, 10);
  }

  static onMasterlistChange(evt) {
    document.getElementById('masterlistRevision').textContent =
      evt.detail.revision;
    document.getElementById('masterlistDate').textContent = evt.detail.date;
  }

  static onGroupsChange(evt) {
    fillGroupsList(evt.detail.groups);
    updateGroupsEditorState(evt.detail.groups);
  }
}
