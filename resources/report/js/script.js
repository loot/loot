/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013-2015    WrinklyNinja

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
var marked;
var l10n;

function processCefError(err) {
    /* Error.stack seems to be Chromium-specific. It gives a lot more useful
       info than just the error message. Also, this can be used to catch any
       promise errors, not just CEF errors. */
    console.log(err.stack);
    closeProgressDialog();
    showMessageBox(l10n.jed.translate('Error').fetch(), err.message);
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
function toggleDisplayCSS(evt) {
    var attr = 'data-hide-' + evt.target.getAttribute('data-class');
    if (evt.target.checked) {
        document.getElementById('main').setAttribute(attr, true);
    } else {
        document.getElementById('main').removeAttribute(attr);
    }
}
function toast(text) {
    var toast = document.getElementById('toast');
    toast.text = text;
    toast.show();
}
function showMessageDialog(title, text, positiveText, closeCallback) {
    var dialog = document.createElement('loot-message-dialog');
    dialog.setButtonText(positiveText, l10n.jed.translate('Cancel').fetch());
    dialog.showModal(title, text, closeCallback);
    document.body.appendChild(dialog);
}
function showMessageBox(title, text) {
    var dialog = document.createElement('loot-message-dialog');
    dialog.setButtonText(l10n.jed.translate('OK').fetch());
    dialog.showModal(title, text);
    document.body.appendChild(dialog);
}

function showProgress(message) {
    var progressDialog = document.getElementById('progressDialog');
    if (message) {
        progressDialog.getElementsByTagName('p')[0].textContent = message;
    }
    if (!progressDialog.opened) {
        progressDialog.showModal();
    }
}
function closeProgressDialog() {
    var progressDialog = document.getElementById('progressDialog');
    if (progressDialog.opened) {
        progressDialog.close();
    }
}
function areSettingsValid() {
    /* Validate inputs individually. */
    var inputs = document.getElementById('settingsDialog').getElementsByTagName('input');
    for (var i = 0; i < inputs.length; ++i) {
        if (!inputs[i].checkValidity()) {
            return false;
            console.log(inputs[i]);
        }
    }
    return true;
}

function handleUnappliedChangesClose(change) {
    showMessageDialog('', l10n.jed.translate('You have not yet applied or cancelled your %s. Are you sure you want to quit?').fetch(change), l10n.jed.translate('Quit').fetch(), function(result){
        if (result) {
            /* Cancel any sorting and close any editors. Cheat by sending a
               cancelSort query for as many times as necessary. */
            var queries = [];
            var numQueries = 0;
            if (!document.getElementById('applySortButton').classList.contains('hidden')) {
                numQueries += 1;
            }
            numQueries += document.body.getAttribute('data-editors');
            for (var i = 0; i < numQueries; ++i) {
                queries.push(loot.query('cancelSort'));
            }
            Promise.all(queries).then(function(){
                window.close();
            }).catch(processCefError);
        }
    });
}

function initVars() {
    loot.query('getVersion').then(function(result){
        try {
            loot.version = JSON.parse(result);

            /* The fourth part of the version string is the build number. Trim it. */
            var pos = loot.version.lastIndexOf('.');
            if (loot.version.length > pos + 1) {
                document.getElementById('LOOTBuild').textContent = loot.version.substring(pos + 1);
            } else {
                document.getElementById('LOOTBuild').textContent = l10n.jed.translate('unknown').fetch();
            }

            loot.version = loot.version.substring(0, pos);
            document.getElementById('LOOTVersion').textContent = loot.version;
            document.getElementById('firstTimeLootVersion').textContent = loot.version;
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
            var messageLangSelect = document.querySelector('link[rel="import"][href$="editable-table.html"]');
            if (messageLangSelect) {
                messageLangSelect = messageLangSelect.import.querySelector('#messageRow').content.querySelector('.language');
            } else {
                messageLangSelect = document.querySelector('#messageRow').content.querySelector('.language');
            }

            for (var i = 0; i < loot.languages.length; ++i) {
                var settingsItem = document.createElement('paper-item');
                settingsItem.setAttribute('value', loot.languages[i].locale);
                settingsItem.setAttribute('noink', '');
                settingsItem.textContent = loot.languages[i].name;
                settingsLangSelect.appendChild(settingsItem);
                messageLangSelect.appendChild(settingsItem.cloneNode(true));
            }
        } catch (e) {
            console.log(e);
            console.log('Response: ' + result);
        }
    }).catch(processCefError);

    loot.query('getInitErrors').then(JSON.parse).then(function(result){
        if (result) {
            var generalMessagesList = document.getElementById('summary').getElementsByTagName('ul')[0];

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

        showProgress('Initialising user interface...');
        Promise.all(parallelPromises).then(function(results) {
            try {
                loot.gameTypes = JSON.parse(results[0]);
            } catch (e) {
                console.log(e);
                console.log('getGameTypes response: ' + results[0]);
            }

            /* Fill in game row template's game type options. */
            var select = document.querySelector('link[rel="import"][href$="editable-table.html"]');
            if (select) {
                select = select.import.querySelector('#gameRow').content.querySelector('.type')
            } else {
                select = document.querySelector('#gameRow').content.querySelector('.type');
            }
            for (var j = 0; j < loot.gameTypes.length; ++j) {
                var item = document.createElement('paper-item');
                item.setAttribute('value', loot.gameTypes[j]);
                item.setAttribute('noink', '');
                item.textContent = loot.gameTypes[j];
                select.appendChild(item);
            }

            try {
                loot.installedGames = JSON.parse(results[1]);
            } catch (e) {
                console.log(e);
                console.log('getInstalledGames response: ' + results[1]);
            }

            try {
                loot.settings = JSON.parse(results[2]);
            } catch (e) {
                console.log(e);
                console.log('getSettings response: ' + results[2]);
            }
        }).then(function(){
            return l10n.getJedInstance(loot.settings.language).then(function(jed){
                l10n.translateStaticText(jed);
                l10n.jed = jed;

                /* Also need to update the settings UI. */
                loot.updateSettingsUI();
            }).catch(processCefError);
        }).then(function(){
            if (result) {
                return new Promise(function(resolve, reject){
                    closeProgressDialog();
                    document.getElementById('settingsButton').click();
                    resolve('');
                });
            } else {
                return loot.query('getGameData').then(function(result){
                    var game = JSON.parse(result, jsonToPlugin);
                    loot.game.folder = game.folder;
                    loot.game.masterlist = game.masterlist;
                    loot.game.globalMessages = game.globalMessages;
                    loot.game.plugins = game.plugins;
                    document.getElementById('cardsNav').data = loot.game.plugins;
                    document.getElementById('main').lastElementChild.data = loot.game.plugins;

                    setTimeout(function() {
                        document.getElementById('cardsNav').updateSize();
                        closeProgressDialog();
                    }, 100);

                    return '';
                }).catch(processCefError);
            }
        }).then(function(){
            if (!loot.settings.lastVersion || loot.settings.lastVersion != loot.version) {
                document.getElementById('firstRun').showModal();
            }
        }).catch(processCefError);
    }).catch(processCefError);
}
function onContentRefresh(evt) {
    /* Send a query for updated load order and plugin header info. */
    showProgress(l10n.jed.translate('Refreshing data...').fetch());
    loot.query('getGameData').then(function(result){
        /* Parse the data sent from C++. */
        try {
            /* We don't want the plugin info creating cards, so don't convert
               to plugin objects. */
            var gameInfo = JSON.parse(result);
        } catch (e) {
            console.log(e);
            console.log('getGameData response: ' + result);
        }

        /* Now overwrite plugin data with the newly sent data. Also update
           card and li vars as they were unset when the game was switched
           from before. */
        var pluginNames = [];
        gameInfo.plugins.forEach(function(plugin){
            var foundPlugin = false;
            for (var i = 0; i < loot.game.plugins.length; ++i) {
                if (loot.game.plugins[i].name == plugin.name) {

                    loot.game.plugins[i].isActive = plugin.isActive;
                    loot.game.plugins[i].isEmpty = plugin.isEmpty;
                    loot.game.plugins[i].loadsBSA = plugin.loadsBSA;
                    loot.game.plugins[i].crc = plugin.crc;
                    loot.game.plugins[i].version = plugin.version;

                    loot.game.plugins[i].modPriority = plugin.modPriority;
                    loot.game.plugins[i].isGlobalPriority = plugin.isGlobalPriority;
                    loot.game.plugins[i].messages = plugin.messages;
                    loot.game.plugins[i].tags = plugin.tags;
                    loot.game.plugins[i].isDirty = plugin.isDirty;

                    foundPlugin = true;
                    break;
                }
            }
            if (!foundPlugin) {
                /* A new plugin. */
                loot.game.plugins.push(new Plugin(plugin));
            }
            pluginNames.push(plugin.name);
        });
        for (var i = 0; i < loot.game.plugins.length;) {
            var foundPlugin = false;
            for (var j = 0; j < pluginNames.length; ++j) {
                if (loot.game.plugins[i].name == pluginNames[j]) {
                    foundPlugin = true;
                    break;
                }
            }
            if (!foundPlugin) {
                /* Remove plugin. */
                loot.game.plugins.splice(i, 1);
            } else {
                ++i;
            }
        }

        /* Reapply filters. */
        setFilteredUIData();

        closeProgressDialog();
    }).catch(processCefError);
}

window.addEventListener('polymer-ready', function(e) {
    /* Set the plugin list's scroll target to its parent. */
    document.getElementById('main').lastElementChild.scrollTarget = document.getElementById('main');

    require(['bower_components/marked/lib/marked', 'js/l10n', 'js/plugin', 'js/loot', 'js/filters', 'js/events.js'], function(markedResponse, l10nResponse) {

        /* Register object observers. */
        Object.observe(loot, loot.observer);
        Object.observe(loot.game, loot.gameObserver);

        marked = markedResponse;
        l10n = l10nResponse;
        /* Make sure settings are what I want. */
        marked.setOptions({
            gfm: true,
            tables: true,
            sanitize: true
        });
        setupEventHandlers();
        initVars();
    });
}, false);
