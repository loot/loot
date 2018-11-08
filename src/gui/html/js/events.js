import {
  askQuestion,
  closeProgress,
  showNotification,
  showProgress
} from './dialog.js';
import {
  fillGroupsList,
  initialiseVirtualLists,
  initialiseAutocompleteBashTags,
  updateSettingsDialog,
  updateEnabledGames,
  enableGameOperations,
  setGameMenuItems,
  updateSelectedGame,
  enable
} from './dom.js';
import Filters from './filters.js';
import Game from './game.js';
import handlePromiseError from './handlePromiseError.js';
import Plugin from './plugin.js';
import query from './query.js';

// Depends on the following globals:
// - loot.filters
// - loot.game
// - loot.l10n
// - loot.settings
// - loot.state
// - loot.installedGames

export function onSidebarFilterToggle(evt) {
  loot.filters[evt.target.id] = evt.target.checked;

  const filter = {
    name: evt.target.id,
    state: evt.target.checked
  };
  query('saveFilterState', { filter }).catch(handlePromiseError);
  loot.filters.apply(loot.game.plugins);
}
export function onContentFilter(evt) {
  loot.filters.contentSearchString = evt.target.value;
  loot.filters.apply(loot.game.plugins);
}
export function onConflictsFilter(evt) {
  /* evt.currentTarget.value is the name of the target plugin, or an empty string
     if the filter has been deactivated. */
  if (evt.currentTarget.value) {
    /* Now get conflicts for the plugin. */
    showProgress(loot.l10n.translate('Identifying conflicting plugins...'));
    loot.filters
      .activateConflictsFilter(evt.currentTarget.value)
      .then(response => {
        const newGamePlugins = [];
        loot.game.plugins.forEach(plugin => {
          const responsePlugin = response.plugins.find(
            item => item.name === plugin.name
          );
          if (responsePlugin) {
            plugin.update(responsePlugin);
            newGamePlugins.push(plugin);
          }
        });

        loot.game.generalMessages = response.generalMessages;

        loot.game.plugins = newGamePlugins;
        loot.filters.apply(loot.game.plugins);

        /* Scroll to the target plugin */
        const list = document.getElementById('pluginCardList');
        const index = list.items.findIndex(
          item => item.name === evt.target.value
        );
        list.scrollToIndex(index);

        closeProgress();
      })
      .catch(handlePromiseError);
  } else {
    loot.filters.deactivateConflictsFilter();
    loot.filters.apply(loot.game.plugins);
  }
}

export function onChangeGame(evt) {
  if (
    evt.detail.item.getAttribute('value') === loot.game.folder ||
    loot.game.folder.length === 0
  ) {
    // Game folder length is zero if LOOT is being initalised.
    return;
  }
  /* Send off a CEF query with the folder name of the new game. */
  query('changeGame', { gameFolder: evt.detail.item.getAttribute('value') })
    .then(result => {
      /* Filters should be re-applied on game change, except the conflicts
       filter. Don't need to deactivate the others beforehand. Strictly not
       deactivating the conflicts filter either, just resetting it's value.
       */
      loot.filters.deactivateConflictsFilter();

      /* Clear the UI of all existing game-specific data. Also
       clear the card and li variables for each plugin object. */
      const generalMessages = document
        .getElementById('summary')
        .getElementsByTagName('ul')[0];
      while (generalMessages.firstElementChild) {
        generalMessages.removeChild(generalMessages.firstElementChild);
      }

      /* Parse the data sent from C++. */
      const gameInfo = JSON.parse(result, Plugin.fromJson);
      loot.game = new Game(gameInfo, loot.l10n);

      loot.game.initialiseUI();

      /* Now update virtual lists. */
      if (loot.filters.areAnyFiltersActive()) {
        loot.filters.apply(loot.game.plugins);
      } else {
        initialiseVirtualLists(loot.game.plugins);
      }

      closeProgress();
    })
    .catch(handlePromiseError);
}
/* Masterlist update process, minus progress dialog. */
function updateMasterlist() {
  showProgress(loot.l10n.translate('Updating and parsing masterlist...'));
  return query('updateMasterlist')
    .then(JSON.parse)
    .then(result => {
      if (result) {
        /* Update JS variables. */
        loot.game.masterlist = result.masterlist;
        loot.game.generalMessages = result.generalMessages;
        loot.game.groups = result.groups;

        /* Update Bash Tag autocomplete suggestions. */
        initialiseAutocompleteBashTags(result.bashTags);

        result.plugins.forEach(resultPlugin => {
          const existingPlugin = loot.game.plugins.find(
            plugin => plugin.name === resultPlugin.name
          );
          if (existingPlugin) {
            existingPlugin.update(resultPlugin);
          }
        });

        showNotification(
          loot.l10n.translateFormatted(
            'Masterlist updated to revision %s.',
            loot.game.masterlist.revision
          )
        );
      } else {
        showNotification(
          loot.l10n.translate('No masterlist update was necessary.')
        );
      }
    })
    .catch(handlePromiseError);
}
export function onUpdateMasterlist() {
  updateMasterlist()
    .then(() => {
      closeProgress();
    })
    .catch(handlePromiseError);
}
export function onSortPlugins() {
  if (loot.filters.deactivateConflictsFilter()) {
    /* Conflicts filter was undone, update the displayed cards. */
    loot.filters.apply(loot.game.plugins);
  }

  let promise = Promise.resolve();
  if (loot.settings.updateMasterlist) {
    promise = promise.then(updateMasterlist);
  }
  return promise
    .then(() => query('sortPlugins'))
    .then(JSON.parse)
    .then(result => {
      if (!result) {
        return;
      }

      loot.game.generalMessages = result.generalMessages;

      if (!result.plugins || result.plugins.length === 0) {
        const message = result.generalMessages.find(item =>
          item.text.startsWith('Cyclic interaction detected')
        );
        const text = message
          ? message.text
          : 'see general messages for details.';
        throw new Error(
          loot.l10n.translateFormatted(
            'Failed to sort plugins. Details: %s',
            text
          )
        );
      }

      /* Check if sorted load order differs from current load order. */
      const loadOrderIsUnchanged = result.plugins.every(
        (plugin, index) =>
          loot.game.plugins[index] &&
          plugin.name === loot.game.plugins[index].name
      );
      if (loadOrderIsUnchanged) {
        result.plugins.forEach(plugin => {
          const existingPlugin = loot.game.plugins.find(
            item => item.name === plugin.name
          );
          if (existingPlugin) {
            existingPlugin.update(plugin);
          }
        });
        /* Send discardUnappliedChanges query. Not doing so prevents LOOT's window
         from closing. */
        query('discardUnappliedChanges');
        closeProgress();
        showNotification(
          loot.l10n.translate('Sorting made no changes to the load order.')
        );
        return;
      }
      loot.game.setSortedPlugins(result.plugins);

      /* Now update the UI for the new order. */
      loot.filters.apply(loot.game.plugins);

      loot.state.enterSortingState();

      closeProgress();
    })
    .catch(handlePromiseError);
}
export function onApplySort() {
  const pluginNames = loot.game.getPluginNames();
  return query('applySort', { pluginNames })
    .then(() => {
      loot.game.applySort();

      loot.state.exitSortingState();
    })
    .catch(handlePromiseError);
}
export function onCancelSort() {
  return query('cancelSort')
    .then(JSON.parse)
    .then(response => {
      loot.game.cancelSort(response.plugins, response.generalMessages);
      /* Sort UI elements again according to stored old load order. */
      loot.filters.apply(loot.game.plugins);

      loot.state.exitSortingState();
    })
    .catch(handlePromiseError);
}

export function onRedatePlugins(/* evt */) {
  askQuestion(
    loot.l10n.translate('Redate Plugins?'),
    loot.l10n.translate(
      'This feature is provided so that modders using the Creation Kit may set the load order it uses. A side-effect is that any subscribed Steam Workshop mods will be re-downloaded by Steam (this does not affect Skyrim Special Edition). Do you wish to continue?'
    ),
    loot.l10n.translate('Redate'),
    result => {
      if (result) {
        query('redatePlugins')
          .then(() => {
            showNotification(
              loot.l10n.translate('Plugins were successfully redated.')
            );
          })
          .catch(handlePromiseError);
      }
    }
  );
}
export function onClearAllMetadata() {
  askQuestion(
    '',
    loot.l10n.translate(
      'Are you sure you want to clear all existing user-added metadata from all plugins?'
    ),
    loot.l10n.translate('Clear'),
    result => {
      if (!result) {
        return;
      }
      query('clearAllMetadata')
        .then(JSON.parse)
        .then(response => {
          if (!response || !response.plugins) {
            return;
          }

          loot.game.clearMetadata(response.plugins);

          showNotification(
            loot.l10n.translate('All user-added metadata has been cleared.')
          );
        })
        .catch(handlePromiseError);
    }
  );
}
export function onCopyContent() {
  let content = {
    messages: [],
    plugins: []
  };

  if (loot.game) {
    content = loot.game.getContent();
  } else {
    const message = document
      .getElementById('summary')
      .getElementsByTagName('ul')[0].firstElementChild;

    const { language = 'en' } = loot.settings || {};

    if (message) {
      content.messages.push({
        type: message.className,
        text: message.textContent,
        language
      });
    }
  }

  query('copyContent', { content })
    .then(() => {
      showNotification(
        loot.l10n.translate("LOOT's content has been copied to the clipboard.")
      );
    })
    .catch(handlePromiseError);
}
export function onCopyLoadOrder() {
  let pluginNames = [];

  if (loot.game && loot.game.plugins) {
    pluginNames = loot.game.getPluginNames();
  }

  query('copyLoadOrder', { pluginNames })
    .then(() => {
      showNotification(
        loot.l10n.translate('The load order has been copied to the clipboard.')
      );
    })
    .catch(handlePromiseError);
}
export function onContentRefresh() {
  /* Send a query for updated load order and plugin header info. */
  query('getGameData')
    .then(result => {
      /* Parse the data sent from C++. */
      const game = JSON.parse(result, Plugin.fromJson);
      loot.game = new Game(game, loot.l10n);

      /* Re-initialise conflicts filter plugin list. */
      Filters.fillConflictsFilterList(loot.game.plugins);

      /* Reapply filters. */
      if (loot.filters.areAnyFiltersActive()) {
        loot.filters.apply(loot.game.plugins);
      } else {
        initialiseVirtualLists(loot.game.plugins);
      }

      closeProgress();
    })
    .catch(handlePromiseError);
}

export function onOpenReadme(evt, relativeFilePath = 'index.html') {
  query('openReadme', { relativeFilePath }).catch(handlePromiseError);
}
export function onOpenLogLocation() {
  query('openLogLocation').catch(handlePromiseError);
}
function handleUnappliedChangesClose(change) {
  askQuestion(
    '',
    loot.l10n.translateFormatted(
      'You have not yet applied or cancelled your %s. Are you sure you want to quit?',
      change
    ),
    loot.l10n.translate('Quit'),
    result => {
      if (!result) {
        return;
      }
      /* Discard any unapplied changes. */
      query('discardUnappliedChanges')
        .then(() => {
          window.close();
        })
        .catch(handlePromiseError);
    }
  );
}
export function onQuit() {
  if (loot.state.isInSortingState()) {
    handleUnappliedChangesClose(loot.l10n.translate('sorted load order'));
  } else if (loot.state.isInEditingState()) {
    handleUnappliedChangesClose(loot.l10n.translate('metadata edits'));
  } else {
    window.close();
  }
}
export function onApplySettings(evt) {
  if (!document.getElementById('gameTable').validate()) {
    evt.stopPropagation();
  }
}
export function onCloseSettingsDialog(evt) {
  if (evt.target.id !== 'settingsDialog') {
    /* The event can be fired by dropdowns in the settings dialog, so ignore
       any events that don't come from the dialog itself. */
    return;
  }
  if (!evt.detail.confirmed) {
    /* Re-apply the existing settings to the settings dialog elements. */
    updateSettingsDialog(loot.settings);
    return;
  }

  /* Update the JS variable values. */
  const settings = {
    enableDebugLogging: document.getElementById('enableDebugLogging').checked,
    game: document.getElementById('defaultGameSelect').value,
    games: document.getElementById('gameTable').getRowsData(false),
    language: document.getElementById('languageSelect').value,
    lastGame: loot.settings.lastGame,
    updateMasterlist: document.getElementById('updateMasterlist').checked,
    enableLootUpdateCheck: document.getElementById('enableLootUpdateCheck')
      .checked,
    filters: loot.settings.filters
  };

  /* Send the settings back to the C++ side. */
  query('closeSettings', { settings })
    .then(JSON.parse)
    .then(response => {
      loot.installedGames = response.installedGames;
      updateEnabledGames(loot.installedGames);
      if (loot.installedGames.length > 0) {
        enableGameOperations(true);
      }
    })
    .catch(handlePromiseError)
    .then(() => {
      loot.settings = settings;
      updateSettingsDialog(loot.settings);
      setGameMenuItems(loot.settings.games);
      updateEnabledGames(loot.installedGames);
      updateSelectedGame(loot.game.folder);
    })
    .then(() => {
      if (loot.installedGames.length > 0 && loot.game.folder.length === 0) {
        /* Initialisation failed and game was configured in settings. */
        onContentRefresh();
      }
    })
    .catch(handlePromiseError);
}

export function onSaveUserGroups(evt) {
  if (evt.target.id !== 'groupsEditorDialog') {
    /* The event can be fired by dropdowns in the settings dialog, so ignore
       any events that don't come from the dialog itself. */
    return;
  }
  const editor = document.getElementById('groupsEditor');
  if (!evt.detail.confirmed) {
    /* Re-apply the existing groups to the editor. */
    editor.setGroups(loot.game.groups);
    return;
  }

  /* Send the settings back to the C++ side. */
  const userGroups = editor.getUserGroups();
  query('saveUserGroups', { userGroups })
    .then(JSON.parse)
    .then(response => {
      loot.game.groups = response;
      fillGroupsList(loot.game.groups);
      editor.setGroups(loot.game.groups);
    })
    .catch(handlePromiseError);
}

export function onEditorOpen(evt) {
  /* Set the editor data. */
  document.getElementById('editor').setEditorData(evt.target.data);

  loot.state.enterEditingState();

  /* Sidebar items have been resized. */
  document.getElementById('cardsNav').notifyResize();

  /* Update the plugin's editor state tracker */
  evt.target.data.isEditorOpen = true;

  /* Set up drag 'n' drop event handlers. */
  const elements = document
    .getElementById('cardsNav')
    .getElementsByTagName('loot-plugin-item');
  for (let i = 0; i < elements.length; i += 1) {
    elements[i].draggable = true;
    elements[i].addEventListener('dragstart', elements[i].onDragStart);
  }

  return query('editorOpened').catch(handlePromiseError);
}
export function onEditorClose(evt) {
  const plugin = loot.game.plugins.find(
    item => item.name === evt.target.querySelector('h1').textContent
  );
  /* Update the plugin's editor state tracker */
  plugin.isEditorOpen = false;

  /* evt.detail is true if the apply button was pressed. */
  const metadata = evt.target.readFromEditor();
  const editorState = {
    applyEdits: evt.detail,
    metadata
  };

  query('editorClosed', { editorState })
    .then(JSON.parse)
    .then(result => {
      plugin.update(result);

      /* Explicitly set userlist to detect when user edits have been removed
       (so result.userlist is not present). */
      plugin.userlist = result.userlist;

      /* Now perform search again. If there is no current search, this won't
       do anything. */
      document.getElementById('searchBar').search();
    })
    .catch(handlePromiseError)
    .then(() => {
      loot.state.exitEditingState();
      /* Sidebar items have been resized. */
      document.getElementById('cardsNav').notifyResize();

      /* Remove drag 'n' drop event handlers. */
      const elements = document
        .getElementById('cardsNav')
        .getElementsByTagName('loot-plugin-item');
      for (let i = 0; i < elements.length; i += 1) {
        elements[i].removeAttribute('draggable');
        elements[i].removeEventListener('dragstart', elements[i].onDragStart);
      }
    })
    .catch(handlePromiseError);
}
export function onCopyMetadata(evt) {
  query('copyMetadata', { pluginName: evt.target.getName() })
    .then(() => {
      showNotification(
        loot.l10n.translateFormatted(
          'The metadata for "%s" has been copied to the clipboard.',
          evt.target.getName()
        )
      );
    })
    .catch(handlePromiseError);
}
export function onClearMetadata(evt) {
  askQuestion(
    '',
    loot.l10n.translateFormatted(
      'Are you sure you want to clear all existing user-added metadata from "%s"?',
      evt.target.getName()
    ),
    loot.l10n.translate('Clear'),
    result => {
      if (!result) {
        return;
      }
      query('clearPluginMetadata', { pluginName: evt.target.getName() })
        .then(JSON.parse)
        .then(plugin => {
          if (!result) {
            return;
          }
          /* Need to empty the UI-side user metadata. */
          const existingPlugin = loot.game.plugins.find(
            item => item.id === evt.target.id
          );
          if (existingPlugin) {
            existingPlugin.userlist = undefined;

            existingPlugin.update(plugin);
          }
          showNotification(
            loot.l10n.translateFormatted(
              'The user-added metadata for "%s" has been cleared.',
              evt.target.getName()
            )
          );
          /* Now perform search again. If there is no current search, this won't
         do anything. */
          document.getElementById('searchBar').search();
        })
        .catch(handlePromiseError);
    }
  );
}

export function onSearchBegin(evt) {
  loot.game.plugins.forEach(plugin => {
    plugin.isSearchResult = false;
  });

  if (!evt.detail.needle) {
    return;
  }

  // Don't push to the target's results property directly, as the
  // change observer doesn't work correctly unless special Polymer APIs
  // are used, which I don't want to get into.
  const results = [];
  loot.game.plugins.forEach((plugin, index) => {
    if (plugin.getCardContent(loot.filters).containsText(evt.detail.needle)) {
      results.push(index);
      plugin.isSearchResult = true;
    }
  });

  evt.target.results = results;
}
export function onSearchEnd(/* evt */) {
  loot.game.plugins.forEach(plugin => {
    plugin.isSearchResult = false;
  });
  document.getElementById('mainToolbar').classList.remove('search');
}
export function onFolderChange(evt) {
  updateSelectedGame(evt.detail.folder);
  /* Enable/disable the redate plugins option. */
  let gameSettings;
  if (loot.settings && loot.settings.games) {
    gameSettings = loot.settings.games.find(
      game =>
        (game.type === 'Skyrim' || game.type === 'Skyrim Special Edition') &&
        game.folder === evt.detail.folder
    );
  }
  enable('redatePluginsButton', gameSettings !== undefined);
}
