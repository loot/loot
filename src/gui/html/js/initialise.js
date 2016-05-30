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
    root.loot.initialise = factory(root.loot.Dialog,
                                    root.loot.DOM,
                                    root.loot.Filters,
                                    root.loot.Game,
                                    root.loot.translateStaticText,
                                    root.loot.Plugin,
                                    root.loot.query,
                                    root.loot.Translator);
  }
}(this, (Dialog, dom, Filters, Game, translateStaticText, Plugin, query, Translator) => {
  function setupEventHandlers() {
    /* Set up handlers for filters. */
    document.getElementById('hideVersionNumbers').addEventListener('change', onSidebarFilterToggle);
    document.getElementById('hideCRCs').addEventListener('change', onSidebarFilterToggle);
    document.getElementById('hideBashTags').addEventListener('change', onSidebarFilterToggle);
    document.getElementById('hideNotes').addEventListener('change', onSidebarFilterToggle);
    document.getElementById('hideDoNotCleanMessages').addEventListener('change', onSidebarFilterToggle);
    document.getElementById('hideInactivePlugins').addEventListener('change', onSidebarFilterToggle);
    document.getElementById('hideAllPluginMessages').addEventListener('change', onSidebarFilterToggle);
    document.getElementById('hideMessagelessPlugins').addEventListener('change', onSidebarFilterToggle);
    document.body.addEventListener('loot-filter-conflicts', onConflictsFilter);

    /* Set up event handlers for content filter. */
    document.getElementById('contentFilter').addEventListener('change', onSidebarFilterToggle);

    /* Set up handlers for buttons. */
    document.getElementById('redatePluginsButton').addEventListener('click', onRedatePlugins);
    document.getElementById('openLogButton').addEventListener('click', onOpenLogLocation);
    document.getElementById('wipeUserlistButton').addEventListener('click', onClearAllMetadata);
    document.getElementById('copyLoadOrderButton').addEventListener('click', onCopyLoadOrder);
    document.getElementById('copyContentButton').addEventListener('click', onCopyContent);
    document.getElementById('refreshContentButton').addEventListener('click', onContentRefresh);
    document.getElementById('settingsButton').addEventListener('click', onShowSettingsDialog);
    document.getElementById('helpButton').addEventListener('click', onOpenReadme);
    document.getElementById('aboutButton').addEventListener('click', onShowAboutDialog);
    document.getElementById('quitButton').addEventListener('click', onQuit);
    document.getElementById('gameMenu').addEventListener('iron-select', onChangeGame);
    document.getElementById('updateMasterlistButton').addEventListener('click', onUpdateMasterlist);
    document.getElementById('sortButton').addEventListener('click', onSortPlugins);
    document.getElementById('applySortButton').addEventListener('click', onApplySort);
    document.getElementById('cancelSortButton').addEventListener('click', onCancelSort);
    document.getElementById('sidebarTabs').addEventListener('iron-select', onSwitchSidebarTab);
    document.getElementById('jumpToGeneralInfo').addEventListener('click', onJumpToGeneralInfo);

    /* Set up search event handlers. */
    document.getElementById('showSearch').addEventListener('click', onSearchOpen);
    document.getElementById('searchBar').addEventListener('loot-search-begin', onSearchBegin);
    document.getElementById('searchBar').addEventListener('loot-search-change-selection', onSearchChangeSelection, false);
    document.getElementById('searchBar').addEventListener('loot-search-end', onSearchEnd);
    window.addEventListener('keyup', onFocusSearch);

    /* Set up event handlers for settings dialog. */
    const settings = document.getElementById('settingsDialog');
    settings.addEventListener('iron-overlay-closed', onCloseSettingsDialog);
    settings.querySelector('[dialog-confirm]').addEventListener('tap', onApplySettings);

    /* Set up handler for opening and closing editors. */
    document.body.addEventListener('loot-editor-open', onEditorOpen);
    document.body.addEventListener('loot-editor-close', onEditorClose);
    document.body.addEventListener('loot-copy-metadata', onCopyMetadata);
    document.body.addEventListener('loot-clear-metadata', onClearMetadata);

    document.getElementById('cardsNav').addEventListener('click', onSidebarClick);
    document.getElementById('cardsNav').addEventListener('dblclick', onSidebarClick);

    /* Set up handler for plugin data changes. */
    document.addEventListener('loot-plugin-message-change', Plugin.onMessageChange);
    document.addEventListener('loot-plugin-message-change', Plugin.onContentChange);
    document.addEventListener('loot-plugin-isdirty-change', Plugin.onIsDirtyChange);
    document.addEventListener('loot-plugin-card-content-change', Plugin.onContentChange);
    document.addEventListener('loot-plugin-card-styling-change', Plugin.onCardStylingChange);
    document.addEventListener('loot-plugin-item-content-change', Plugin.onItemContentChange);

    /* Set up event handlers for game member variable changes. */
    document.addEventListener('loot-game-folder-change', Game.onFolderChange);
    document.addEventListener('loot-game-masterlist-change', Game.onMasterlistChange);
    document.addEventListener('loot-game-global-messages-change', Game.onGlobalMessagesChange);
    document.addEventListener('loot-game-plugins-change', Game.onPluginsChange);
  }

  function applyEnabledFilters(filters, settings, plugins) {
    if (!filters) {
      return;
    }

    if (settings.filters) {
      for (const filter in settings.filters) {
        filters[filter] = settings.filters[filter];
        document.getElementById(filter).checked = filters[filter];
      }
    }

    if (filters.hideMessagelessPlugins
        || filters.hideInactivePlugins
        || filters.hideNotes
        || filters.hideDoNotCleanMessages
        || filters.hideAllPluginMessages) {
      filterPluginData(plugins, filters);
    }

    if (filters.hideVersionNumbers) {
      document.getElementById('hideVersionNumbers').dispatchEvent(new Event('change'));
    }

    if (filters.hideCRCs) {
      document.getElementById('hideCRCs').dispatchEvent(new Event('change'));
    }

    if (filters.hideBashTags) {
      document.getElementById('hideBashTags').dispatchEvent(new Event('change'));
    }
  }

  function setVersion(appData) {
    return query('getVersion').then(JSON.parse).then((result) => {
      /* The fourth part of the version string is the build number. Trim it. */
      const pos = result.lastIndexOf('.');
      appData.version = result.substring(0, pos);
      document.getElementById('LOOTVersion').textContent = appData.version;
      document.getElementById('firstTimeLootVersion').textContent = appData.version;
      document.getElementById('LOOTBuild').textContent = result.substring(pos + 1);
    });
  }

  function setLanguages() {
    return query('getLanguages').then(JSON.parse).then((result) => {
      /* Now fill in language options. */
      const settingsLangSelect = document.getElementById('languageSelect');
      const messageLangSelect = dom.getElementInTableRowTemplate('messageRow', 'language');

      result.forEach((language) => {
        const settingsItem = document.createElement('paper-item');
        settingsItem.setAttribute('value', language.locale);
        settingsItem.textContent = language.name;
        settingsLangSelect.appendChild(settingsItem);
        messageLangSelect.appendChild(settingsItem.cloneNode(true));
      });

      messageLangSelect.setAttribute('value', messageLangSelect.firstElementChild.getAttribute('value'));
    });
  }

  function displayInitErrors() {
    return query('getInitErrors').then(JSON.parse).then((result) => {
      if (!result) {
        return result;
      }
      const generalMessagesList = document.getElementById('summary').getElementsByTagName('ul')[0];

      result.forEach((message) => {
        const li = document.createElement('li');
        li.className = 'error';
        /* Use the Marked library for Markdown formatting support. */
        li.innerHTML = window.marked(message);
        generalMessagesList.appendChild(li);
      });

      document.getElementById('filterTotalMessageNo').textContent = result.length;
      document.getElementById('totalMessageNo').textContent = result.length;
      document.getElementById('totalErrorNo').textContent = result.length;

      return result;
    });
  }

  function setGameTypes() {
    return query('getGameTypes').then(JSON.parse).then((result) => {
      /* Fill in game row template's game type options. */
      const select = dom.getElementInTableRowTemplate('gameRow', 'type');
      result.forEach((gameType) => {
        const item = document.createElement('paper-item');
        item.setAttribute('value', gameType);
        item.textContent = gameType;
        select.appendChild(item);
      });
      select.setAttribute('value', select.firstElementChild.getAttribute('value'));
    });
  }

  function setInstalledGames(appData) {
    return query('getInstalledGames').then(JSON.parse).then((installedGames) => {
      appData.installedGames = installedGames;
      dom.updateEnabledGames(installedGames);
    });
  }

  function setSettings(appData) {
    return query('getSettings').then(JSON.parse).then((result) => {
      appData.settings = result;
      dom.updateSettingsDialog(appData.settings);
      loot.DOM.setGameMenuItems(appData.settings.games);
      loot.DOM.updateEnabledGames(appData.installedGames);
      loot.DOM.updateSelectedGame(appData.game.folder);
    });
  }

  function setGameData(appData) {
    return query('getGameData').then((result) => {
      const game = JSON.parse(result, Plugin.fromJson);
      appData.game = new Game(game, appData.l10n);
      document.getElementById('cardsNav').items = appData.game.plugins;
      document.getElementById('pluginCardList').items = appData.game.plugins;
      applyEnabledFilters(appData.filters, appData.settings, appData.game.plugins);
      Dialog.closeProgress();
    });
  }

  return () => {
    Dialog.showProgress('Initialising user interface...');

    /* Make sure settings are what I want. */
    window.marked.setOptions({
      gfm: true,
      tables: true,
      sanitize: true,
    });
    setupEventHandlers();

    loot.version = '';
    loot.settings = {};
    loot.l10n = new Translator();
    loot.game = new Game({}, loot.l10n);
    loot.filters = new Filters(loot.l10n);

    Promise.all([
      setLanguages(),
      setGameTypes(),
      setInstalledGames(loot),
      setVersion(loot),
      setSettings(loot),
    ]).then(() => {
      /* Translate static text. */
      loot.l10n = new Translator(loot.settings.language);
      return loot.l10n.load();
    }).then(() => {
      loot.filters = new Filters(loot.l10n);
      translateStaticText(loot.l10n);
      /* Also need to update the settings UI. */
      dom.updateSettingsDialog(loot.settings);
      loot.DOM.setGameMenuItems(loot.settings.games);
      loot.DOM.updateEnabledGames(loot.installedGames);
      loot.DOM.updateSelectedGame(loot.game.folder);
    }).then(() => {
      return displayInitErrors();
    }).then((result) => {
      if (result) {
        Dialog.closeProgress();
        document.getElementById('settingsButton').click();
        return Promise.resolve();
      }
      return setGameData(loot);
    }).then(() => {
      if (!loot.settings.lastVersion || loot.settings.lastVersion !== loot.version) {
        document.getElementById('firstRun').open();
      }
    }).catch(handlePromiseError);
  };
}));
