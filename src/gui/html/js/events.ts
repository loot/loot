import * as marked from 'marked';

import { IronListElement } from '@polymer/iron-list';
import { PaperCheckboxElement } from '@polymer/paper-checkbox';
import {
  askQuestion,
  closeProgress,
  showNotification,
  showProgress
} from './dialog';
import {
  fillGroupsList,
  initialiseAutocompleteBashTags,
  updateSettingsDialog,
  enableGameOperations,
  setGameMenuItems,
  updateSelectedGame,
  enable
} from './dom';
import Game from './game';
import handlePromiseError from './handlePromiseError';
import { Plugin } from './plugin';
import {
  changeGame,
  updateMasterlist as updateMasterlistQuery,
  sortPlugins,
  cancelSort,
  clearAllMetadata,
  getGameData,
  closeSettings,
  saveUserGroups,
  editorClosed,
  clearPluginMetadata,
  EditorState,
  saveFilterState,
  discardUnappliedChanges,
  applySort,
  redatePlugins,
  copyContent,
  copyLoadOrder,
  openReadme,
  openLogLocation,
  editorOpened,
  copyMetadata
} from './query';
import {
  FilterStates,
  GameContent,
  GameSettings,
  LootSettings
} from './interfaces';
import EditableTable from '../elements/editable-table';
import LootGroupsEditor from '../elements/loot-groups-editor';
import LootPluginCard from '../elements/loot-plugin-card';
import LootPluginItem from '../elements/loot-plugin-item';
import LootPluginEditor from '../elements/loot-plugin-editor';
import LootDropdownMenu from '../elements/loot-dropdown-menu';
import LootSearchToolbar from '../elements/loot-search-toolbar';
import { getElementById, querySelector } from './dom/helpers';

interface PaperCheckboxChangeEvent extends Event {
  target: EventTarget & {
    id: keyof FilterStates;
    checked: boolean;
  };
}

function isPaperCheckboxChangeEvent(
  evt: Event
): evt is PaperCheckboxChangeEvent {
  return evt.target !== null && 'id' in evt.target && 'checked' in evt.target;
}

interface PaperInputChangeEvent extends Event {
  target: EventTarget & { value: string };
}

function isPaperInputChangeEvent(evt: Event): evt is PaperInputChangeEvent {
  return evt.target !== null && 'value' in evt.target;
}

interface LootDropdownMenuChangeEvent extends Event {
  currentTarget: EventTarget & { value: string };
  target: EventTarget & { value: string };
}

function isLootDropdownMenuChangeEvent(
  evt: Event
): evt is LootDropdownMenuChangeEvent {
  return (
    evt.currentTarget !== null &&
    'value' in evt.currentTarget &&
    evt.target !== null &&
    'value' in evt.target
  );
}

interface LootDropdownMenuSelectEvent extends Event {
  detail: { item: Element };
}

function isLootDropdownMenuSelectEvent(
  evt: Event
): evt is LootDropdownMenuSelectEvent {
  // Not ideal, but iron-select isn't a CustomEvent, so a typesafe check for
  // evt.detail.item is tricky.
  return evt.type === 'iron-select';
}

interface IronOverlayClosedEvent extends Event {
  target: EventTarget & Element;
  detail: { confirmed: boolean };
}

function isIronOverlayClosedEvent(evt: Event): evt is IronOverlayClosedEvent {
  // Not ideal, but iron-select isn't a CustomEvent, so a typesafe check for
  // evt.detail.item is tricky.
  return evt.type === 'iron-overlay-closed' && evt.target instanceof Element;
}

interface LootPluginEditorOpenEvent extends CustomEvent {
  target: EventTarget & { data: Plugin };
}

function isPluginEditorOpenEvent(evt: Event): evt is LootPluginEditorOpenEvent {
  return evt.target !== null && 'data' in evt.target;
}

interface LootPluginEditorCloseEvent extends CustomEvent {
  target: EventTarget & LootPluginEditor;
}

function isPluginEditorCloseEvent(
  evt: Event
): evt is LootPluginEditorCloseEvent {
  return evt.target instanceof LootPluginEditor;
}

interface LootCopyMetadataEvent extends CustomEvent {
  target: EventTarget & LootPluginCard;
}

function isCopyMetadataEvent(evt: Event): evt is LootCopyMetadataEvent {
  return evt.target instanceof LootPluginCard;
}

interface LootClearMetadataEvent extends CustomEvent {
  target: EventTarget & LootPluginCard;
}

function isClearMetadataEvent(evt: Event): evt is LootClearMetadataEvent {
  return evt.target instanceof LootPluginCard;
}

interface LootSearchBeginEvent extends CustomEvent {
  target: EventTarget & LootSearchToolbar;
}

function isSearchBeginEvent(evt: Event): evt is LootSearchBeginEvent {
  return evt.target instanceof LootSearchToolbar;
}

interface LootGameFolderChangeEvent extends CustomEvent {
  detail: { folder: string };
}

function isGameFolderChangeEvent(evt: Event): evt is LootGameFolderChangeEvent {
  return evt instanceof CustomEvent && typeof evt.detail.folder === 'string';
}

export function onSidebarFilterToggle(evt: Event): void {
  if (!isPaperCheckboxChangeEvent(evt)) {
    throw new TypeError(`Expected a PaperCheckboxChangeEvent, got ${evt}`);
  }

  window.loot.filters[evt.target.id] = evt.target.checked;

  saveFilterState(evt.target.id, evt.target.checked).catch(handlePromiseError);

  if (window.loot.game !== undefined) {
    window.loot.filters.apply(window.loot.game.plugins);
  }
}

export function onContentFilter(evt: Event): void {
  if (!isPaperInputChangeEvent(evt)) {
    throw new TypeError(`Expected a PaperInputChangeEvent, got ${evt}`);
  }

  window.loot.filters.contentSearchString = evt.target.value;
  if (window.loot.game !== undefined) {
    window.loot.filters.apply(window.loot.game.plugins);
  }
}

export function onConflictsFilter(evt: Event): void {
  if (!isLootDropdownMenuChangeEvent(evt)) {
    throw new TypeError(`Expected a LootDropdownMenuChangeEvent, got ${evt}`);
  }

  const currentGame = window.loot.game;
  if (currentGame === undefined) {
    throw new Error('Attempted to filter conflicts with no game loaded.');
  }

  /* evt.currentTarget.value is the name of the target plugin, or an empty string
     if the filter has been deactivated. */
  if (evt.currentTarget.value) {
    /* Now get conflicts for the plugin. */
    showProgress(
      window.loot.l10n.translate('Identifying conflicting plugins...')
    );

    window.loot.filters
      .activateConflictsFilter(evt.currentTarget.value)
      .then(response => {
        currentGame.generalMessages = response.generalMessages;
        currentGame.plugins = currentGame.plugins.reduce(
          (plugins: Plugin[], plugin) => {
            const responsePlugin = response.plugins.find(
              item => item.name === plugin.name
            );
            if (responsePlugin) {
              plugin.update(responsePlugin);
              plugins.push(plugin);
            }
            return plugins;
          },
          []
        );

        window.loot.filters.apply(currentGame.plugins);

        /* Scroll to the target plugin */
        const list = getElementById('pluginCardList') as IronListElement;
        if (list.items === null || list.items === undefined) {
          throw new Error('Plugin card list is null or undefined');
        }

        const index = list.items.findIndex(
          item => item.name === evt.target.value
        );
        list.scrollToIndex(index);

        closeProgress();
      })
      .catch(handlePromiseError);
  } else {
    window.loot.filters.deactivateConflictsFilter();
    window.loot.filters.apply(currentGame.plugins);
  }
}

export function onChangeGame(evt: Event): void {
  if (!isLootDropdownMenuSelectEvent(evt)) {
    throw new TypeError(`Expected a LootDropdownMenuSelectEvent, got ${evt}`);
  }

  const newGameFolder = evt.detail.item.getAttribute('value');
  if (
    newGameFolder === null ||
    (window.loot.game !== undefined &&
      newGameFolder === window.loot.game.folder)
  ) {
    // window.loot.game is undefined if LOOT is being initalised.
    return;
  }

  /* Send off a CEF query with the folder name of the new game. */
  changeGame(newGameFolder)
    .then(result => {
      /* Filters should be re-applied on game change, except the conflicts
       filter. Don't need to deactivate the others beforehand. Strictly not
       deactivating the conflicts filter either, just resetting it's value.
       */
      window.loot.filters.deactivateConflictsFilter();

      /* Clear the UI of all existing game-specific data. Also
       clear the card and li variables for each plugin object. */
      const generalMessages = getElementById('summary').getElementsByTagName(
        'ul'
      )[0];
      while (generalMessages.firstElementChild) {
        generalMessages.removeChild(generalMessages.firstElementChild);
      }

      window.loot.game = new Game(result, window.loot.l10n);
      window.loot.game.initialiseUI(window.loot.filters);

      closeProgress();
    })
    .catch(handlePromiseError);
}

/* Masterlist update process, minus progress dialog. */
function updateMasterlist(): Promise<void> {
  const currentGame = window.loot.game;
  if (currentGame === undefined) {
    // There's nothing to do if no game has been loaded.
    // eslint-disable-next-line no-console
    throw new Error('Attempted to update masterlist with no game loaded.');
  }

  showProgress(
    window.loot.l10n.translate('Updating and parsing masterlist...')
  );
  return updateMasterlistQuery()
    .then(result => {
      if (result) {
        /* Update JS variables. */
        currentGame.masterlist = result.masterlist;
        currentGame.generalMessages = result.generalMessages;
        currentGame.setGroups(result.groups);

        /* Update Bash Tag autocomplete suggestions. */
        initialiseAutocompleteBashTags(result.bashTags);

        result.plugins.forEach(resultPlugin => {
          const existingPlugin = currentGame.plugins.find(
            plugin => plugin.name === resultPlugin.name
          );
          if (existingPlugin) {
            existingPlugin.update(resultPlugin);
          }
        });

        showNotification(
          window.loot.l10n.translateFormatted(
            'Masterlist updated to revision %s.',
            currentGame.masterlist.revision
          )
        );
      } else {
        showNotification(
          window.loot.l10n.translate('No masterlist update was necessary.')
        );
      }
    })
    .catch(handlePromiseError);
}
export function onUpdateMasterlist(): void {
  updateMasterlist()
    .then(() => {
      closeProgress();
    })
    .catch(handlePromiseError);
}

export function onSortPlugins(): Promise<void> {
  const currentGame = window.loot.game;
  if (currentGame === undefined) {
    throw new Error('Attempted to sort plugins with no game loaded.');
  }

  if (window.loot.filters.deactivateConflictsFilter()) {
    /* Conflicts filter was undone, update the displayed cards. */
    window.loot.filters.apply(currentGame.plugins);
  }

  let promise = Promise.resolve();
  // window.loot.settings being undefined is an unexpected failure state, no
  // point updating the masterlist in it.
  if (window.loot.settings && window.loot.settings.updateMasterlist) {
    promise = promise.then(updateMasterlist);
  }
  return promise
    .then(sortPlugins)
    .then(result => {
      if (!result) {
        return;
      }

      currentGame.generalMessages = result.generalMessages;

      if (!result.plugins || result.plugins.length === 0) {
        const message = result.generalMessages.find(item =>
          item.text.startsWith(
            window.loot.l10n.translate('Cyclic interaction detected')
          )
        );
        const text = message
          ? marked(message.text)
          : 'see general messages for details.';
        throw new Error(
          window.loot.l10n.translateFormatted(
            'Failed to sort plugins. Details: %s',
            text
          )
        );
      }

      /* Check if sorted load order differs from current load order. */
      const loadOrderIsUnchanged = result.plugins.every(
        (plugin, index) =>
          currentGame.plugins[index] &&
          plugin.name === currentGame.plugins[index].name
      );
      if (loadOrderIsUnchanged) {
        result.plugins.forEach(plugin => {
          const existingPlugin = currentGame.plugins.find(
            item => item.name === plugin.name
          );
          if (existingPlugin) {
            existingPlugin.update(plugin);
          }
        });
        /* Send discardUnappliedChanges query. Not doing so prevents LOOT's window
         from closing. */
        discardUnappliedChanges();
        closeProgress();
        showNotification(
          window.loot.l10n.translate(
            'Sorting made no changes to the load order.'
          )
        );
        return;
      }
      currentGame.setSortedPlugins(result.plugins);

      /* Now update the UI for the new order. */
      window.loot.filters.apply(currentGame.plugins);

      window.loot.state.enterSortingState();

      closeProgress();
    })
    .catch(handlePromiseError);
}

export function onApplySort(): Promise<void> {
  const currentGame = window.loot.game;
  if (currentGame === undefined) {
    throw new Error('Attempted to apply sort with no game loaded.');
  }

  const pluginNames = currentGame.getPluginNames();
  return applySort(pluginNames)
    .then(() => {
      currentGame.applySort();

      window.loot.state.exitSortingState();
    })
    .catch(handlePromiseError);
}

export function onCancelSort(): Promise<void> {
  const currentGame = window.loot.game;
  if (currentGame === undefined) {
    throw new Error('Attempted to cancel sort with no game loaded.');
  }

  return cancelSort()
    .then(response => {
      currentGame.cancelSort(response.plugins, response.generalMessages);
      /* Sort UI elements again according to stored old load order. */
      window.loot.filters.apply(currentGame.plugins);

      window.loot.state.exitSortingState();
    })
    .catch(handlePromiseError);
}

export function onRedatePlugins(/* evt */): void {
  askQuestion(
    window.loot.l10n.translate('Redate Plugins?'),
    window.loot.l10n.translate(
      'This feature is provided so that modders using the Creation Kit may set the load order it uses. A side-effect is that any subscribed Steam Workshop mods will be re-downloaded by Steam (this does not affect Skyrim Special Edition). Do you wish to continue?'
    ),
    window.loot.l10n.translate('Redate'),
    result => {
      if (result) {
        redatePlugins()
          .then(() => {
            showNotification(
              window.loot.l10n.translate('Plugins were successfully redated.')
            );
          })
          .catch(handlePromiseError);
      }
    }
  );
}

export function onClearAllMetadata(): void {
  const currentGame = window.loot.game;
  if (currentGame === undefined) {
    throw new Error(
      'Attempted to clear all user metadata with no game loaded.'
    );
  }

  askQuestion(
    '',
    window.loot.l10n.translate(
      'Are you sure you want to clear all existing user-added metadata from all plugins?'
    ),
    window.loot.l10n.translate('Clear'),
    result => {
      if (!result) {
        return;
      }
      clearAllMetadata()
        .then(plugins => {
          currentGame.clearMetadata(plugins);

          showNotification(
            window.loot.l10n.translate(
              'All user-added metadata has been cleared.'
            )
          );
        })
        .catch(handlePromiseError);
    }
  );
}

export function onCopyContent(): void {
  let content: GameContent = {
    messages: [],
    plugins: []
  };

  if (window.loot.game) {
    content = window.loot.game.getContent();
  } else {
    const message = getElementById('summary').getElementsByTagName('ul')[0]
      .firstElementChild;

    const { language = 'en' } = window.loot.settings || {};

    if (message) {
      content.messages.push({
        type: message.className,
        text: message.textContent || '',
        language,
        condition: ''
      });
    }
  }

  copyContent(content)
    .then(() => {
      showNotification(
        window.loot.l10n.translate(
          "LOOT's content has been copied to the clipboard."
        )
      );
    })
    .catch(handlePromiseError);
}

export function onCopyLoadOrder(): void {
  let pluginNames: string[] = [];

  if (window.loot.game && window.loot.game.plugins) {
    pluginNames = window.loot.game.getPluginNames();
  }

  copyLoadOrder(pluginNames)
    .then(() => {
      showNotification(
        window.loot.l10n.translate(
          'The load order has been copied to the clipboard.'
        )
      );
    })
    .catch(handlePromiseError);
}

export function onContentRefresh(): void {
  /* Send a query for updated load order and plugin header info. */
  getGameData()
    .then(result => {
      window.loot.game = new Game(result, window.loot.l10n);
      window.loot.game.initialiseUI(window.loot.filters);

      closeProgress();
    })
    .catch(handlePromiseError);
}

export function onOpenReadme(evt: Event): void {
  let relativeFilePath = 'index.html';
  if (evt instanceof CustomEvent && evt.detail.relativeFilePath) {
    relativeFilePath = evt.detail.relativeFilePath;
  }

  openReadme(relativeFilePath).catch(handlePromiseError);
}

export function onOpenLogLocation(): void {
  openLogLocation().catch(handlePromiseError);
}

function handleUnappliedChangesClose(change: string): void {
  askQuestion(
    '',
    window.loot.l10n.translateFormatted(
      'You have not yet applied or cancelled your %s. Are you sure you want to quit?',
      change
    ),
    window.loot.l10n.translate('Quit'),
    result => {
      if (!result) {
        return;
      }
      /* Discard any unapplied changes. */
      discardUnappliedChanges()
        .then(() => {
          window.close();
        })
        .catch(handlePromiseError);
    }
  );
}

export function onQuit(): void {
  if (window.loot.state.isInSortingState()) {
    handleUnappliedChangesClose(
      window.loot.l10n.translate('sorted load order')
    );
  } else if (window.loot.state.isInEditingState()) {
    handleUnappliedChangesClose(window.loot.l10n.translate('metadata edits'));
  } else {
    window.close();
  }
}

export function onApplySettings(evt: Event): void {
  if (!(getElementById('gameTable') as EditableTable).validate()) {
    evt.stopPropagation();
  }
}

export function onCloseSettingsDialog(evt: Event): void {
  if (!isIronOverlayClosedEvent(evt)) {
    throw new TypeError(`Expected a IronOverlayClosedEvent, got ${evt}`);
  }

  if (window.loot.settings === undefined) {
    throw new Error(
      'Attempted to close settings dialog with no settings loaded.'
    );
  }

  if (evt.target.id !== 'settingsDialog') {
    /* The event can be fired by dropdowns in the settings dialog, so ignore
       any events that don't come from the dialog itself. */
    return;
  }
  if (!evt.detail.confirmed) {
    /* Re-apply the existing settings to the settings dialog elements. */
    updateSettingsDialog(window.loot.settings);
    return;
  }

  /* Update the JS variable values. */
  const settings: LootSettings = {
    enableDebugLogging:
      (getElementById('enableDebugLogging') as PaperCheckboxElement).checked ||
      false,
    game: (getElementById('defaultGameSelect') as LootDropdownMenu).value,
    games: (getElementById('gameTable') as EditableTable).getRowsData(
      false
    ) as GameSettings[],
    language: (getElementById('languageSelect') as LootDropdownMenu).value,
    theme: (getElementById('themeSelect') as LootDropdownMenu).value,
    updateMasterlist:
      (getElementById('updateMasterlist') as PaperCheckboxElement).checked ||
      false,
    enableLootUpdateCheck:
      (getElementById('enableLootUpdateCheck') as PaperCheckboxElement)
        .checked || false,
    filters: window.loot.settings.filters,
    lastVersion: window.loot.settings.lastVersion,
    languages: window.loot.settings.languages
  };

  /* Send the settings back to the C++ side. */
  closeSettings(settings)
    .then(installedGames => {
      window.loot.installedGames = installedGames;
      if (window.loot.installedGames.length > 0) {
        enableGameOperations(true);
      }
    })
    .catch(handlePromiseError)
    .then(() => {
      window.loot.settings = settings;
      updateSettingsDialog(window.loot.settings);
      setGameMenuItems(window.loot.settings.games, window.loot.installedGames);
      if (window.loot.game !== undefined) {
        updateSelectedGame(window.loot.game.folder);
      }
    })
    .then(() => {
      if (
        window.loot.installedGames.length > 0 &&
        window.loot.game !== undefined &&
        window.loot.game.folder.length === 0
      ) {
        /* Initialisation failed and game was configured in settings. */
        onContentRefresh();
      }
    })
    .catch(handlePromiseError);
}

export function onSaveUserGroups(evt: Event): void {
  if (!isIronOverlayClosedEvent(evt)) {
    throw new TypeError(`Expected a IronOverlayClosedEvent, got ${evt}`);
  }

  const currentGame = window.loot.game;
  if (currentGame === undefined) {
    throw new Error('Attempted to save user groups with no game loaded.');
  }

  if (evt.target.id !== 'groupsEditorDialog') {
    /* The event can be fired by dropdowns in the settings dialog, so ignore
       any events that don't come from the dialog itself. */
    return;
  }
  const editor = getElementById('groupsEditor') as LootGroupsEditor;
  if (!evt.detail.confirmed) {
    /* Re-apply the existing groups to the editor. */
    editor.setGroups(currentGame.groups);
    return;
  }

  /* Send the settings back to the C++ side. */
  const userGroups = editor.getUserGroups();
  saveUserGroups(userGroups)
    .then(response => {
      currentGame.setGroups(response);
      fillGroupsList(currentGame.groups);
      editor.setGroups(currentGame.groups);
    })
    .catch(handlePromiseError);
}

export function onEditorOpen(evt: Event): Promise<string | void> {
  if (!isPluginEditorOpenEvent(evt)) {
    throw new TypeError(`Expected a LootPluginEditorOpenEvent, got ${evt}`);
  }

  /* Set the editor data. */
  (getElementById('editor') as LootPluginEditor).setEditorData(evt.target.data);

  window.loot.state.enterEditingState();

  /* Sidebar items have been resized. */
  (getElementById('cardsNav') as IronListElement).notifyResize();

  /* Update the plugin's editor state tracker */
  evt.target.data.isEditorOpen = true;

  /* Set up drag 'n' drop event handlers. */
  const elements = getElementById('cardsNav').getElementsByTagName(
    'loot-plugin-item'
  );

  for (const element of elements) {
    const item = element as LootPluginItem;
    item.draggable = true;
    item.addEventListener('dragstart', item.onDragStart);
  }

  return editorOpened().catch(handlePromiseError);
}

export function onEditorClose(evt: Event): void {
  if (!isPluginEditorCloseEvent(evt)) {
    throw new TypeError(`Expected a LootPluginEditorCloseEvent, got ${evt}`);
  }

  if (window.loot.game === undefined) {
    throw new Error('Attempted to save metadata edits with no game loaded.');
  }

  const pluginName = querySelector(evt.target, 'h1').textContent;
  const plugin = window.loot.game.plugins.find(
    item => item.name === pluginName
  );
  if (plugin === undefined) {
    throw new Error(`Cannot find plugin with name "${pluginName}"`);
  }

  /* Update the plugin's editor state tracker */
  plugin.isEditorOpen = false;

  /* evt.detail is true if the apply button was pressed. */
  const metadata = evt.target.readFromEditor();
  const editorState: EditorState = {
    applyEdits: evt.detail,
    metadata
  };

  editorClosed(editorState)
    .then(result => {
      plugin.update(result);

      /* Now perform search again. If there is no current search, this won't
       do anything. */
      (getElementById('searchBar') as LootSearchToolbar).search();
    })
    .catch(handlePromiseError)
    .then(() => {
      window.loot.state.exitEditingState();
      /* Sidebar items have been resized. */
      (getElementById('cardsNav') as IronListElement).notifyResize();

      /* Remove drag 'n' drop event handlers. */
      const elements = getElementById('cardsNav').getElementsByTagName(
        'loot-plugin-item'
      );

      for (const element of elements) {
        const item = element as LootPluginItem;
        item.removeAttribute('draggable');
        item.removeEventListener('dragstart', item.onDragStart);
      }
    })
    .catch(handlePromiseError);
}

export function onCopyMetadata(evt: Event): void {
  if (!isCopyMetadataEvent(evt)) {
    throw new TypeError(`Expected a LootCopyMetadataEvent, got ${evt}`);
  }

  copyMetadata(evt.target.getName())
    .then(() => {
      showNotification(
        window.loot.l10n.translateFormatted(
          'The metadata for "%s" has been copied to the clipboard.',
          evt.target.getName() || ''
        )
      );
    })
    .catch(handlePromiseError);
}

export function onClearMetadata(evt: Event): void {
  if (!isClearMetadataEvent(evt)) {
    throw new TypeError(`Expected a LootClearMetadataEvent, got ${evt}`);
  }

  const currentGame = window.loot.game;
  if (currentGame === undefined) {
    throw new Error('Attempted to clear user metadata with no game loaded.');
  }

  askQuestion(
    '',
    window.loot.l10n.translateFormatted(
      'Are you sure you want to clear all existing user-added metadata from "%s"?',
      evt.target.getName() || ''
    ),
    window.loot.l10n.translate('Clear'),
    result => {
      if (!result) {
        return;
      }
      clearPluginMetadata(evt.target.getName())
        .then(plugin => {
          if (!result) {
            return;
          }
          /* Need to empty the UI-side user metadata. */
          const existingPlugin = currentGame.plugins.find(
            item => item.id === evt.target.id
          );
          if (existingPlugin) {
            existingPlugin.update(plugin);
          }
          showNotification(
            window.loot.l10n.translateFormatted(
              'The user-added metadata for "%s" has been cleared.',
              evt.target.getName() || ''
            )
          );
          /* Now perform search again. If there is no current search, this won't do anything. */
          (getElementById('searchBar') as LootSearchToolbar).search();
        })
        .catch(handlePromiseError);
    }
  );
}

export function onSearchBegin(evt: Event): void {
  if (!isSearchBeginEvent(evt)) {
    throw new TypeError(`Expected a LootSearchBeginEvent, got ${evt}`);
  }

  if (window.loot.game === undefined) {
    throw new Error('Attempted to search content with no game loaded.');
  }

  window.loot.game.plugins.forEach(plugin => {
    plugin.isSearchResult = false;
  });

  if (!evt.detail.needle) {
    return;
  }

  evt.target.results = window.loot.game.plugins.reduce(
    (indices: number[], plugin, index) => {
      if (
        plugin
          .getCardContent(window.loot.filters)
          .containsText(evt.detail.needle)
      ) {
        indices.push(index);
        plugin.isSearchResult = true;
      }
      return indices;
    },
    []
  );
}

export function onSearchEnd(/* evt */): void {
  if (window.loot.game === undefined) {
    throw new Error('Attempted to search content with no game loaded.');
  }

  window.loot.game.plugins.forEach(plugin => {
    plugin.isSearchResult = false;
  });
  getElementById('mainToolbar').classList.remove('search');
}

export function onFolderChange(evt: Event): void {
  if (!isGameFolderChangeEvent(evt)) {
    throw new TypeError(`Expected a LootGameFolderChangeEvent, got ${evt}`);
  }

  updateSelectedGame(evt.detail.folder);
  /* Enable/disable the redate plugins option. */
  let gameSettings;
  if (window.loot.settings && window.loot.settings.games) {
    gameSettings = window.loot.settings.games.find(
      game =>
        (game.type === 'Skyrim' || game.type === 'Skyrim Special Edition') &&
        game.folder === evt.detail.folder
    );
  }
  enable('redatePluginsButton', gameSettings !== undefined);
}
