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
                                    root.loot.handlePromiseError,
                                    root.loot.translateStaticText,
                                    root.loot.Plugin,
                                    root.loot.query,
                                    root.loot.Translator);
  }
}(this, (Dialog,
         dom,
         Filters,
         Game,
         handlePromiseError,
         translateStaticText,
         Plugin,
         query,
         Translator) => {
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
    document.getElementById('contentFilter').addEventListener('change', onContentFilter);
    document.getElementById('conflictsFilter').addEventListener('iron-select', onConflictsFilter);
    document.addEventListener('loot-filter-conflicts-deactivate', Filters.onDeactivateConflictsFilter);

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
    document.addEventListener('loot-game-folder-change', onFolderChange);
    document.addEventListener('loot-game-masterlist-change', Game.onMasterlistChange);
    document.addEventListener('loot-game-global-messages-change', Game.onGlobalMessagesChange);
    document.addEventListener('loot-game-plugins-change', Game.onPluginsChange);
  }

  function splitVersion(version) {
    const lastPeriodIndex = version.lastIndexOf('.');
    return {
      release: version.substring(0, lastPeriodIndex),
      build: version.substring(lastPeriodIndex + 1),
    };
  }

  function setVersion(appData) {
    return query('getVersion').then(JSON.parse).then(splitVersion).then((version) => {
      appData.version = version.release;
      dom.setVersion(version);
    });
  }

  function setLanguages() {
    return query('getLanguages').then(JSON.parse).then(dom.fillLanguagesList);
  }

  function getInitErrors() {
    return query('getInitErrors').then((result) => {
      if (JSON.parse(result)) {
        throw new Error(result);
      }
    });
  }

  function handleInitErrors(error) {
    dom.listInitErrors(JSON.parse(error.message));
    Dialog.closeProgress();
    dom.openDialog('settingsDialog');
  }

  function setGameTypes() {
    return query('getGameTypes').then(JSON.parse).then(dom.fillGameTypesList);
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
      dom.setGameMenuItems(appData.settings.games);
      dom.updateEnabledGames(appData.installedGames);
      dom.updateSelectedGame(appData.game.folder);
    });
  }

  function setGameData(appData) {
    return query('getGameData').then((result) => {
      const game = JSON.parse(result, Plugin.fromJson);
      appData.game = new Game(game, appData.l10n);

      dom.initialiseVirtualLists(appData.game.plugins);
      appData.Filters.fillConflictsFilterList(appData.game.plugins);
      appData.filters.load(appData.settings.filters);
      appData.filters.apply(appData.game.plugins);

      Dialog.closeProgress();
    });
  }

  return (loot) => {
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
      dom.setGameMenuItems(loot.settings.games);
      dom.updateEnabledGames(loot.installedGames);
      dom.updateSelectedGame(loot.game.folder);
    }).then(getInitErrors)
    .then(() => setGameData(loot))
    .catch(handleInitErrors)
    .then(() => {
      if (loot.settings.lastVersion !== loot.version) {
        dom.openDialog('firstRun');
      }
    }).catch(handlePromiseError);
  };
}));
