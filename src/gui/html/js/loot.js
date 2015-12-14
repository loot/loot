var loot = {
    installedGames: [],
    settings: {},
    game: {
        folder: '',
        globalMessages: [],
        masterlist: {},
        plugins: [],

        /* Call whenever game is changed or game menu / game table are rewritten. */
        updateSelectedGame: function() {
            document.getElementById('gameMenu').value = this.folder;

            /* Also disable deletion of the game's row in the settings dialog. */
            var table = document.getElementById('gameTable');
            for (var i = 0; i < table.tBodies[0].rows.length; ++i) {
                if (table.tBodies[0].rows[i].getElementsByClassName('folder').length > 0) {
                    if (table.tBodies[0].rows[i].getElementsByClassName('folder')[0].value == this.folder) {
                        table.setReadOnly(table.tBodies[0].rows[i], ['delete']);
                    } else {
                        table.setReadOnly(table.tBodies[0].rows[i], ['delete'], false);
                    }
                }
            }
        },

        /* Call whenever game is changed. */
        updateRedatePluginsButtonState: function() {
            /* Also enable/disable the redate plugins option. */
            var index = undefined;
            for (var i = 0; i < loot.settings.games.length; ++i) {
                if (loot.settings.games[i].folder == this.folder) {
                    index = i;
                    break;
                }
            }
            var redateButton = document.getElementById('redatePluginsButton');
            if (index != undefined && loot.settings.games[index].type == 'Skyrim') {
                redateButton.removeAttribute('disabled');
            } else {
                redateButton.setAttribute('disabled', true);
            }
        },

        pluginsObserver: function(changes) {
            changes.forEach(function(change){
                /* Need to handle plugin addition and removal.
                   Update plugin and message counts. */
                var totalChange = 0;
                var warnChange = 0;
                var errorChange = 0;
                var activeChange = 0;
                var dirtyChange = 0;

                if (change.addedCount > 0) {
                    /* Addition */
                    for (var i = change.index; i < change.index + change.addedCount; ++i) {
                        if (change.object[i].isActive) {
                            ++activeChange;
                        }
                        if (change.object[i].isDirty) {
                            ++dirtyChange;
                        }
                        if (change.object[i].messages) {
                            totalChange += change.object[i].messages.length;
                            change.object[i].messages.forEach(function(message) {
                                if (message.type == 'warn') {
                                    ++warnChange;
                                } else if (message.type == 'error') {
                                    ++errorChange;
                                }
                            });
                        }
                    }
                }
                if (change.removed.length > 0) {
                    /* Removal */
                    change.removed.forEach(function(plugin){
                        if (plugin.isActive) {
                            --activeChange;
                        }
                        if (plugin.isDirty) {
                            --dirtyChange;
                        }
                        if (plugin.messages) {
                            totalChange -= plugin.messages.length;
                            plugin.messages.forEach(function(message) {
                                if (message.type == 'warn') {
                                    --warnChange;
                                } else if (message.type == 'error') {
                                    --errorChange;
                                }
                            });
                        }
                    });
                }
                document.getElementById('filterTotalMessageNo').textContent = parseInt(document.getElementById('filterTotalMessageNo').textContent, 10) + totalChange;
                document.getElementById('totalMessageNo').textContent = document.getElementById('filterTotalMessageNo').textContent;
                document.getElementById('totalWarningNo').textContent = parseInt(document.getElementById('totalWarningNo').textContent, 10) + warnReduction;
                document.getElementById('totalErrorNo').textContent = parseInt(document.getElementById('totalErrorNo').textContent, 10) + errorReduction;
                document.getElementById('filterTotalPluginNo').textContent = parseInt(document.getElementById('filterTotalPluginNo').textContent, 10) + change.addedCount - change.removed.length;
                document.getElementById('totalPluginNo').textContent = document.getElementById('filterTotalPluginNo').textContent;
                document.getElementById('activePluginNo').textContent = parseInt(document.getElementById('activePluginNo').textContent, 10) + activeReduction;
                document.getElementById('dirtyPluginNo').textContent = parseInt(document.getElementById('dirtyPluginNo').textContent, 10) + dirtyReduction;
            });
        }
    },

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
        this.game.updateSelectedGame();
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

    /* Observer for loot.game members. */
    gameObserver: function(changes) {
        changes.forEach(function(change){
            if (change.name == 'folder') {
                change.object.updateSelectedGame();
                change.object.updateRedatePluginsButtonState();
            } else if (change.name == 'masterlist') {
                if (change.object[change.name] && change.object[change.name].revision) {
                    document.getElementById('masterlistRevision').textContent = change.object[change.name].revision;
                } else {
                    document.getElementById('masterlistRevision').textContent = loot.l10n.translate("N/A");
                }
                if (change.object[change.name] && change.object[change.name].date) {
                    document.getElementById('masterlistDate').textContent = change.object[change.name].date;
                } else {
                    document.getElementById('masterlistDate').textContent = loot.l10n.translate("N/A");
                }
            } else if (change.name == 'globalMessages') {
                /* For the messages, they don't have a JS 'class' so need to everything
                   here. Count up the global message types that exist, then remove them
                   and add the new ones, counting their types, then update the message
                   counts. I tried using an observer for this, but it wouldn't fire for
                   some reason. */

                var oldTotal = change.oldValue.length;
                var newTotal = 0;
                var oldWarn = 0;
                var newWarn = 0;
                var oldErr = 0;
                var newErr = 0;

                /* Count up old warnings and errors. */
                change.oldValue.forEach(function(message){
                    if (message.type == 'warn') {
                        ++oldWarn;
                    } else if (message.type == 'error') {
                        ++oldErr;
                    }
                });

                /* Remove old messages from UI. */
                var generalMessagesList = document.getElementById('summary').getElementsByTagName('ul')[0];
                while (generalMessagesList.firstElementChild) {
                    generalMessagesList.removeChild(generalMessagesList.firstElementChild);
                }

                /* Add new messages. */
                if (change.object[change.name]) {
                    newTotal = change.object[change.name].length;
                    change.object[change.name].forEach(function(message){
                        var li = document.createElement('li');
                        li.className = message.type;
                        /* Use the Marked library for Markdown formatting support. */
                        li.innerHTML = marked(message.content[0].str);
                        generalMessagesList.appendChild(li);

                        if (li.className == 'warn') {
                            ++newWarn;
                        } else if (li.className == 'error') {
                            ++newErr;
                        }
                    });
                }

                /* Update message counts in UI. */
                document.getElementById('filterTotalMessageNo').textContent = parseInt(document.getElementById('filterTotalMessageNo').textContent, 10) + newTotal - oldTotal;
                document.getElementById('totalMessageNo').textContent = document.getElementById('filterTotalMessageNo').textContent;
                document.getElementById('totalWarningNo').textContent = parseInt(document.getElementById('totalWarningNo').textContent, 10) + newWarn - oldWarn;
                document.getElementById('totalErrorNo').textContent = parseInt(document.getElementById('totalErrorNo').textContent, 10) + newErr - oldErr;
            } else if (change.name == 'plugins') {
                /* Update plugin and message counts. Unlike for global messages
                   it's not worth calculating the count differences, just count
                   from zero. */
                var totalMessageNo = 0;
                var warnMessageNo = 0;
                var errorMessageNo = 0;
                var activePluginNo = 0;
                var dirtyPluginNo = 0;

                if (change.object.globalMessages) {
                    totalMessageNo = change.object.globalMessages.length;
                    change.object.globalMessages.forEach(function(message){
                        if (message.type == 'warn') {
                            ++warnMessageNo;
                        } else if (message.type == 'error') {
                            ++errorMessageNo;
                        }
                    });
                }

                change.object[change.name].forEach(function(plugin) {
                    if (plugin.isActive) {
                        ++activePluginNo;
                    }
                    if (plugin.isDirty) {
                        ++dirtyPluginNo;
                    }
                    if (plugin.messages) {
                        totalMessageNo += plugin.messages.length;
                        plugin.messages.forEach(function(message) {
                            if (message.type == 'warn') {
                                ++warnMessageNo;
                            } else if (message.type == 'error') {
                                ++errorMessageNo;
                            }
                        });
                    }
                });
                document.getElementById('filterTotalMessageNo').textContent = totalMessageNo;
                document.getElementById('totalMessageNo').textContent = totalMessageNo;
                document.getElementById('totalWarningNo').textContent = warnMessageNo;
                document.getElementById('totalErrorNo').textContent = errorMessageNo;
                document.getElementById('filterTotalPluginNo').textContent = change.object[change.name].length;
                document.getElementById('totalPluginNo').textContent = change.object[change.name].length;
                document.getElementById('activePluginNo').textContent = activePluginNo;
                document.getElementById('dirtyPluginNo').textContent = dirtyPluginNo;

                /* Register a new array observer. */
                Array.observe(change.object[change.name], change.object.pluginsObserver);
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
