import marked from 'marked/marked.min';

function getElementInTableRowTemplate(rowTemplateId, elementClass) {
  const select = document.querySelector(
    'link[rel="import"][href$="editable-table-rows.html"]'
  );
  if (select) {
    return select.import
      .querySelector(`#${rowTemplateId}`)
      .content.querySelector(`.${elementClass}`);
  }
  return document
    .querySelector(`#${rowTemplateId}`)
    .content.querySelector(`.${elementClass}`);
}

function createGameItem(game) {
  const menuItem = document.createElement('paper-item');
  menuItem.setAttribute('value', game.folder);
  menuItem.textContent = game.name;

  return menuItem;
}

function createLanguageItem(language) {
  const item = document.createElement('paper-item');
  item.setAttribute('value', language.locale);
  item.textContent = language.name;

  return item;
}

export function createMessageItem(type, content) {
  const li = document.createElement('li');
  li.className = type;
  /* Use the Marked library for Markdown formatting support. */
  li.innerHTML = marked(content);

  return li;
}

function createGameTypeItem(gameType) {
  const item = document.createElement('paper-item');
  item.setAttribute('value', gameType);
  item.textContent = gameType;

  return item;
}

function createGroupItem(group) {
  const item = document.createElement('paper-item');
  item.setAttribute('value', group.name);
  item.textContent = group.name;

  return item;
}

function forceSelectDefaultValue(element) {
  element.setAttribute(
    'value',
    element.firstElementChild.getAttribute('value')
  );
}

export function show(elementId, showElement = true) {
  document.getElementById(elementId).hidden = !showElement;
}

export function enable(elementOrId, enableElement = true) {
  let element = elementOrId;
  if (typeof element === 'string' || element instanceof String) {
    element = document.getElementById(element);
  }

  if (enableElement) {
    element.removeAttribute('disabled');
  } else {
    element.setAttribute('disabled', '');
  }
}

export function openDialog(dialogElementId) {
  document.getElementById(dialogElementId).open();
}

export function initialiseVirtualLists(plugins) {
  document.getElementById('cardsNav').items = plugins;
  document.getElementById('pluginCardList').items = plugins;
}

export function updateSelectedGame(gameFolder) {
  document.getElementById('gameMenu').value = gameFolder;

  /* Also disable deletion of the game's row in the settings dialog. */
  const table = document.getElementById('gameTable');
  for (let i = 0; i < table.querySelector('tbody').rows.length; i += 1) {
    const folderElements = table
      .querySelector('tbody')
      .rows[i].getElementsByClassName('folder');
    if (folderElements.length === 1) {
      table.setReadOnly(
        table.querySelector('tbody').rows[i],
        ['delete'],
        folderElements[0].value === gameFolder
      );
    }
  }
}

export function updateEnabledGames(installedGames) {
  const gameMenuItems = document.getElementById('gameMenu').children;
  for (let i = 0; i < gameMenuItems.length; i += 1) {
    enable(
      gameMenuItems[i],
      installedGames.indexOf(gameMenuItems[i].getAttribute('value')) !== -1
    );
  }
}

export function setGameMenuItems(games) {
  const gameMenu = document.getElementById('gameMenu');

  /* First make sure game listing elements don't have any existing entries. */
  while (gameMenu.firstElementChild) {
    gameMenu.removeChild(gameMenu.firstElementChild);
  }

  games.forEach(game => {
    gameMenu.appendChild(createGameItem(game));
  });
}

export function updateSettingsDialog(settings) {
  const gameSelect = document.getElementById('defaultGameSelect');
  const gameTable = document.getElementById('gameTable');

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

  gameSelect.value = settings.game;
  document.getElementById('languageSelect').value = settings.language;
  document.getElementById('enableDebugLogging').checked =
    settings.enableDebugLogging;
  document.getElementById('updateMasterlist').checked =
    settings.updateMasterlist;
  document.getElementById('enableLootUpdateCheck').checked =
    settings.enableLootUpdateCheck;
}

export function fillGameTypesList(gameTypes) {
  const select = getElementInTableRowTemplate('gameRow', 'type');

  gameTypes.forEach(gameType => {
    select.appendChild(createGameTypeItem(gameType));
  });

  forceSelectDefaultValue(select);
  select.setAttribute('value', select.firstElementChild.getAttribute('value'));
}

export function fillGroupsList(groups) {
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

export function fillLanguagesList(languages) {
  const settingsLangSelect = document.getElementById('languageSelect');
  const messageLangSelect = getElementInTableRowTemplate(
    'messageRow',
    'language'
  );

  languages.forEach(language => {
    const settingsItem = createLanguageItem(language);
    settingsLangSelect.appendChild(settingsItem);
    messageLangSelect.appendChild(settingsItem.cloneNode(true));
  });

  forceSelectDefaultValue(messageLangSelect);
}

export function initialiseGroupsEditor(getter) {
  document.getElementById('groupsEditor').setGroupPluginNamesGetter(getter);
}

export function updateGroupsEditorState(groups) {
  document.getElementById('groupsEditor').setGroups(groups);
}

export function appendGeneralMessages(messages) {
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

export function listInitErrors(errorMessages) {
  if (!errorMessages) {
    return;
  }

  appendGeneralMessages(
    errorMessages.map(element => ({
      type: 'error',
      content: element
    }))
  );

  document.getElementById('filterTotalMessageNo').textContent =
    errorMessages.length;
  document.getElementById('totalMessageNo').textContent = errorMessages.length;
  document.getElementById('totalErrorNo').textContent = errorMessages.length;
}

export function onJumpToGeneralInfo() {
  document.getElementById('pluginCardList').scroll(0, 0);
}

export function onShowAboutDialog() {
  document.getElementById('about').open();
}

export function onSwitchSidebarTab(evt) {
  document.getElementById(evt.target.selected).parentElement.selected =
    evt.target.selected;
}

export function onSidebarClick(evt) {
  if (evt.target.hasAttribute('data-index')) {
    const index = parseInt(evt.target.getAttribute('data-index'), 10);
    document.getElementById('pluginCardList').scrollToIndex(index);

    if (evt.type === 'dblclick') {
      /* Double-clicking can select the item's text, clear the selection in
          case that has happened. */
      window.getSelection().removeAllRanges();

      if (
        document.body.getAttribute('data-state') !== 'editing' &&
        document.body.getAttribute('data-state') !== 'sorting'
      ) {
        document
          .getElementById(evt.target.getAttribute('data-id'))
          .onShowEditor();
      }
    }
  }
}

export function onShowSettingsDialog() {
  document.getElementById('settingsDialog').open();
}

export function onOpenGroupsEditor() {
  document.getElementById('groupsEditorDialog').open();
}
export function onGroupsEditorOpened() {
  document.getElementById('groupsEditor').render();
}

export function onFocusSearch(evt) {
  // Focus search if ctrl-f is pressed.
  if (evt.ctrlKey && evt.keyCode === 70) {
    document.getElementById('mainToolbar').classList.add('search');
    document.getElementById('searchBar').focusInput();
  }
}

export function onSearchOpen() {
  document.getElementById('mainToolbar').classList.add('search');
  document.getElementById('searchBar').focusInput();
}

export function onSearchChangeSelection(evt) {
  document.getElementById('pluginCardList').scrollToIndex(evt.detail.selection);
}

export function initialiseAutocompleteFilenames(filenames) {
  getElementInTableRowTemplate('fileRow', 'name').setAttribute(
    'source',
    JSON.stringify(filenames)
  );
}

export function initialiseAutocompleteBashTags(tags) {
  getElementInTableRowTemplate('tagRow', 'name').setAttribute(
    'source',
    JSON.stringify(tags)
  );
}

export function setUIState(state) {
  document.body.setAttribute('data-state', state);
}

export function enableGameOperations(shouldEnable) {
  document.getElementById('sortButton').disabled = !shouldEnable;
  document.getElementById('updateMasterlistButton').disabled = !shouldEnable;
  document.getElementById('wipeUserlistButton').disabled = !shouldEnable;
  document.getElementById('copyLoadOrderButton').disabled = !shouldEnable;
  document.getElementById('refreshContentButton').disabled = !shouldEnable;
}
