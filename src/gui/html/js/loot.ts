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
import { LootSettings, LootVersion } from './interfaces';
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
  setDocumentFontFamily,
  initialiseSettingsDialog
} from './dom';
import Filters from './filters';
import Game from './game';
import handlePromiseError from './handlePromiseError';
import { Plugin } from './plugin';
import {
  getVersion,
  getInitErrors,
  getGameTypes,
  getInstalledGames,
  getSettings,
  getGameData,
  getAutoSort,
  getThemes
} from './query';
import State from './state';
import translateStaticText from './translateStaticText';
import Translator from './translator';
import updateExists from './updateExists';
import {
  getElementById,
  incrementCounterText,
  querySelector,
  getTextAsInt
} from './dom/helpers';

function addEventListeners(): void {
  /* Set up handlers for filters. */
  getElementById('hideVersionNumbers').addEventListener(
    'change',
    onSidebarFilterToggle
  );
  getElementById('hideCRCs').addEventListener('change', onSidebarFilterToggle);
  getElementById('hideBashTags').addEventListener(
    'change',
    onSidebarFilterToggle
  );
  getElementById('hideNotes').addEventListener('change', onSidebarFilterToggle);
  getElementById('hideDoNotCleanMessages').addEventListener(
    'change',
    onSidebarFilterToggle
  );
  getElementById('hideInactivePlugins').addEventListener(
    'change',
    onSidebarFilterToggle
  );
  getElementById('hideAllPluginMessages').addEventListener(
    'change',
    onSidebarFilterToggle
  );
  getElementById('hideMessagelessPlugins').addEventListener(
    'change',
    onSidebarFilterToggle
  );
  getElementById('contentFilter').addEventListener('change', onContentFilter);
  getElementById('conflictsFilter').addEventListener(
    'value-changed',
    onConflictsFilter
  );
  document.addEventListener(
    'loot-filter-conflicts-deactivate',
    Filters.onDeactivateConflictsFilter
  );

  /* Set up handlers for buttons. */
  getElementById('redatePluginsButton').addEventListener(
    'click',
    onRedatePlugins
  );
  getElementById('openLogButton').addEventListener('click', onOpenLogLocation);
  getElementById('groupsEditorButton').addEventListener(
    'click',
    onOpenGroupsEditor
  );
  getElementById('wipeUserlistButton').addEventListener(
    'click',
    onClearAllMetadata
  );
  getElementById('copyLoadOrderButton').addEventListener(
    'click',
    onCopyLoadOrder
  );
  getElementById('copyContentButton').addEventListener('click', onCopyContent);
  getElementById('refreshContentButton').addEventListener(
    'click',
    onContentRefresh
  );
  getElementById('settingsButton').addEventListener(
    'click',
    onShowSettingsDialog
  );
  getElementById('helpButton').addEventListener('click', onOpenReadme);
  getElementById('aboutButton').addEventListener('click', onShowAboutDialog);
  getElementById('quitButton').addEventListener('click', onQuit);
  getElementById('gameMenu').addEventListener('iron-select', onChangeGame);
  getElementById('updateMasterlistButton').addEventListener(
    'click',
    onUpdateMasterlist
  );
  getElementById('sortButton').addEventListener('click', onSortPlugins);
  getElementById('applySortButton').addEventListener('click', onApplySort);
  getElementById('cancelSortButton').addEventListener('click', onCancelSort);
  getElementById('sidebarTabs').addEventListener(
    'iron-select',
    onSwitchSidebarTab
  );
  getElementById('jumpToGeneralInfo').addEventListener(
    'click',
    onJumpToGeneralInfo
  );

  /* Set up search event handlers. */
  getElementById('showSearch').addEventListener('click', onSearchOpen);
  getElementById('searchBar').addEventListener(
    'loot-search-begin',
    onSearchBegin
  );
  getElementById('searchBar').addEventListener(
    'loot-search-change-selection',
    onSearchChangeSelection,
    false
  );
  getElementById('searchBar').addEventListener('loot-search-end', onSearchEnd);
  window.addEventListener('keyup', onFocusSearch);

  /* Set up event handlers for groups editor dialog. */
  const groupsEditor = getElementById('groupsEditorDialog');
  groupsEditor.addEventListener('iron-overlay-closed', onSaveUserGroups);
  groupsEditor.addEventListener('iron-overlay-opened', onGroupsEditorOpened);
  groupsEditor.addEventListener('loot-open-readme', onOpenReadme);

  /* Set up event handlers for settings dialog. */
  const settings = getElementById('settingsDialog');
  settings.addEventListener('iron-overlay-closed', onCloseSettingsDialog);
  querySelector(settings, '[dialog-confirm]').addEventListener(
    'tap',
    onApplySettings
  );

  /* Set up handler for opening and closing editors. */
  document.body.addEventListener('loot-editor-open', onEditorOpen);
  document.body.addEventListener('loot-editor-close', onEditorClose);
  document.body.addEventListener('loot-copy-metadata', onCopyMetadata);
  document.body.addEventListener('loot-clear-metadata', onClearMetadata);

  getElementById('cardsNav').addEventListener('click', onSidebarClick);
  getElementById('cardsNav').addEventListener('dblclick', onSidebarClick);

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

function handleInitErrors(errors: string[]): void {
  listInitErrors(errors);
  closeProgress();
  enableGameOperations(false);
  openDialog('settingsDialog');
}

function initialiseGameTypesList(): Promise<void> {
  return getGameTypes().then(fillGameTypesList);
}

async function initialiseUISettings(settings: LootSettings): Promise<void> {
  const themes = await getThemes();

  initialiseSettingsDialog(settings, themes);
  updateSettingsDialog(settings);

  const currentLanguage = settings.languages.find(
    language => language.locale === settings.language
  );
  if (currentLanguage && currentLanguage.fontFamily) {
    setDocumentFontFamily(currentLanguage.fontFamily);
  }
}

function checkForLootUpdate(l10n: Translator): Promise<void> {
  return getVersion()
    .then(version => updateExists(version.release, version.build))
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

  incrementCounterText('totalMessageNo', 1);
  incrementCounterText('totalErrorNo', 1);
}

function getErrorCount(): number {
  return getTextAsInt('totalErrorNo');
}

function autoSort(l10n: Translator): Promise<void> {
  return getAutoSort()
    .then(shouldAutoSort => {
      if (shouldAutoSort) {
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

export default class Loot {
  public filters: Filters;

  public game?: Game;

  public l10n: Translator;

  public settings?: LootSettings;

  public state: State;

  public installedGames: string[];

  public version: LootVersion;

  // Used by C++ callbacks.
  public showProgress: (text: string) => void;

  // Used by C++ callbacks.
  public onQuit: () => void;

  public constructor() {
    this.l10n = new Translator();
    this.filters = new Filters(this.l10n);
    this.state = new State();
    this.installedGames = [];
    this.version = {
      release: 'unknown',
      build: 'unknown'
    };

    this.showProgress = showProgress;
    this.onQuit = onQuit;
  }

  private async loadLootData(): Promise<void> {
    const [version, installedGames, settings] = await Promise.all([
      getVersion(),
      getInstalledGames(),
      getSettings()
    ]);

    this.version = version;
    this.installedGames = installedGames;
    this.settings = settings;
  }

  private async loadGameData(): Promise<void> {
    const gameData = await getGameData();
    this.game = new Game(gameData, this.l10n);
  }

  private async initialiseGeneralUIElements(): Promise<void> {
    if (this.settings === undefined) {
      throw new Error('Failed to load settings');
    }

    await initialiseGameTypesList();
    await initialiseUISettings(this.settings);

    /* Translate static text. */
    await this.l10n.load(this.settings.language);

    this.filters = new Filters(this.l10n);
    this.filters.load(this.settings.filters);

    translateStaticText(this.l10n, this.version);

    setGameMenuItems(this.settings.games, this.installedGames);
  }

  public async initialise(): Promise<void> {
    showProgress('Initialising user interface...');

    addEventListeners();

    try {
      await this.loadLootData();
      await this.initialiseGeneralUIElements();
    } catch (error) {
      handlePromiseError(error);
    }

    try {
      if (this.settings === undefined) {
        throw new Error('Failed to load settings');
      }

      const initErrors = await getInitErrors();

      if (initErrors.length > 0) {
        handleInitErrors(initErrors);
      } else {
        await this.loadGameData();

        if (this.game === undefined) {
          throw new Error('Failed to load game');
        }

        this.game.initialiseUI(this.filters);

        closeProgress();

        await autoSort(this.l10n);
      }

      if (this.settings.lastVersion !== this.version.release) {
        openDialog('firstRun');
      }

      if (this.settings.enableLootUpdateCheck) {
        await checkForLootUpdate(this.l10n);
      }
    } catch (error) {
      handlePromiseError(error);
    }
  }
}
