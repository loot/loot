function openLogLocation(evt) {
    loot.query('openLogLocation').catch(processCefError);
}
function changeGame(evt) {
    /* Check that the selected game isn't the current one. */
    if (evt.target.className.indexOf('core-selected') != -1) {
        return;
    }

    /* Send off a CEF query with the folder name of the new game. */
    showProgress(l10n.jed.translate('Loading game data...').fetch());
    var request = JSON.stringify({
        name: 'changeGame',
        args: [
            evt.currentTarget.getAttribute('value')
        ]
    });
    loot.query(request).then(function(result){
        /* Filters should be re-applied on game change, except the conflicts
           filter. Don't need to deactivate the others beforehand. Strictly not
           deactivating the conflicts filter either, just resetting it's value.
           */
        document.body.removeAttribute('data-conflicts');

        /* Clear the UI of all existing game-specific data. Also
           clear the card and li variables for each plugin object. */
        var globalMessages = document.getElementById('summary').getElementsByTagName('ul')[0];
        while (globalMessages.firstElementChild) {
            globalMessages.removeChild(globalMessages.firstElementChild);
        }

        /* Parse the data sent from C++. */
        try {
            var gameInfo = JSON.parse(result, jsonToPlugin);
            loot.game.folder = gameInfo.folder;
            loot.game.masterlist = gameInfo.masterlist;
            loot.game.globalMessages = gameInfo.globalMessages;
            loot.game.plugins = gameInfo.plugins;

            /* Reset virtual list positions. */
            document.getElementById('cardsNav').scrollToItem(0);
            document.getElementById('main').lastElementChild.scrollToItem(0);

            /* Now update virtual lists. */
            setFilteredUIData();
        } catch (e) {
            console.log(e);
            console.log('changeGame response: ' + result);
        }

        closeProgressDialog();
    }).catch(processCefError);
}
function openReadme(evt) {
    loot.query('openReadme').catch(processCefError);
}
/* Masterlist update process, minus progress dialog. */
function updateMasterlistNoProgress() {
    return loot.query('updateMasterlist').then(JSON.parse).then(function(result){
        if (result) {
            /* Update JS variables. */
            loot.game.masterlist = result.masterlist;
            loot.game.globalMessages = result.globalMessages;

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
            /* Hack to stop cards overlapping. */
            document.getElementById('main').lastElementChild.updateSize();

            toast(l10n.jed.translate('Masterlist updated to revision %s.').fetch(loot.game.masterlist.revision));
        } else {
            toast(l10n.jed.translate('No masterlist update was necessary.').fetch());
        }
    }).catch(processCefError);
}
function updateMasterlist(evt) {
    showProgress(l10n.jed.translate('Updating masterlist...').fetch());
    updateMasterlistNoProgress().then(function(result){
        closeProgressDialog();
    }).catch(processCefError);
}
function sortPlugins(evt) {
    var mlistUpdate;
    if (loot.settings.updateMasterlist) {
        mlistUpdate = updateMasterlistNoProgress();
    } else {
        mlistUpdate = new Promise(function(resolve, reject){
            resolve('');
        });
    }
    mlistUpdate.then(function(){
        showProgress(l10n.jed.translate('Sorting plugins...').fetch());
        loot.query('sortPlugins').then(JSON.parse).then(function(result){
            if (result) {
                loot.game.oldLoadOrder = loot.game.plugins;
                loot.game.loadOrder = [];
                result.forEach(function(plugin){
                    var found = false;
                    for (var i = 0; i < loot.game.plugins.length; ++i) {
                        if (loot.game.plugins[i].name == plugin.name) {
                            loot.game.plugins[i].crc = plugin.crc;
                            loot.game.plugins[i].isEmpty = plugin.isEmpty;

                            loot.game.plugins[i].messages = plugin.messages;
                            loot.game.plugins[i].tags = plugin.tags;
                            loot.game.plugins[i].isDirty = plugin.isDirty;

                            loot.game.loadOrder.push(loot.game.plugins[i]);

                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        loot.game.plugins[i].push(new Plugin(plugin));
                        loot.game.loadOrder.push(loot.game.plugins[i]);
                    }
                });

                if (loot.settings.neverTellMeTheOdds) {
                    /* Array shuffler from <https://stackoverflow.com/questions/6274339/how-can-i-shuffle-an-array-in-javascript> */
                    for(var j, x, i = loot.game.loadOrder.length; i; j = Math.floor(Math.random() * i), x = loadOrder[--i], loot.game.loadOrder[i] = loot.game.loadOrder[j], loot.game.loadOrder[j] = x);
                }

                /* Now update the UI for the new order. */
                loot.game.plugins = loot.game.loadOrder;
                setFilteredUIData();

                /* Now hide the masterlist update buttons, and display the accept and
                   cancel sort buttons. */
                hideElement(document.getElementById('updateMasterlistButton'));
                hideElement(document.getElementById('sortButton'));
                showElement(document.getElementById('applySortButton'));
                showElement(document.getElementById('cancelSortButton'));

                /* Disable changing game. */
                document.getElementById('gameMenu').setAttribute('disabled', '');
                closeProgressDialog();
            }
        }).catch(processCefError);
    }).catch(processCefError);
}
function applySort(evt) {
    var loadOrder = [];
    loot.game.plugins.forEach(function(plugin){
        loadOrder.push(plugin.name);
    });
    var request = JSON.stringify({
        name: 'applySort',
        args: [
            loadOrder
        ]
    });
    return loot.query(request).then(function(result){
        /* Remove old load order storage. */
        delete loot.game.loadOrder;
        delete loot.game.oldLoadOrder;

        /* Now show the masterlist update buttons, and hide the accept and
           cancel sort buttons. */
        showElement(document.getElementById('updateMasterlistButton'));
        showElement(document.getElementById('sortButton'));
        hideElement(document.getElementById('applySortButton'));
        hideElement(document.getElementById('cancelSortButton'));

        /* Enable changing game. */
        document.getElementById('gameMenu').removeAttribute('disabled');
    }).catch(processCefError);
}
function cancelSort(evt) {
    return loot.query('cancelSort').then(function(){
        /* Sort UI elements again according to stored old load order. */
        loot.game.plugins = loot.game.oldLoadOrder;
        setFilteredUIData();
        delete loot.game.loadOrder;
        delete loot.game.oldLoadOrder;

        /* Now show the masterlist update buttons, and hide the accept and
           cancel sort buttons. */
        showElement(document.getElementById('updateMasterlistButton'));
        showElement(document.getElementById('sortButton'));
        hideElement(document.getElementById('applySortButton'));
        hideElement(document.getElementById('cancelSortButton'));

        /* Enable changing game. */
        document.getElementById('gameMenu').removeAttribute('disabled');
    }).catch(processCefError);
}
function redatePlugins(evt) {
    if (evt.target.hasAttribute('disabled')) {
        return;
    }

    showMessageDialog(l10n.jed.translate('Redate Plugins?').fetch(), l10n.jed.translate('This feature is provided so that modders using the Creation Kit may set the load order it uses. A side-effect is that any subscribed Steam Workshop mods will be re-downloaded by Steam. Do you wish to continue?').fetch(), l10n.jed.translate('Redate').fetch(), function(result){
        if (result) {
            loot.query('redatePlugins').then(function(response){
                toast('Plugins were successfully redated.');
            }).catch(processCefError);
        }
    });
}
function clearAllMetadata(evt) {
    showMessageDialog('', l10n.jed.translate('Are you sure you want to clear all existing user-added metadata from all plugins?').fetch(), l10n.jed.translate('Clear').fetch(), function(result){
        if (result) {
            loot.query('clearAllMetadata').then(JSON.parse).then(function(result){
                if (result) {
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

                    toast(l10n.jed.translate('All user-added metadata has been cleared.').fetch());
                }
            }).catch(processCefError);
        }
    });
}
function copyContent(evt) {
    var messages = [];
    var plugins = [];

    if (loot.game) {
        if (loot.game.globalMessages) {
            loot.game.globalMessages.forEach(function(message){
                var m = {};
                messages.push({
                    type: message.type,
                    content: message.content[0].str
                });
            });
        }
        if (loot.game.plugins) {
            loot.game.plugins.forEach(function(plugin){
                plugins.push({
                    name: plugin.name,
                    crc: plugin.crc,
                    version: plugin.version,
                    isActive: plugin.isActive,
                    isEmpty: plugin.isEmpty,
                    loadsBSA: plugin.loadsBSA,

                    modPriority: plugin.modPriority,
                    isGlobalPriority: plugin.isGlobalPriority,
                    messages: plugin.messages,
                    tags: plugin.tags,
                    isDirty: plugin.isDirty
                });
            });
        }
    } else {
        var message = document.getElementById('summary').getElementsByTagName('ul')[0].firstElementChild;
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

    loot.query(request).then(function(){
        toast(l10n.jed.translate("LOOT's content has been copied to the clipboard.").fetch());
    }).catch(processCefError);
}
function switchSidebarTab(evt) {
    if (evt.detail.isSelected) {
        document.getElementById(evt.target.selected).parentElement.selected = evt.target.selected;
    }
}

function showAboutDialog(evt) {
    document.getElementById('about').showModal();
}
function closeSettingsDialog(evt) {
    if (evt.target.classList.contains('accept')) {
        if (!areSettingsValid()) {
            return;
        }

        /* Update the JS variable values. */
        var settings = {
            enableDebugLogging: document.getElementById('enableDebugLogging').checked,
            game: document.getElementById('defaultGameSelect').value,
            games: document.getElementById('gameTable').getRowsData(false),
            language: document.getElementById('languageSelect').value,
            lastGame: loot.settings.lastGame,
            updateMasterlist: document.getElementById('updateMasterlist').checked,
            filters: loot.settings.filters,
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
        }).catch(processCefError);
    } else {
        /* Re-apply the existing settings to the settings dialog elements. */
        loot.updateSettingsUI();
    }
    evt.target.parentElement.close();
}

function showSettingsDialog(evt) {
    document.getElementById('settingsDialog').showModal();
}
function focusSearch(evt) {
    if (evt.ctrlKey && evt.keyCode == 70) { //'f'
        document.getElementById('searchBox').focus();
    }
}
function handleEditorOpen(evt) {
    /* Set up drag 'n' drop event handlers. */
    var elements = document.getElementById('cardsNav').getElementsByTagName('loot-plugin-item');
    for (var i = 0; i < elements.length; ++i) {
        elements[i].draggable = true;
        elements[i].addEventListener('dragstart', elements[i].handleDragStart, false);
    }

    /* Now show editor. */
    evt.target.classList.toggle('flip');

    /* Enable priority hover in plugins list and enable header
       buttons if this is the only editor instance. */
    var numEditors = 0;
    if (document.body.hasAttribute('data-editors')) {
        numEditors = parseInt(document.body.getAttribute('data-editors'), 10);
    }
    ++numEditors;

    if (numEditors == 1) {
        /* Set the edit mode toggle attribute. */
        document.getElementById('cardsNav').setAttribute('data-editModeToggle', '');
        /* Disable the toolbar elements. */
        document.getElementById('wipeUserlistButton').setAttribute('disabled', '');
        document.getElementById('copyContentButton').setAttribute('disabled', '');
        document.getElementById('refreshContentButton').setAttribute('disabled', '');
        document.getElementById('settingsButton').setAttribute('disabled', '');
        document.getElementById('gameMenu').setAttribute('disabled', '');
        document.getElementById('updateMasterlistButton').setAttribute('disabled', '');
        document.getElementById('sortButton').setAttribute('disabled', '');
    }
    document.body.setAttribute('data-editors', numEditors);
    document.getElementById('cardsNav').updateSize();

    return loot.query('editorOpened').catch(processCefError);
}
function handleEditorClose(evt) {
    /* evt.detail is true if the apply button was pressed. */
    if (evt.detail) {
        /* Need to record the editor control values and work out what's
           changed, and update any UI elements necessary. Offload the
           majority of the work to the C++ side of things. */

        var edits = evt.target.readFromEditor(evt.target.data);

        var request = JSON.stringify({
            name: 'editorClosed',
            args: [
                edits
            ]
        });
        loot.query(request).then(JSON.parse).then(function(result){
            if (result) {
                evt.target.data.modPriority = result.modPriority;
                evt.target.data.isGlobalPriority = result.isGlobalPriority;
                evt.target.data.messages = result.messages;
                evt.target.data.tags = result.tags;
                evt.target.data.isDirty = result.isDirty;

                evt.target.data.userlist = edits.userlist;
                delete evt.target.data.editor;
            }
        }).catch(processCefError);
    } else {
        /* Don't need to record changes, but still need to notify C++ side that
           the editor has been closed. */
        loot.query('editorClosed').catch(processCefError);
    }

    /* Now hide editor. */
    evt.target.classList.toggle('flip');

    /* Remove drag 'n' drop event handlers. */
    var elements = document.getElementById('cardsNav').getElementsByTagName('loot-plugin-item');
    for (var i = 0; i < elements.length; ++i) {
        elements[i].removeAttribute('draggable');
        elements[i].removeEventListener('dragstart', elements[i].handleDragStart, false);
    }

    /* Disable priority hover in plugins list and enable header
       buttons if this is the only editor instance. */
    var numEditors = parseInt(document.body.getAttribute('data-editors'), 10);
    --numEditors;

    if (numEditors == 0) {
        document.body.removeAttribute('data-editors');
        /* Set the edit mode toggle attribute. */
        document.getElementById('cardsNav').setAttribute('data-editModeToggle', '');
        /* Re-enable toolbar elements. */
        document.getElementById('wipeUserlistButton').removeAttribute('disabled');
        document.getElementById('copyContentButton').removeAttribute('disabled');
        document.getElementById('refreshContentButton').removeAttribute('disabled');
        document.getElementById('settingsButton').removeAttribute('disabled');
        document.getElementById('gameMenu').removeAttribute('disabled');
        document.getElementById('updateMasterlistButton').removeAttribute('disabled');
        document.getElementById('sortButton').removeAttribute('disabled');
    } else {
        document.body.setAttribute('data-editors', numEditors);
    }
    document.getElementById('cardsNav').updateSize();
}
function handleConflictsFilter(evt) {
    /* evt.detail is true if the filter has been activated. */
    if (evt.detail) {
        /* Un-highlight any existing filter plugin. */
        var cards = document.getElementById('main').getElementsByTagName('loot-plugin-card');
        for (var i = 0; i < cards.length; ++i) {
            cards[i].classList.toggle('highlight', false);
            if (cards[i] != evt.target) {
                cards[i].deactivateConflictFilter();
            }
        }
        evt.target.classList.toggle('highlight', true);
        document.body.setAttribute('data-conflicts', evt.target.getName());
    } else {
        evt.target.classList.toggle('highlight', false);
        document.body.removeAttribute('data-conflicts');
    }
    setFilteredUIData(evt);
}
function handleCopyMetadata(evt) {
    /* evt.detail is the name of the plugin. */
    var request = JSON.stringify({
        name: 'copyMetadata',
        args: [
            evt.target.getName(),
        ]
    });

    loot.query(request).then(function(){
        toast(l10n.jed.translate('The metadata for "%s" has been copied to the clipboard.').fetch(evt.target.getName()));
    }).catch(processCefError);
}
function handleClearMetadata(evt) {
    showMessageDialog('', l10n.jed.translate('Are you sure you want to clear all existing user-added metadata from "%s"?').fetch(evt.target.getName()), l10n.jed.translate('Clear').fetch(), function(result){
        if (result) {
            var request = JSON.stringify({
                name: 'clearPluginMetadata',
                args: [
                    evt.target.getName()
                ]
            });

            loot.query(request).then(JSON.parse).then(function(result){
                if (result) {
                    /* Need to empty the UI-side user metadata. */
                    for (var i = 0; i < loot.game.plugins.length; ++i) {
                        if (loot.game.plugins[i].id == evt.target.id) {
                            loot.game.plugins[i].userlist = undefined;

                            loot.game.plugins[i].modPriority = result.modPriority;
                            loot.game.plugins[i].isGlobalPriority = result.isGlobalPriority;
                            loot.game.plugins[i].messages = result.messages;
                            loot.game.plugins[i].tags = result.tags;
                            loot.game.plugins[i].isDirty = result.isDirty;

                            break;
                        }
                    }
                    toast(l10n.jed.translate('The user-added metadata for "%s" has been cleared.').fetch(evt.target.getName()));
                }
            }).catch(processCefError);
        }
    });
}
function handleSidebarClick(evt) {
    if (evt.target.hasAttribute('data-index')) {
        document.getElementById('main').lastElementChild.scrollToItem(evt.target.getAttribute('data-index'));

        if (evt.type == 'dblclick') {
            var card = document.getElementById(evt.target.getAttribute('data-id'));
            if (!card.classList.contains('flip')) {
                document.getElementById(evt.target.getAttribute('data-id')).onShowEditor();
            }
        }
    }
}
function handleQuit(evt) {
    if (!document.getElementById('applySortButton').classList.contains('hidden')) {
        handleUnappliedChangesClose(l10n.jed.translate('sorted load order').fetch());
    } else if (document.body.hasAttribute('data-editors')) {
        handleUnappliedChangesClose(l10n.jed.translate('metadata edits').fetch());
    } else {
        window.close();
    }
}
function jumpToGeneralInfo(evt) {
    window.location.hash = '';
    document.getElementById('main').scrollTop = 0;
}
function setupEventHandlers() {
    /*Set up handlers for filters.*/
    document.getElementById('hideVersionNumbers').addEventListener('change', toggleDisplayCSS, false);
    document.getElementById('hideCRCs').addEventListener('change', toggleDisplayCSS, false);
    document.getElementById('hideBashTags').addEventListener('change', toggleDisplayCSS, false);
    document.getElementById('hideNotes').addEventListener('change', setFilteredUIData, false);
    document.getElementById('hideDoNotCleanMessages').addEventListener('change', setFilteredUIData, false);
    document.getElementById('hideInactivePlugins').addEventListener('change', setFilteredUIData, false);
    document.getElementById('hideAllPluginMessages').addEventListener('change', setFilteredUIData, false);
    document.getElementById('hideMessagelessPlugins').addEventListener('change', setFilteredUIData, false);
    document.body.addEventListener('loot-filter-conflicts', handleConflictsFilter, false);

    /* Set up event handlers for content filter. */
    document.getElementById('searchBox').addEventListener('change', setFilteredUIData, false);
    window.addEventListener('keyup', focusSearch, false);

    /* Set up handlers for buttons. */
    document.getElementById('redatePluginsButton').addEventListener('click', redatePlugins, false);
    document.getElementById('openLogButton').addEventListener('click', openLogLocation, false);
    document.getElementById('wipeUserlistButton').addEventListener('click', clearAllMetadata, false);
    document.getElementById('copyContentButton').addEventListener('click', copyContent, false);
    document.getElementById('refreshContentButton').addEventListener('click', onContentRefresh, false);
    document.getElementById('settingsButton').addEventListener('click', showSettingsDialog, false);
    document.getElementById('helpButton').addEventListener('click', openReadme, false);
    document.getElementById('aboutButton').addEventListener('click', showAboutDialog, false);
    document.getElementById('quitButton').addEventListener('click', handleQuit, false);
    document.getElementById('updateMasterlistButton').addEventListener('click', updateMasterlist, false);
    document.getElementById('sortButton').addEventListener('click', sortPlugins, false);
    document.getElementById('applySortButton').addEventListener('click', applySort, false);
    document.getElementById('cancelSortButton').addEventListener('click', cancelSort, false);
    document.getElementById('sidebarTabs').addEventListener('core-select', switchSidebarTab, false);
    document.getElementById('jumpToGeneralInfo').addEventListener('click', jumpToGeneralInfo, false);

    /* Set up event handlers for settings dialog. */
    var settings = document.getElementById('settingsDialog');
    settings.getElementsByClassName('accept')[0].addEventListener('click', closeSettingsDialog, false);
    settings.getElementsByClassName('cancel')[0].addEventListener('click', closeSettingsDialog, false);

    /* Set up handler for opening and closing editors. */
    document.body.addEventListener('loot-editor-open', handleEditorOpen, false);
    document.body.addEventListener('loot-editor-close', handleEditorClose, false);
    document.body.addEventListener('loot-copy-metadata', handleCopyMetadata, false);
    document.body.addEventListener('loot-clear-metadata', handleClearMetadata, false);

    document.getElementById('cardsNav').addEventListener('click', handleSidebarClick, false);
    document.getElementById('cardsNav').addEventListener('dblclick', handleSidebarClick, false);
}