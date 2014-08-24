/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013-2014    WrinklyNinja

    This file is part of LOOT.

    LOOT is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    LOOT is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with LOOT.  If not, see
    <http://www.gnu.org/licenses/>.
*/
'use strict';
var loot = {};

/* Returns a cefQuery as a Promise. */
loot.query = function(request) {
    return new Promise(function(resolve, reject) {
        window.cefQuery({
        request: request,
        persistent: false,
        onSuccess: resolve,
        onFailure: function(errorCode, errorMessage) {
                reject(Error('Error code: ' + errorCode + '; ' + errorMessage))
            }
        });
    })
}
function processCefError(err) {
    /* Error.stack seems to be Chromium-specific. It gives a lot more useful
       info than just the error message. */
    console.log(err.stack);
    showMessageBox('error', 'Error', err.message);
}

var marked;
function saveFilterState(evt) {
    if (evt.currentTarget.checked) {
        loot.settings.filters[evt.currentTarget.id] = true;
    } else {
        delete loot.settings.filters[evt.currentTarget.id];
    }

    var request = JSON.stringify({
        name: 'saveFilterState',
        args: [
            evt.currentTarget.id,
            evt.currentTarget.checked
        ]
    });

    loot.query(request).catch(processCefError);
}
function applySavedFilters() {
    if (loot.settings.filters) {
        for (var key in loot.settings.filters) {
            var elem = document.getElementById(key);
            if (elem) {
                elem.dispatchEvent(new MouseEvent('click'));
            }
        }
    }
}
function isVisible(element) {
    return (element.className.indexOf('hidden') == -1);
}
function showElement(element) {
    if (element != null) {
        element.classList.toggle('hidden', false);
    }
}
function hideElement(element) {
    if (element != null) {
        element.classList.toggle('hidden', true);
    }
}
function highlightElement(element, state) {
    element.classList.toggle('highlight', state);
}
function toggleDisplayCSS(evt) {
    var e = document.getElementsByClassName(evt.target.getAttribute('data-class'));
    if (evt.target.checked) {
        for (var i = 0, z = e.length; i < z; i++) {
            e[i].classList.toggle('hidden', true);
        }
    } else {
        for (var i = 0, z = e.length; i < z; i++) {
            e[i].classList.toggle('hidden', false);
        }
    }
}

function getConflictingPluginsFromFilter() {
    var conflictsPlugin = document.body.getAttribute('data-conflicts');
    if (conflictsPlugin) {
        /* Now get conflicts for the plugin. */
        var request = JSON.stringify({
            name: 'getConflictingPlugins',
            args: [
                conflictsPlugin
            ]
        });

        return loot.query(request).then(JSON.parse).then(function(result){
            if (result) {
                for (var key in result.crcs) {
                    for (var i = 0; i < loot.game.plugins.length; ++i) {
                        if (loot.game.plugins[i].name == key) {
                            loot.game.plugins[i].crc = result.crcs[key];
                            break;
                        }
                    }
                }
                if (result.conflicts) {
                    return result.conflicts;
                } else {
                    /* No conflicts. Filter everything but the plugin itself. */
                    return [ conflictsPlugin ];
                }
            }
            return [];
        }).catch(processCefError);
    }

    return Promise.resolve([]);
}
function togglePlugins(evt) {
    var cards = document.getElementsByTagName('main')[0].getElementsByTagName('plugin-card');
    var entries = document.getElementById('pluginsNav').children;
    var hiddenPluginNo = 0;
    var hiddenMessageNo = 0;
    if (cards.length != entries.length) {
        throw Error("Error: Number of plugins in sidebar doesn't match number of plugins in main area!");
    }
    /* The conflict filter, if enabled, executes C++ code, so needs to be
       handled using a promise, so the rest of the function should wait until
       it is completed.
    */
    getConflictingPluginsFromFilter().then(function(conflicts) {
        for (var i = 0; i < cards.length; ++i) {
            var isConflictingPlugin = false;
            var isMessageless = true;
            var hasInactivePluginMessages = false;
            var messages = cards[i].getElementsByTagName('ul')[0].getElementsByTagName('li');
            if (cards[i].getAttribute('data-active') == 'false') {
                hasInactivePluginMessages = true;
            }
            if (conflicts.indexOf(cards[i].getName()) != -1) {
                isConflictingPlugin = true;
            }
            for (var j = 0; j < messages.length; ++j) {
                var hasPluginMessages = false;
                var hasNotes = false;
                var hasDoNotCleanMessages = false;
                if (messages[j].parentElement.parentElement.id != 'generalMessages') {
                    hasPluginMessages = true;
                }
                if (messages[j].className.indexOf('say') != -1) {
                    hasNotes = true;
                }
                if (messages[j].textContent.indexOf('Do not clean.') != -1) {
                    hasDoNotCleanMessages = true;
                }
                if ((document.getElementById('hideAllPluginMessages').checked && hasPluginMessages)
                    || (document.getElementById('hideNotes').checked && hasNotes)
                    || (document.getElementById('hideDoNotCleanMessages').checked && hasDoNotCleanMessages)
                    || (document.getElementById('hideInactivePluginMessages').checked && hasInactivePluginMessages)) {
                    hideElement(messages[j]);
                    ++hiddenMessageNo;
                } else {
                    showElement(messages[j]);
                }
                if (messages[j].className.indexOf('hidden') == -1) {
                    isMessageless = false;
                    break;
                }
            }
            if ((document.getElementById('hideMessagelessPlugins').checked && isMessageless)
                || conflicts.length > 0 && !isConflictingPlugin) {
                hideElement(cards[i]);
                hideElement(entries[i - 2]);
                ++hiddenPluginNo;
            } else {
                showElement(cards[i]);
                showElement(entries[i - 2]);
            }
        }
        document.getElementById('hiddenMessageNo').textContent = hiddenMessageNo;
        document.getElementById('hiddenPluginNo').textContent = hiddenPluginNo;
    });
}
function showMessageDialog(title, text, closeCallback) {
    var dialog = new MessageDialog();

    document.body.appendChild(dialog);
    dialog.showModal('warn', title, text, closeCallback);
}
function showMessageBox(type, title, text) {
    var dialog = new MessageDialog();

    document.body.appendChild(dialog);
    dialog.showModal(type, title, text);
}
function openLogLocation(evt) {
    loot.query('openLogLocation').catch(processCefError);
}
function updateSelectedGame() {
    /* Highlight game in menu. Could use fa-chevron-right instead. */
    var gameMenuItems = document.getElementById('gameMenu').children[0].children;
    for (var i = 0; i < gameMenuItems.length; ++i) {
        if (loot.game && gameMenuItems[i].getAttribute('data-folder') == loot.game.folder) {
            gameMenuItems[i].getElementsByClassName('fa')[0].classList.toggle('fa-angle-double-right', true);
        } else {
            gameMenuItems[i].getElementsByClassName('fa')[0].classList.toggle('fa-angle-double-right', false);
        }
    }

    /* Also enable/disable the redate plugins option. */
    var index = undefined;
    for (var i = 0; i < loot.settings.games.length; ++i) {
        if (loot.game && loot.settings.games[i].folder == loot.game.folder) {
            index = i;
        }
    }
    if (index != undefined && loot.settings.games[index].type == 'Skyrim') {
        document.getElementById('redatePluginsButton').classList.toggle('disabled', false);
    } else {
        document.getElementById('redatePluginsButton').classList.toggle('disabled', true);
    }

    /* Also disable deletion of the game's row in the settings dialog. */
    var rows = document.getElementById('gameTable').getElementsByTagName('tbody')[0].getElementsByTagName('tr');
    for (var i = 0; i < rows.length; ++i) {
        if (rows[i].getElementsByClassName('folder').length > 0) {
            if (loot.game && rows[i].getElementsByClassName('folder')[0].value == loot.game.folder) {
                document.getElementById('gameTable').setReadOnly(rows[i], ['fa-trash-o']);
            } else {
                document.getElementById('gameTable').setReadOnly(rows[i], ['fa-trash-o'], false);
            }
        }
    }
}
function changeGame(evt) {
    /* First store current game info in loot.games object.
       This object may not exist, so initialise it using the corresponding
       loot.settings.games object. */
    var index = undefined;
    if (loot.games) {
        for (var i = 0; i < loot.games.length; ++i) {
            if (loot.games[i].folder == loot.game.folder) {
                index = i;
            }
        }
    }
    if (index == undefined) {
        for (var i = 0; i < loot.settings.games.length; ++i) {
            if (loot.settings.games[i].folder == loot.game.folder) {
                index = i;
            }
        }

        loot.games = [];
        loot.games.push(loot.settings.games[index]);
        index = 0;

    }
    loot.games[index].globalMessages = loot.game.globalMessages;
    loot.games[index].masterlist = loot.game.masterlist;
    loot.games[index].plugins = loot.game.plugins;


    /* Now send off a CEF query with the folder name of the new game. */
    var request = JSON.stringify({
        name: 'changeGame',
        args: [
            evt.currentTarget.getAttribute('data-folder')
        ]
    });
    loot.query(request).then(function(result){
        /* For each filter, save its state then deactivate it if it is activated.
           All filters apart from the conflicts filter will be re-activated
           after the new game has loaded. */
        var elements = document.getElementById('filters').getElementsByTagName('input');
        var activeFilters = [];
        for (var i = 0; i < elements.length; ++i) {
            if (elements[i].checked) {
                activeFilters.push(elements[i]);
                elements[i].click();
            }
        }
        /* Need to do something slightly different for the conflicts filter.
           Edit its state and run the filter function manually. */
        if (document.body.hasAttribute('data-conflicts')) {
            document.body.removeAttribute('data-conflicts');
            /* Don't need to supply an event arg because togglePlugins doesn't
               actually use it. */
            togglePlugins();
        }

        /* Clear the UI of all existing game-specific data. Also
           clear the card and li variables for each plugin object. */
        loot.games[index].plugins.forEach(function(plugin){
            plugin.card.parentElement.removeChild(plugin.card);
            plugin.card = undefined;
            plugin.li.parentElement.removeChild(plugin.li);
            plugin.li = undefined;
        });
        var globalMessages = document.getElementById('generalMessages').getElementsByTagName('ul')[0];
        while (globalMessages.firstElementChild) {
            globalMessages.removeChild(globalMessages.firstElementChild);
        }

        /* Now update interface for new data. */
        updateInterfaceWithGameInfo(result);

        /* Now update game menu to highlight the newly selected game. */
        updateSelectedGame();

        /* Reapply previously active filters. */
        activeFilters.forEach(function(filter){
            filter.click();
        });
    }).catch(processCefError);
}
function openReadme(evt) {
    loot.query('openReadme').catch(processCefError);
}
function updateMasterlist(evt) {
    loot.query('updateMasterlist').then(JSON.parse).then(function(result){
        if (result == null) {
            return;
        }
        /* Update JS variables. */

        loot.game.masterlist.revision = result.masterlist.revision;
        loot.game.masterlist.date = result.masterlist.date;

        result.plugins.forEach(function(plugin){
            for (var i = 0; i < loot.game.plugins.length; ++i) {
                if (loot.game.plugins[i].name == plugin.name) {
                    loot.game.plugins[i].isDirty = plugin.isDirty;
                    loot.game.plugins[i].isGlobalPriority = plugin.isGlobalPriority;
                    loot.game.plugins[i].masterlist = plugin.masterlist;
                    loot.game.plugins[i].messages = plugin.messages;
                    loot.game.plugins[i].modPriority = plugin.modPriority;
                    loot.game.plugins[i].tags = plugin.tags;
                    break;
                }
            }
        });

        /* For the messages, they don't have a JS 'class' so need to everything
           here. Count up the global message types that exist, then remove them
           and add the new ones, counting their types, then update the message
           counts. I tried using an observer for this, but it wouldn't fire for
           some reason. */

        var oldTotal = loot.game.globalMessages.length;
        var newTotal = result.globalMessages.length;
        var oldWarn = 0;
        var newWarn = 0;
        var oldErr = 0;
        var newErr = 0;

        loot.game.globalMessages.forEach(function(message){
            if (message.type == 'warn') {
                ++oldWarn;
            } else if (message.type == 'error') {
                ++oldErr;
            }
        });

        var generalMessagesList = document.getElementById('generalMessages').getElementsByTagName('ul')[0];
        while (generalMessagesList.firstElementChild) {
            generalMessagesList.removeChild(generalMessagesList.firstElementChild);
        }

        result.globalMessages.forEach(function(message){
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

        document.getElementById('filterTotalMessageNo').textContent = parseInt(document.getElementById('filterTotalMessageNo').textContent, 10) + newTotal - oldTotal;
        document.getElementById('totalMessageNo').textContent = parseInt(document.getElementById('totalMessageNo').textContent, 10) + newTotal - oldTotal;
        document.getElementById('totalWarningNo').textContent = parseInt(document.getElementById('totalWarningNo').textContent, 10) + newWarn - oldWarn;
        document.getElementById('totalErrorNo').textContent = parseInt(document.getElementById('totalErrorNo').textContent, 10) + newErr - oldErr;

        loot.game.globalMessages = result.globalMessages;
    }).catch(processCefError);
}
function sortUIElements(pluginNames) {
    /* pluginNames is an array of plugin names in their sorted order. Rearrange
       the plugin cards and nav entries to match it. */
    var main = document.getElementsByTagName('main')[0];
    var pluginsNav = document.getElementById('pluginsNav');
    var entries = pluginsNav.children;
    if (main.children.length - 2 != entries.length) {
        throw Error("Error: Number of plugins in sidebar doesn't match number of plugins in main area!");
    }
    pluginNames.forEach(function(name){
        var card = document.getElementById(name.replace(/\s+/g, ''));
        var li;
        for (var i = 0; i < entries.length; ++i) {
            if (entries[i].getElementsByClassName('name')[0].textContent == name) {
                li = entries[i];
            }
        }

        /* Easiest just to remove them and add them on at the end. */
        main.removeChild(card);
        main.appendChild(card);
        pluginsNav.removeChild(li);
        pluginsNav.appendChild(li);
    });
}
function sortPlugins(evt) {
    if (loot.settings.updateMasterlist) {
        updateMasterlist(evt);
    }
    loot.query('sortPlugins').then(JSON.parse).then(function(result){
        if (result) {
            for (var key in result.crcs) {
                for (var i = 0; i < loot.game.plugins.length; ++i) {
                    if (loot.game.plugins[i].name == key) {
                        loot.game.plugins[i].crc = result.crcs[key];
                        break;
                    }
                }
            }

            if (loot.neverTellMeTheOdds) {
                /* Array shuffler from <https://stackoverflow.com/questions/6274339/how-can-i-shuffle-an-array-in-javascript> */
                for(var j, x, i = result.loadOrder.length; i; j = Math.floor(Math.random() * i), x = result.loadOrder[--i], result.loadOrder[i] = result.loadOrder[j], result.loadOrder[j] = x);
            }

            /* Record the previous order in case the user cancels sorting. */
            /* Start at 2 to skip summary and general messages. */
            var cards = document.getElementsByTagName('main')[0].children;
            loot.newLoadOrder = result.loadOrder;
            loot.lastLoadOrder = [];
            for (var i = 2; i < cards.length; ++i) {
                loot.lastLoadOrder.push(cards[i].getElementsByTagName('h1')[0].textContent);
            }
            /* Now update the UI for the new order. */
            sortUIElements(result.loadOrder);

            /* Now hide the masterlist update buttons, and display the accept and
               cancel sort buttons. */
            hideElement(document.getElementById('updateMasterlistButton'));
            hideElement(document.getElementById('sortButton'));
            showElement(document.getElementById('applySortButton'));
            showElement(document.getElementById('cancelSortButton'));
        }
    }).catch(processCefError);
}
function applySort(evt) {
    var request = JSON.stringify({
        name: 'applySort',
        args: [
            loot.newLoadOrder
        ]
    });
    loot.query(request).then(function(result){
        /* Remove old load order storage. */
        delete loot.lastLoadOrder;
        delete loot.newLoadOrder;

        /* Now show the masterlist update buttons, and hide the accept and
           cancel sort buttons. */
        showElement(document.getElementById('updateMasterlistButton'));
        showElement(document.getElementById('sortButton'));
        hideElement(document.getElementById('applySortButton'));
        hideElement(document.getElementById('cancelSortButton'));
    }).catch(processCefError);
}
function cancelSort(evt) {
    /* Sort UI elements again according to stored old load order. */
    sortUIElements(loot.lastLoadOrder);
    delete loot.lastLoadOrder;
    delete loot.newLoadOrder;

    /* Now show the masterlist update buttons, and hide the accept and
       cancel sort buttons. */
    showElement(document.getElementById('updateMasterlistButton'));
    showElement(document.getElementById('sortButton'));
    hideElement(document.getElementById('applySortButton'));
    hideElement(document.getElementById('cancelSortButton'));
}
function redatePlugins(evt) {
    if (evt.target.classList.contains('disabled')) {
        return;
    }

    showMessageDialog('Redate Plugins', 'This feature is provided so that modders using the Creation Kit may set the load order it uses. A side-effect is that any subscribed Steam Workshop mods will be re-downloaded by Steam. Do you wish to continue?', function(result){
        if (result) {
            loot.query('redatePlugins').then(function(response){
                showMessageBox('info', 'Redate Plugins', 'Plugins were successfully redated.');
            }).catch(processCefError);
        }
    });

    //showMessageBox('info', 'Redate Plugins', 'Plugins were successfully redated.');
}
function clearAllMetadata(evt) {
    showMessageDialog('Clear All Metadata', 'Are you sure you want to clear all existing user-added metadata from all plugins?', function(result){
        if (result) {
            loot.query('clearAllMetadata').then(JSON.parse).then(function(result){
                /* Need to empty the UI-side user metadata. */
                result.forEach(function(plugin){
                    for (var i = 0; i < loot.game.plugins.length; ++i) {
                        if (loot.game.plugins[i].name == plugin.name) {
                            loot.game.plugins[i].userlist = undefined;

                            loot.game.plugins[i].modPriority = plugin.modPriority;
                            loot.game.plugins[i].isGlobalPriority = plugin.isGlobalPriority;
                            loot.game.plugins[i].messages = plugin.messages;
                            loot.game.plugins[i].tags = plugin.tags;
                            loot.game.plugins[i].isDirty = plugin.isDirty;

                            break;
                        }
                    }
                });
            }).catch(processCefError);
        }
    });
}
function copyContent(evt) {
    var messages = [];
    var plugins = [];

    if (loot.game) {
        loot.game.globalMessages.forEach(function(message){
            var m = {};
            messages.push({
                type: message.type,
                content: message.content[0].str
            });
        });
        loot.game.plugins.forEach(function(plugin){
            plugins.push({
                name: plugin.name,
                crc: plugin.crc,
                version: plugin.version,
                isActive: plugin.isActive,
                isDummy: plugin.isDummy,
                loadsBSA: plugin.loadsBSA,

                modPriority: plugin.modPriority,
                isGlobalPriority: plugin.isGlobalPriority,
                messages: plugin.messages,
                tags: plugin.tags,
                isDirty: plugin.isDirty
            });
        });
    } else {
        var message = document.getElementById('generalMessagesList').getElementsByTagName('ul')[0].firstElementChild;
        if (message) {
            messages.push({
                type: 'error',
                content: message.textContent
            });
        }
    }

    var request = JSON.stringify({
        name: 'copyContent',
        args: [{
            messages: messages,
            plugins: plugins
        }]
    });

    loot.query(request).catch(processCefError);
}
function handlePluginDrop(evt) {
    evt.stopPropagation();

    if (evt.currentTarget.tagName == 'TABLE' && (evt.currentTarget.id == 'req' || evt.currentTarget.id == 'inc' || evt.currentTarget.id == 'loadAfter')) {
        var data = {
            name: evt.dataTransfer.getData('text/plain')
        };
        evt.currentTarget.addRow(data);
    }

    return false;
}
function handlePluginDragStart(evt) {
    evt.dataTransfer.effectAllowed = 'copy';
    evt.dataTransfer.setData('text/plain', evt.target.getElementsByClassName('name')[0].textContent);
}
function handlePluginDragOver(evt) {
    evt.preventDefault();
    evt.dataTransfer.dropEffect = 'copy';
}

function areSettingsValid() {
    /* Validate inputs individually. */
    var inputs = document.getElementById('settings').getElementsByTagName('input');
    for (var i = 0; i < inputs.length; ++i) {
        if (!inputs[i].checkValidity()) {
            return false;
            console.log(inputs[i]);
        }
    }
    return true;
}
/* Close any open menus. */
function onBodyClick(evt) {
    var toolbarMenus = document.querySelectorAll('header > ol > li > ol');
    for (var i = 0; i < toolbarMenus.length; ++i) {
        if (!evt.detail.newMenu || evt.detail.newMenu != toolbarMenus[i]) {
            hideElement(toolbarMenus[i]);
        }
    }

    var pluginMenus = document.getElementsByTagName('plugin-menu');
    for (var i = 0; i < pluginMenus.length; ++i) {
        if (!evt.detail.newMenu || evt.detail.newMenu != pluginMenus[i]) {
            pluginMenus[i].parentElement.removeChild(pluginMenus[i]);
        }
    }
}
function openMenu(evt) {
    if (!isVisible(evt.currentTarget.firstElementChild)) {
        showElement(evt.currentTarget.firstElementChild)

        /* To prevent the click event closing this menu just after it was
           opened, stop the current event and send off a new one. */
        evt.stopPropagation();

        document.body.dispatchEvent(new CustomEvent('click', {
            bubbles: true,
            detail: {
                newMenu: evt.currentTarget.firstElementChild
            }
        }));
    }
}
function toggleFiltersList(evt) {
    var filters = document.getElementById('filters');
    var plugins = document.getElementById('pluginsNav');

    if (isVisible(filters)) {
        hideElement(filters);
        showElement(plugins);
    } else {
        showElement(filters);
        hideElement(plugins);
    }
}

function closeAboutDialog(evt) {
    evt.target.parentElement.close();
}
function showAboutDialog(evt) {
    document.getElementById('about').showModal();
}
function updateSettingsUI() {
    var gameSelect = document.getElementById('defaultGameSelect');
    var gameMenu = document.getElementById('gameMenu').firstElementChild;
    var gameTable = document.getElementById('gameTable');

    /* First make sure game listing elements don't have any existing entries. */
    while (gameSelect.children.length > 1) {
        gameSelect.removeChild(gameSelect.lastElementChild);
    }
    while (gameMenu.firstElementChild) {
        gameMenu.firstElementChild.removeEventListener('click', changeGame, false);
        gameMenu.removeChild(gameMenu.firstElementChild);
    }
    gameTable.clear();

    /* Now fill with new values. */
    for (var i = 0; i < loot.settings.games.length; ++i) {
        var option = document.createElement('option');
        option.value = loot.settings.games[i].folder;
        option.textContent = loot.settings.games[i].name;
        gameSelect.appendChild(option);

        var li = document.createElement('li');
        li.setAttribute('data-folder', loot.settings.games[i].folder);

        var icon = document.createElement('span');
        icon.className = 'fa fa-fw';
        li.appendChild(icon);

        var text = document.createElement('span');
        text.textContent = loot.settings.games[i].name;
        li.appendChild(text);

        gameMenu.appendChild(li);

        if (loot.installedGames.indexOf(loot.settings.games[i].folder) == -1) {
            li.classList.toggle('disabled', true);
        } else {
            li.addEventListener('click', changeGame, false);
        }

        var row = gameTable.addRow(loot.settings.games[i]);
        gameTable.setReadOnly(row, ['name','folder','type']);
    }

    /* Highlight game in menu. */
    updateSelectedGame();

    gameSelect.value = loot.settings.game;
    document.getElementById('languageSelect').value = loot.settings.language;
    document.getElementById('debugVerbositySelect').value = loot.settings.debugVerbosity;
    document.getElementById('updateMasterlist').checked = loot.settings.updateMasterlist;
}
function closeSettingsDialog(evt) {
    if (!areSettingsValid()) {
        return;
    }

    var dialog = evt.target.parentElement.parentElement;
    if (evt.target.classList.contains('accept')) {
        /* Update the JS variable values. */
        var settings = {
            debugVerbosity: document.getElementById('debugVerbositySelect').value,
            game: document.getElementById('defaultGameSelect').value,
            games: document.getElementById('gameTable').getRowsData(false),
            language: document.getElementById('languageSelect').value,
            lastGame: loot.settings.lastGame,
            updateMasterlist: document.getElementById('updateMasterlist').checked,
        };

        /* Send the settings back to the C++ side. */
        var request = JSON.stringify({
            name: 'closeSettings',
            args: [
                settings
            ]
        });
        loot.query(request).then(function(result){

            try {
                loot.installedGames = JSON.parse(result);
            } catch (e) {
                console.log(e);
                console.log('getInstalledGames response: ' + results[1]);
            }

            loot.settings = settings;
            updateSettingsUI();
        }).catch(processCefError);
    } else {
        /* Re-apply the existing settings to the settings dialog elements. */
        updateSettingsUI();
    }
    dialog.close();
}

function showSettingsDialog(evt) {
    document.getElementById('settings').showModal();
}

function getDialogParent(element) {
    var element = element.parentElement;
    while (element) {
        if (element.nodeName == 'DIALOG') {
            return element;
        }
        element = element.parentElement;
    }
    return null;
}
function showHoverText(evt) {
    hideHoverText(evt);

    var hoverText = document.createElement('div');
    hoverText.id = 'hoverText';
    hoverText.textContent = evt.target.title;

    if (!evt.target.id) {
        evt.target.id = 'hoverTarget';
    }
    hoverText.setAttribute('data-target', evt.target.id);

    var rect = evt.target.getBoundingClientRect();

    var dialog = getDialogParent(evt.target);
    if (dialog) {
        dialog.appendChild(hoverText);
    } else {
        document.body.appendChild(hoverText);
    }
    hoverText.style.left = (rect.left + evt.target.offsetWidth/2) + 'px';
    hoverText.style.top = (rect.bottom + 10) + 'px';
}
function hideHoverText(evt) {
    var hoverText = document.getElementById('hoverText');

    if (hoverText) {
        var hoverTarget = document.getElementById('hoverTarget');
        if (hoverTarget) {
            hoverTarget.id = '';
        }
        hoverText.parentElement.removeChild(hoverText);
    }
}
function startSearch(evt) {
    if (evt.target.value == '') {
        loot.query('cancelFind').then(function(result){
            evt.target.focus();
        }).catch(processCefError);
    } else {
        var request = JSON.stringify({
            name: 'find',
            args: [
                evt.target.value
            ]
        });

        loot.query(request).then(function(result){
            evt.target.focus();
        }).catch(processCefError);
    }

}
function focusSearch(evt) {
    if (evt.ctrlKey && evt.keyCode == 70) { //'f'
        document.getElementById('searchBox').focus();
    }
}
function setupEventHandlers() {
    var elements;
    /*Set up filter value and CSS setting storage read/write handlers.*/
    elements = document.getElementById('filters').getElementsByTagName('input');
    for (var i = 0; i < elements.length; ++i) {
        elements[i].addEventListener('click', saveFilterState, false);
    }

    /*Set up handlers for filters.*/
    document.getElementById('hideVersionNumbers').addEventListener('click', toggleDisplayCSS, false);
    document.getElementById('hideCRCs').addEventListener('click', toggleDisplayCSS, false);
    document.getElementById('hideBashTags').addEventListener('click', toggleDisplayCSS, false);
    document.getElementById('hideNotes').addEventListener('click', togglePlugins, false);
    document.getElementById('hideDoNotCleanMessages').addEventListener('click', togglePlugins, false);
    document.getElementById('hideInactivePluginMessages').addEventListener('click', togglePlugins, false);
    document.getElementById('hideAllPluginMessages').addEventListener('click', togglePlugins, false);
    document.getElementById('hideMessagelessPlugins').addEventListener('click', togglePlugins, false);

    /* Set up handlers for buttons. */
    document.getElementById('fileMenu').addEventListener('click', openMenu, false);
    document.getElementById('redatePluginsButton').addEventListener('click', redatePlugins, false);
    document.getElementById('openLogButton').addEventListener('click', openLogLocation, false);
    document.getElementById('wipeUserlistButton').addEventListener('click', clearAllMetadata, false);
    document.getElementById('copyContentButton').addEventListener('click', copyContent, false);
    document.getElementById('gameMenu').addEventListener('click', openMenu, false);
    document.getElementById('settingsButton').addEventListener('click', showSettingsDialog, false);
    document.getElementById('helpMenu').addEventListener('click', openMenu, false);
    document.getElementById('helpButton').addEventListener('click', openReadme, false);
    document.getElementById('aboutButton').addEventListener('click', showAboutDialog, false);
    document.getElementById('updateMasterlistButton').addEventListener('click', updateMasterlist, false);
    document.getElementById('sortButton').addEventListener('click', sortPlugins, false);
    document.getElementById('applySortButton').addEventListener('click', applySort, false);
    document.getElementById('cancelSortButton').addEventListener('click', cancelSort, false);
    document.getElementById('filtersToggle').addEventListener('click', toggleFiltersList, false);

    /* Set up event handlers for settings dialog. */
    var settings = document.getElementById('settings');
    settings.getElementsByClassName('accept')[0].addEventListener('click', closeSettingsDialog, false);
    settings.getElementsByClassName('cancel')[0].addEventListener('click', closeSettingsDialog, false);

    /* Set up about dialog handlers. */
    document.getElementById('about').getElementsByTagName('button')[0].addEventListener('click', closeAboutDialog, false);

    /* Set up event handler for hover text. */
    var hoverTargets = document.querySelectorAll('[title]');
    for (var i = 0; i < hoverTargets.length; ++i) {
        hoverTargets[i].addEventListener('mouseenter', showHoverText, false);
        hoverTargets[i].addEventListener('mouseleave', hideHoverText, false);
    }

    /* Set up event handlers for content search. */
    var searchBox = document.getElementById('searchBox');
    searchBox.addEventListener('input', startSearch, false);
    searchBox.addEventListener('search', startSearch, false);
    window.addEventListener('keyup', focusSearch, false);

    /* Set up handler for closing menus. */
    document.body.addEventListener('click', onBodyClick, false);
}
function initVars() {
    loot.query('getVersion').then(function(result){
        try {
            loot.version = JSON.parse(result);

            document.getElementById('LOOTVersion').textContent = loot.version;
        } catch (e) {
            console.log(e);
            console.log('Response: ' + result);
        }
    }).catch(processCefError);

    loot.query('getLanguages').then(function(result){
        try {
            loot.languages = JSON.parse(result);

            /* Now fill in language options. */
            var settingsLangSelect = document.getElementById('languageSelect');
            var messageLangSelect = document.getElementById('messageRow').content.querySelector('.language');
            for (var i = 0; i < loot.languages.length; ++i) {
                var option = document.createElement('option');
                option.value = loot.languages[i].locale;
                option.textContent = loot.languages[i].name;
                settingsLangSelect.appendChild(option);
                messageLangSelect.appendChild(option.cloneNode(true));
            }
        } catch (e) {
            console.log(e);
            console.log('Response: ' + result);
        }
    }).catch(processCefError);

    loot.query('getInitErrors').then(JSON.parse).then(function(result){
        if (result) {
            var generalMessagesList = document.getElementById('generalMessages').getElementsByTagName('ul')[0];

            result.forEach(function(message){
                var li = document.createElement('li');
                li.className = 'error';
                /* Use the Marked library for Markdown formatting support. */
                li.innerHTML = marked(message);
                generalMessagesList.appendChild(li);
            });

            document.getElementById('filterTotalMessageNo').textContent = result.length;
            document.getElementById('totalMessageNo').textContent = result.length;
            document.getElementById('totalErrorNo').textContent = result.length;
        }

        var parallelPromises = [
            loot.query('getGameTypes'),
            loot.query('getInstalledGames'),
            loot.query('getSettings'),
        ];

        if (!result) {
            parallelPromises.push(loot.query('getGameData'));
        }

        Promise.all(parallelPromises).then(function(results) {
            try {
                loot.gameTypes = JSON.parse(results[0]);
            } catch (e) {
                console.log(e);
                console.log('getGameTypes response: ' + results[0]);
            }

            /* Fill in game row template's game type options. */
            var select = document.getElementById('gameRow').content.querySelector('select');
            for (var j = 0; j < loot.gameTypes.length; ++j) {
                var option = document.createElement('option');
                option.value = loot.gameTypes[j];
                option.textContent = loot.gameTypes[j];
                select.appendChild(option);
            }

            try {
                loot.installedGames = JSON.parse(results[1]);
            } catch (e) {
                console.log(e);
                console.log('getInstalledGames response: ' + results[1]);
            }

            if (results.length > 3) {
                updateInterfaceWithGameInfo(results[3]);
            }

            try {
                loot.settings = JSON.parse(results[2]);
                if (loot.settings.filters == undefined) {
                    loot.settings.filters = {};
                }
                applySavedFilters();
                updateSettingsUI();
            } catch (e) {
                console.log(e);
                console.log('getSettings response: ' + results[2]);
            }

            if (results.length == 3) {
                document.getElementById('settingsButton').click();
            }

        }).catch(processCefError);

    }).catch(processCefError);
}

function masterlistObserver(changes) {
    changes.forEach(function(change){
        if (change.name == 'revision') {
            document.getElementById('masterlistRevision').textContent = change.object[change.name];
        } else if (change.name == 'date') {
            document.getElementById('masterlistDate').textContent = change.object[change.name];
        }
    });
}
function updateInterfaceWithGameInfo(response) {

    try {
        loot.game = JSON.parse(response, jsonToPlugin);
    } catch (e) {
        console.log(e);
        console.log('getGameData response: ' + response);
    }

    var totalMessageNo = 0;
    var warnMessageNo = 0;
    var errorMessageNo = 0;
    var activePluginNo = 0;
    var dirtyPluginNo = 0;

    /* Fill report with data. */
    document.getElementById('masterlistRevision').textContent = loot.game.masterlist.revision;
    document.getElementById('masterlistDate').textContent = loot.game.masterlist.date;

    var generalMessagesList = document.getElementById('generalMessages').getElementsByTagName('ul')[0];
    loot.game.globalMessages.forEach(function(message){
        var li = document.createElement('li');
        li.className = message.type;
        /* Use the Marked library for Markdown formatting support. */
        li.innerHTML = marked(message.content[0].str);
        generalMessagesList.appendChild(li);

        if (li.className == 'warn') {
            ++warnMessageNo;
        } else if (li.className == 'error') {
            ++errorMessageNo;
        }
    });
    totalMessageNo = loot.game.globalMessages.length;
    var pluginsList = document.getElementsByTagName('main')[0];
    var pluginsNav = document.getElementById('pluginsNav');
    loot.game.plugins.forEach(function(plugin) {

        if (plugin.isActive) {
            ++activePluginNo;
        }

        if (plugin.isDirty) {
            ++dirtyPluginNo;
        }

        if (plugin.messages && plugin.messages.length != 0) {
            plugin.messages.forEach(function(message) {

                if (message.type == 'warn') {
                    ++warnMessageNo;
                } else if (message.type == 'error') {
                    ++errorMessageNo;
                }
                ++totalMessageNo;
            });
        }
    });
    document.getElementById('filterTotalMessageNo').textContent = totalMessageNo;
    document.getElementById('totalMessageNo').textContent = totalMessageNo;
    document.getElementById('totalWarningNo').textContent = warnMessageNo;
    document.getElementById('totalErrorNo').textContent = errorMessageNo;
    document.getElementById('filterTotalPluginNo').textContent = loot.game.plugins.length;
    document.getElementById('totalPluginNo').textContent = loot.game.plugins.length;
    document.getElementById('activePluginNo').textContent = activePluginNo;
    document.getElementById('dirtyPluginNo').textContent = dirtyPluginNo;

    /* Observe changes to update UI as appropriate. */
    Object.observe(loot.game.masterlist, masterlistObserver);
}

require.config({
    baseUrl: "js",
  });
require(['marked', 'order!custom', 'order!plugin'], function(response) {
    marked = response;
    /* Make sure settings are what I want. */
    marked.setOptions({
        gfm: true,
        tables: true,
        sanitize: true
    });
    setupEventHandlers();
    initVars();
});
