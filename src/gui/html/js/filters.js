import handlePromiseError from './handlePromiseError.js';
import query from './query';

export default class Filters {
  constructor(l10n) {
    /* Plugin filters */
    this.hideMessagelessPlugins = false;
    this.hideInactivePlugins = false;
    this.conflictingPluginNames = [];
    this.contentSearchString = '';

    /* Plugin content filters */
    this.hideVersionNumbers = false;
    this.hideCRCs = false;
    this.hideBashTags = false;
    this.hideAllPluginMessages = false;
    this.hideNotes = false;
    this.hideDoNotCleanMessages = false;

    this._doNotCleanString = l10n.translate('Do not clean').toLowerCase();
  }

  pluginFilter(plugin) {
    if (this.hideInactivePlugins && !plugin.isActive) {
      return false;
    }

    if (
      this.conflictingPluginNames.length !== 0 &&
      this.conflictingPluginNames.indexOf(plugin.name) === -1
    ) {
      return false;
    }

    if (
      this.hideMessagelessPlugins &&
      plugin.getCardContent(this).messages.length === 0
    ) {
      return false;
    }

    if (
      this.contentSearchString.length !== 0 &&
      !plugin.getCardContent(this).containsText(this.contentSearchString)
    ) {
      return false;
    }

    return true;
  }

  messageFilter(message) {
    if (this.hideAllPluginMessages) {
      return false;
    }

    if (this.hideNotes && message.type === 'say') {
      return false;
    }

    if (
      this.hideDoNotCleanMessages &&
      message.text.toLowerCase().indexOf(this._doNotCleanString) !== -1
    ) {
      return false;
    }

    return true;
  }

  deactivateConflictsFilter() {
    const wasEnabled = this.conflictingPluginNames.length > 0;

    this.conflictingPluginNames = [];

    document.dispatchEvent(new CustomEvent('loot-filter-conflicts-deactivate'));

    return wasEnabled;
  }

  activateConflictsFilter(targetPluginName) {
    if (!targetPluginName) {
      return Promise.resolve([]);
    }

    /* Filter everything but the plugin itself if there are no
    conflicts. */
    this.conflictingPluginNames = [targetPluginName];

    return query('getConflictingPlugins', { pluginName: targetPluginName })
      .then(JSON.parse)
      .then(response => {
        response.plugins = response.plugins.map(plugin => {
          if (plugin.conflicts) {
            this.conflictingPluginNames.push(plugin.metadata.name);
          }
          return plugin.metadata;
        });

        return response;
      })
      .catch(handlePromiseError);
  }

  areAnyFiltersActive() {
    return (
      this.hideMessagelessPlugins ||
      this.hideInactivePlugins ||
      this.conflictingPluginNames.length !== 0 ||
      this.contentSearchString.length !== 0 ||
      this.hideVersionNumbers ||
      this.hideCRCs ||
      this.hideBashTags ||
      this.hideAllPluginMessages ||
      this.hideNotes ||
      this.hideDoNotCleanMessages
    );
  }

  load(filterSettings) {
    if (filterSettings) {
      Object.getOwnPropertyNames(filterSettings).forEach(filter => {
        this[filter] = filterSettings[filter];
        document.getElementById(filter).checked = this[filter];
      });
    }
  }

  apply(plugins) {
    const filteredPlugins = plugins.filter(this.pluginFilter, this);

    document.getElementById('cardsNav').items = filteredPlugins;
    document.getElementById('pluginCardList').items = filteredPlugins;

    const pluginCards = document.getElementById('pluginCardList').children;
    for (let i = 0; i < pluginCards.length; i += 1) {
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
    document.getElementById('hiddenPluginNo').textContent =
      plugins.length - filteredPlugins.length;
    document.getElementById('hiddenMessageNo').textContent = plugins.reduce(
      (previousValue, currentValue) =>
        previousValue +
        currentValue.messages.length -
        currentValue.getCardContent(this).messages.length,
      0
    );
  }

  static fillConflictsFilterList(plugins) {
    const list = document.getElementById('conflictsFilter');

    /* Remove any existing plugin items. */
    while (list.children.length > 1) {
      list.removeChild(list.lastElementChild);
    }

    plugins.forEach(plugin => {
      const item = document.createElement('paper-item');
      item.setAttribute('value', plugin.name);
      item.textContent = plugin.name;

      list.appendChild(item);
    });
  }

  static onDeactivateConflictsFilter() {
    document.getElementById('conflictsFilter').value = '';
  }
}
