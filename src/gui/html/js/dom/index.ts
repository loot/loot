import { PaperDialogElement } from '@polymer/paper-dialog';
import { IronListElement } from '@polymer/iron-list';
import { IronSelectableBehavior } from '@polymer/iron-selector/iron-selectable';
import { PaperIconButtonElement } from '@polymer/paper-icon-button';
import { PaperIconItemElement } from '@polymer/paper-item/paper-icon-item';
import { PaperToggleButtonElement } from '@polymer/paper-toggle-button';
import { PaperListboxElement } from '@polymer/paper-listbox';
import { PaperButtonElement } from '@polymer/paper-button';
import { PaperInputElement } from '@polymer/paper-input/paper-input';
import LootPluginCard from '../../elements/loot-plugin-card';
import LootGroupsEditor from '../../elements/loot-groups-editor';
import LootSearchToolbar from '../../elements/loot-search-toolbar';
import LootDropdownMenu from '../../elements/loot-dropdown-menu';
import {
  Game,
  Group,
  Language,
  LootSettings,
  SourcedGroup
} from '../interfaces';
import {
  createGameItem,
  createGameTypeItem,
  createGroupItem,
  createLanguageItem,
  createMessageItem,
  createThemeItem
} from './createItem';
import { Plugin } from '../plugin';
import { getElementById, getAttribute, querySelector } from './helpers';

interface Message {
  type: string;
  content: string;
}

interface IronSelectEvent extends CustomEvent {
  target: EventTarget & IronSelectableBehavior;
}

function isIronSelectEvent(evt: Event): evt is IronSelectEvent {
  return evt.target !== null && 'selected' in evt.target;
}

interface LootSearchChangeSelectionEvent extends CustomEvent {
  detail: { selection: number };
}

function isLootSearchChangeSelectionEvent(
  evt: Event
): evt is LootSearchChangeSelectionEvent {
  return evt instanceof CustomEvent && typeof evt.detail.selection === 'number';
}

function getElementInTableRowTemplate(
  rowTemplateId: string,
  elementClass: string
): Element {
  const template = querySelector(
    document,
    `#${rowTemplateId}`
  ) as HTMLTemplateElement;

  const element = querySelector(template.content, `.${elementClass}`);

  return element;
}

function forceSelectDefaultValue(element: Element): void {
  if (element.firstElementChild === null) {
    throw new Error('Expected element to have at least one child element');
  }

  const value = element.firstElementChild.getAttribute('value');
  if (value === null) {
    throw new Error(
      "Expected element's first element child to have a 'value' attribute"
    );
  }

  element.setAttribute('value', value);
}

function fillLanguagesList(languages: Language[]): void {
  const settingsLangSelect = getElementById('languageSelect');
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
  getElementById(elementId).hidden = !showElement;
}

export function enable(
  elementOrId: Element | string,
  enableElement = true
): void {
  let element = elementOrId;
  if (typeof element === 'string') {
    element = getElementById(element);
  }

  if (enableElement) {
    element.removeAttribute('disabled');
  } else {
    element.setAttribute('disabled', '');
  }
}

export function openDialog(dialogElementId: string): void {
  (getElementById(dialogElementId) as PaperDialogElement).open();
}

export function initialiseVirtualLists(plugins: Plugin[]): void {
  (getElementById('cardsNav') as IronListElement).items = plugins;
  (getElementById('pluginCardList') as IronListElement).items = plugins;
}

export function updateSelectedGame(gameFolder: string): void {
  const gameMenu = getElementById('gameMenu') as LootDropdownMenu;

  // Check first because assigning the same value still fires off an iron-select
  // event.
  if (gameMenu.value !== gameFolder) {
    gameMenu.value = gameFolder;
  }
}

export function setGameMenuItems(
  games: Game[],
  installedGames: string[]
): void {
  const gameMenu = getElementById('gameMenu');

  /* First make sure game listing elements don't have any existing entries. */
  while (gameMenu.firstElementChild) {
    gameMenu.removeChild(gameMenu.firstElementChild);
  }

  games.forEach(game => {
    const gameItem = gameMenu.appendChild(createGameItem(game));
    enable(
      gameItem,
      installedGames.indexOf(getAttribute(gameItem, 'value')) !== -1
    );
  });
}

export function initialiseSettingsDialog(
  settings: LootSettings,
  themes: string[]
): void {
  const themeSelect = getElementById('themeSelect') as LootDropdownMenu;

  if (themeSelect.childElementCount === 1) {
    themes.forEach(theme => themeSelect.appendChild(createThemeItem(theme)));
  }

  fillLanguagesList(settings.languages);
}

export function updateSettingsDialog(settings: LootSettings): void {
  const gameSelect = getElementById('defaultGameSelect') as LootDropdownMenu;
  const languageSelect = getElementById('languageSelect') as LootDropdownMenu;
  const themeSelect = getElementById('themeSelect') as LootDropdownMenu;
  const gamesList = getElementById('settingsSidebarList');

  /* First make sure game listing elements don't have any existing entries.
     Leave the autodetect entry in the game select dropdown. */
  while (gameSelect.children.length > 1) {
    gameSelect.removeChild(gameSelect.children[1]);
  }
  /* Leave the empty hidden child in the list of game with settings. */
  while (gamesList.children.length > 2) {
    gamesList.removeChild(gamesList.children[2]);
  }

  /* Now fill with new values. */
  settings.games.forEach(game => {
    gameSelect.appendChild(createGameItem(game));
    gamesList.appendChild(createGameItem(game));
  });

  gameSelect.value = settings.game;
  languageSelect.value = settings.language;
  themeSelect.value = settings.theme;

  (getElementById('enableDebugLogging') as PaperToggleButtonElement).checked =
    settings.enableDebugLogging;

  (getElementById('updateMasterlist') as PaperToggleButtonElement).checked =
    settings.updateMasterlist;

  (getElementById('settingsPreludeURL') as PaperInputElement).value =
    settings.preludeURL;

  (getElementById('settingsPreludeBranch') as PaperInputElement).value =
    settings.preludeBranch;
}

export function fillGameTypesList(gameTypes: string[]): void {
  const select = getElementById('settingsGameTypeDropdown');

  gameTypes.forEach(gameType => {
    select.appendChild(createGameTypeItem(gameType));
  });

  forceSelectDefaultValue(select);
}

export function fillGroupsLists(groups: Group[]): void {
  const shadowRoot = getElementById('editor').shadowRoot;
  if (shadowRoot === null) {
    throw new Error('Expected element with ID "editor" to have a shadow root');
  }

  const groupsSelect = querySelector(shadowRoot, '#group');
  const groupsFilter = getElementById('groupsFilter');

  while (groupsSelect.firstElementChild) {
    groupsSelect.removeChild(groupsSelect.firstElementChild);
  }

  while (groupsFilter.children.length > 1) {
    groupsFilter.removeChild(groupsFilter.children[1]);
  }

  groups.forEach(group => {
    groupsSelect.appendChild(createGroupItem(group));
    groupsFilter.appendChild(createGroupItem(group));
  });

  if (groups.length > 0) {
    forceSelectDefaultValue(groupsSelect);
  }
}

export function initialiseGroupsEditor(
  getter: (groupName: string) => string[]
): void {
  const editor = getElementById('groupsEditor') as LootGroupsEditor;
  editor.setGroupPluginNamesGetter(getter);
}

export function updateGroupsEditorState(groups: SourcedGroup[]): void {
  const editor = getElementById('groupsEditor') as LootGroupsEditor;
  editor.setGroups(groups);
}

export function appendGeneralMessages(messages: Message[]): void {
  if (!messages) {
    return;
  }

  const generalMessagesList =
    getElementById('summary').getElementsByTagName('ul')[0];
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
    getElementById(id).textContent = errorMessages.length.toString();
  });
}

export function onJumpToGeneralInfo(): void {
  getElementById('pluginCardList').scroll(0, 0);
}

export function onShowAboutDialog(): void {
  (getElementById('about') as PaperDialogElement).open();
}

export function onSwitchTab(evt: Event): void {
  if (!isIronSelectEvent(evt)) {
    throw new TypeError(`Expected a IronSelectEvent, got ${evt.type}`);
  }

  if (typeof evt.target.selected === 'string') {
    const selector = getElementById(evt.target.selected)
      .parentElement as HTMLElement & IronSelectableBehavior;

    selector.selected = evt.target.selected;
  }
}

export function onSidebarClick(evt: MouseEvent): void {
  if (
    evt.target instanceof HTMLElement &&
    evt.target.hasAttribute('data-index')
  ) {
    const index = parseInt(getAttribute(evt.target, 'data-index'), 10);

    const list = getElementById('pluginCardList') as IronListElement;
    list.scrollToIndex(index);

    if (evt.type === 'dblclick') {
      /* Double-clicking can select the item's text, clear the selection in
          case that has happened. */
      const selection = window.getSelection();
      if (selection !== null) {
        selection.removeAllRanges();
      }

      if (
        document.body.getAttribute('data-state') !== 'editing' &&
        document.body.getAttribute('data-state') !== 'sorting'
      ) {
        const pluginCard = getElementById(
          getAttribute(evt.target, 'data-id')
        ) as LootPluginCard;

        pluginCard.onShowEditor();
      }
    }
  }
}

export function clearGameSettingsInputs(): void {
  forceSelectDefaultValue(getElementById('settingsGameTypeDropdown'));

  const inputElementIds = [
    'settingsGameName',
    'settingsGameFolder',
    'settingsGameMaster',
    'settingsGameMasterlistUrl',
    'settingsGameMasterlistBranch',
    'settingsGamePath',
    'settingsGameRegistry',
    'settingsGameLocalPath'
  ];

  for (const inputId of inputElementIds) {
    const input = getElementById(inputId) as PaperInputElement;
    input.value = '';
    input.disabled = true;
  }

  (getElementById('deleteGameButton') as PaperButtonElement).disabled = true;
}

export function initialiseGameSettingsInputsForNewGame(): void {
  const gameTypeDropdown = getElementById(
    'settingsGameTypeDropdown'
  ) as LootDropdownMenu;
  forceSelectDefaultValue(gameTypeDropdown);
  gameTypeDropdown.disabled = false;

  const inputElementIds = [
    'settingsGameName',
    'settingsGameFolder',
    'settingsGameMaster',
    'settingsGameMasterlistUrl',
    'settingsGameMasterlistBranch',
    'settingsGamePath',
    'settingsGameRegistry',
    'settingsGameLocalPath'
  ];

  for (const inputId of inputElementIds) {
    const input = getElementById(inputId) as PaperInputElement;
    input.value = '';
    input.disabled = false;
  }

  (getElementById('deleteGameButton') as PaperButtonElement).disabled = false;
}

export function initialiseGameSettingsUI(): void {
  const gamesList = getElementById(
    'settingsSidebarList'
  ) as PaperListboxElement;

  // Initially select the General sidebar item.
  gamesList.select('');
}

export function onShowSettingsDialog(): void {
  const gamesSettings = window.loot.settings?.games ?? [];
  window.loot.unappliedGamesSettings = new Map(
    gamesSettings.map(settings => [settings.folder, settings])
  );

  initialiseGameSettingsUI();
  (getElementById('settingsDialog') as PaperDialogElement).open();
}

export function onOpenGroupsEditor(): void {
  (getElementById('groupsEditorDialog') as PaperDialogElement).open();
}
export function onGroupsEditorOpened(): void {
  (getElementById('groupsEditor') as LootGroupsEditor).render();
}

export function onFocusSearch(evt: KeyboardEvent): void {
  // Focus search if ctrl-f is pressed.
  if (evt.ctrlKey && evt.keyCode === 70) {
    getElementById('mainToolbar').classList.add('search');
    (getElementById('searchBar') as LootSearchToolbar).focusInput();
  }
}

export function onSearchOpen(): void {
  getElementById('mainToolbar').classList.add('search');
  (getElementById('searchBar') as LootSearchToolbar).focusInput();
}

export function onSearchChangeSelection(evt: Event): void {
  if (!isLootSearchChangeSelectionEvent(evt)) {
    throw new TypeError(
      `Expected a LootSearchChangeSelectionEvent, got ${evt.type}`
    );
  }

  const list = getElementById('pluginCardList') as IronListElement;
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
  (getElementById('sortButton') as PaperIconButtonElement).disabled =
    !shouldEnable;

  (
    getElementById('updateMasterlistButton') as PaperIconButtonElement
  ).disabled = !shouldEnable;

  (getElementById('groupsEditorButton') as PaperIconItemElement).disabled =
    !shouldEnable;

  (getElementById('wipeUserlistButton') as PaperIconItemElement).disabled =
    !shouldEnable;

  (getElementById('copyLoadOrderButton') as PaperIconItemElement).disabled =
    !shouldEnable;

  (getElementById('refreshContentButton') as PaperIconItemElement).disabled =
    !shouldEnable;
}

export function setDocumentFontFamily(fontFamily: string): void {
  document.documentElement.style.setProperty(
    '--loot-font-family',
    `${fontFamily}, ${getComputedStyle(document.documentElement).fontFamily}`
  );
}
