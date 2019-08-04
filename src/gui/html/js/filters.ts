import { PaperCheckboxElement } from '@polymer/paper-checkbox';
import { IronListElement } from '@polymer/iron-list';
import handlePromiseError from './handlePromiseError';
import query from './query';
import Translator from './translator';
import { Plugin } from './plugin';
import {
  SimpleMessage,
  DerivedPluginMetadata,
  FilterStates,
  MainContent
} from './interfaces';
import LootPluginCard from '../elements/loot-plugin-card';
import LootSearchToolbar from '../elements/loot-search-toolbar';
import LootDropdownMenu from '../elements/loot-dropdown-menu';

interface PluginData {
  metadata: DerivedPluginMetadata;
  conflicts: boolean;
}

interface GetConflictingPluginsQueryResponse {
  generalMessages: SimpleMessage[];
  plugins: PluginData[];
}

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

    return query('getConflictingPlugins', { pluginName: targetPluginName })
      .then(JSON.parse)
      .then((response: GetConflictingPluginsQueryResponse) => {
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
    if (filterSettings) {
      Object.getOwnPropertyNames(filterSettings).forEach(
        (filter: keyof FilterStates) => {
          this[filter] = filterSettings[filter];
          (document.getElementById(
            filter
          ) as PaperCheckboxElement).checked = this[filter];
        }
      );
    }
  }

  public apply(plugins: Plugin[]): void {
    const filteredPlugins = plugins.filter(this.pluginFilter, this);

    const cardsNav = document.getElementById('cardsNav') as IronListElement;
    const cardsList = document.getElementById(
      'pluginCardList'
    ) as IronListElement;

    cardsNav.items = filteredPlugins;
    cardsList.items = filteredPlugins;

    const pluginCards = cardsList.children;
    for (let i = 0; i < pluginCards.length; i += 1) {
      const card = pluginCards[i] as LootPluginCard;
      // TODO: This cast won't be necessary once LootPluginCard is converted to TypeScript.
      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      if ((card as any).data) {
        card.updateContent(true);
      }
    }
    cardsNav.notifyResize();
    cardsList.notifyResize();

    /* Now perform search again. If there is no current search, this won't
    do anything. */
    (document.getElementById('searchBar') as LootSearchToolbar).search();

    /* Re-count all hidden plugins and messages. */
    document.getElementById('hiddenPluginNo').textContent = (
      plugins.length - filteredPlugins.length
    ).toString();
    document.getElementById('hiddenMessageNo').textContent = plugins
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

  public static onDeactivateConflictsFilter(): void {
    (document.getElementById('conflictsFilter') as LootDropdownMenu).value = '';
  }
}
