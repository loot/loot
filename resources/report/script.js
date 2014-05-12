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
function showElement(element) {
    if (element != null) {
        if (element.className.indexOf('hidden') != -1) {
            element.className = element.className.replace('hidden', '');
        }
    }
}
function hideElement(element) {
    if (element != null) {
        if (element.className.indexOf('hidden') == -1) {
            element.className += ' hidden';
        }
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
function showSection(evt) {
    var elemArr = document.getElementById('nav').querySelectorAll('.button[data-section]');
    var i = elemArr.length - 1;
    while (i > -1) {
        hideElement(document.getElementById(elemArr[i].getAttribute('data-section')));
        i--;
    }
    showElement(document.getElementById(evt.target.getAttribute('data-section')));
    var elem = document.querySelector('#nav div.current[data-section]');
    if (elem != null) {
        elem.className = elem.className.replace('current', '');
    }
    if (evt.target.className.indexOf('current') == -1) {
        evt.target.className += ' current';
    } /*Also enable/disable filters based on current page.*/
    if (evt.target.getAttribute('data-section') == 'plugins') {
        showElement(document.getElementById('filtersToggle'));
        if (document.getElementById('filtersToggle').className.indexOf('current') != -1) {
            showElement(filters);
        }
    } else {
        hideElement(document.getElementById('filtersToggle'));
        hideElement(document.getElementById('filters'));
    }
}
function toggleMessages(evt) {
    var listItems = document.getElementById('plugins').getElementsByTagName('li');
    var i = listItems.length - 1;
    var hiddenNo = parseInt(document.getElementById('hiddenMessageNo').textContent, 10);
    while (i > -1) {
        var divs = listItems[i].getElementsByTagName('div');
        if (divs.length == 0) {
            var filterMatch = false;
            if (evt.target.id == 'hideAllPluginMessages') {
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
        i--;
    }
	document.getElementById('hiddenMessageNo').textContent = hiddenNo;
	togglePlugins(evt);
}
function togglePlugins(evt) {
    var plugins = document.getElementById('plugins').getElementsByTagName('ul')[0].children;
    var i = plugins.length - 1;
    var hiddenNo = parseInt(document.getElementById('hiddenPluginNo').textContent, 10);
    while (i > -1) {
        var isMessageless = true;
        var messages = plugins[i].getElementsByTagName('li');
        var j = messages.length - 1;
        while (j > -1) {
            if (messages[j].className.indexOf('hidden') == -1) {
                isMessageless = false;
                break;
            }
            j--;
        }
        if (document.getElementById('hideMessagelessPlugins').checked && isMessageless) {
            if (plugins[i].className.indexOf('hidden') == -1) {
                hiddenNo++;
                hideElement(plugins[i]);
            }
        } else if (plugins[i].className.indexOf('hidden') !== -1) {
            hiddenNo--;
            showElement(plugins[i]);
        }
        i--;
    }
    document.getElementById('hiddenPluginNo').textContent = hiddenNo;
}
function setupEventHandlers() {
    var i, elemArr;
    if (isStorageSupported()) { /*Set up filter value and CSS setting storage read/write handlers.*/
        elemArr = document.getElementById('filters').getElementsByTagName('input');
        i = elemArr.length - 1;
        while (i > -1) {
            elemArr[i].addEventListener('click', saveCheckboxState, false);
            i--;
        }
    }
    document.getElementById('filtersToggle').addEventListener('click', toggleFilters, false);
    /*Set up handlers for section display.*/
    elemArr = document.getElementById('nav').querySelectorAll('.button[data-section]');
    var i = elemArr.length - 1;
    while (i > -1) {
        elemArr[i].addEventListener('click', showSection, false);
        i--;
    }
    document.getElementById('hideVersionNumbers').addEventListener('click', toggleDisplayCSS, false);
    document.getElementById('hideCRCs').addEventListener('click', toggleDisplayCSS, false);
    document.getElementById('hideBashTags').addEventListener('click', toggleDisplayCSS, false);
    document.getElementById('hideNotes').addEventListener('click', toggleMessages, false);
    document.getElementById('hideDoNotCleanMessages').addEventListener('click', toggleMessages, false);
    document.getElementById('hideInactivePluginMessages').addEventListener('click', toggleMessages, false);
    document.getElementById('hideAllPluginMessages').addEventListener('click', toggleMessages, false);
    document.getElementById('hideMessagelessPlugins').addEventListener('click', togglePlugins, false);
}
function processURLParams() {
    /* Get the data path from the URL and load it. */
    var pos = document.URL.indexOf("?data=");
    if (pos != -1) {
        var datapath = 'file:///' + document.URL.substring(pos+6);
        console.log(datapath);
        require([datapath], function(){
            var totalMessageNo = 0;
            var warnMessageNo = 0;
            var errorMessageNo = 0;
            /* Fill report with data. */
            document.getElementById('lootVersion').textContent = data.lootVersion;
            document.getElementById('masterlistRevision').textContent = data.masterlist.revision;
            document.getElementById('masterlistDate').textContent = data.masterlist.date;
            document.getElementById('masterlistUpdating').textContent = data.masterlist.updaterEnabled;
            var generalMessagesList = document.getElementById('generalMessagesList');
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
            var pluginsList = document.getElementById('pluginsList');
            for (var i = 0; i < data.plugins.length; ++i) {
                var li = document.createElement('li');

                li.setAttribute('data-active', data.plugins[i].isActive);

                var mod = document.createElement('div');
                mod.className = 'mod';
                mod.textContent = data.plugins[i].name;
                li.appendChild(mod);

                var crc = document.createElement('div');
                crc.className = 'crc';
                crc.textContent = 'CRC: ' + data.plugins[i].crc;
                li.appendChild(crc);

                if (data.plugins[i].version) {
                    var version = document.createElement('div');
                    version.className = 'version';
                    version.textContent = data.plugins[i].version;
                    li.appendChild(version);
                }

                if (data.plugins[i].tagsAdd && data.plugins[i].tagsAdd.length != 0) {
                    var tagAdd = document.createElement('div');
                    tagAdd.className = 'tag add';
                    tagAdd.textContent = data.plugins[i].tagsAdd.join(', ');
                    li.appendChild(tagAdd);
                }

                if (data.plugins[i].tagRemove && data.plugins[i].tagRemove.length != 0) {
                    var tagRemove = document.createElement('div');
                    tagRemove.className = 'tag remove';
                    tagRemove.textContent = data.plugins[i].tagsRemove.join(', ');
                    li.appendChild(tagRemove);
                }

                if (data.plugins[i].messages && data.plugins[i].messages.length != 0) {
                    var ul = document.createElement('ul');
                    for (var j = 0; j < data.plugins[i].messages.length; ++j) {
                        var messageLi = document.createElement('li');
                        messageLi.className = data.plugins[i].messages[j].type;
                        /* innerHTML is open to abuse, but for hyperlinking it's too useful. */
                        messageLi.innerHTML = data.plugins[i].messages[j].content;
                        ul.appendChild(messageLi);

                        if (messageLi.className == 'warn') {
                            warnMessageNo++;
                        } else if (messageLi.className == 'error') {
                            errorMessageNo++;
                        }
                        totalMessageNo++;
                    }
                    li.appendChild(ul);
                }
                pluginsList.appendChild(li);
            }
            document.getElementById('totalMessageNo').textContent = totalMessageNo;
            document.getElementById('filterTotalMessageNo').textContent = totalMessageNo;
            document.getElementById('totalPluginNo').textContent = data.plugins.length;
            document.getElementById('totalWarningNo').textContent = warnMessageNo;
            document.getElementById('totalErrorNo').textContent = errorMessageNo;

            /* Now apply translated UI strings. */
            for (var id in data.l10n) {
                var elem = document.getElementById(id);
                if (elem) {
                    elem.textContent = data.l10n[id];
                }
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