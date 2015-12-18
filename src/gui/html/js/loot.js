var loot = {
    installedGames: [],
    settings: {},

    /* Call whenever installedGames is changed or game menu is rewritten. */
    updateEnabledGames: function() {
        /* Update the disabled games in the game menu. */
        var gameMenuItems = document.getElementById('gameMenu').children;
        for (var i = 0; i < gameMenuItems.length; ++i) {
            if (this.installedGames.indexOf(gameMenuItems[i].getAttribute('value')) == -1) {
                gameMenuItems[i].setAttribute('disabled', true);
                gameMenuItems[i].removeEventListener('click', onChangeGame, false);
            } else {
                gameMenuItems[i].removeAttribute('disabled');
                gameMenuItems[i].addEventListener('click', onChangeGame, false);
            }
        }
    },

    /* Call whenever settings are changed. */
    updateSettingsUI: function() {
        var gameSelect = document.getElementById('defaultGameSelect');
        var gameMenu = document.getElementById('gameMenu');
        var gameTable = document.getElementById('gameTable');

        /* First make sure game listing elements don't have any existing entries. */
        while (gameSelect.children.length > 1) {
            gameSelect.removeChild(gameSelect.lastElementChild);
        }
        while (gameMenu.firstElementChild) {
            gameMenu.firstElementChild.removeEventListener('click', onChangeGame, false);
            gameMenu.removeChild(gameMenu.firstElementChild);
        }
        gameTable.clear();

        /* Now fill with new values. */
        this.settings.games.forEach(function(game){
            var menuItem = document.createElement('paper-item');
            menuItem.setAttribute('value', game.folder);
            menuItem.setAttribute('noink', '');
            menuItem.textContent = game.name;
            gameMenu.appendChild(menuItem);
            gameSelect.appendChild(menuItem.cloneNode(true));

            var row = gameTable.addRow(game);
            gameTable.setReadOnly(row, ['name','folder','type']);
        });

        gameSelect.value = this.settings.game;
        document.getElementById('languageSelect').value = this.settings.language;
        document.getElementById('enableDebugLogging').checked = this.settings.enableDebugLogging;
        document.getElementById('updateMasterlist').checked = this.settings.updateMasterlist;

        this.updateEnabledGames();
        updateSelectedGame(this.game.folder);
    },

    /* Observer for loot members. */
    observer: function(changes) {
        changes.forEach(function(change){
            if (change.name == 'installedGames') {
                change.object.updateEnabledGames();
            } else if (change.name == 'settings') {
                change.object.updateSettingsUI();
            }
        });
    },

    /* Returns a cefQuery as a Promise. */
    query: function(request) {
        return new Promise(function(resolve, reject) {
            window.cefQuery({
                request: request,
                persistent: false,
                onSuccess: resolve,
                onFailure: function(errorCode, errorMessage) {
                    reject(Error('Error code: ' + errorCode + '; ' + errorMessage))
                }
            });
        });
    }
};
