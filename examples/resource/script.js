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
            localStorage.setItem(evt.currentTarget.id, evt.currentTarget.checked);
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
            elem.checked = true;
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
    if (document.getElementById('filtersToggle').className.indexOf('current') == -1) {
        showElement(filters);
        evt.target.className += ' current';
    } else {
        hideElement(filters);
        evt.target.className = evt.target.className.replace('current', '');
    }
}
function showSection(evt) {
    elemArr = document.getElementById('nav').querySelectorAll('.button[data-section]');
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
    var elemArr = document.getElementById('filters').getElementsByTagName('input');
    for (var i = 0, z = elemArr.length; i < z; i++) {
        if (evt.target.getAttribute('data-section') == 'plugins') {
            elemArr[i].disabled = false;
        } else {
            elemArr[i].disabled = true;
        }
    }
}
function toggleMessages(evt) {
    var listItems = document.getElementById('plugins').getElementsByTagName('li');
    var i = listItems.length - 1;
    var hiddenNo = parseInt(document.getElementById('hiddenMessageNo').textContent);
    while (i > -1) {
        var spans = listItems[i].getElementsByTagName('span');
        if (spans.length == 0 || spans[0].className.indexOf('mod') == -1) {
            var filterMatch = false;
            if (evt.target.id == 'hideAllPluginMessages') {
                filterMatch = true;
            } else if (evt.target.id == 'hideNotes' && listItems[i].className.indexOf('note') != -1) {
                filterMatch = true;
            } else if (evt.target.id == 'hideBashTags' && listItems[i].className.indexOf('tag') != -1) {
                filterMatch = true;
            } else if (evt.target.id == 'hideDoNotCleanMessages' && listItems[i].className.indexOf('dirty') != -1 && (listItems[i].textContent.indexOf('Do not clean.') != -1 || listItems[i].textContent.indexOf('Do not clean.') != -1)) {
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
    var event = document.createEvent('Event');
    event.initEvent('click', true, true);
    document.getElementById('hideMessagelessPlugins').dispatchEvent(event);
}
function togglePlugins(evt) {
    var plugins = document.getElementById('plugins').getElementsByTagName('ul')[0].childNodes;
    var i = plugins.length - 1;
    var hiddenNo = parseInt(document.getElementById('hiddenPluginNo').textContent);
    while (i > -1) {
        if (plugins[i].nodeType == Node.ELEMENT_NODE) {
            var isMessageless = true, isInactive = true;
            var messages = plugins[i].getElementsByTagName('li');
            var j = messages.length - 1;
            while (j > -1) {
                if (messages[j].className.indexOf('hidden') == -1) {
                    isMessageless = false;
                    break;
                }
                j--;
            }
            if (plugins[i].getElementsByClassName('active').length != 0) {
                isInactive = false;
            }
            if ((document.getElementById('hideMessagelessPlugins').checked && isMessageless) || (document.getElementById('hideInactivePlugins').checked && isInactive)) {
                if (plugins[i].className.indexOf('hidden') == -1) {
                    hiddenNo++;
                    hideElement(plugins[i]);
                }
            } else if (plugins[i].className.indexOf('hidden') != -1) {
                hiddenNo--;
                showElement(plugins[i]);
            }
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
    document.getElementById('hideActiveLabel').addEventListener('click', toggleDisplayCSS, false);
    document.getElementById('hideChecksums').addEventListener('click', toggleDisplayCSS, false);
    document.getElementById('hideNotes').addEventListener('click', toggleMessages, false);
    document.getElementById('hideBashTags').addEventListener('click', toggleMessages, false);
    document.getElementById('hideDoNotCleanMessages').addEventListener('click', toggleMessages, false);
    document.getElementById('hideAllPluginMessages').addEventListener('click', toggleMessages, false);
    document.getElementById('hideInactivePlugins').addEventListener('click', togglePlugins, false);
    document.getElementById('hideMessagelessPlugins').addEventListener('click', togglePlugins, false);
}
function init() {
    setupEventHandlers();
    if (isStorageSupported()) {
        loadSettings();
    }
    var elemArr = document.getElementById('filters').getElementsByTagName('input');
    for (var i = 0, z = elemArr.length; i < z; i++) {
        elemArr[i].disabled = true;
    }
    showElement(document.getElementsByTagName('section')[0])
}
init();
