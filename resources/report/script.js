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
        if (!isVisible(element)) {
            element.className = element.className.replace('hidden', '');
        }
    }
}
function hideElement(element) {
    if (element != null) {
        if (isVisible(element)) {
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
    /*Set up handlers for filters.*/
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
                content.children[0].children[0].textContent = data.plugins[i].name;
                content.children[0].children[0].href = '#' + data.plugins[i].name.replace(/\s+/g, '');

                clone = document.importNode(content, true);
                pluginsNav.appendChild(clone);

                var content = document.getElementById('pluginSection').content;
                var section = content.children[0];

                section.setAttribute('data-active', data.plugins[i].isActive);
                section.id = data.plugins[i].name.replace(/\s+/g, '');

                if (data.plugins[i].isActive) {
                    ++activePluginNo;
                }

                if (data.plugins[i].isDirty) {
                    ++dirtyPluginNo;
                }

                section.children[0].textContent = data.plugins[i].name;

                section.children[1].textContent = 'CRC: ' + data.plugins[i].crc;

                if (data.plugins[i].version) {
                    section.children[2].textContent = data.plugins[i].version;
                } else {
                    section.children[2].className += ' hidden';
                }

                if (data.plugins[i].tagsAdd && data.plugins[i].tagsAdd.length != 0) {
                    section.children[3].textContent = data.plugins[i].tagsAdd.join(', ');
                } else {
                    section.children[3].className += ' hidden';
                }

                if (data.plugins[i].tagRemove && data.plugins[i].tagRemove.length != 0) {
                    section.children[4].textContent = data.plugins[i].tagsRemove.join(', ');
                } else {
                    section.children[4].className += ' hidden';
                }


                if (data.plugins[i].messages && data.plugins[i].messages.length != 0) {
                    for (var j = 0; j < data.plugins[i].messages.length; ++j) {
                        var messageLi = document.createElement('li');
                        messageLi.className = data.plugins[i].messages[j].type;
                        /* innerHTML is open to abuse, but for hyperlinking it's too useful. */
                        messageLi.innerHTML = data.plugins[i].messages[j].content;
                        section.children[5].appendChild(messageLi);

                        if (messageLi.className == 'warn') {
                            warnMessageNo++;
                        } else if (messageLi.className == 'error') {
                            errorMessageNo++;
                        }
                        totalMessageNo++;
                    }
                } else {
                    section.children[5].className += ' hidden';
                }
                clone = document.importNode(content, true);
                pluginsList.appendChild(clone);
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

var menus = document.getElementsByTagName('header')[0].getElementsByTagName('li');
var overlay = document.getElementById('overlay');

for (var i = 0; i < menus.length; ++i) {
    menus[i].addEventListener('click', function(evt) {
        var action = evt.currentTarget.getAttribute('data-action');
        var target = document.getElementById(evt.currentTarget.getAttribute('data-target'));
        if (action == 'view-ui') {
            if (isVisible(target)) {
                hideElement(target);
                if (target.getAttribute('data-overlay')) {
                    overlay.setAttribute('data-dialog', '');
                    hideElement(overlay);
                }
            } else {
                showElement(target);
                if (target.getAttribute('data-overlay') == '1') {
                    overlay.setAttribute('data-dialog', target.id);
                    showElement(overlay);
                }
            }
        }
    }, false);

    overlay.addEventListener('click', function(evt) {
        hideElement(overlay);
        hideElement(document.getElementById(overlay.getAttribute('data-dialog')));
    }, false);
}