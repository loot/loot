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
  const request = JSON.stringify({
    name: 'getConflictingPlugins',
    args: [
      pluginName,
    ],
  });

  showProgress(loot.l10n.translate('Checking if plugins have been loaded...'));

  return loot.query(request).then(JSON.parse).then((result) => {
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
