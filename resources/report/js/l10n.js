'use strict';

(function (root, factory) {
    if (typeof define === 'function' && define.amd) {
        // AMD. Register as an anonymous module.
        define(['bower_components/Jed/jed', 'bower_components/jed-gettext-parser/jedGettextParser'], factory);
    } else {
        // Browser globals
        root.amdWeb = factory(root.jed, root.jedGettextParser);
    }
}(this, function (jed, jedGettextParser) {

    var defaultData = {
        "messages": {
            "": {
                "domain" : "messages",
                "lang" : "en",
                "plural_forms" : "nplurals=2; plural=(n != 1);"
            }
        }
    };

    return {

        loadLocaleData: function(locale) {
            if (locale == 'en') {
                return new Promise(function(resolve, reject){
                    resolve(defaultData);
                });
            }

            var url = 'loot://l10n/' + locale + '/LC_MESSAGES/loot.mo';

            return new Promise(function(resolve, reject){
                var xhr = new XMLHttpRequest();
                xhr.open('GET', url);
                xhr.responseType = 'arraybuffer';
                xhr.addEventListener('readystatechange', function(evt){
                    if (evt.target.readyState == 4) {
                        /* Status is 0 for local file URL loading. */
                        if (evt.target.status >= 200 && evt.target.status < 400) {
                            resolve(jedGettextParser.mo.parse(evt.target.response));
                        } else {
                            reject(new Error('XHR Error'));
                        }
                    }
                }, false);
                xhr.send();
            });
        },

        translateStaticText: function(l10n) {
            /* Plugin card template. */
            var pluginCard = document.querySelector('link[rel="import"][href$="loot-plugin-card.html"]');
            if (pluginCard) {
                pluginCard = pluginCard.import.querySelector('template').content;
            } else {
                pluginCard = document.querySelector('polymer-element[name="loot-plugin-card"]').querySelector('template').content;
            }

            pluginCard.getElementById('activeTick').setAttribute('label', l10n.translate("Active Plugin").fetch());
            pluginCard.getElementById('isMaster').setAttribute('label', l10n.translate("Master File").fetch());
            pluginCard.getElementById('emptyPlugin').setAttribute('label', l10n.translate("Empty Plugin").fetch());
            pluginCard.getElementById('loadsBSA').setAttribute('label', l10n.translate("Loads BSA").fetch());
            pluginCard.getElementById('hasUserEdits').setAttribute('label', l10n.translate("Has User Metadata").fetch());

            pluginCard.getElementById('showOnlyConflicts').previousElementSibling.textContent = l10n.translate("Show Only Conflicts").fetch();
            pluginCard.getElementById('editMetadata').lastChild.textContent = l10n.translate("Edit Metadata").fetch();
            pluginCard.getElementById('copyMetadata').lastChild.textContent = l10n.translate("Copy Metadata").fetch();
            pluginCard.getElementById('clearMetadata').lastChild.textContent = l10n.translate("Clear User Metadata").fetch();

            /* Plugin editor template. */
            var pluginEditor = document.querySelector('link[rel="import"][href$="loot-plugin-card.html"]');
            if (pluginEditor) {
                pluginEditor = pluginEditor.import.querySelector('link[rel="import"][href$="loot-plugin-editor.html"]').import.querySelector('template').content;
            } else {
                pluginEditor = document.querySelector('polymer-element[name="loot-plugin-editor"]').querySelector('template').content;
            }

            pluginEditor.getElementById('activeTick').setAttribute('label', l10n.translate("Active Plugin").fetch());
            pluginEditor.getElementById('isMaster').setAttribute('label', l10n.translate("Master File").fetch());
            pluginEditor.getElementById('emptyPlugin').setAttribute('label', l10n.translate("Empty Plugin").fetch());
            pluginEditor.getElementById('loadsBSA').setAttribute('label', l10n.translate("Loads BSA").fetch());

            pluginEditor.getElementById('enableEdits').previousElementSibling.textContent = l10n.translate("Enable Edits").fetch();
            pluginEditor.getElementById('globalPriority').parentElement.parentElement.setAttribute('label', l10n.translate("Global priorities are compared against all other plugins. Normal priorities are compared against only conflicting plugins.").fetch());
            pluginEditor.getElementById('globalPriority').previousElementSibling.textContent = l10n.translate("Global Priority").fetch();
            pluginEditor.getElementById('priorityValue').parentElement.previousElementSibling.textContent = l10n.translate("Priority Value").fetch();

            pluginEditor.getElementById('tableTabs').querySelector('[data-for=main]').textContent = l10n.translate("Main").fetch();
            pluginEditor.getElementById('tableTabs').querySelector('[data-for=loadAfter]').textContent = l10n.translate("Load After").fetch();
            pluginEditor.getElementById('tableTabs').querySelector('[data-for=req]').textContent = l10n.translate("Requirements").fetch();
            pluginEditor.getElementById('tableTabs').querySelector('[data-for=inc]').textContent = l10n.translate("Incompatibilities").fetch();
            pluginEditor.getElementById('tableTabs').querySelector('[data-for=message]').textContent = l10n.translate("Messages").fetch();
            pluginEditor.getElementById('tableTabs').querySelector('[data-for=tags]').textContent = l10n.translate("Bash Tags").fetch();
            pluginEditor.getElementById('tableTabs').querySelector('[data-for=dirty]').textContent = l10n.translate("Dirty Info").fetch();
            pluginEditor.getElementById('tableTabs').querySelector('[data-for=locations]').textContent = l10n.translate("Locations").fetch();

            pluginEditor.getElementById('loadAfter').querySelector('th:first-child').textContent = l10n.translate("Filename").fetch();
            pluginEditor.getElementById('loadAfter').querySelector('th:nth-child(2)').textContent = l10n.translate("Display Name").fetch();
            pluginEditor.getElementById('loadAfter').querySelector('th:nth-child(3)').textContent = l10n.translate("Condition").fetch();

            pluginEditor.getElementById('req').querySelector('th:first-child').textContent = l10n.translate("Filename").fetch();
            pluginEditor.getElementById('req').querySelector('th:nth-child(2)').textContent = l10n.translate("Display Name").fetch();
            pluginEditor.getElementById('req').querySelector('th:nth-child(3)').textContent = l10n.translate("Condition").fetch();

            pluginEditor.getElementById('inc').querySelector('th:first-child').textContent = l10n.translate("Filename").fetch();
            pluginEditor.getElementById('inc').querySelector('th:nth-child(2)').textContent = l10n.translate("Display Name").fetch();
            pluginEditor.getElementById('inc').querySelector('th:nth-child(3)').textContent = l10n.translate("Condition").fetch();

            pluginEditor.getElementById('message').querySelector('th:first-child').textContent = l10n.translate("Type").fetch();
            pluginEditor.getElementById('message').querySelector('th:nth-child(2)').textContent = l10n.translate("Content").fetch();
            pluginEditor.getElementById('message').querySelector('th:nth-child(3)').textContent = l10n.translate("Condition").fetch();
            pluginEditor.getElementById('message').querySelector('th:nth-child(4)').textContent = l10n.translate("Language").fetch();

            pluginEditor.getElementById('tags').querySelector('th:first-child').textContent = l10n.translate("Add/Remove").fetch();
            pluginEditor.getElementById('tags').querySelector('th:nth-child(2)').textContent = l10n.translate("Bash Tag").fetch();
            pluginEditor.getElementById('tags').querySelector('th:nth-child(3)').textContent = l10n.translate("Condition").fetch();

            pluginEditor.getElementById('dirty').querySelector('th:first-child').textContent = l10n.translate("CRC").fetch();
            pluginEditor.getElementById('dirty').querySelector('th:nth-child(2)').textContent = l10n.translate("ITM Count").fetch();
            pluginEditor.getElementById('dirty').querySelector('th:nth-child(3)').textContent = l10n.translate("Deleted References").fetch();
            pluginEditor.getElementById('dirty').querySelector('th:nth-child(4)').textContent = l10n.translate("Deleted Navmeshes").fetch();
            pluginEditor.getElementById('dirty').querySelector('th:nth-child(5)').textContent = l10n.translate("Cleaning Utility").fetch();

            pluginEditor.getElementById('locations').querySelector('th:first-child').textContent = l10n.translate("URL").fetch();
            pluginEditor.getElementById('locations').querySelector('th:nth-child(2)').textContent = l10n.translate("Name").fetch();

            pluginEditor.getElementById('accept').parentElement.setAttribute('label', l10n.translate("Apply").fetch());
            pluginEditor.getElementById('cancel').parentElement.setAttribute('label', l10n.translate("Cancel").fetch());

            /* Plugin List Item Template */
            var pluginItem = document.querySelector('link[rel="import"][href$="loot-plugin-item.html"]');
            if (pluginItem) {
                pluginItem = pluginItem.import.querySelector('template').content;
            } else {
                pluginItem = document.querySelector('polymer-element[name="loot-plugin-item"]').querySelector('template').content;
            }
            pluginItem.querySelector('#secondary core-tooltip').setAttribute('label', l10n.translate("Global Priority").fetch());
            pluginItem.getElementById('hasUserEditsTooltip').textContent = l10n.translate("Has User Metadata").fetch();
            pluginItem.getElementById('editorIsOpenTooltip').textContent = l10n.translate("Editor Is Open").fetch();

            /* File row template */
            var fileRow = document.querySelector('link[rel="import"][href$="editable-table.html"]');
            if (fileRow) {
                fileRow = fileRow.import.querySelector('#fileRow').content;
            } else {
                fileRow = document.querySelector('#fileRow').content;
            }
            fileRow.querySelector('loot-validated-input').setAttribute('error', l10n.translate("A filename is required.").fetch());
            fileRow.querySelector('core-tooltip').setAttribute('label', l10n.translate("Delete Row").fetch());

            /* Message row template */
            var messageRow = document.querySelector('link[rel="import"][href$="editable-table.html"]');
            if (messageRow) {
                messageRow = messageRow.import.querySelector('#messageRow').content;
            } else {
                messageRow = document.querySelector('#messageRow').content;
            }
            messageRow.querySelector('.type').children[0].textContent = l10n.translate("Note").fetch();
            messageRow.querySelector('.type').children[1].textContent = l10n.translate("Warning").fetch();
            messageRow.querySelector('.type').children[2].textContent = l10n.translate("Error").fetch();
            messageRow.querySelector('loot-validated-input').setAttribute('error', l10n.translate("A content string is required.").fetch());
            messageRow.querySelector('core-tooltip').setAttribute('label', l10n.translate("Delete Row").fetch());

            /* Tag row template */
            var tagRow = document.querySelector('link[rel="import"][href$="editable-table.html"]');
            if (tagRow) {
                tagRow = tagRow.import.querySelector('#tagRow').content;
            } else {
                tagRow = document.querySelector('#tagRow').content;
            }
            tagRow.querySelector('.type').children[0].textContent = l10n.translate("Add").fetch();
            tagRow.querySelector('.type').children[1].textContent = l10n.translate("Remove").fetch();
            tagRow.querySelector('loot-validated-input').setAttribute('error', l10n.translate("A name is required.").fetch());
            tagRow.querySelector('core-tooltip').setAttribute('label', l10n.translate("Delete Row").fetch());

            /* Dirty Info row template */
            var dirtyInfoRow = document.querySelector('link[rel="import"][href$="editable-table.html"]');
            if (dirtyInfoRow) {
                dirtyInfoRow = dirtyInfoRow.import.querySelector('#dirtyInfoRow').content;
            } else {
                dirtyInfoRow = document.querySelector('#dirtyInfoRow').content;
            }
            dirtyInfoRow.querySelector('loot-validated-input.crc').setAttribute('error', l10n.translate("A CRC is required.").fetch());
            dirtyInfoRow.querySelector('loot-validated-input.itm').setAttribute('error', l10n.translate("Values must be integers.").fetch());
            dirtyInfoRow.querySelector('loot-validated-input.udr').setAttribute('error', l10n.translate("Values must be integers.").fetch());
            dirtyInfoRow.querySelector('loot-validated-input.nav').setAttribute('error', l10n.translate("Values must be integers.").fetch());
            dirtyInfoRow.querySelector('loot-validated-input.util').setAttribute('error', l10n.translate("A utility name is required.").fetch());
            dirtyInfoRow.querySelector('core-tooltip').setAttribute('label', l10n.translate("Delete Row").fetch());

            /* Location row template */
            var locationRow = document.querySelector('link[rel="import"][href$="editable-table.html"]');
            if (locationRow) {
                locationRow = locationRow.import.querySelector('#locationRow').content;
            } else {
                locationRow = document.querySelector('#locationRow').content;
            }
            locationRow.querySelector('loot-validated-input').setAttribute('error', l10n.translate("A link is required.").fetch());
            locationRow.querySelector('core-tooltip').setAttribute('label', l10n.translate("Delete Row").fetch());

            /* Game row template */
            var gameRow = document.querySelector('link[rel="import"][href$="editable-table.html"]');
            if (gameRow) {
                gameRow = gameRow.import.querySelector('#gameRow').content;
            } else {
                gameRow = document.querySelector('#gameRow').content;
            }
            gameRow.querySelector('loot-validated-input.name').setAttribute('error', l10n.translate("A name is required.").fetch());
            gameRow.querySelector('loot-validated-input.folder').setAttribute('error', l10n.translate("A folder is required.").fetch());
            gameRow.querySelector('core-tooltip').setAttribute('label', l10n.translate("Delete Row").fetch());

            /* New row template */
            var newRow = document.querySelector('link[rel="import"][href$="editable-table.html"]');
            if (newRow) {
                newRow = newRow.import.querySelector('#newRow').content;
            } else {
                newRow = document.querySelector('#newRow').content;
            }
            newRow.querySelector('core-tooltip').setAttribute('label', l10n.translate("Add New Row").fetch());

            /* Main toolbar */
            document.getElementById('jumpToGeneralInfo').parentElement.label = l10n.translate("Jump To General Information").fetch();
            document.getElementById('sortButton').parentElement.label = l10n.translate("Sort Plugins").fetch();
            document.getElementById('updateMasterlistButton').parentElement.label = l10n.translate("Update Masterlist").fetch();
            document.getElementById('applySortButton').textContent = l10n.translate("Apply").fetch();
            document.getElementById('cancelSortButton').textContent = l10n.translate("Cancel").fetch();
            document.getElementById('showSearch').parentElement.label = l10n.translate("Search Cards").fetch();

            /* Toolbar menu */
            document.getElementById('redatePluginsButton').lastElementChild.textContent = l10n.translate("Redate Plugins").fetch();
            document.getElementById('openLogButton').lastElementChild.textContent = l10n.translate("Open Debug Log Location").fetch();
            document.getElementById('wipeUserlistButton').lastElementChild.textContent = l10n.translate("Clear All User Metadata").fetch();
            document.getElementById('copyLoadOrderButton').lastElementChild.textContent = l10n.translate("Copy Load Order").fetch();
            document.getElementById('copyContentButton').lastElementChild.textContent = l10n.translate("Copy Content").fetch();
            document.getElementById('refreshContentButton').lastElementChild.textContent = l10n.translate("Refresh Content").fetch();
            document.getElementById('helpButton').lastElementChild.textContent = l10n.translate("View Documentation").fetch();
            document.getElementById('aboutButton').lastElementChild.textContent = l10n.translate("About").fetch();
            document.getElementById('settingsButton').lastElementChild.textContent = l10n.translate("Settings").fetch();
            document.getElementById('quitButton').lastElementChild.textContent = l10n.translate("Quit").fetch();

            /* Search bar */
            document.getElementById('searchBar').shadowRoot.getElementById('search').label = l10n.translate("Search cards").fetch();

            /* Nav items */
            document.getElementById('sidebarTabs').firstElementChild.textContent = l10n.translate("Plugins").fetch();
            document.getElementById('sidebarTabs').firstElementChild.nextElementSibling.textContent = l10n.translate("Filters").fetch();
            document.getElementById('contentFilter').parentElement.label = l10n.translate("Press Enter or click outside the input to set the filter.").fetch();
            document.getElementById('contentFilter').label = l10n.translate("Filter content").fetch();

            /* Filters */
            document.getElementById('hideVersionNumbers').label = l10n.translate("Hide version numbers").fetch();
            document.getElementById('hideCRCs').label = l10n.translate("Hide CRCs").fetch();
            document.getElementById('hideBashTags').label = l10n.translate("Hide Bash Tags").fetch();
            document.getElementById('hideNotes').label = l10n.translate("Hide notes").fetch();
            document.getElementById('hideDoNotCleanMessages').label = l10n.translate("Hide 'Do not clean' messages").fetch();
            document.getElementById('hideAllPluginMessages').label = l10n.translate("Hide all plugin messages").fetch();
            document.getElementById('hideInactivePlugins').label = l10n.translate("Hide inactive plugins").fetch();
            document.getElementById('hideMessagelessPlugins').label = l10n.translate("Hide messageless plugins").fetch();
            document.getElementById('hiddenPluginsTxt').textContent = l10n.translate("Hidden plugins:").fetch();
            document.getElementById('hiddenMessagesTxt').textContent = l10n.translate("Hidden messages:").fetch();

            /* Summary */
            document.getElementById('summary').firstElementChild.textContent = l10n.translate("General Information").fetch();
            document.getElementById('masterlistRevision').previousElementSibling.textContent = l10n.translate("Masterlist Revision").fetch();
            document.getElementById('masterlistDate').previousElementSibling.textContent = l10n.translate("Masterlist Date").fetch();
            document.getElementById('totalWarningNo').previousElementSibling.textContent = l10n.translate("Warnings").fetch();
            document.getElementById('totalErrorNo').previousElementSibling.textContent = l10n.translate("Errors").fetch();
            document.getElementById('totalMessageNo').previousElementSibling.textContent = l10n.translate("Total Messages").fetch();
            document.getElementById('activePluginNo').previousElementSibling.textContent = l10n.translate("Active Plugins").fetch();
            document.getElementById('dirtyPluginNo').previousElementSibling.textContent = l10n.translate("Dirty Plugins").fetch();
            document.getElementById('totalPluginNo').previousElementSibling.textContent = l10n.translate("Total Plugins").fetch();

            /* Settings dialog */
            document.getElementById('settingsDialog').heading = l10n.translate("Settings").fetch();

            var defaultGameSelect = document.getElementById('defaultGameSelect');
            defaultGameSelect.previousElementSibling.textContent = l10n.translate("Default Game").fetch();
            defaultGameSelect.firstElementChild.textContent = l10n.translate("Autodetect").fetch();
            /* The selected text doesn't update, so force that translation. */
            defaultGameSelect.shadowRoot.querySelector('paper-dropdown-menu').selectedItemLabel = defaultGameSelect.shadowRoot.querySelector('core-menu').selectedItem.textContent;

            document.getElementById('languageSelect').previousElementSibling.textContent = l10n.translate("Language").fetch();
            document.getElementById('languageSelect').previousElementSibling.label = l10n.translate("Language changes will be applied after LOOT is restarted.").fetch();

            document.getElementById('enableDebugLogging').previousElementSibling.textContent = l10n.translate("Enable debug logging").fetch();
            document.getElementById('enableDebugLogging').parentElement.label = l10n.translate("The output is logged to the LOOTDebugLog.txt file.").fetch();

            document.getElementById('updateMasterlist').previousElementSibling.textContent = l10n.translate("Update masterlist before sorting").fetch();

            var gameTable = document.getElementById('gameTable');
            gameTable.querySelector('th:first-child').textContent = l10n.translate("Name").fetch();
            gameTable.querySelector('th:nth-child(2)').textContent = l10n.translate("Base Game").fetch();
            gameTable.querySelector('th:nth-child(3)').textContent = l10n.translate("LOOT Folder").fetch();
            gameTable.querySelector('th:nth-child(4)').textContent = l10n.translate("Master File").fetch();
            gameTable.querySelector('th:nth-child(5)').textContent = l10n.translate("Masterlist Repository URL").fetch();
            gameTable.querySelector('th:nth-child(6)').textContent = l10n.translate("Masterlist Repository Branch").fetch();
            gameTable.querySelector('th:nth-child(7)').textContent = l10n.translate("Install Path").fetch();
            gameTable.querySelector('th:nth-child(8)').textContent = l10n.translate("Install Path Registry Key").fetch();

            /* As the game table is attached on launch, its "Add New Row"
               tooltip doesn't benefit from the template translation above. */
            gameTable.querySelector('tr:last-child core-tooltip').setAttribute('label', l10n.translate("Add New Row").fetch());

            document.getElementById('settingsDialog').getElementsByClassName('accept')[0].textContent = l10n.translate("Apply").fetch();
            document.getElementById('settingsDialog').getElementsByClassName('cancel')[0].textContent = l10n.translate("Cancel").fetch();

            /* First-run dialog */
            var firstRun = document.getElementById('firstRun');
            firstRun.heading = l10n.translate("First-Time Tips").fetch();

            firstRun.querySelector('li:nth-child(3)').textContent = l10n.translate("CRCs are only displayed after plugins have been loaded, either by conflict filtering, or by sorting.").fetch();
            firstRun.querySelector('li:nth-child(4)').textContent = l10n.translate("Double-click a plugin in the sidebar to quickly open its metadata editor. Multiple metadata editors can be opened at once.").fetch();
            firstRun.querySelector('li:nth-child(5)').textContent = l10n.translate("Plugins can be drag and dropped from the sidebar into editors' \"load after, \"requirements\" and \"incompatibility\" tables.").fetch();
            firstRun.querySelector('li:nth-child(6)').textContent = l10n.translate("Some features are disabled while there is an editor open, or while there is a sorted load order that has not been applied or discarded.").fetch();
            firstRun.querySelector('li:last-child').textContent = l10n.translate("Many interface elements have tooltips. If you don't know what something is, try hovering your mouse over it to find out. Otherwise, LOOT's documentation can be accessed through the main menu.").fetch();

            firstRun.getElementsByTagName('paper-button')[0].textContent = l10n.translate("OK").fetch();
        },

        getJedInstance: function(locale) {
            return this.loadLocaleData(locale).catch(function(error){
                console.log(error);
                return defaultData;
            }).then(function(result){
                return new jed({
                    'locale_data': result,
                    'domain': 'messages'
                });
            });
        }

    };
}));
