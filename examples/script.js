'use strict';
// shim for older browsers:
if (!document.getElementsByClassName) {
    document.getElementsByClassName = (function(){
        // Utility function to traverse the DOM:
        function traverse (node, callback) {
            callback(node);
            for (var i=0;i < node.childNodes.length; i++) {
                traverse(node.childNodes[i],callback);
            }
        }

        // Actual definition of getElementsByClassName
        return function (name) {
            var result = [];
            traverse(document.body,function(node){
                if (node.className && node.className.indexOf(name) != -1) {
                    result.push(node);
                }
            });
            return result;
        }
    })()
}
function showElement(element) {
    if (element != null) {
        if (element.className.indexOf('hidden') != -1) {
            element.className = element.className.replace(' hidden', '');
        } else if (element.className.indexOf('visible') == -1) {
            element.className += ' visible';
        }
    }
}
function hideElement(element) {
    if (element != null) {
        if (element.className.indexOf('visible') != -1) {
            element.className = element.className.replace('visible', '');
        } else if (element.className.indexOf('hidden') == -1) {
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
            e[i].className = e[i].className.replace(' hidden', '');
        }
    }
}
function toggleFilters(evt) {
    if (document.getElementById('filtersToggle').className.indexOf('current') == -1) {
        showElement(filters);
        evt.target.className += ' current';
    } else {
        hideElement(filters);
        evt.target.className = evt.target.className.replace(' current', '');
    }
}
function showSection(evt) {
    hideElement(document.querySelector('#summary,#plugins'));
    showElement(document.getElementById(evt.target.getAttribute('data-section')));
    var elem = document.querySelector('#nav div.current[data-section]');
    if (elem != null) {
        elem.className = elem.className.replace(' current', '');
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
    var elemArr = document.getElementById('filters').getElementsByTagName('input');
    for (var i = 0, z = elemArr.length; i < z; i++) {
        elemArr[i].disabled = true;
    }
    showElement(document.getElementsByTagName('section')[0])
}
init();
