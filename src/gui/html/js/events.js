function saveFilterState(evt) {
    var request = JSON.stringify({
        name: 'saveFilterState',
        args: [
            evt.target.id,
            evt.target.checked,
        ]
    });
    loot.query(request).catch(processCefError);
}
function onToggleDisplayCSS(evt) {
    var attr = 'data-hide-' + evt.target.getAttribute('data-class');
    if (evt.target.checked) {
        document.getElementById('main').setAttribute(attr, true);
    } else {
        document.getElementById('main').removeAttribute(attr);
    }

    if (evt.target.id != 'hideBashTags') {
        /* Now perform search again. If there is no current search, this won't
           do anything. */
        document.getElementById('searchBar').search();
    }
}
function onToggleBashTags(evt) {
    onToggleDisplayCSS(evt);
    document.getElementById('main').lastElementChild.updateSize();
    /* Now perform search again. If there is no current search, this won't
       do anything. */
    document.getElementById('searchBar').search();
}
function onOpenLogLocation(evt) {
    loot.query('openLogLocation').catch(processCefError);
}
function onChangeGame(evt) {
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
function onOpenReadme(evt) {
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
function onUpdateMasterlist(evt) {
    showProgress(l10n.jed.translate('Updating masterlist...').fetch());
    updateMasterlistNoProgress().then(function(result){
        closeProgressDialog();
    }).catch(processCefError);
}
function onSortPlugins(evt) {
    if (document.body.hasAttribute('data-conflicts')) {
        /* Deactivate any existing plugin conflict filter. */
        for (var i = 0; i < loot.game.plugins.length; ++i) {
            loot.game.plugins[i].isConflictFilterChecked = false;
        }
        /* Un-highlight any existing filter plugin. */
        var cards = document.getElementById('main').getElementsByTagName('loot-plugin-card');
        for (var i = 0; i < cards.length; ++i) {
            cards[i].classList.toggle('highlight', false);
        }
        document.body.removeAttribute('data-conflicts');
    }

    var promise = Promise.resolve('');
    if (loot.settings.updateMasterlist) {
        promise = promise.then(updateMasterlistNoProgress());
    }
    promise.then(function(){
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
                        loot.game.plugins.push(new Plugin(plugin));
                        loot.game.loadOrder.push(loot.game.plugins[loot.game.plugins.length - 1]);
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
function onApplySort(evt) {
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
function onCancelSort(evt) {
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
function onRedatePlugins(evt) {
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
function onClearAllMetadata(evt) {
    showMessageDialog('', l10n.jed.translate('Are you sure you want to clear all existing user-added metadata from all plugins?').fetch(), l10n.jed.translate('Clear').fetch(), function(result){
        if (result) {
            loot.query('clearAllMetadata').then(JSON.parse).then(function(result){
                if (result) {
                    /* Need to empty the UI-side user metadata. */
                    result.forEach(function(plugin){
                        for (var i = 0; i < loot.game.plugins.length; ++i) {
                            if (loot.game.plugins[i].name == plugin.name) {
                                loot.game.plugins[i].userlist = undefined;
                                loot.game.plugins[i].editor = undefined;

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
function onCopyContent(evt) {
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
                    loadsArchive: plugin.loadsArchive,

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
function onCopyLoadOrder(evt) {
    var plugins = [];

    if (loot.game) {
        if (loot.game.plugins) {
            loot.game.plugins.forEach(function(plugin){
                plugins.push(plugin.name);
            });
        }
    }

    var request = JSON.stringify({
        name: 'copyLoadOrder',
        args: [
            plugins
        ]
    });

    loot.query(request).then(function(){
        toast(l10n.jed.translate("The load order has been copied to the clipboard.").fetch());
    }).catch(processCefError);
}
function onSwitchSidebarTab(evt) {
    if (evt.detail.isSelected) {
        document.getElementById(evt.target.selected).parentElement.selected = evt.target.selected;
    }
}
function onShowAboutDialog(evt) {
    document.getElementById('about').showModal();
}
function areSettingsValid() {
    /* Validate inputs individually. */
    var inputs = document.getElementById('settingsDialog').getElementsByTagName('loot-validated-input');
    for (var i = 0; i < inputs.length; ++i) {
        if (!inputs[i].checkValidity()) {
            return false;
        }
    }
    return true;
}
function onCloseSettingsDialog(evt) {
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
function onShowSettingsDialog(evt) {
    document.getElementById('settingsDialog').showModal();
}
function onFocusSearch(evt) {
    if (evt.ctrlKey && evt.keyCode == 70) { //'f'
        document.getElementById('mainToolbar').classList.add('search');
        document.getElementById('searchBar').focusInput();
    }
}
function onEditorOpen(evt) {
    /* Set up drag 'n' drop event handlers. */
    var elements = document.getElementById('cardsNav').getElementsByTagName('loot-plugin-item');
    for (var i = 0; i < elements.length; ++i) {
        elements[i].draggable = true;
        elements[i].addEventListener('dragstart', elements[i].onDragStart, false);
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
function onEditorClose(evt) {
    /* evt.detail is true if the apply button was pressed. */
    var promise;
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
        promise = loot.query(request).then(JSON.parse).then(function(result){
            if (result) {
                evt.target.data.modPriority = result.modPriority;
                evt.target.data.isGlobalPriority = result.isGlobalPriority;
                evt.target.data.messages = result.messages;
                evt.target.data.tags = result.tags;
                evt.target.data.isDirty = result.isDirty;

                evt.target.data.userlist = edits.userlist;

                /* Now perform search again. If there is no current search, this won't
                   do anything. */
                document.getElementById('searchBar').search();
            }
        });
    } else {
        /* Don't need to record changes, but still need to notify C++ side that
           the editor has been closed. */
        promise = loot.query('editorClosed');
    }
    promise.then(function(){
        delete evt.target.data.editor;

        /* Now hide editor. */
        evt.target.classList.toggle('flip');
        evt.target.data.isEditorOpen = false;

        /* Remove drag 'n' drop event handlers. */
        var elements = document.getElementById('cardsNav').getElementsByTagName('loot-plugin-item');
        for (var i = 0; i < elements.length; ++i) {
            elements[i].removeAttribute('draggable');
            elements[i].removeEventListener('dragstart', elements[i].onDragStart, false);
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
    }).catch(processCefError);
}
function onConflictsFilter(evt) {
    /* Deactivate any existing plugin conflict filter. */
    for (var i = 0; i < loot.game.plugins.length; ++i) {
        if (loot.game.plugins[i].id != evt.target.id) {
            loot.game.plugins[i].isConflictFilterChecked = false;
        }
    }
    /* Un-highlight any existing filter plugin. */
    var cards = document.getElementById('main').getElementsByTagName('loot-plugin-card');
    for (var i = 0; i < cards.length; ++i) {
        cards[i].classList.toggle('highlight', false);
    }
    /* evt.detail is true if the filter has been activated. */
    if (evt.detail) {
        document.body.setAttribute('data-conflicts', evt.target.getName());
        evt.target.classList.toggle('highlight', true);
    } else {
        document.body.removeAttribute('data-conflicts');
    }
    setFilteredUIData(evt);
}
function onCopyMetadata(evt) {
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
function onClearMetadata(evt) {
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
                            loot.game.plugins[i].editor = undefined;

                            loot.game.plugins[i].modPriority = result.modPriority;
                            loot.game.plugins[i].isGlobalPriority = result.isGlobalPriority;
                            loot.game.plugins[i].messages = result.messages;
                            loot.game.plugins[i].tags = result.tags;
                            loot.game.plugins[i].isDirty = result.isDirty;

                            break;
                        }
                    }
                    toast(l10n.jed.translate('The user-added metadata for "%s" has been cleared.').fetch(evt.target.getName()));
                    /* Now perform search again. If there is no current search, this won't
                       do anything. */
                    document.getElementById('searchBar').search();
                }
            }).catch(processCefError);
        }
    });
}
function onSidebarClick(evt) {
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
function onQuit(evt) {
    if (!document.getElementById('applySortButton').classList.contains('hidden')) {
        handleUnappliedChangesClose(l10n.jed.translate('sorted load order').fetch());
    } else if (document.body.hasAttribute('data-editors')) {
        handleUnappliedChangesClose(l10n.jed.translate('metadata edits').fetch());
    } else {
        window.close();
    }
}
function onJumpToGeneralInfo(evt) {
    window.location.hash = '';
    document.getElementById('main').scrollTop = 0;
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
                    loot.game.plugins[i].loadsArchive = plugin.loadsArchive;
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
function onSearchOpen(evt) {
    document.getElementById('mainToolbar').classList.add('search');
    document.getElementById('searchBar').focusInput();
}
function onSearchClose(evt) {
    document.getElementById('mainToolbar').classList.remove('search');
}
function setupEventHandlers() {
    /*Set up handlers for filters.*/
    document.getElementById('hideVersionNumbers').addEventListener('change', onToggleDisplayCSS, false);
    document.getElementById('hideVersionNumbers').addEventListener('change', saveFilterState, false);
    document.getElementById('hideCRCs').addEventListener('change', onToggleDisplayCSS, false);
    document.getElementById('hideCRCs').addEventListener('change', saveFilterState, false);
    document.getElementById('hideBashTags').addEventListener('change', onToggleBashTags, false);
    document.getElementById('hideBashTags').addEventListener('change', saveFilterState, false);
    document.getElementById('hideNotes').addEventListener('change', setFilteredUIData, false);
    document.getElementById('hideNotes').addEventListener('change', saveFilterState, false);
    document.getElementById('hideDoNotCleanMessages').addEventListener('change', setFilteredUIData, false);
    document.getElementById('hideDoNotCleanMessages').addEventListener('change', saveFilterState, false);
    document.getElementById('hideInactivePlugins').addEventListener('change', setFilteredUIData, false);
    document.getElementById('hideInactivePlugins').addEventListener('change', saveFilterState, false);
    document.getElementById('hideAllPluginMessages').addEventListener('change', setFilteredUIData, false);
    document.getElementById('hideAllPluginMessages').addEventListener('change', saveFilterState, false);
    document.getElementById('hideMessagelessPlugins').addEventListener('change', setFilteredUIData, false);
    document.getElementById('hideMessagelessPlugins').addEventListener('change', saveFilterState, false);
    document.body.addEventListener('loot-filter-conflicts', onConflictsFilter, false);

    /* Set up event handlers for content filter. */
    document.getElementById('contentFilter').addEventListener('change', setFilteredUIData, false);

    /* Set up handlers for buttons. */
    document.getElementById('redatePluginsButton').addEventListener('click', onRedatePlugins, false);
    document.getElementById('openLogButton').addEventListener('click', onOpenLogLocation, false);
    document.getElementById('wipeUserlistButton').addEventListener('click', onClearAllMetadata, false);
    document.getElementById('copyLoadOrderButton').addEventListener('click', onCopyLoadOrder, false);
    document.getElementById('copyContentButton').addEventListener('click', onCopyContent, false);
    document.getElementById('refreshContentButton').addEventListener('click', onContentRefresh, false);
    document.getElementById('settingsButton').addEventListener('click', onShowSettingsDialog, false);
    document.getElementById('helpButton').addEventListener('click', onOpenReadme, false);
    document.getElementById('aboutButton').addEventListener('click', onShowAboutDialog, false);
    document.getElementById('quitButton').addEventListener('click', onQuit, false);
    document.getElementById('updateMasterlistButton').addEventListener('click', onUpdateMasterlist, false);
    document.getElementById('sortButton').addEventListener('click', onSortPlugins, false);
    document.getElementById('applySortButton').addEventListener('click', onApplySort, false);
    document.getElementById('cancelSortButton').addEventListener('click', onCancelSort, false);
    document.getElementById('sidebarTabs').addEventListener('core-select', onSwitchSidebarTab, false);
    document.getElementById('jumpToGeneralInfo').addEventListener('click', onJumpToGeneralInfo, false);

    /* Set up search event handlers. */
    document.getElementById('showSearch').addEventListener('click', onSearchOpen, false);
    document.getElementById('searchBar').addEventListener('loot-search-close', onSearchClose, false);
    window.addEventListener('keyup', onFocusSearch, false);

    /* Set up event handlers for settings dialog. */
    var settings = document.getElementById('settingsDialog');
    settings.getElementsByClassName('accept')[0].addEventListener('click', onCloseSettingsDialog, false);
    settings.getElementsByClassName('cancel')[0].addEventListener('click', onCloseSettingsDialog, false);

    /* Set up handler for opening and closing editors. */
    document.body.addEventListener('loot-editor-open', onEditorOpen, false);
    document.body.addEventListener('loot-editor-close', onEditorClose, false);
    document.body.addEventListener('loot-copy-metadata', onCopyMetadata, false);
    document.body.addEventListener('loot-clear-metadata', onClearMetadata, false);

    document.getElementById('cardsNav').addEventListener('click', onSidebarClick, false);
    document.getElementById('cardsNav').addEventListener('dblclick', onSidebarClick, false);
}
