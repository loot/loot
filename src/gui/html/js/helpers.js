'use strict';
function handlePromiseError(err) {
  /* Error.stack seems to be Chromium-specific. */
  console.log(err.stack);
  loot.Dialog.closeProgress();
  loot.Dialog.showMessage(loot.l10n.translate('Error'), err.message);
}
function getConflictingPlugins(pluginName) {
  if (!pluginName) {
    return Promise.resolve([]);
  }

  /* Now get conflicts for the plugin. */
  loot.Dialog.showProgress(loot.l10n.translate('Identifying conflicting plugins...'));

  return loot.query('getConflictingPlugins', pluginName).then(JSON.parse).then((result) => {
    const conflicts = [pluginName];
    if (result) {
      /* Filter everything but the plugin itself if there are no
         conflicts. */
      for (const key in result) {
        if (result[key].conflicts) {
          conflicts.push(key);
        }
        const plugin = loot.game.plugins.find(item => item.name === key);
        if (plugin) {
          plugin.update(result[key]);
        }
      }
    }
    loot.Dialog.closeProgress();
    return conflicts;
  }).catch(handlePromiseError);
}
function filterPluginData(plugins, filters) {
  getConflictingPlugins(filters.conflictTargetPluginName).then((conflictingPluginNames) => {
    filters.conflictingPluginNames = conflictingPluginNames;
    return plugins.filter(filters.pluginFilter, filters);
  }).then((filteredPlugins) => {
    document.getElementById('cardsNav').items = filteredPlugins;
    document.getElementById('pluginCardList').items = filteredPlugins;

    const pluginCards = document.getElementById('pluginCardList').children;
    for (let i = 0; i < pluginCards.length; ++i) {
      if (pluginCards[i].data) {
        pluginCards[i].updateContent(true);
      }
    }
    document.getElementById('cardsNav').notifyResize();
    document.getElementById('pluginCardList').notifyResize();

    /* Now perform search again. If there is no current search, this won't
       do anything. */
    document.getElementById('searchBar').search();

    /* Re-count all hidden plugins and messages. */
    document.getElementById('hiddenPluginNo').textContent = plugins.length - filteredPlugins.length;
    let hiddenMessageNo = 0;
    plugins.forEach((plugin) => {
      hiddenMessageNo += plugin.messages.length - plugin.getCardContent(filters).messages.length;
    });
    document.getElementById('hiddenMessageNo').textContent = hiddenMessageNo;
  }).catch(handlePromiseError);
}
