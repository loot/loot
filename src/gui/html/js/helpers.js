'use strict';
function processCefError(err) {
    /* Error.stack seems to be Chromium-specific. It gives a lot more useful
       info than just the error message. Also, this can be used to catch any
       promise errors, not just CEF errors. */
    console.log(err.stack);
    closeProgressDialog();
    showMessageBox(loot.l10n.translate('Error'), err.message);
}

function showElement(element) {
    if (element != null) {
        element.classList.toggle('hidden', false);
    }
}
function hideElement(element) {
    if (element != null) {
        element.classList.toggle('hidden', true);
    }
}
function toast(text) {
    var toast = document.getElementById('toast');
    toast.text = text;
    toast.show();
}
function showMessageDialog(title, text, positiveText, closeCallback) {
    var dialog = document.createElement('loot-message-dialog');
    dialog.setButtonText(positiveText, loot.l10n.translate('Cancel'));
    dialog.showModal(title, text, closeCallback);
    document.body.appendChild(dialog);
}
function showMessageBox(title, text) {
    var dialog = document.createElement('loot-message-dialog');
    dialog.setButtonText(loot.l10n.translate('OK'));
    dialog.showModal(title, text);
    document.body.appendChild(dialog);
}

function showProgress(message) {
    var progressDialog = document.getElementById('progressDialog');
    if (message) {
        progressDialog.getElementsByTagName('p')[0].textContent = message;
    }
    if (!progressDialog.opened) {
        progressDialog.showModal();
    }
}
function closeProgressDialog() {
    var progressDialog = document.getElementById('progressDialog');
    if (progressDialog.opened) {
        progressDialog.close();
    }
}
function handleUnappliedChangesClose(change) {
    showMessageDialog('', loot.l10n.translate('You have not yet applied or cancelled your %s. Are you sure you want to quit?', change), loot.l10n.translate('Quit'), function(result){
        if (result) {
            /* Cancel any sorting and close any editors. Cheat by sending a
               cancelSort query for as many times as necessary. */
            var queries = [];
            var numQueries = 0;
            if (!document.getElementById('applySortButton').classList.contains('hidden')) {
                numQueries += 1;
            }
            numQueries += document.body.getAttribute('data-editors');
            for (var i = 0; i < numQueries; ++i) {
                queries.push(loot.query('cancelSort'));
            }
            Promise.all(queries).then(function(){
                window.close();
            }).catch(processCefError);
        }
    });
}
function getConflictingPlugins(pluginName)  {
  if (!pluginName) {
    return Promise.resolve([]);
  }

  /* Now get conflicts for the plugin. */
  showProgress(loot.l10n.translate('Checking if plugins have been loaded...'));

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
      closeProgressDialog();
      return conflicts;
    }
    closeProgressDialog();
    return [pluginName];
  }).catch(processCefError);
}
function setFilteredUIData(filtersState) {
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
    var gameMenuItems = document.getElementById('gameMenu').children;
    for (var i = 0; i < gameMenuItems.length; ++i) {
        if (installedGames.indexOf(gameMenuItems[i].getAttribute('value')) == -1) {
            gameMenuItems[i].setAttribute('disabled', true);
            gameMenuItems[i].removeEventListener('click', onChangeGame, false);
        } else {
            gameMenuItems[i].removeAttribute('disabled');
            gameMenuItems[i].addEventListener('click', onChangeGame, false);
        }
    }
}
function setInstalledGames(installedGames) {
  loot.installedGames = installedGames;
  updateEnabledGames(installedGames);
}
/* Call whenever settings are changed. */
function updateSettingsUI() {
    var gameSelect = document.getElementById('defaultGameSelect');
    var gameMenu = document.getElementById('gameMenu');
    var gameTable = document.getElementById('gameTable');

    /* First make sure game listing elements don't have any existing entries. */
    while (gameSelect.children.length > 1) {
        gameSelect.removeChild(gameSelect.lastElementChild);
    }
    while (gameMenu.firstElementChild) {
        gameMenu.firstElementChild.removeEventListener('click', onChangeGame, false);
        gameMenu.removeChild(gameMenu.firstElementChild);
    }
    gameTable.clear();

    /* Now fill with new values. */
    loot.settings.games.forEach(function(game){
        var menuItem = document.createElement('paper-item');
        menuItem.setAttribute('value', game.folder);
        menuItem.setAttribute('noink', '');
        menuItem.textContent = game.name;
        gameMenu.appendChild(menuItem);
        gameSelect.appendChild(menuItem.cloneNode(true));

        var row = gameTable.addRow(game);
        gameTable.setReadOnly(row, ['name','folder','type']);
    });

    gameSelect.value = loot.settings.game;
    document.getElementById('languageSelect').value = loot.settings.language;
    document.getElementById('enableDebugLogging').checked = loot.settings.enableDebugLogging;
    document.getElementById('updateMasterlist').checked = loot.settings.updateMasterlist;

    updateEnabledGames(loot.installedGames);
    updateSelectedGame(loot.game.folder);
}
