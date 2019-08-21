import { PaperCheckboxElement } from '@polymer/paper-checkbox';
import { IronListElement } from '@polymer/iron-list';
import handlePromiseError from './handlePromiseError';
import { getConflictingPlugins } from './query';
import Translator from './translator';
import { Plugin } from './plugin';
import { SimpleMessage, FilterStates, MainContent } from './interfaces';
import LootPluginCard from '../elements/loot-plugin-card';
import LootSearchToolbar from '../elements/loot-search-toolbar';
import LootDropdownMenu from '../elements/loot-dropdown-menu';
import { getElementById } from './dom/helpers';

export default class Filters implements FilterStates {
  public hideMessagelessPlugins: boolean;

  public hideInactivePlugins: boolean;

  public hideVersionNumbers: boolean;

  public hideCRCs: boolean;

  public hideBashTags: boolean;

  public hideAllPluginMessages: boolean;

  public hideNotes: boolean;

  public hideDoNotCleanMessages: boolean;

  public conflictingPluginNames: string[];

  public contentSearchString: string;

  private _doNotCleanString: string;

  public constructor(l10n: Translator) {
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

  public pluginFilter(plugin: Plugin): boolean {
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

  public messageFilter(message: SimpleMessage): boolean {
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

  public deactivateConflictsFilter(): boolean {
    const wasEnabled = this.conflictingPluginNames.length > 0;

    this.conflictingPluginNames = [];

    document.dispatchEvent(new CustomEvent('loot-filter-conflicts-deactivate'));

    return wasEnabled;
  }

  public activateConflictsFilter(
    targetPluginName: string
  ): Promise<MainContent> {
    const noData: MainContent = {
      generalMessages: [],
      plugins: []
    };

    if (!targetPluginName) {
      return Promise.resolve(noData);
    }

    /* Filter everything but the plugin itself if there are no
    conflicts. */
    this.conflictingPluginNames = [targetPluginName];

    return getConflictingPlugins(targetPluginName)
      .then(response => {
        const plugins = response.plugins.map(plugin => {
          if (plugin.conflicts) {
            this.conflictingPluginNames.push(plugin.metadata.name);
          }
          return plugin.metadata;
        });

        return {
          generalMessages: response.generalMessages,
          plugins
        };
      })
      .catch(error => {
        handlePromiseError(error);
        return noData;
      });
  }

  public areAnyFiltersActive(): boolean {
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

  public load(filterSettings: FilterStates): void {
    if (!filterSettings) {
      return;
    }

    const filters: (keyof FilterStates)[] = [
      'hideMessagelessPlugins',
      'hideInactivePlugins',
      'hideVersionNumbers',
      'hideCRCs',
      'hideBashTags',
      'hideAllPluginMessages',
      'hideNotes',
      'hideDoNotCleanMessages'
    ];

    filters.forEach(filter => {
      this[filter] = filterSettings[filter];
      (getElementById(filter) as PaperCheckboxElement).checked =
        filterSettings[filter];
    });
  }

  public apply(plugins: Plugin[]): void {
    const filteredPlugins = plugins.filter(this.pluginFilter, this);

    const cardsNav = getElementById('cardsNav') as IronListElement;
    const cardsList = getElementById('pluginCardList') as IronListElement;

    cardsNav.items = filteredPlugins;
    cardsList.items = filteredPlugins;

    for (const pluginCard of cardsList.children) {
      const card = pluginCard as LootPluginCard;
      if (card.data) {
        card.updateContent(true, false);
      }
    }
    cardsNav.notifyResize();
    cardsList.notifyResize();

    /* Now perform search again. If there is no current search, this won't
    do anything. */
    (getElementById('searchBar') as LootSearchToolbar).search();

    /* Re-count all hidden plugins and messages. */
    getElementById('hiddenPluginNo').textContent = (
      plugins.length - filteredPlugins.length
    ).toString();

    getElementById('hiddenMessageNo').textContent = plugins
      .reduce(
        (previousValue, currentValue) =>
          previousValue +
          currentValue.messages.length -
          currentValue.getCardContent(this).messages.length,
        0
      )
      .toString();
  }

  public static fillConflictsFilterList(plugins: Plugin[]): void {
    const list = getElementById('conflictsFilter');

    /* Remove any existing plugin items. */
    while (list.children.length > 1) {
      list.removeChild(list.children[1]);
    }

    plugins.forEach(plugin => {
      const item = document.createElement('paper-item');
      item.setAttribute('value', plugin.name);
      item.textContent = plugin.name;

      list.appendChild(item);
    });
  }

  public static onDeactivateConflictsFilter(): void {
    (getElementById('conflictsFilter') as LootDropdownMenu).value = '';
  }
}
