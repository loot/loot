'use strict';

(function (root, factory) {
    if (typeof define === 'function' && define.amd) {
        // AMD. Register as an anonymous module.
        define(['jed', 'jedGettextParser'], factory);
    } else {
        // Browser globals
        root.amdWeb = factory(root.jed, root.jedGettextParser);
    }
}(this, function (jed, jedGettextParser) {
    //use b in some fashion.

    // Just return a value to define the module export.
    // This example returns an object, but the module
    // can return a function as the exported value.
    return {

        loadLocaleData: function(locale) {

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
            var pluginCard = document.getElementById('pluginCard').content;
            pluginCard.getElementById('activeTick').title = l10n.translate("Active Plugin").fetch();
            pluginCard.getElementById('dummyPlugin').title = l10n.translate("Dummy Plugin").fetch();
            pluginCard.getElementById('loadsBSA').title = l10n.translate("Loads BSA").fetch();
            pluginCard.getElementById('hasUserEdits').title = l10n.translate("Has User Metadata").fetch();
            pluginCard.getElementById('menuButton').title = l10n.translate("Click to open the plugin menu.").fetch();

            pluginCard.getElementById('enableEdits').nextElementSibling.textContent = l10n.translate("Enable Edits").fetch();
            pluginCard.getElementById('globalPriority').nextElementSibling.title = l10n.translate("Global priorities are compared against all other plugins. Normal priorities are compared against only conflicting plugins.").fetch();
            pluginCard.getElementById('globalPriority').nextElementSibling.textContent = l10n.translate("Global Priority").fetch();
            pluginCard.getElementById('priorityValue').previousElementSibling.textContent = l10n.translate("Priority Value").fetch();

            pluginCard.getElementById('tableTabs').querySelector('[data-for=loadAfter]').textContent = l10n.translate("Load After").fetch();
            pluginCard.getElementById('tableTabs').querySelector('[data-for=req]').textContent = l10n.translate("Requirements").fetch();
            pluginCard.getElementById('tableTabs').querySelector('[data-for=inc]').textContent = l10n.translate("Incompatibilities").fetch();
            pluginCard.getElementById('tableTabs').querySelector('[data-for=message]').textContent = l10n.translate("Messages").fetch();
            pluginCard.getElementById('tableTabs').querySelector('[data-for=tags]').textContent = l10n.translate("Bash Tags").fetch();
            pluginCard.getElementById('tableTabs').querySelector('[data-for=dirty]').textContent = l10n.translate("Dirty Info").fetch();

            pluginCard.getElementById('loadAfter').querySelector('th:first-child').textContent = l10n.translate("Filename").fetch();
            pluginCard.getElementById('loadAfter').querySelector('th:nth-child(2)').textContent = l10n.translate("Display Name").fetch();
            pluginCard.getElementById('loadAfter').querySelector('th:nth-child(3)').textContent = l10n.translate("Condition").fetch();
            pluginCard.getElementById('loadAfter').querySelector('td:first-child').textContent = l10n.translate("Add new row...").fetch();

            pluginCard.getElementById('req').querySelector('th:first-child').textContent = l10n.translate("Filename").fetch();
            pluginCard.getElementById('req').querySelector('th:nth-child(2)').textContent = l10n.translate("Display Name").fetch();
            pluginCard.getElementById('req').querySelector('th:nth-child(3)').textContent = l10n.translate("Condition").fetch();
            pluginCard.getElementById('req').querySelector('td:first-child').textContent = l10n.translate("Add new row...").fetch();

            pluginCard.getElementById('inc').querySelector('th:first-child').textContent = l10n.translate("Filename").fetch();
            pluginCard.getElementById('inc').querySelector('th:nth-child(2)').textContent = l10n.translate("Display Name").fetch();
            pluginCard.getElementById('inc').querySelector('th:nth-child(3)').textContent = l10n.translate("Condition").fetch();
            pluginCard.getElementById('inc').querySelector('td:first-child').textContent = l10n.translate("Add new row...").fetch();

            pluginCard.getElementById('message').querySelector('th:first-child').textContent = l10n.translate("Type").fetch();
            pluginCard.getElementById('message').querySelector('th:nth-child(2)').textContent = l10n.translate("Content").fetch();
            pluginCard.getElementById('message').querySelector('th:nth-child(3)').textContent = l10n.translate("Condition").fetch();
            pluginCard.getElementById('message').querySelector('th:nth-child(4)').textContent = l10n.translate("Language").fetch();
            pluginCard.getElementById('message').querySelector('td:first-child').textContent = l10n.translate("Add new row...").fetch();

            pluginCard.getElementById('tags').querySelector('th:first-child').textContent = l10n.translate("Add/Remove").fetch();
            pluginCard.getElementById('tags').querySelector('th:nth-child(2)').textContent = l10n.translate("Bash Tag").fetch();
            pluginCard.getElementById('tags').querySelector('th:nth-child(3)').textContent = l10n.translate("Condition").fetch();
            pluginCard.getElementById('tags').querySelector('td:first-child').textContent = l10n.translate("Add new row...").fetch();

            pluginCard.getElementById('dirty').querySelector('th:first-child').textContent = l10n.translate("CRC").fetch();
            pluginCard.getElementById('dirty').querySelector('th:nth-child(2)').textContent = l10n.translate("ITM Count").fetch();
            pluginCard.getElementById('dirty').querySelector('th:nth-child(3)').textContent = l10n.translate("UDR Count").fetch();
            pluginCard.getElementById('dirty').querySelector('th:nth-child(4)').textContent = l10n.translate("Deleted Navmeshes").fetch();
            pluginCard.getElementById('dirty').querySelector('th:nth-child(5)').textContent = l10n.translate("Cleaning Utility").fetch();
            pluginCard.getElementById('dirty').querySelector('td:first-child').textContent = l10n.translate("Add new row...").fetch();

            pluginCard.getElementById('accept').textContent = l10n.translate("Apply").fetch();
            pluginCard.getElementById('cancel').textContent = l10n.translate("Cancel").fetch();

            /* Plugin List Item Template */
            var pluginLI = document.getElementById('pluginLI');
            pluginLI.getElementsByClassName('dummyPlugin')[0].title = l10n.translate("Dummy Plugin").fetch();
            pluginLI.getElementsByClassName('loadsBSA')[0].title = l10n.translate("Loads BSA").fetch();
            pluginLI.getElementsByClassName('hasUserEdits')[0].title = l10n.translate("Has User Metadata").fetch();
            pluginLI.getElementsByClassName('hasGlobalPriority')[0].title = l10n.translate("Priority Is Global").fetch();

            /* Plugin menu template */
            var pluginMenu = document.getElementById('pluginMenu');
            pluginMenu.getElementById('showOnlyConflicts').nextSibling.textContent = l10n.translate("Show Only Conflicts").fetch();
            pluginMenu.getElementById('editMetadata').firstChild.nextSibling.textContent = l10n.translate("Edit Metadata").fetch();
            pluginMenu.getElementById('copyMetadata').firstChild.nextSibling.textContent = l10n.translate("Copy Metadata As Text").fetch();
            pluginMenu.getElementById('clearMetadata').firstChild.nextSibling.textContent = l10n.translate("Clear User Metadata").fetch();

            /* Message row template */
            var messageRow = document.getElementById('messageRow');
            messageRow.getElementsByClassName('type')[0].children[0].textContent = l10n.translate("Note").fetch();
            messageRow.getElementsByClassName('type')[0].children[1].textContent = l10n.translate("Warning").fetch();
            messageRow.getElementsByClassName('type')[0].children[2].textContent = l10n.translate("Error").fetch();

            /* Tag row template */
            var tagRow = document.getElementById('tagRow');
            tagRow.getElementsByClassName('type')[0].children[0].textContent = l10n.translate("Add").fetch();
            tagRow.getElementsByClassName('type')[0].children[1].textContent = l10n.translate("Remove").fetch();

            /* File menu */
            document.getElementById('fileMenu').firstChild.textContent = l10n.translate("File").fetch();
            document.getElementById('redatePluginsButton').firstChild.nextSibling.textContent = l10n.translate("Redate Plugins").fetch();
            document.getElementById('openLogButton').firstChild.nextSibling.textContent = l10n.translate("Open Debug Log Location").fetch();
            document.getElementById('wipeUserlistButton').firstChild.nextSibling.textContent = l10n.translate("Clear All User Metadata").fetch();
            document.getElementById('copyContentButton').firstChild.nextSibling.textContent = l10n.translate("Copy Content As Text").fetch();
            document.getElementById('refreshContentButton').firstChild.nextSibling.textContent = l10n.translate("Refresh Content").fetch();

            /* Game menu */
            document.getElementById('gameMenu').firstChild.textContent = l10n.translate("Game").fetch();

            /* Help menu */
            document.getElementById('helpMenu').firstChild.textContent = l10n.translate("Help").fetch();
            document.getElementById('helpButton').firstChild.nextSibling.textContent = l10n.translate("View Documentation").fetch();
            document.getElementById('aboutButton').firstChild.nextSibling.textContent = l10n.translate("About").fetch();

            /* Header buttons */
            document.getElementById('settingsButton').textContent = l10n.translate("Settings").fetch();
            document.getElementById('updateMasterlistButton').firstChild.nextSibling.textContent = l10n.translate("Update Masterlist").fetch();
            document.getElementById('sortButton').firstChild.nextSibling.textContent = l10n.translate("Sort").fetch();
            document.getElementById('applySortButton').firstChild.nextSibling.textContent = l10n.translate("Apply").fetch();
            document.getElementById('cancelSortButton').firstChild.nextSibling.textContent = l10n.translate("Cancel").fetch();

            /* Nav items */
            document.getElementById('container').querySelector('nav > ol li > a[href=#summary]').textContent = l10n.translate("Summary").fetch();
            document.getElementById('container').querySelector('nav > ol li > a[href=#generalMessages]').textContent = l10n.translate("General Messages").fetch();
            document.getElementById('filtersToggle').firstChild.nextSibling.textContent = l10n.translate("Filters").fetch();
            document.getElementById('searchBox').parentElement.title = l10n.translate("Press Enter when searching to select the next match.").fetch();

            /* Filters */
            document.getElementById('hideVersionNumbers').nextElementSibling.textContent = l10n.translate("Hide version numbers").fetch();
            document.getElementById('hideCRCs').nextElementSibling.textContent = l10n.translate("Hide CRCs").fetch();
            document.getElementById('hideBashTags').nextElementSibling.textContent = l10n.translate("Hide Bash Tags").fetch();
            document.getElementById('hideNotes').nextElementSibling.textContent = l10n.translate("Hide notes").fetch();
            document.getElementById('hideDoNotCleanMessages').nextElementSibling.textContent = l10n.translate("Hide 'Do not clean' messages").fetch();
            document.getElementById('hideInactivePluginMessages').nextElementSibling.textContent = l10n.translate("Hide inactive plugin messages").fetch();
            document.getElementById('hideAllPluginMessages').nextElementSibling.textContent = l10n.translate("Hide all plugin messages").fetch();
            document.getElementById('hideMessagelessPlugins').nextElementSibling.textContent = l10n.translate("Hide messageless plugins").fetch();
            document.getElementById('hiddenPluginsTxt').textContent = l10n.translate("Hidden plugins").fetch();
            document.getElementById('hiddenMessagesTxt').textContent = l10n.translate("Hidden messages").fetch();

            /* Summary */
            document.getElementById('summary').firstElementChild.textContent = l10n.translate("Summary").fetch();
            document.getElementById('masterlistRevision').previousSibling.textContent = l10n.translate("Masterlist Revision").fetch();
            document.getElementById('masterlistDate').previousSibling.textContent = l10n.translate("Masterlist Date").fetch();
            document.getElementById('totalWarningNo').previousSibling.textContent = l10n.translate("Warnings").fetch();
            document.getElementById('totalErrorNo').previousSibling.textContent = l10n.translate("Errors").fetch();
            document.getElementById('totalMessageNo').previousSibling.textContent = l10n.translate("Total Messages").fetch();
            document.getElementById('activePluginNo').previousSibling.textContent = l10n.translate("Active Plugins").fetch();
            document.getElementById('dirtyPluginNo').previousSibling.textContent = l10n.translate("Dirty Plugins").fetch();
            document.getElementById('totalPluginNo').previousSibling.textContent = l10n.translate("Total Plugins").fetch();

            /* General messages */
            document.getElementById('generalMessages').firstElementChild.textContent = l10n.translate("General Messages").fetch();

            /* Settings dialog */
            document.getElementById('settings').firstElementChild.textContent = l10n.translate("Settings").fetch();

            document.getElementById('defaultGameSelect').previousElementSibling.textContent = l10n.translate("Default Game").fetch();
            document.getElementById('defaultGameSelect').children[0].textContent = l10n.translate("Autodetect").fetch();

            document.getElementById('languageSelect').previousElementSibling.textContent = l10n.translate("Language").fetch();

            document.getElementById('enableDebugLogging').nextElementSibling.textContent = l10n.translate("Enable debug logging").fetch();
            document.getElementById('enableDebugLogging').nextElementSibling.title = l10n.translate("The output is logged to the LOOTDebugLog.txt file.").fetch();

            document.getElementById('updateMasterlist').nextElementSibling.textContent = l10n.translate("Update masterlist before sorting").fetch();

            document.getElementById('autoRefresh').nextElementSibling.textContent = l10n.translate("Automatically refresh content on window refocus").fetch();

            var gameTable = document.getElementById('gameTable');
            gameTable.querySelector('th:first-child').textContent = l10n.translate("Name").fetch();
            gameTable.querySelector('th:nth-child(2)').textContent = l10n.translate("Base Game").fetch();
            gameTable.querySelector('th:nth-child(3)').textContent = l10n.translate("LOOT Folder").fetch();
            gameTable.querySelector('th:nth-child(4)').textContent = l10n.translate("Master File").fetch();
            gameTable.querySelector('th:nth-child(5)').textContent = l10n.translate("Masterlist Repository URL").fetch();
            gameTable.querySelector('th:nth-child(6)').textContent = l10n.translate("Masterlist Repository Branch").fetch();
            gameTable.querySelector('th:nth-child(7)').textContent = l10n.translate("Install Path").fetch();
            gameTable.querySelector('th:nth-child(8)').textContent = l10n.translate("Install Path Registry Key").fetch();
            gameTable.querySelector('td').textContent = l10n.translate("Add new row...").fetch();

            document.getElementById('languageRestartTxt').textContent = l10n.translate("Language changes will be applied after LOOT is restarted.").fetch();

            document.getElementById('settings').getElementsByClassName('accept')[0].textContent = l10n.translate("Apply").fetch();
            document.getElementById('settings').getElementsByClassName('cancel')[0].textContent = l10n.translate("Cancel").fetch();

            /* First-run dialog */
            var firstRun = document.getElementById('firstRun');
            firstRun.getElementsByTagName('h1').textContent = l10n.translate("First-Time Tips").fetch();

            firstRun.querySelector('li:nth-child(4)').textContent = l10n.translate("Multiple metadata editors can be opened at once, and the menu bar is disabled while any editors are open.").fetch();
            firstRun.querySelector('li:last-child').textContent = l10n.translate("Many interface elements have tooltips. If you don't know what something is, try hovering your mouse over it to find out.").fetch();

            firstRun.getElementsByClassName('accept')[0].textContent = l10n.translate("OK").fetch();
        },

        getJedInstance: function(locale) {
            return this.loadLocaleData(locale).then(function(result){
                return new jed({
                    'locale_data': result,
                    'domain': 'messages'
                });
            });
        }

    };
}));


