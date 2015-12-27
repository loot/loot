'use strict';
function showElement(element) {
  if (element !== null) {
    element.classList.toggle('hidden', false);
  }
}
function hideElement(element) {
  if (element !== null) {
    element.classList.toggle('hidden', true);
  }
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
  const gameMenuItems = document.getElementById('gameMenu').children;
  for (let i = 0; i < gameMenuItems.length; ++i) {
    if (installedGames.indexOf(gameMenuItems[i].getAttribute('value')) === -1) {
      gameMenuItems[i].setAttribute('disabled', true);
      gameMenuItems[i].removeEventListener('click', onChangeGame);
    } else {
      gameMenuItems[i].removeAttribute('disabled');
      gameMenuItems[i].addEventListener('click', onChangeGame);
    }
  }
}
/* Call whenever settings are changed. */
function updateSettingsDialog(settings, installedGames, gameFolder) {
  const gameSelect = document.getElementById('defaultGameSelect');
  const gameMenu = document.getElementById('gameMenu');
  const gameTable = document.getElementById('gameTable');

  /* First make sure game listing elements don't have any existing entries. */
  while (gameSelect.children.length > 1) {
    gameSelect.removeChild(gameSelect.lastElementChild);
  }
  while (gameMenu.firstElementChild) {
    gameMenu.firstElementChild.removeEventListener('click', onChangeGame);
    gameMenu.removeChild(gameMenu.firstElementChild);
  }
  gameTable.clear();

  /* Now fill with new values. */
  settings.games.forEach((game) => {
    const menuItem = document.createElement('paper-item');
    menuItem.setAttribute('value', game.folder);
    menuItem.setAttribute('noink', '');
    menuItem.textContent = game.name;
    gameMenu.appendChild(menuItem);
    gameSelect.appendChild(menuItem.cloneNode(true));

    const row = gameTable.addRow(game);
    gameTable.setReadOnly(row, ['name', 'folder', 'type']);
  });

  gameSelect.value = settings.game;
  document.getElementById('languageSelect').value = settings.language;
  document.getElementById('enableDebugLogging').checked = settings.enableDebugLogging;
  document.getElementById('updateMasterlist').checked = settings.updateMasterlist;

  updateEnabledGames(installedGames);
  updateSelectedGame(gameFolder);
}
