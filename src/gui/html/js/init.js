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

function restoreFilterStates() {
  if (loot.settings.filters && loot.filters) {
    loot.filters.hideMessagelessPlugins = loot.settings.filters.hideMessagelessPlugins;
    loot.filters.hideInactivePlugins = loot.settings.filters.hideInactivePlugins;
    loot.filters.hideNotes = loot.settings.filters.hideNotes;
    loot.filters.hideDoNotCleanMessages = loot.settings.filters.hideDoNotCleanMessages;
    loot.filters.hideAllPluginMessages = loot.settings.filters.hideAllPluginMessages;
    loot.filters.hideVersionNumbers = loot.settings.filters.hideVersionNumbers;
    loot.filters.hideCRCs = loot.settings.filters.hideCRCs;
    loot.filters.hideBashTags = loot.settings.filters.hideBashTags;

    document.getElementById('hideMessagelessPlugins').checked = loot.settings.filters.hideMessagelessPlugins;
    document.getElementById('hideInactivePlugins').checked = loot.settings.filters.hideInactivePlugins;
    document.getElementById('hideNotes').checked = loot.settings.filters.hideNotes;
    document.getElementById('hideDoNotCleanMessages').checked = loot.settings.filters.hideDoNotCleanMessages;
    document.getElementById('hideAllPluginMessages').checked = loot.settings.filters.hideAllPluginMessages;
    document.getElementById('hideVersionNumbers').checked = loot.settings.filters.hideVersionNumbers;
    document.getElementById('hideCRCs').checked = loot.settings.filters.hideCRCs;
    document.getElementById('hideBashTags').checked = loot.settings.filters.hideBashTags;
  }
}

function applyEnabledFilters() {
  if (!loot.filters) {
    return;
  }

  if (loot.filters.hideMessagelessPlugins
      || loot.filters.hideInactivePlugins
      || loot.filters.hideNotes
      || loot.filters.hideDoNotCleanMessages
      || loot.filters.hideAllPluginMessages) {
    setFilteredUIData();
  }

  if (loot.filters.hideVersionNumbers) {
    document.getElementById('hideVersionNumbers').dispatchEvent(new Event('change'));
  }

  if (loot.filters.hideCRCs) {
    document.getElementById('hideCRCs').dispatchEvent(new Event('change'));
  }

  if (loot.filters.hideBashTags) {
    document.getElementById('hideBashTags').dispatchEvent(new Event('change'));
  }
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
                document.getElementById('LOOTBuild').textContent = loot.l10n.translate('unknown');
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

            messageLangSelect.setAttribute('value', messageLangSelect.firstElementChild.getAttribute('value'));
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

        loot.Dialog.showProgress('Initialising user interface...');
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
            select.setAttribute('value', select.firstElementChild.getAttribute('value'));

            try {
                setInstalledGames(JSON.parse(results[1]));
            } catch (e) {
                console.log(e);
                console.log('getInstalledGames response: ' + results[1]);
            }

            try {
                loot.settings = JSON.parse(results[2]);
                updateSettingsUI();
                restoreFilterStates();
            } catch (e) {
                console.log(e);
                console.log('getSettings response: ' + results[2]);
            }
        }).then(function(){
            /* Translate static text. */
            loot.l10n = new loot.Translator(loot.settings.language);
            loot.l10n.load().then(() => {
              loot.translateStaticText(loot.l10n);
              /* Also need to update the settings UI. */
              updateSettingsUI();
            }).catch(processCefError);
        }).then(function(){
            if (result) {
                return new Promise(function(resolve, reject){
                    loot.Dialog.closeProgress();
                    document.getElementById('settingsButton').click();
                    resolve('');
                });
            } else {
                return loot.query('getGameData').then(function(result){
                    var game = JSON.parse(result, loot.Plugin.fromJson);
                    loot.game.folder = game.folder;
                    loot.game.masterlist = game.masterlist;
                    loot.game.globalMessages = game.globalMessages;
                    loot.game.plugins = game.plugins;
                    document.getElementById('cardsNav').data = loot.game.plugins;
                    document.getElementById('main').lastElementChild.data = loot.game.plugins;
                    applyEnabledFilters();

                    setTimeout(function() {
                        document.getElementById('cardsNav').updateSize();
                        loot.Dialog.closeProgress();
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

window.addEventListener('polymer-ready', function(e) {
    /* Set the plugin list's scroll target to its parent. */
    document.getElementById('main').lastElementChild.scrollTarget = document.getElementById('main');

    /* Make sure settings are what I want. */
    marked.setOptions({
        gfm: true,
        tables: true,
        sanitize: true
    });
    setupEventHandlers();
    initVars();
}, false);
