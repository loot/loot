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
function applyEnabledFilters() {
  if (!loot.filters) {
    return;
  }
  function setupEventHandlers() {
    /* Set up handlers for filters. */
    document.getElementById('hideVersionNumbers').addEventListener('change', onToggleDisplayCSS);
    document.getElementById('hideVersionNumbers').addEventListener('change', saveFilterState);
    document.getElementById('hideCRCs').addEventListener('change', onToggleDisplayCSS);
    document.getElementById('hideCRCs').addEventListener('change', saveFilterState);
    document.getElementById('hideBashTags').addEventListener('change', onToggleBashTags);
    document.getElementById('hideBashTags').addEventListener('change', saveFilterState);
    document.getElementById('hideNotes').addEventListener('change', onSidebarFilterToggle);
    document.getElementById('hideDoNotCleanMessages').addEventListener('change', onSidebarFilterToggle);
    document.getElementById('hideInactivePlugins').addEventListener('change', onSidebarFilterToggle);
    document.getElementById('hideAllPluginMessages').addEventListener('change', onSidebarFilterToggle);
    document.getElementById('hideMessagelessPlugins').addEventListener('change', onSidebarFilterToggle);
    document.body.addEventListener('loot-filter-conflicts', onConflictsFilter);

  if (loot.settings.filters) {
    for (const filter in loot.settings.filters) {
      loot.filters[filter] = loot.settings.filters[filter];
      document.getElementById(filter).checked = loot.filters[filter];
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
    document.getElementById('updateMasterlistButton').addEventListener('click', onUpdateMasterlist);
    document.getElementById('sortButton').addEventListener('click', onSortPlugins);
    document.getElementById('applySortButton').addEventListener('click', onApplySort);
    document.getElementById('cancelSortButton').addEventListener('click', onCancelSort);
    document.getElementById('sidebarTabs').addEventListener('core-select', onSwitchSidebarTab);
    document.getElementById('jumpToGeneralInfo').addEventListener('click', onJumpToGeneralInfo);

    /* Set up search event handlers. */
    document.getElementById('showSearch').addEventListener('click', onSearchOpen);
    document.getElementById('searchBar').addEventListener('loot-search-close', onSearchClose);
    window.addEventListener('keyup', onFocusSearch);

    /* Set up event handlers for settings dialog. */
    const settings = document.getElementById('settingsDialog');
    settings.getElementsByClassName('accept')[0].addEventListener('click', onCloseSettingsDialog);
    settings.getElementsByClassName('cancel')[0].addEventListener('click', onCloseSettingsDialog);

    /* Set up handler for opening and closing editors. */
    document.body.addEventListener('loot-editor-open', onEditorOpen);
    document.body.addEventListener('loot-editor-close', onEditorClose);
    document.body.addEventListener('loot-copy-metadata', onCopyMetadata);
    document.body.addEventListener('loot-clear-metadata', onClearMetadata);

    document.getElementById('cardsNav').addEventListener('click', onSidebarClick);
    document.getElementById('cardsNav').addEventListener('dblclick', onSidebarClick);

    /* Set up handler for plugin message and dirty info changes. */
    document.addEventListener('loot-plugin-message-change', onPluginMessageChange);
    document.addEventListener('loot-plugin-isdirty-change', onPluginIsDirtyChange);

    /* Set up event handlers for game member variable changes. */
    document.addEventListener('loot-game-folder-change', onGameFolderChange);
    document.addEventListener('loot-game-masterlist-change', onGameMasterlistChange);
    document.addEventListener('loot-game-global-messages-change', onGameGlobalMessagesChange);
    document.addEventListener('loot-game-plugins-change', onGamePluginsChange);
  }

    }
  }

  if (loot.filters.hideMessagelessPlugins
      || loot.filters.hideInactivePlugins
      || loot.filters.hideNotes
      || loot.filters.hideDoNotCleanMessages
      || loot.filters.hideAllPluginMessages) {
    setFilteredUIData();
  }

  if (loot.filters.hideVersionNumbers) {
    document.getElementById('hideVersionNumbers').dispatchEvent(new Event('change'));
  }

  if (loot.filters.hideCRCs) {
    document.getElementById('hideCRCs').dispatchEvent(new Event('change'));
  }

  if (loot.filters.hideBashTags) {
    document.getElementById('hideBashTags').dispatchEvent(new Event('change'));
  }
}

function getVersion() {
  return loot.query('getVersion').then(JSON.parse).then((result) => {
    /* The fourth part of the version string is the build number. Trim it. */
    const pos = result.lastIndexOf('.');
    loot.version = result.substring(0, pos);
    document.getElementById('LOOTVersion').textContent = loot.version;
    document.getElementById('firstTimeLootVersion').textContent = loot.version;
    document.getElementById('LOOTBuild').textContent = result.substring(pos + 1);
  });
}

function getLanguages() {
  return loot.query('getLanguages').then(JSON.parse).then((result) => {
    /* Now fill in language options. */
    const settingsLangSelect = document.getElementById('languageSelect');
    let messageLangSelect = document.querySelector('link[rel="import"][href$="editable-table.html"]');
    if (messageLangSelect) {
      messageLangSelect = messageLangSelect.import.querySelector('#messageRow').content.querySelector('.language');
    } else {
      messageLangSelect = document.querySelector('#messageRow').content.querySelector('.language');
    }

    for (let i = 0; i < result.length; ++i) {
      const settingsItem = document.createElement('paper-item');
      settingsItem.setAttribute('value', result[i].locale);
      settingsItem.setAttribute('noink', '');
      settingsItem.textContent = result[i].name;
      settingsLangSelect.appendChild(settingsItem);
      messageLangSelect.appendChild(settingsItem.cloneNode(true));
    }

    messageLangSelect.setAttribute('value', messageLangSelect.firstElementChild.getAttribute('value'));
  });
}

function getInitErrors() {
  return loot.query('getInitErrors').then(JSON.parse).then((result) => {
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

function getGameTypes() {
  return loot.query('getGameTypes').then(JSON.parse).then((result) => {
    /* Fill in game row template's game type options. */
    let select = document.querySelector('link[rel="import"][href$="editable-table.html"]');
    if (select) {
      select = select.import.querySelector('#gameRow').content.querySelector('.type');
    } else {
      select = document.querySelector('#gameRow').content.querySelector('.type');
    }
    for (let j = 0; j < result.length; ++j) {
      const item = document.createElement('paper-item');
      item.setAttribute('value', result[j]);
      item.setAttribute('noink', '');
      item.textContent = result[j];
      select.appendChild(item);
    }
    select.setAttribute('value', select.firstElementChild.getAttribute('value'));
  });
}

function getInstalledGames() {
  return loot.query('getInstalledGames').then(JSON.parse).then(setInstalledGames);
}

function getSettings() {
  return loot.query('getSettings').then(JSON.parse).then((result) => {
    loot.settings = result;
    updateSettingsUI();
  });
}

function getGameData() {
  return loot.query('getGameData').then((result) => {
    const game = JSON.parse(result, loot.Plugin.fromJson);
    loot.game = new loot.Game(game, loot.l10n);
    document.getElementById('cardsNav').data = loot.game.plugins;
    document.getElementById('main').lastElementChild.data = loot.game.plugins;
    applyEnabledFilters();

    setTimeout(() => {
      document.getElementById('cardsNav').updateSize();
      loot.Dialog.closeProgress();
    }, 100);
  });
}

function initialise() {
  loot.Dialog.showProgress('Initialising user interface...');
  /* Set the plugin list's scroll target to its parent. */
  document.getElementById('pluginCardList').scrollTarget = document.getElementById('main');

  /* Make sure settings are what I want. */
  window.marked.setOptions({
    gfm: true,
    tables: true,
    sanitize: true,
  });
  setupEventHandlers();

  loot.l10n = new loot.Translator();
  loot.l10n.load().then(() => {
    loot.filters = new loot.Filters(loot.l10n);
    loot.game = new loot.Game({}, loot.l10n);
  }).then(() => {
    return Promise.all([
      getVersion(),
      getLanguages(),
      getGameTypes(),
      getInstalledGames(),
      getSettings(),
    ]);
  }).then(() => {
    /* Translate static text. */
    loot.l10n = new loot.Translator(loot.settings.language);
    return loot.l10n.load();
  }).then(() => {
    loot.translateStaticText(loot.l10n);
    /* Also need to update the settings UI. */
    updateSettingsUI();
  }).then(() => {
    return getInitErrors();
  }).then((result) => {
    if (result) {
      loot.Dialog.closeProgress();
      document.getElementById('settingsButton').click();
      return Promise.resolve('');
    }
    return getGameData();
  }).then(() => {
    if (!loot.settings.lastVersion || loot.settings.lastVersion !== loot.version) {
      document.getElementById('firstRun').showModal();
    }
  }).catch(handlePromiseError);
}

window.addEventListener('polymer-ready', initialise);
