'use strict';
function filterPluginData(plugins, filters) {
  const filteredPlugins = plugins.filter(filters.pluginFilter, filters);

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
}
