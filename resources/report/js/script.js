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
function isStorageSupported() {
    try {
        return ('localStorage' in window && window['localStorage'] !== null && window['localStorage'] !== undefined);
    } catch (e) {
        return false;
    }
}
function saveCheckboxState(evt) {
    if (evt.currentTarget.checked) {
        try {
            localStorage.setItem(evt.currentTarget.id, true);
        } catch (e) {
            if (e == QUOTA_EXCEEDED_ERR) {
                alert('Web storage quota for this document has been exceeded. Please empty your browser\'s cache. Note that this will delete all locally stored data.');
            }
        }
    } else {
        localStorage.removeItem(evt.currentTarget.id);
    }
}
function loadSettings() {
    var i = localStorage.length - 1;
    while (i > -1) {
        var elem = document.getElementById(localStorage.key(i));
        if (elem != null && 'defaultChecked' in elem) {
			elem.dispatchEvent(new MouseEvent('click'));
        }
        i--;
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
function stepUnhideElement(element) {
    if (element != null && element.className.indexOf('hidden') != -1) {
        element.className = element.className.replace('hidden', '');
    }
}
function stepHideElement(element) {
    if (element != null) {
        element.className += ' hidden';
    }
}
function toggleDisplayCSS(evt) {
    var e = document.getElementsByClassName(evt.target.getAttribute('data-class'));
    if (evt.target.checked) {
        for (var i = 0, z = e.length; i < z; i++) {
            e[i].className += ' hidden';
        }
    } else {
        for (var i = 0, z = e.length; i < z; i++) {
            e[i].className = e[i].className.replace('hidden', '');
        }
    }
}
function toggleFilters(evt) {
    var filters = document.getElementById('filters');
    if (document.getElementById('filtersToggle').className.indexOf('current') == -1) {
        showElement(filters);
        evt.target.className += ' current';
    } else {
        hideElement(filters);
        evt.target.className = evt.target.className.replace('current', '');
    }
}
function toggleMessages(evt) {
    // Start at 2nd section to skip summary.
    var listItems = document.getElementById('main').querySelectorAll('ul li');
    var hiddenNo = parseInt(document.getElementById('hiddenMessageNo').textContent, 10);
    for (var i = 0; i < listItems.length; ++i) {
        var filterMatch = false;
        if (evt.target.id == 'hideAllPluginMessages' && listItems[i].parentElement.parentElement.id != 'generalMessages') {
            filterMatch = true;
        } else if (evt.target.id == 'hideNotes' && listItems[i].className.indexOf('say') != -1) {
            filterMatch = true;
        } else if (evt.target.id == 'hideDoNotCleanMessages' && listItems[i].textContent.indexOf('Do not clean.') != -1) {
            filterMatch = true;
        } else if (evt.target.id == 'hideInactivePluginMessages' && listItems[i].parentElement.parentElement.getAttribute('data-active') == 'false') {
            filterMatch = true;
        }
        if (filterMatch) {
            if (evt.target.checked) {
                if (listItems[i].className.indexOf('hidden') == -1) {
                    hiddenNo++;
                }
                stepHideElement(listItems[i]);
            } else {
                stepUnhideElement(listItems[i]);
                if (listItems[i].className.indexOf('hidden') == -1) {
                    hiddenNo--;
                }
            }
        }
    }
	document.getElementById('hiddenMessageNo').textContent = hiddenNo;
	togglePlugins(evt);
}
function togglePlugins(evt) {
    var sections = document.getElementById('main').children;
    var hiddenNo = parseInt(document.getElementById('hiddenPluginNo').textContent, 10);
    // Start at 3rd section to skip summary and general messages.
    for (var i = 2; i < sections.length; ++i) {
        var isMessageless = true;
        var messages = sections[i].getElementsByTagName('ul')[0].getElementsByTagName('li');
        for (var j = 0; j < messages.length; ++j) {
            if (messages[j].className.indexOf('hidden') == -1) {
                isMessageless = false;
                break;
            }
        }
        if (document.getElementById('hideMessagelessPlugins').checked && isMessageless) {
            if (sections[i].className.indexOf('hidden') == -1) {
                hiddenNo++;
                hideElement(sections[i]);
            }
        } else if (sections[i].className.indexOf('hidden') !== -1) {
            hiddenNo--;
            showElement(sections[i]);
        }
    }
    document.getElementById('hiddenPluginNo').textContent = hiddenNo;
}
function hideDialog(evt) {
    hideElement(document.getElementById('overlay'));
    var target = document.getElementById(evt.target.getAttribute('data-dialog'));
    hideElement(target);
    if (target.id == 'modalDialog') {
        document.body.removeChild(target);
    }
}
function showMessageDialog(title, text) {
    var content = document.getElementById('messageDialog').content;
    var clone = document.importNode(content, true);
    document.body.appendChild(clone);
    clone = document.body.lastElementChild;

    clone.id = 'modalDialog';

    clone.getElementsByTagName('span')[0].className += ' warn';

    clone.getElementsByTagName('h1')[0].textContent = title;
    clone.getElementsByTagName('p')[0].textContent = text;

    var overlay = document.getElementById('overlay');
    overlay.removeEventListener('click', hideDialog, false);
    showElement(overlay);

    clone.getElementsByClassName('accept')[0].setAttribute('data-dialog', clone.id);
    clone.getElementsByClassName('accept')[0].addEventListener('click', hideDialog, false);

    clone.getElementsByClassName('cancel')[0].setAttribute('data-dialog', clone.id);
    clone.getElementsByClassName('cancel')[0].addEventListener('click', hideDialog, false);
}
function showMessageBox(type, title, text) {

}
function redatePlugins() {
    showMessageDialog('Redate Plugins', 'This feature is provided so that modders using the Creation Kit may set the load order it uses. A side-effect is that any subscribed Steam Workshop mods will be re-downloaded by Steam. Do you wish to continue?');

    //showMessageBox('info', 'Redate Plugins', 'Plugins were successfully redated.');
}
function clearAllMetadata() {
    showMessageDialog('Clear All Metadata', 'Are you sure you want to clear all existing user-added metadata from all plugins?');
}
function clearMetadata(filename) {
    showMessageDialog('Clear Plugin Metadata', 'Are you sure you want to clear all existing user-added metadata from "' + filename + '"?');
}
function processButtonClick(evt) {
    var overlay = document.getElementById('overlay');
    var action = evt.currentTarget.getAttribute('data-action');
    if (action == 'view-ui') {
        var target = document.getElementById(evt.currentTarget.getAttribute('data-target'));
        if (isVisible(target)) {
            hideElement(target);
            if (target.getAttribute('data-overlay')) {
                overlay.setAttribute('data-dialog', '');
                hideElement(overlay);
            } else {
                var replace = target.getAttribute('data-replace');
                if (replace) {
                    showElement(document.getElementById(replace));
                }
            }
        } else {
            showElement(target);
            if (target.getAttribute('data-overlay') == '1') {
                overlay.setAttribute('data-dialog', target.id);
                overlay.addEventListener('click', hideDialog, false);
                showElement(overlay);
            } else {
                var replace = target.getAttribute('data-replace');
                if (replace) {
                    hideElement(document.getElementById(replace));
                }
            }
        }
    } else if (action == 'show-menu') {
        target = evt.target.nextSibling;
        if (isVisible(target)) {
            hideElement(target);
        } else {
            showElement(target);
        }
    } else if (action == 'redate-plugins') {
        redatePlugins();
    } else if (action == 'clear-metadata') {
        clearMetadata(evt.target.getAttribute('data-target'));
    } else if (action == 'wipe-userlist') {
        clearAllMetadata();
    } else if (action == 'show-editor') {
        document.getElementById(evt.target.getAttribute('data-target')).classList.toggle('flip');
    }
}
function toggleInputRO(evt) {
    if (evt.target.readOnly) {
        evt.target.removeAttribute('readonly');
    } else {
        evt.target.setAttribute('readonly', '');
    }
}
function removeGameRow(evt) {
    evt.target.parentElement.parentElement.parentElement.removeChild(evt.target.parentElement.parentElement);
}
function addNewGameRow(evt) {
    var clone = evt.currentTarget.cloneNode(true);
    var inputs = clone.getElementsByTagName('input');
    for (var i = 0; i < inputs.length; ++i) {
        inputs[i].removeAttribute('readonly');
        inputs[i].addEventListener('dblclick', toggleInputRO, false);
    }
    clone.getElementsByClassName('name')[0].placeholder = '';
    showElement(clone.getElementsByClassName('type')[0]);
    showElement(clone.getElementsByClassName('fa-trash-o')[0]);
    clone.getElementsByClassName('fa-trash-o')[0].addEventListener('click', removeGameRow, false);
    evt.currentTarget.parentElement.insertBefore(clone, evt.currentTarget);
}
function setupEventHandlers() {
    var elements;
    if (isStorageSupported()) { /*Set up filter value and CSS setting storage read/write handlers.*/
        elements = document.getElementById('filters').getElementsByTagName('input');
        for (var i = 0; i < elements.length; ++i) {
            elements[i].addEventListener('click', saveCheckboxState, false);
        }
    }
    /*Set up handlers for filters.*/
    document.getElementById('hideVersionNumbers').addEventListener('click', toggleDisplayCSS, false);
    document.getElementById('hideCRCs').addEventListener('click', toggleDisplayCSS, false);
    document.getElementById('hideBashTags').addEventListener('click', toggleDisplayCSS, false);
    document.getElementById('hideNotes').addEventListener('click', toggleMessages, false);
    document.getElementById('hideDoNotCleanMessages').addEventListener('click', toggleMessages, false);
    document.getElementById('hideInactivePluginMessages').addEventListener('click', toggleMessages, false);
    document.getElementById('hideAllPluginMessages').addEventListener('click', toggleMessages, false);
    document.getElementById('hideMessagelessPlugins').addEventListener('click', togglePlugins, false);
    /* Set up handlers for buttons. */
    elements = document.querySelectorAll('[data-action]');
    for (var i = 0; i < elements.length; ++i) {
        elements[i].addEventListener('click', processButtonClick, false);
    }
    /* Set up handlers for game table. */
    elements = document.getElementById('gameTable').getElementsByTagName('tbody')[0].getElementsByTagName('tr');
    for (var i = 0; i < elements.length - 1; ++i) {
        var inputs = elements[i].getElementsByTagName('input');
        for (var j = 0; j < inputs.length; ++j) {
            inputs[j].addEventListener('dblclick', toggleInputRO, false);
        }
        elements[i].getElementsByClassName('fa-trash-o')[0].addEventListener('click', removeGameRow, false);
    }
    elements[elements.length - 1].addEventListener('dblclick', addNewGameRow, false);
}
function processURLParams() {
    /* Get the data path from the URL and load it. */
    /*var pos = document.URL.indexOf("?data=");*/
    var pos = 0;
    if (pos != -1) {
        /*var datapath = 'file:///' + document.URL.substring(pos+6);*/
        var datapath = 'testdata'
        console.log(datapath);
        require([datapath], function(){
            var totalMessageNo = 0;
            var warnMessageNo = 0;
            var errorMessageNo = 0;
            var activePluginNo = 0;
            var dirtyPluginNo = 0;
            /* Fill report with data. */
            document.getElementById('LOOTVersion').textContent = data.lootVersion;
            document.getElementById('masterlistRevision').textContent = data.masterlist.revision.substr(0, 9);
            document.getElementById('masterlistDate').textContent = data.masterlist.date;
            var generalMessagesList = document.getElementById('generalMessages').getElementsByTagName('ul')[0];
            for (var i = 0; i < data.globalMessages.length; ++i) {
                var li = document.createElement('li');
                li.className = data.globalMessages[i].type;
                /* innerHTML is open to abuse, but for hyperlinking it's too useful. */
                li.innerHTML = data.globalMessages[i].content;
                generalMessagesList.appendChild(li);

                if (li.className == 'warn') {
                    warnMessageNo++;
                } else if (li.className == 'error') {
                    errorMessageNo++;
                }
            }
            totalMessageNo = data.globalMessages.length;
            var pluginsList = document.getElementById('main');
            var pluginsNav = document.getElementById('pluginsNav');
            for (var i = 0; i < data.plugins.length; ++i) {
                var content, clone;
                /* First add link to navbar. */
                content = document.getElementById('pluginNav').content;
                clone = document.importNode(content, true);
                pluginsNav.appendChild(clone);
                clone = pluginsNav.lastElementChild;

                clone.getElementsByTagName('a')[0].textContent = data.plugins[i].name;
                clone.getElementsByTagName('a')[0].href = '#' + data.plugins[i].name.replace(/\s+/g, '');

                if (data.plugins[i].isDummy) {
                    clone.getElementsByClassName('dummyPlugin')[0].className += ' fa fa-eye-slash';
                }

                if (data.plugins[i].loadsBSA) {
                    clone.getElementsByClassName('loadsBSA')[0].className += ' fa fa-paperclip';
                }

                if (data.plugins[i].hasUserEdits) {
                    /* This won't actually be handled anything like this in the real data implementation. */
                    clone.getElementsByClassName('hasUserEdits')[0].className += ' fa fa-user';
                }

                /* Now add plugin 'card'. */
                content = document.getElementById('pluginSection').content;
                clone = document.importNode(content, true);
                pluginsList.appendChild(clone);
                clone = pluginsList.lastElementChild;

                clone.setAttribute('data-active', data.plugins[i].isActive);
                clone.id = data.plugins[i].name.replace(/\s+/g, '');

                if (data.plugins[i].isActive) {
                    ++activePluginNo;
                }

                if (data.plugins[i].isDirty) {
                    ++dirtyPluginNo;
                }

                clone.getElementsByTagName('h1')[0].textContent = data.plugins[i].name;
                clone.getElementsByTagName('h1')[1].textContent = data.plugins[i].name;

                clone.getElementsByClassName('crc')[0].textContent = 'CRC: ' + data.plugins[i].crc;
                clone.getElementsByClassName('crc')[1].textContent = 'CRC: ' + data.plugins[i].crc;

                if (data.plugins[i].isDummy) {
                    showElement(clone.getElementsByClassName('dummyPlugin')[0]);
                }

                if (data.plugins[i].loadsBSA) {
                    showElement(clone.getElementsByClassName('loadsBSA')[0]);
                }

                if (data.plugins[i].hasUserEdits) {
                    /* This won't actually be handled anything like this in the real data implementation. */
                    showElement(clone.getElementsByClassName('hasUserEdits')[0]);
                }

                if (data.plugins[i].version) {
                    clone.getElementsByClassName('version')[0].textContent = 'Version: ' + data.plugins[i].version;
                    clone.getElementsByClassName('version')[1].textContent = 'Version: ' + data.plugins[i].version;
                } else {
                    hideElement(clone.getElementsByClassName('version')[0]);
                    hideElement(clone.getElementsByClassName('version')[1]);
                }

                if (data.plugins[i].tagsAdd && data.plugins[i].tagsAdd.length != 0) {
                    clone.getElementsByClassName('tag add')[0].textContent = data.plugins[i].tagsAdd.join(', ');
                } else {
                    hideElement(clone.getElementsByClassName('tag add')[0]);
                }

                if (data.plugins[i].tagRemove && data.plugins[i].tagRemove.length != 0) {
                    clone.getElementsByClassName('tag remove')[0].textContent = data.plugins[i].tagsRemove.join(', ');
                } else {
                    hideElement(clone.getElementsByClassName('tag remove')[0]);
                }

                clone.getElementsByClassName('editMetadata')[0].setAttribute('data-target', data.plugins[i].name);
                clone.getElementsByClassName('copyMetadata')[0].setAttribute('data-target', data.plugins[i].name);
                clone.getElementsByClassName('clearMetadata')[0].setAttribute('data-target', clone.id);

                if (data.plugins[i].messages && data.plugins[i].messages.length != 0) {
                    for (var j = 0; j < data.plugins[i].messages.length; ++j) {
                        var messageLi = document.createElement('li');
                        messageLi.className = data.plugins[i].messages[j].type;
                        /* innerHTML is open to abuse, but for hyperlinking it's too useful. */
                        messageLi.innerHTML = data.plugins[i].messages[j].content;
                        clone.getElementsByTagName('ul')[0].appendChild(messageLi);

                        if (messageLi.className == 'warn') {
                            warnMessageNo++;
                        } else if (messageLi.className == 'error') {
                            errorMessageNo++;
                        }
                        totalMessageNo++;
                    }
                } else {
                    clone.getElementsByTagName('ul')[0].className += ' hidden';
                }
            }
            document.getElementById('filterTotalMessageNo').textContent = totalMessageNo;
            document.getElementById('totalMessageNo').textContent = totalMessageNo;
            document.getElementById('totalWarningNo').textContent = warnMessageNo;
            document.getElementById('totalErrorNo').textContent = errorMessageNo;
            document.getElementById('filterTotalPluginNo').textContent = data.plugins.length;
            document.getElementById('totalPluginNo').textContent = data.plugins.length;
            document.getElementById('activePluginNo').textContent = activePluginNo;
            document.getElementById('dirtyPluginNo').textContent = dirtyPluginNo;

            /* Now apply translated UI strings. */
            for (var id in data.l10n) {
                var elem = document.getElementById(id);
                if (elem) {
                    elem.textContent = data.l10n[id];
                }
            }

            /* Fill in game row template's game type options. */
            var select = document.getElementById('gameRow').content.querySelector('.name');
            for (var j = 0; j < data.gameTypes.length; ++j) {
                var option = document.createElement('option');
                option.value = data.gameTypes[j];
                option.textContent = data.gameTypes[j];
                select.appendChild(option);
            }

            /* Now fill game lists/table. */
            var gameSelect = document.getElementById('defaultGameSelect');
            var gameMenu = document.getElementById('gameMenu');
            var gameTableBody = document.getElementById('gameTable').getElementsByTagName('tbody')[0];
            for (var i = 0; i < data.games.length; ++i) {
                var option = document.createElement('option');
                option.value = data.games[i].folder;
                option.textContent = data.games[i].name;
                gameSelect.appendChild(option);

                var li = document.createElement('li');
                li.setAttribute('data-action', 'change-game');
                li.setAttribute('data-target', data.games[i].folder);
                li.textContent = data.games[i].name;
                gameMenu.appendChild(li);

                var content = document.getElementById('gameRow').content;
                var clone = document.importNode(content, true);
                gameTableBody.appendChild(clone);
                clone = gameTableBody.lastElementChild;
                clone.getElementsByClassName('name')[0].value = data.games[i].name;
                clone.getElementsByClassName('type')[0].value = data.games[i].type;
                clone.getElementsByClassName('folder')[0].value = data.games[i].folder;
                clone.getElementsByClassName('masterFile')[0].value = data.games[i].masterFile;
                clone.getElementsByClassName('url')[0].value = data.games[i].url;
                clone.getElementsByClassName('branch')[0].value = data.games[i].branch;
                clone.getElementsByClassName('path')[0].value = data.games[i].path;
                clone.getElementsByClassName('registryKey')[0].value = data.games[i].registryKey;
            }
            /* Add row for creating new rows. */
            var content = document.getElementById('gameRow').content;
            var clone = document.importNode(content, true);
            gameTableBody.appendChild(clone);
            clone = gameTableBody.lastElementChild;
            clone.getElementsByClassName('name')[0].placeholder = 'Add new row...';
            hideElement(clone.getElementsByClassName('type')[0]);
            hideElement(clone.getElementsByClassName('fa-trash-o')[0]);

            /* Now fill in language options. */
            var langSelect = document.getElementById('languageSelect');
            for (var i = 0; i < data.languages.length; ++i) {
                var option = document.createElement('option');
                option.value = data.languages[i];
                option.textContent = data.languages[i];
                langSelect.appendChild(option);
            }

            /* Now initialise the rest of the report. */
            setupEventHandlers();
            if (isStorageSupported()) {
                loadSettings();
            }
            showElement(document.getElementsByTagName('section')[0]);
        });
    }
}
processURLParams();
