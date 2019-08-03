import { PaperDialogElement } from '@polymer/paper-dialog';
import { IronListElement } from '@polymer/iron-list';
import { IronSelectableBehavior } from '@polymer/iron-selector/iron-selectable';
import { PaperIconButtonElement } from '@polymer/paper-icon-button';
import { PaperIconItemElement } from '@polymer/paper-item/paper-icon-item';
import { PaperToggleButtonElement } from '@polymer/paper-toggle-button';
import { PaperInputElement } from '@polymer/paper-input/paper-input';
import EditableTable from '../../elements/editable-table';
import LootPluginCard from '../../elements/loot-plugin-card';
import LootGroupsEditor from '../../elements/loot-groups-editor';
import LootSearchToolbar from '../../elements/loot-search-toolbar';
import LootDropdownMenu from '../../elements/loot-dropdown-menu';
import { Game, Group, Language, SourcedGroup } from '../interfaces';
import {
  createGameItem,
  createGameTypeItem,
  createGroupItem,
  createLanguageItem,
  createMessageItem
} from './createItem';

interface Message {
  type: string;
  content: string;
}

interface GameSettings {
  type: string;
  name: string;
  master: string;
  registry: string;
  folder: string;
  repo: string;
  branch: string;
  path: string;
  localPath: string;
}

interface Filters {
  [id: string]: boolean;
}

interface LootSettings {
  game: string;
  games: GameSettings[];
  lastVersion: string;
  language: string;
  languages: Language[];
  enableDebugLogging: boolean;
  updateMasterlist: boolean;
  enableLootUpdateCheck: boolean;
  filters: Filters;
}

function getElementInTableRowTemplate(
  rowTemplateId: string,
  elementClass: string
): HTMLElement {
  const template = document.querySelector(
    `#${rowTemplateId}`
  ) as HTMLTemplateElement;

  return template.content.querySelector(`.${elementClass}`);
}

function forceSelectDefaultValue(element: Element): void {
  element.setAttribute(
    'value',
    element.firstElementChild.getAttribute('value')
  );
}

function fillLanguagesList(languages: Language[]): void {
  const settingsLangSelect = document.getElementById('languageSelect');
  const messageLangSelect = getElementInTableRowTemplate(
    'messageRow',
    'language'
  );

  if (settingsLangSelect.childElementCount > 0) {
    // Languages are read-only while LOOT is running, so don't re-fill the lists.
    return;
  }

  languages.forEach(language => {
    const settingsItem = createLanguageItem(language);
    settingsLangSelect.appendChild(settingsItem);
    messageLangSelect.appendChild(settingsItem.cloneNode(true));
  });

  forceSelectDefaultValue(messageLangSelect);
}

export * from './createItem';

export function show(elementId: string, showElement = true): void {
  document.getElementById(elementId).hidden = !showElement;
}

export function enable(
  elementOrId: Element | string,
  enableElement = true
): void {
  let element = elementOrId;
  if (typeof element === 'string') {
    element = document.getElementById(element);
  }

  if (enableElement) {
    element.removeAttribute('disabled');
  } else {
    element.setAttribute('disabled', '');
  }
}

export function openDialog(dialogElementId: string): void {
  (document.getElementById(dialogElementId) as PaperDialogElement).open();
}

export function initialiseVirtualLists(plugins: Plugin[]): void {
  (document.getElementById('cardsNav') as IronListElement).items = plugins;
  (document.getElementById(
    'pluginCardList'
  ) as IronListElement).items = plugins;
}

export function updateSelectedGame(gameFolder: string): void {
  (document.getElementById('gameMenu') as LootDropdownMenu).value = gameFolder;

  /* Also disable deletion of the game's row in the settings dialog. */
  const table = document.getElementById('gameTable') as EditableTable;
  for (let i = 0; i < table.querySelector('tbody').rows.length; i += 1) {
    const folderElements = table
      .querySelector('tbody')
      .rows[i].getElementsByClassName('folder');

    if (folderElements.length === 1) {
      table.setReadOnly(
        table.querySelector('tbody').rows[i],
        ['delete'],
        (folderElements[0] as PaperInputElement).value === gameFolder
      );
    }
  }
}

export function updateEnabledGames(installedGames: string[]): void {
  const gameMenuItems = document.getElementById('gameMenu').children;
  for (let i = 0; i < gameMenuItems.length; i += 1) {
    enable(
      gameMenuItems[i],
      installedGames.indexOf(gameMenuItems[i].getAttribute('value')) !== -1
    );
  }
}

export function setGameMenuItems(games: Game[]): void {
  const gameMenu = document.getElementById('gameMenu');

  /* First make sure game listing elements don't have any existing entries. */
  while (gameMenu.firstElementChild) {
    gameMenu.removeChild(gameMenu.firstElementChild);
  }

  games.forEach(game => {
    gameMenu.appendChild(createGameItem(game));
  });
}

export function updateSettingsDialog(settings: LootSettings): void {
  const gameSelect = document.getElementById(
    'defaultGameSelect'
  ) as LootDropdownMenu;

  const languageSelect = document.getElementById(
    'languageSelect'
  ) as LootDropdownMenu;

  const gameTable = document.getElementById('gameTable') as EditableTable;

  /* First make sure game listing elements don't have any existing entries. */
  while (gameSelect.children.length > 1) {
    gameSelect.removeChild(gameSelect.lastElementChild);
  }
  gameTable.clear();

  /* Now fill with new values. */
  settings.games.forEach(game => {
    gameSelect.appendChild(createGameItem(game));

    const row = gameTable.addRow(game);
    gameTable.setReadOnly(row, ['name', 'folder', 'type']);
  });

  fillLanguagesList(settings.languages);

  gameSelect.value = settings.game;
  languageSelect.value = settings.language;

  (document.getElementById(
    'enableDebugLogging'
  ) as PaperToggleButtonElement).checked = settings.enableDebugLogging;

  (document.getElementById(
    'updateMasterlist'
  ) as PaperToggleButtonElement).checked = settings.updateMasterlist;

  (document.getElementById(
    'enableLootUpdateCheck'
  ) as PaperToggleButtonElement).checked = settings.enableLootUpdateCheck;
}

export function fillGameTypesList(gameTypes: string[]): void {
  const select = getElementInTableRowTemplate('gameRow', 'type');

  gameTypes.forEach(gameType => {
    select.appendChild(createGameTypeItem(gameType));
  });

  forceSelectDefaultValue(select);
  select.setAttribute('value', select.firstElementChild.getAttribute('value'));
}

export function fillGroupsList(groups: Group[]): void {
  const groupsSelect = document
    .getElementById('editor')
    .shadowRoot.querySelector('#group');

  while (groupsSelect.firstElementChild) {
    groupsSelect.removeChild(groupsSelect.firstElementChild);
  }

  groups.forEach(group => {
    groupsSelect.appendChild(createGroupItem(group));
  });

  if (groups.length > 0) {
    forceSelectDefaultValue(groupsSelect);
  }
}

export function initialiseGroupsEditor(
  getter: (groupName: string) => string[]
): void {
  const editor = document.getElementById('groupsEditor') as LootGroupsEditor;
  editor.setGroupPluginNamesGetter(getter);
}

export function updateGroupsEditorState(groups: SourcedGroup[]): void {
  const editor = document.getElementById('groupsEditor') as LootGroupsEditor;
  editor.setGroups(groups);
}

export function appendGeneralMessages(messages: Message[]): void {
  if (!messages) {
    return;
  }

  const generalMessagesList = document
    .getElementById('summary')
    .getElementsByTagName('ul')[0];
  messages.forEach(message => {
    generalMessagesList.appendChild(
      createMessageItem(message.type, message.content)
    );
  });
}

export function listInitErrors(errorMessages: string[]): void {
  if (!errorMessages) {
    return;
  }

  appendGeneralMessages(
    errorMessages.map(element => ({
      type: 'error',
      content: element
    }))
  );

  ['filterTotalMessageNo', 'totalMessageNo', 'totalErrorNo'].forEach(id => {
    document.getElementById(id).textContent = errorMessages.length.toString();
  });
}

export function onJumpToGeneralInfo(): void {
  document.getElementById('pluginCardList').scroll(0, 0);
}

export function onShowAboutDialog(): void {
  (document.getElementById('about') as PaperDialogElement).open();
}

interface IronSelectEvent extends CustomEvent {
  target: EventTarget & IronSelectableBehavior;
}

export function onSwitchSidebarTab(evt: IronSelectEvent): void {
  if (typeof evt.target.selected === 'string') {
    const selector = document.getElementById(evt.target.selected)
      .parentElement as HTMLElement & IronSelectableBehavior;

    selector.selected = evt.target.selected;
  }
}

export function onSidebarClick(evt: MouseEvent): void {
  if (
    evt.target instanceof HTMLElement &&
    evt.target.hasAttribute('data-index')
  ) {
    const index = parseInt(evt.target.getAttribute('data-index'), 10);

    const list = document.getElementById('pluginCardList') as IronListElement;
    list.scrollToIndex(index);

    if (evt.type === 'dblclick') {
      /* Double-clicking can select the item's text, clear the selection in
          case that has happened. */
      window.getSelection().removeAllRanges();

      if (
        document.body.getAttribute('data-state') !== 'editing' &&
        document.body.getAttribute('data-state') !== 'sorting'
      ) {
        const pluginCard = document.getElementById(
          evt.target.getAttribute('data-id')
        ) as LootPluginCard;

        pluginCard.onShowEditor();
      }
    }
  }
}

export function onShowSettingsDialog(): void {
  (document.getElementById('settingsDialog') as PaperDialogElement).open();
}

export function onOpenGroupsEditor(): void {
  (document.getElementById('groupsEditorDialog') as PaperDialogElement).open();
}
export function onGroupsEditorOpened(): void {
  (document.getElementById('groupsEditor') as LootGroupsEditor).render();
}

export function onFocusSearch(evt: KeyboardEvent): void {
  // Focus search if ctrl-f is pressed.
  if (evt.ctrlKey && evt.keyCode === 70) {
    document.getElementById('mainToolbar').classList.add('search');
    (document.getElementById('searchBar') as LootSearchToolbar).focusInput();
  }
}

export function onSearchOpen(): void {
  document.getElementById('mainToolbar').classList.add('search');
  (document.getElementById('searchBar') as LootSearchToolbar).focusInput();
}

interface LootSearchChangeSelectionEvent extends CustomEvent {
  detail: { selection: number };
}

export function onSearchChangeSelection(
  evt: LootSearchChangeSelectionEvent
): void {
  const list = document.getElementById('pluginCardList') as IronListElement;
  list.scrollToIndex(evt.detail.selection);
}

export function initialiseAutocompleteFilenames(filenames: string[]): void {
  getElementInTableRowTemplate('fileRow', 'name').setAttribute(
    'source',
    JSON.stringify(filenames)
  );
}

export function initialiseAutocompleteBashTags(tags: string[]): void {
  getElementInTableRowTemplate('tagRow', 'name').setAttribute(
    'source',
    JSON.stringify(tags)
  );
}

export function setUIState(state: string): void {
  document.body.setAttribute('data-state', state);
}

export function enableGameOperations(shouldEnable: boolean): void {
  (document.getElementById(
    'sortButton'
  ) as PaperIconButtonElement).disabled = !shouldEnable;

  (document.getElementById(
    'updateMasterlistButton'
  ) as PaperIconButtonElement).disabled = !shouldEnable;

  (document.getElementById(
    'wipeUserlistButton'
  ) as PaperIconItemElement).disabled = !shouldEnable;

  (document.getElementById(
    'copyLoadOrderButton'
  ) as PaperIconItemElement).disabled = !shouldEnable;

  (document.getElementById(
    'refreshContentButton'
  ) as PaperIconItemElement).disabled = !shouldEnable;
}

export function setDocumentFontFamily(fontFamily: string): void {
  document.documentElement.style.setProperty(
    '--loot-font-family',
    `${fontFamily}, ${getComputedStyle(document.documentElement).fontFamily}`
  );
}
