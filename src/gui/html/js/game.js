'use strict';

(function exportModule(root, factory) {
  if (typeof define === 'function' && define.amd) {
    // AMD. Register as an anonymous module.
    define([], factory);
  } else {
    // Browser globals
    root.loot = root.loot || {};
    root.loot.Game = factory(root.marked);
  }
}(this, (marked) => {
  return class Game {
    constructor(obj, l10n) {
      this.folder = obj.folder || '';
      this.globalMessages = obj.globalMessages || [];
      this.masterlist = obj.masterlist || {};
      this.plugins = obj.plugins || [];

      this.loadOrder = undefined;
      this.oldLoadOrder = undefined;

      this._notApplicableString = l10n.translate('N/A');
    }

    get folder() {
      return this._folder;
    }

    set folder(folder) {
      if (folder !== this._folder) {
        document.dispatchEvent(new CustomEvent('loot-game-folder-change', {
          detail: { folder },
        }));
      }

      this._folder = folder;
    }

    get globalMessages() {
      return this._globalMessages;
    }

    set globalMessages(globalMessages) {
      /* Update the message counts. */
      let oldTotal = 0;
      let newTotal = 0;
      let oldWarns = 0;
      let newWarns = 0;
      let oldErrs = 0;
      let newErrs = 0;

      if (this._globalMessages) {
        oldTotal = this._globalMessages.length;
        this._globalMessages.forEach((message) => {
          if (message.type === 'warn') {
            ++oldWarns;
          } else if (message.type === 'error') {
            ++oldErrs;
          }
        });
      }

      if (globalMessages) {
        newTotal = globalMessages.length;

        globalMessages.forEach((message) => {
          if (message.type === 'warn') {
            ++newWarns;
          } else if (message.type === 'error') {
            ++newErrs;
          }
        });
      }

      if (newTotal !== oldTotal || newWarns !== oldWarns || newErrs !== oldErrs) {
        document.dispatchEvent(new CustomEvent('loot-game-global-messages-change', {
          detail: {
            totalDiff: newTotal - oldTotal,
            warningDiff: newWarns - oldWarns,
            errorDiff: newErrs - oldErrs,
            messages: globalMessages,
          },
        }));
      }

      this._globalMessages = globalMessages;
    }

    get masterlist() {
      return this._masterlist;
    }

    set masterlist(masterlist) {
      if (masterlist !== this._masterlist
          && (masterlist === undefined || this._masterlist === undefined
          || masterlist.revision !== this._masterlist.revision
          || masterlist.date !== this._masterlist.date)) {
        let revision = this._notApplicableString;
        let date = this._notApplicableString;
        if (masterlist && masterlist.revision) {
          revision = masterlist.revision;
        }
        if (masterlist && masterlist.date) {
          date = masterlist.date;
        }

        document.dispatchEvent(new CustomEvent('loot-game-masterlist-change', {
          detail: {
            revision,
            date,
          },
        }));
      }

      this._masterlist = masterlist;
    }

    get plugins() {
      return this._plugins;
    }

    set plugins(plugins) {
      /* Update plugin and message counts. Unlike for global messages
         it's not worth calculating the count differences, just count
         from zero. */
      let totalMessageNo = 0;
      let warnMessageNo = 0;
      let errorMessageNo = 0;
      let activePluginNo = 0;
      let dirtyPluginNo = 0;

      /* Include global messages in the count. */
      if (this.globalMessages) {
        totalMessageNo = this.globalMessages.length;
        this.globalMessages.forEach((message) => {
          if (message.type === 'warn') {
            ++warnMessageNo;
          } else if (message.type === 'error') {
            ++errorMessageNo;
          }
        });
      }

      plugins.forEach((plugin) => {
        if (plugin.isActive) {
          ++activePluginNo;
        }
        if (plugin.isDirty) {
          ++dirtyPluginNo;
        }
        if (plugin.messages) {
          totalMessageNo += plugin.messages.length;
          plugin.messages.forEach((message) => {
            if (message.type === 'warn') {
              ++warnMessageNo;
            } else if (message.type === 'error') {
              ++errorMessageNo;
            }
          });
        }
      });

      document.dispatchEvent(new CustomEvent('loot-game-plugins-change', {
        detail: {
          valuesAreTotals: true,
          totalMessageNo,
          warnMessageNo,
          errorMessageNo,
          totalPluginNo: plugins.length,
          activePluginNo,
          dirtyPluginNo,
        },
      }));

      this._plugins = plugins;
    }

    appendPlugin(plugin) {
      let totalChange = 0;
      let warnChange = 0;
      let errorChange = 0;
      let activeChange = 0;
      let dirtyChange = 0;

      if (plugin.isActive) {
        ++activeChange;
      }
      if (plugin.isDirty) {
        ++dirtyChange;
      }
      if (plugin.messages) {
        totalChange += plugin.messages.length;
        plugin.messages.forEach((message) => {
          if (message.type === 'warn') {
            ++warnChange;
          } else if (message.type === 'error') {
            ++errorChange;
          }
        });
      }

      document.dispatchEvent(new CustomEvent('loot-game-plugins-change', {
        detail: {
          valuesAreTotals: false,
          totalMessageNo: totalChange,
          warnMessageNo: warnChange,
          errorMessageNo: errorChange,
          totalPluginNo: 1,
          activePluginNo: activeChange,
          dirtyPluginNo: dirtyChange,
        },
      }));

      this._plugins.push(plugin);
    }

    removePluginAtIndex(index) {
      let totalChange = 0;
      let warnChange = 0;
      let errorChange = 0;
      let activeChange = 0;
      let dirtyChange = 0;

      if (this._plugins[index].isActive) {
        --activeChange;
      }
      if (this._plugins[index].isDirty) {
        --dirtyChange;
      }
      if (this._plugins[index].messages) {
        totalChange -= this._plugins[index].messages.length;
        this._plugins[index].messages.forEach((message) => {
          if (message.type === 'warn') {
            --warnChange;
          } else if (message.type === 'error') {
            --errorChange;
          }
        });
      }

      document.dispatchEvent(new CustomEvent('loot-game-plugins-change', {
        detail: {
          valuesAreTotals: false,
          totalMessageNo: totalChange,
          warnMessageNo: warnChange,
          errorMessageNo: errorChange,
          totalPluginNo: -1,
          activePluginNo: activeChange,
          dirtyPluginNo: dirtyChange,
        },
      }));

      this._plugins.splice(index, 1);
    }

    static onPluginsChange(evt) {
      if (!evt.detail.valuesAreTotals) {
        evt.detail.totalMessageNo += parseInt(document.getElementById('totalMessageNo').textContent, 10);
        evt.detail.warnMessageNo += parseInt(document.getElementById('totalWarningNo').textContent, 10);
        evt.detail.errorMessageNo += parseInt(document.getElementById('totalErrorNo').textContent, 10);
        evt.detail.totalPluginNo += parseInt(document.getElementById('totalPluginNo').textContent, 10);
        evt.detail.activePluginNo += parseInt(document.getElementById('activePluginNo').textContent, 10);
        evt.detail.dirtyPluginNo += parseInt(document.getElementById('dirtyPluginNo').textContent, 10);
      }

      document.getElementById('filterTotalMessageNo').textContent = evt.detail.totalMessageNo;
      document.getElementById('totalMessageNo').textContent = evt.detail.totalMessageNo;
      document.getElementById('totalWarningNo').textContent = evt.detail.warnMessageNo;
      document.getElementById('totalErrorNo').textContent = evt.detail.errorMessageNo;

      document.getElementById('filterTotalPluginNo').textContent = evt.detail.totalPluginNo;
      document.getElementById('totalPluginNo').textContent = evt.detail.totalPluginNo;
      document.getElementById('activePluginNo').textContent = evt.detail.activePluginNo;
      document.getElementById('dirtyPluginNo').textContent = evt.detail.dirtyPluginNo;
    }
    static onGlobalMessagesChange(evt) {
      document.getElementById('filterTotalMessageNo').textContent = parseInt(document.getElementById('filterTotalMessageNo').textContent, 10) + evt.detail.totalDiff;
      document.getElementById('totalMessageNo').textContent = parseInt(document.getElementById('totalMessageNo').textContent, 10) + evt.detail.totalDiff;
      document.getElementById('totalWarningNo').textContent = parseInt(document.getElementById('totalWarningNo').textContent, 10) + evt.detail.warningDiff;
      document.getElementById('totalErrorNo').textContent = parseInt(document.getElementById('totalErrorNo').textContent, 10) + evt.detail.errorDiff;

      /* Remove old messages from UI. */
      const generalMessagesList = document.getElementById('summary').getElementsByTagName('ul')[0];
      while (generalMessagesList.firstElementChild) {
        generalMessagesList.removeChild(generalMessagesList.firstElementChild);
      }

      /* Add new messages. */
      if (evt.detail.messages) {
        evt.detail.messages.forEach((message) => {
          const li = document.createElement('li');
          li.className = message.type;
          /* Use the Marked library for Markdown formatting support. */
          li.innerHTML = marked(message.content[0].str);
          generalMessagesList.appendChild(li);
        });
      }
    }
    static onMasterlistChange(evt) {
      document.getElementById('masterlistRevision').textContent = evt.detail.revision;
      document.getElementById('masterlistDate').textContent = evt.detail.date;
    }
    static onFolderChange(evt) {
      loot.dom.updateSelectedGame(evt.detail.folder);
      /* Enable/disable the redate plugins option. */
      let gameSettings = undefined;
      if (loot.settings && loot.settings.games) {
        gameSettings = loot.settings.games.find((game) => {
          return game.folder === evt.detail.folder;
        });
      }
      const redateButton = document.getElementById('redatePluginsButton');
      if (gameSettings && gameSettings.type === 'Skyrim') {
        redateButton.removeAttribute('disabled');
      } else {
        redateButton.setAttribute('disabled', true);
      }
    }
  };
}));
