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
import {
  onSidebarFilterToggle,
  onContentFilter,
  onConflictsFilter,
  onChangeGame,
  onUpdateMasterlist,
  onSortPlugins,
  onApplySort,
  onCancelSort,
  onRedatePlugins,
  onClearAllMetadata,
  onCopyContent,
  onCopyLoadOrder,
  onContentRefresh,
  onOpenReadme,
  onOpenLogLocation,
  onSaveUserGroups,
  onQuit,
  onApplySettings,
  onCloseSettingsDialog,
  onEditorOpen,
  onEditorClose,
  onCopyMetadata,
  onClearMetadata,
  onSearchBegin,
  onSearchEnd,
  onFolderChange
} from './events';
import { closeProgress, showProgress } from './dialog';
import {
  onOpenGroupsEditor,
  onGroupsEditorOpened,
  onShowSettingsDialog,
  onShowAboutDialog,
  onSwitchSidebarTab,
  onJumpToGeneralInfo,
  onSearchOpen,
  onFocusSearch,
  onSearchChangeSelection,
  onSidebarClick,
  listInitErrors,
  enableGameOperations,
  openDialog,
  fillGameTypesList,
  updateSettingsDialog,
  setGameMenuItems,
  appendGeneralMessages,
  setDocumentFontFamily
} from './dom';
import Filters from './filters';
import Game from './game';
import handlePromiseError from './handlePromiseError';
import { Plugin } from './plugin';
import query from './query';
import State from './state';
import translateStaticText from './translateStaticText';
import Translator from './translator';
import updateExists from './updateExists';
import {
  LootVersion,
  LootSettings,
  GameData,
  GetInstalledGamesResponse
} from './interfaces';
import Loot from '../type-declarations/loot.d';

interface GetInitErrorsResponse {
  errors: string[];
}

interface GetGameTypesResponse {
  gameTypes: string[];
}

interface GetAutoSortResponse {
  autoSort: boolean;
}

function setupEventHandlers(): void {
  /* Set up handlers for filters. */
  document
    .getElementById('hideVersionNumbers')
    .addEventListener('change', onSidebarFilterToggle);
  document
    .getElementById('hideCRCs')
    .addEventListener('change', onSidebarFilterToggle);
  document
    .getElementById('hideBashTags')
    .addEventListener('change', onSidebarFilterToggle);
  document
    .getElementById('hideNotes')
    .addEventListener('change', onSidebarFilterToggle);
  document
    .getElementById('hideDoNotCleanMessages')
    .addEventListener('change', onSidebarFilterToggle);
  document
    .getElementById('hideInactivePlugins')
    .addEventListener('change', onSidebarFilterToggle);
  document
    .getElementById('hideAllPluginMessages')
    .addEventListener('change', onSidebarFilterToggle);
  document
    .getElementById('hideMessagelessPlugins')
    .addEventListener('change', onSidebarFilterToggle);
  document
    .getElementById('contentFilter')
    .addEventListener('change', onContentFilter);
  document
    .getElementById('conflictsFilter')
    .addEventListener('value-changed', onConflictsFilter);
  document.addEventListener(
    'loot-filter-conflicts-deactivate',
    Filters.onDeactivateConflictsFilter
  );

  /* Set up handlers for buttons. */
  document
    .getElementById('redatePluginsButton')
    .addEventListener('click', onRedatePlugins);
  document
    .getElementById('openLogButton')
    .addEventListener('click', onOpenLogLocation);
  document
    .getElementById('groupsEditorButton')
    .addEventListener('click', onOpenGroupsEditor);
  document
    .getElementById('wipeUserlistButton')
    .addEventListener('click', onClearAllMetadata);
  document
    .getElementById('copyLoadOrderButton')
    .addEventListener('click', onCopyLoadOrder);
  document
    .getElementById('copyContentButton')
    .addEventListener('click', onCopyContent);
  document
    .getElementById('refreshContentButton')
    .addEventListener('click', onContentRefresh);
  document
    .getElementById('settingsButton')
    .addEventListener('click', onShowSettingsDialog);
  document.getElementById('helpButton').addEventListener('click', onOpenReadme);
  document
    .getElementById('aboutButton')
    .addEventListener('click', onShowAboutDialog);
  document.getElementById('quitButton').addEventListener('click', onQuit);
  document
    .getElementById('gameMenu')
    .addEventListener('iron-select', onChangeGame);
  document
    .getElementById('updateMasterlistButton')
    .addEventListener('click', onUpdateMasterlist);
  document
    .getElementById('sortButton')
    .addEventListener('click', onSortPlugins);
  document
    .getElementById('applySortButton')
    .addEventListener('click', onApplySort);
  document
    .getElementById('cancelSortButton')
    .addEventListener('click', onCancelSort);
  document
    .getElementById('sidebarTabs')
    .addEventListener('iron-select', onSwitchSidebarTab);
  document
    .getElementById('jumpToGeneralInfo')
    .addEventListener('click', onJumpToGeneralInfo);

  /* Set up search event handlers. */
  document.getElementById('showSearch').addEventListener('click', onSearchOpen);
  document
    .getElementById('searchBar')
    .addEventListener('loot-search-begin', onSearchBegin);
  document
    .getElementById('searchBar')
    .addEventListener(
      'loot-search-change-selection',
      onSearchChangeSelection,
      false
    );
  document
    .getElementById('searchBar')
    .addEventListener('loot-search-end', onSearchEnd);
  window.addEventListener('keyup', onFocusSearch);

  /* Set up event handlers for groups editor dialog. */
  const groupsEditor = document.getElementById('groupsEditorDialog');
  groupsEditor.addEventListener('iron-overlay-closed', onSaveUserGroups);
  groupsEditor.addEventListener('iron-overlay-opened', onGroupsEditorOpened);
  groupsEditor.addEventListener('loot-open-readme', onOpenReadme);

  /* Set up event handlers for settings dialog. */
  const settings = document.getElementById('settingsDialog');
  settings.addEventListener('iron-overlay-closed', onCloseSettingsDialog);
  settings
    .querySelector('[dialog-confirm]')
    .addEventListener('tap', onApplySettings);

  /* Set up handler for opening and closing editors. */
  document.body.addEventListener('loot-editor-open', onEditorOpen);
  document.body.addEventListener('loot-editor-close', onEditorClose);
  document.body.addEventListener('loot-copy-metadata', onCopyMetadata);
  document.body.addEventListener('loot-clear-metadata', onClearMetadata);

  document.getElementById('cardsNav').addEventListener('click', onSidebarClick);
  document
    .getElementById('cardsNav')
    .addEventListener('dblclick', onSidebarClick);

  /* Set up handler for plugin data changes. */
  document.addEventListener(
    'loot-plugin-message-change',
    Plugin.onMessageChange
  );
  document.addEventListener(
    'loot-plugin-message-change',
    Plugin.onContentChange
  );
  document.addEventListener(
    'loot-plugin-cleaning-data-change',
    Plugin.onCleaningDataChange
  );
  document.addEventListener(
    'loot-plugin-card-content-change',
    Plugin.onContentChange
  );
  document.addEventListener(
    'loot-plugin-card-styling-change',
    Plugin.onCardStylingChange
  );
  document.addEventListener(
    'loot-plugin-item-content-change',
    Plugin.onItemContentChange
  );

  /* Set up event handlers for game member variable changes. */
  document.addEventListener('loot-game-folder-change', onFolderChange);
  document.addEventListener(
    'loot-game-masterlist-change',
    Game.onMasterlistChange
  );
  document.addEventListener(
    'loot-game-global-messages-change',
    Game.onGeneralMessagesChange
  );
  document.addEventListener('loot-game-plugins-change', Game.onPluginsChange);
  document.addEventListener('loot-game-groups-change', Game.onGroupsChange);
}

function setVersion(appData: Loot): Promise<void> {
  return query('getVersion')
    .then(JSON.parse)
    .then((version: LootVersion) => {
      appData.version = version;
    });
}

function getInitErrors(): Promise<string[]> {
  return query('getInitErrors')
    .then(JSON.parse)
    .then((response: GetInitErrorsResponse) => response.errors);
}

function handleInitErrors(errors: string[]): void {
  listInitErrors(errors);
  closeProgress();
  enableGameOperations(false);
  openDialog('settingsDialog');
}

function setGameTypes(): Promise<void> {
  return query('getGameTypes')
    .then(JSON.parse)
    .then((response: GetGameTypesResponse) => response.gameTypes)
    .then(fillGameTypesList);
}

function setInstalledGames(appData: Loot): Promise<void> {
  return query('getInstalledGames')
    .then(JSON.parse)
    .then((response: GetInstalledGamesResponse) => {
      appData.installedGames = response.installedGames;
    });
}

function setSettings(appData: Loot): Promise<void> {
  return query('getSettings')
    .then(JSON.parse)
    .then((result: LootSettings) => {
      appData.settings = result;
      updateSettingsDialog(appData.settings);

      const currentLanguage = result.languages.find(
        language => language.locale === result.language
      );
      if (currentLanguage && currentLanguage.fontFamily) {
        setDocumentFontFamily(currentLanguage.fontFamily);
      }
    });
}

function setGameData(appData: Loot): Promise<void> {
  return query('getGameData')
    .then(JSON.parse)
    .then((result: GameData) => {
      appData.game = new Game(result, appData.l10n);
      appData.game.initialiseUI(appData.filters);

      closeProgress();
    });
}

function checkForLootUpdate(l10n: Translator): Promise<void> {
  return query('getVersion')
    .then(JSON.parse)
    .then((version: LootVersion) =>
      updateExists(version.release, version.build)
    )
    .catch(() => {
      appendGeneralMessages([
        {
          type: 'error',
          content: l10n.translate(
            'Failed to check for LOOT updates! You can check your LOOTDebugLog.txt (you can get to it through the main menu) for more information.'
          )
        }
      ]);

      return false;
    })
    .then(isUpdateAvailable => {
      if (!isUpdateAvailable) {
        return;
      }

      appendGeneralMessages([
        {
          type: 'warn',
          content: l10n.translateFormatted(
            'A [new release](%s) of LOOT is available.',
            'https://github.com/loot/loot/releases/latest'
          )
        }
      ]);
    });
}

function appendGeneralErrorMessage(content: string): void {
  appendGeneralMessages([
    {
      type: 'error',
      content
    }
  ]);

  document.getElementById('totalMessageNo').textContent = (
    parseInt(document.getElementById('totalMessageNo').textContent, 10) + 1
  ).toString();
  document.getElementById('totalErrorNo').textContent = (
    parseInt(document.getElementById('totalErrorNo').textContent, 10) + 1
  ).toString();
}

function getErrorCount(): number {
  return parseInt(document.getElementById('totalErrorNo').textContent, 10);
}

function autoSort(l10n: Translator): Promise<void> {
  return query('getAutoSort')
    .then(JSON.parse)
    .then((response: GetAutoSortResponse) => {
      if (response.autoSort) {
        if (getErrorCount() === 0) {
          return onSortPlugins()
            .then(onApplySort)
            .then(() => {
              if (getErrorCount() === 0) {
                onQuit();
              }
            });
        }

        appendGeneralErrorMessage(
          l10n.translate(
            'Auto-sort has been cancelled as there is at least one error message displayed.'
          )
        );
      }

      return Promise.resolve(undefined);
    })
    .catch(handlePromiseError);
}

export default function initialise(loot: Loot): void {
  showProgress('Initialising user interface...');

  setupEventHandlers();

  loot.l10n = new Translator();
  loot.filters = new Filters(loot.l10n);
  loot.state = new State();

  Promise.all([
    setGameTypes(),
    setInstalledGames(loot),
    setVersion(loot),
    setSettings(loot)
  ])
    .then(() => {
      /* Translate static text. */
      loot.l10n = new Translator(loot.settings.language);
      return loot.l10n.load();
    })
    .then(() => {
      loot.filters = new Filters(loot.l10n);
      loot.filters.load(loot.settings.filters);

      translateStaticText(loot.l10n, loot.version);

      setGameMenuItems(loot.settings.games, loot.installedGames);
    })
    .catch(handlePromiseError)
    .then(getInitErrors)
    .then(initErrors => {
      if (initErrors.length > 0) {
        handleInitErrors(initErrors);
        return Promise.resolve();
      }

      return setGameData(loot);
    })
    .then(() => autoSort(loot.l10n))
    .then(() => {
      if (loot.settings.lastVersion !== loot.version.release) {
        openDialog('firstRun');
      }
    })
    .then(() => {
      if (loot.settings.enableLootUpdateCheck) {
        return checkForLootUpdate(loot.l10n);
      }

      return Promise.resolve();
    })
    .catch(handlePromiseError);
}
