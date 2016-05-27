'use strict';
(function exportModule(root, factory) {
  if (typeof define === 'function' && define.amd) {
    // AMD. Register as an anonymous module.
    define([], factory);
  } else {
    // Browser globals
    root.loot = root.loot || {};
    root.loot.dom = factory();
  }
}(this, () => {
  return {
    getElementInTableRowTemplate(rowTemplateId, elementClass) {
      const select = document.querySelector('link[rel="import"][href$="editable-table.html"]');
      if (select) {
        return select.import.querySelector(`#${rowTemplateId}`).content.querySelector(`.${elementClass}`);
      }
      return document.querySelector(`#${rowTemplateId}`).content.querySelector(`.${elementClass}`);
    },

    show(elementId, showElement = true) {
      document.getElementById(elementId).hidden = !showElement;
    },

    enable(elementId, enableElement = true) {
      if (enableElement) {
        document.getElementById(elementId).removeAttribute('disabled');
      } else {
        document.getElementById(elementId).setAttribute('disabled', '');
      }
    },

    updateSelectedGame(gameFolder) {
      document.getElementById('gameMenu').value = gameFolder;

      /* Also disable deletion of the game's row in the settings dialog. */
      const table = document.getElementById('gameTable');
      for (let i = 0; i < table.tBodies[0].rows.length; ++i) {
        const folderElements = table.tBodies[0].rows[i].getElementsByClassName('folder');
        if (folderElements.length === 1) {
          table.setReadOnly(table.tBodies[0].rows[i],
                            ['delete'],
                            folderElements[0].value === gameFolder);
        }
      }
    },

    updateEnabledGames(installedGames) {
      const gameMenuItems = document.getElementById('gameMenu').children;
      for (let i = 0; i < gameMenuItems.length; ++i) {
        if (installedGames.indexOf(gameMenuItems[i].getAttribute('value')) === -1) {
          gameMenuItems[i].setAttribute('disabled', true);
        } else {
          gameMenuItems[i].removeAttribute('disabled');
        }
      }
    },

    createGameItem(game) {
      const menuItem = document.createElement('paper-item');
      menuItem.setAttribute('value', game.folder);
      menuItem.textContent = game.name;

      return menuItem;
    },

    setGameMenuItems(games) {
      const gameMenu = document.getElementById('gameMenu');

      /* First make sure game listing elements don't have any existing entries. */
      while (gameMenu.firstElementChild) {
        gameMenu.removeChild(gameMenu.firstElementChild);
      }

      games.forEach((game) => {
        gameMenu.appendChild(this.createGameItem(game));
      });
    },

    updateSettingsDialog(settings) {
      const gameSelect = document.getElementById('defaultGameSelect');
      const gameTable = document.getElementById('gameTable');

      /* First make sure game listing elements don't have any existing entries. */
      while (gameSelect.children.length > 1) {
        gameSelect.removeChild(gameSelect.lastElementChild);
      }
      gameTable.clear();

      /* Now fill with new values. */
      settings.games.forEach((game) => {
        gameSelect.appendChild(this.createGameItem(game));

        const row = gameTable.addRow(game);
        gameTable.setReadOnly(row, ['name', 'folder', 'type']);
      });

      gameSelect.value = settings.game;
      document.getElementById('languageSelect').value = settings.language;
      document.getElementById('enableDebugLogging').checked = settings.enableDebugLogging;
      document.getElementById('updateMasterlist').checked = settings.updateMasterlist;
    },
  };
}));
