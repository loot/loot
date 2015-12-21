'use strict';
function handlePromiseError(err) {
  /* Error.stack seems to be Chromium-specific. */
  console.log(err.stack);
  loot.Dialog.closeProgress();
  loot.Dialog.showMessage(loot.l10n.translate('Error'), err.message);
}

function showElement(element) {
  if (element !== null) {
    element.classList.toggle('hidden', false);
  }
}
function hideElement(element) {
  if (element !== null) {
    element.classList.toggle('hidden', true);
  }
}
function handleUnappliedChangesClose(change) {
  loot.Dialog.askQuestion('', loot.l10n.translate('You have not yet applied or cancelled your %s. Are you sure you want to quit?', change), loot.l10n.translate('Quit'), (result) => {
    if (!result) {
      return;
    }
    /* Cancel any sorting and close any editors. Cheat by sending a
       cancelSort query for as many times as necessary. */
    const queries = [];
    let numQueries = 0;
    if (!document.getElementById('applySortButton').classList.contains('hidden')) {
      numQueries += 1;
    }
    numQueries += document.body.getAttribute('data-editors');
    for (let i = 0; i < numQueries; ++i) {
      queries.push(loot.query('cancelSort'));
    }
    Promise.all(queries).then(() => {
      window.close();
    }).catch(handlePromiseError);
  });
}
function getConflictingPlugins(pluginName) {
  if (!pluginName) {
    return Promise.resolve([]);
  }

  /* Now get conflicts for the plugin. */
  loot.Dialog.showProgress(loot.l10n.translate('Checking if plugins have been loaded...'));

  return loot.query('getConflictingPlugins', pluginName).then(JSON.parse).then((result) => {
    if (result) {
      /* Filter everything but the plugin itself if there are no
         conflicts. */
      const conflicts = [pluginName];
      for (const key in result) {
        if (result[key].conflicts) {
          conflicts.push(key);
        }
        for (let i = 0; i < loot.game.plugins.length; ++i) {
          if (loot.game.plugins[i].name === key) {
            loot.game.plugins[i].crc = result[key].crc;
            loot.game.plugins[i].isEmpty = result[key].isEmpty;

            loot.game.plugins[i].messages = result[key].messages;
            loot.game.plugins[i].tags = result[key].tags;
            loot.game.plugins[i].isDirty = result[key].isDirty;
            break;
          }
        }
      }
      loot.Dialog.closeProgress();
      return conflicts;
    }
    loot.Dialog.closeProgress();
    return [pluginName];
  }).catch(handlePromiseError);
}
function setFilteredUIData() {
  getConflictingPlugins(loot.filters.conflictTargetPluginName).then((conflictingPluginNames) => {
    loot.filters.conflictingPluginNames = conflictingPluginNames;
    return loot.game.plugins.filter(loot.filters.pluginFilter, loot.filters);
  }).then((filteredPlugins) => {
    document.getElementById('cardsNav').data = filteredPlugins;
    document.getElementById('pluginCardList').data = filteredPlugins;

    filteredPlugins.forEach((plugin) => {
      const element = document.getElementById(plugin.id);
      if (element) {
        element.onMessagesChange();
      }
    });

    /* Now perform search again. If there is no current search, this won't
       do anything. */
    document.getElementById('searchBar').search();

    /* Re-count all hidden plugins and messages. */
    document.getElementById('hiddenPluginNo').textContent = loot.game.plugins.length - filteredPlugins.length;
    let hiddenMessageNo = 0;
    loot.game.plugins.forEach((plugin) => {
      hiddenMessageNo += plugin.messages.length - plugin.getCardContent(loot.filters).messages.length;
    });
    document.getElementById('hiddenMessageNo').textContent = hiddenMessageNo;
  });
}
/* Call whenever game is changed or game menu / game table are rewritten. */
function updateSelectedGame(gameFolder) {
  document.getElementById('gameMenu').value = gameFolder;

  /* Also disable deletion of the game's row in the settings dialog. */
  const table = document.getElementById('gameTable');
  for (let i = 0; i < table.tBodies[0].rows.length; ++i) {
    if (table.tBodies[0].rows[i].getElementsByClassName('folder').length > 0) {
      if (table.tBodies[0].rows[i].getElementsByClassName('folder')[0].value === gameFolder) {
        table.setReadOnly(table.tBodies[0].rows[i], ['delete']);
      } else {
        table.setReadOnly(table.tBodies[0].rows[i], ['delete'], false);
      }
    }
  }
}

/* Call whenever installedGames is changed or game menu is rewritten. */
function updateEnabledGames(installedGames) {
  /* Update the disabled games in the game menu. */
  const gameMenuItems = document.getElementById('gameMenu').children;
  for (let i = 0; i < gameMenuItems.length; ++i) {
    if (installedGames.indexOf(gameMenuItems[i].getAttribute('value')) === -1) {
      gameMenuItems[i].setAttribute('disabled', true);
      gameMenuItems[i].removeEventListener('click', onChangeGame);
    } else {
      gameMenuItems[i].removeAttribute('disabled');
      gameMenuItems[i].addEventListener('click', onChangeGame);
    }
  }
}
function setInstalledGames(installedGames) {
  loot.installedGames = installedGames;
  updateEnabledGames(installedGames);
}
/* Call whenever settings are changed. */
function updateSettingsUI() {
  const gameSelect = document.getElementById('defaultGameSelect');
  const gameMenu = document.getElementById('gameMenu');
  const gameTable = document.getElementById('gameTable');

  /* First make sure game listing elements don't have any existing entries. */
  while (gameSelect.children.length > 1) {
    gameSelect.removeChild(gameSelect.lastElementChild);
  }
  while (gameMenu.firstElementChild) {
    gameMenu.firstElementChild.removeEventListener('click', onChangeGame);
    gameMenu.removeChild(gameMenu.firstElementChild);
  }
  gameTable.clear();

  /* Now fill with new values. */
  loot.settings.games.forEach((game) => {
    const menuItem = document.createElement('paper-item');
    menuItem.setAttribute('value', game.folder);
    menuItem.setAttribute('noink', '');
    menuItem.textContent = game.name;
    gameMenu.appendChild(menuItem);
    gameSelect.appendChild(menuItem.cloneNode(true));

    const row = gameTable.addRow(game);
    gameTable.setReadOnly(row, ['name', 'folder', 'type']);
  });

  gameSelect.value = loot.settings.game;
  document.getElementById('languageSelect').value = loot.settings.language;
  document.getElementById('enableDebugLogging').checked = loot.settings.enableDebugLogging;
  document.getElementById('updateMasterlist').checked = loot.settings.updateMasterlist;

  updateEnabledGames(loot.installedGames);
  updateSelectedGame(loot.game.folder);
}
