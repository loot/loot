'use strict';

(function (root, factory) {
    if (typeof define === 'function' && define.amd) {
        // AMD. Register as an anonymous module.
        define(['bower_components/Jed/jed', 'js/jedGettextParser'], factory);
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

            pluginCard.getElementById('front').querySelector('.activeTick').label = l10n.translate("Active Plugin").fetch();
            pluginCard.getElementById('editor').querySelector('.activeTick').label = l10n.translate("Active Plugin").fetch();
            pluginCard.getElementById('front').querySelector('.dummyPlugin').label = l10n.translate("Dummy Plugin").fetch();
            pluginCard.getElementById('editor').querySelector('.dummyPlugin').label = l10n.translate("Dummy Plugin").fetch();
            pluginCard.getElementById('front').querySelector('.loadsBSA').label = l10n.translate("Loads BSA").fetch();
            pluginCard.getElementById('editor').querySelector('.loadsBSA').label = l10n.translate("Loads BSA").fetch();
            pluginCard.getElementById('hasUserEdits').label = l10n.translate("Has User Metadata").fetch();

            pluginCard.getElementById('enableEdits').label = l10n.translate("Enable Edits").fetch();
            pluginCard.getElementById('globalPriority').parentElement.label = l10n.translate("Global priorities are compared against all other plugins. Normal priorities are compared against only conflicting plugins.").fetch();
            pluginCard.getElementById('globalPriority').label = l10n.translate("Global Priority").fetch();
            pluginCard.getElementById('priorityValue').previousElementSibling.textContent = l10n.translate("Priority Value").fetch();

            pluginCard.getElementById('fileTabs').querySelector('[data-for=loadAfter]').textContent = l10n.translate("Load After").fetch();
            pluginCard.getElementById('fileTabs').querySelector('[data-for=req]').textContent = l10n.translate("Requirements").fetch();
            pluginCard.getElementById('fileTabs').querySelector('[data-for=inc]').textContent = l10n.translate("Incompatibilities").fetch();
            pluginCard.getElementById('miscTabs').querySelector('[data-for=message]').textContent = l10n.translate("Messages").fetch();
            pluginCard.getElementById('miscTabs').querySelector('[data-for=tags]').textContent = l10n.translate("Bash Tags").fetch();
            pluginCard.getElementById('miscTabs').querySelector('[data-for=dirty]').textContent = l10n.translate("Dirty Info").fetch();
            pluginCard.getElementById('miscTabs').querySelector('[data-for=locations]').textContent = l10n.translate("Locations").fetch();

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
            pluginCard.getElementById('dirty').querySelector('th:nth-child(3)').textContent = l10n.translate("Deleted References").fetch();
            pluginCard.getElementById('dirty').querySelector('th:nth-child(4)').textContent = l10n.translate("Deleted Navmeshes").fetch();
            pluginCard.getElementById('dirty').querySelector('th:nth-child(5)').textContent = l10n.translate("Cleaning Utility").fetch();
            pluginCard.getElementById('dirty').querySelector('td:first-child').textContent = l10n.translate("Add new row...").fetch();

            pluginCard.getElementById('locations').querySelector('th:first-child').textContent = l10n.translate("URL").fetch();
            pluginCard.getElementById('locations').querySelector('th:nth-child(2)').textContent = l10n.translate("Version").fetch();
            pluginCard.getElementById('locations').querySelector('td:first-child').textContent = l10n.translate("Add new row...").fetch();

            pluginCard.getElementById('accept').textContent = l10n.translate("Apply").fetch();
            pluginCard.getElementById('cancel').textContent = l10n.translate("Cancel").fetch();

            pluginCard.getElementById('showOnlyConflicts').label = l10n.translate("Show Only Conflicts").fetch();
            pluginCard.getElementById('editMetadata').lastChild.textContent = l10n.translate("Edit Metadata").fetch();
            pluginCard.getElementById('copyMetadata').lastChild.textContent = l10n.translate("Copy Metadata As Text").fetch();
            pluginCard.getElementById('clearMetadata').lastChild.textContent = l10n.translate("Clear User Metadata").fetch();

            /* Plugin List Item Template */
            var pluginItem = document.querySelector('link[rel="import"][href$="loot-plugin-item.html"]');
            if (pluginItem) {
                pluginItem = pluginItem.import.querySelector('template').content;
            } else {
                pluginItem = document.querySelector('polymer-element[name="loot-plugin-item"]').querySelector('template').content;
            }
            pluginItem.querySelector('#dummyPlugin').label = l10n.translate("Dummy Plugin").fetch();
            pluginItem.querySelector('#loadsBSA').label = l10n.translate("Loads BSA").fetch();
            pluginItem.querySelector('#hasUserEdits').label = l10n.translate("Has User Metadata").fetch();
            pluginItem.querySelector('#hasGlobalPriority').label = l10n.translate("Priority Is Global").fetch();

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

            /* Tag row template */
            var tagRow = document.querySelector('link[rel="import"][href$="editable-table.html"]');
            if (tagRow) {
                tagRow = tagRow.import.querySelector('#tagRow').content;
            } else {
                tagRow = document.querySelector('#tagRow').content;
            }
            tagRow.querySelector('.type').children[0].textContent = l10n.translate("Add").fetch();
            tagRow.querySelector('.type').children[1].textContent = l10n.translate("Remove").fetch();

            /* Toolbar menu */
            document.getElementById('redatePluginsButton').lastChild.textContent = l10n.translate("Redate Plugins").fetch();
            document.getElementById('openLogButton').lastChild.textContent = l10n.translate("Open Debug Log Location").fetch();
            document.getElementById('wipeUserlistButton').lastChild.textContent = l10n.translate("Clear All User Metadata").fetch();
            document.getElementById('copyContentButton').lastChild.textContent = l10n.translate("Copy Content As Text").fetch();
            document.getElementById('refreshContentButton').lastChild.textContent = l10n.translate("Refresh Content").fetch();
            document.getElementById('helpButton').lastChild.textContent = l10n.translate("View Documentation").fetch();
            document.getElementById('aboutButton').lastChild.textContent = l10n.translate("About").fetch();
            document.getElementById('settingsButton').lastChild.textContent = l10n.translate("Settings").fetch();
            document.getElementById('applySortButton').textContent = l10n.translate("Apply").fetch();
            document.getElementById('cancelSortButton').textContent = l10n.translate("Cancel").fetch();

            /* Nav items */
            document.getElementById('container').querySelector('div[drawer] > core-menu > paper-item[label=Summary]').lastChild.textContent = l10n.translate("Summary").fetch();
            document.getElementById('container').querySelector('div[drawer] > core-menu > paper-item[label^=General]').lastChild.textContent = l10n.translate("General Messages").fetch();
            document.getElementById('sidebarTabs').firstElementChild.textContent = l10n.translate("Plugins").fetch();
            document.getElementById('sidebarTabs').firstElementChild.nextElementSibling.textContent = l10n.translate("Filters").fetch();
            document.getElementById('searchBox').parentElement.label = l10n.translate("Press Enter when searching to select the next match.").fetch();

            /* Filters */
            document.getElementById('hideVersionNumbers').label = l10n.translate("Hide version numbers").fetch();
            document.getElementById('hideCRCs').label = l10n.translate("Hide CRCs").fetch();
            document.getElementById('hideBashTags').label = l10n.translate("Hide Bash Tags").fetch();
            document.getElementById('hideNotes').label = l10n.translate("Hide notes").fetch();
            document.getElementById('hideDoNotCleanMessages').label = l10n.translate("Hide 'Do not clean' messages").fetch();
            document.getElementById('hideInactivePluginMessages').label = l10n.translate("Hide inactive plugin messages").fetch();
            document.getElementById('hideAllPluginMessages').label = l10n.translate("Hide all plugin messages").fetch();
            document.getElementById('hideMessagelessPlugins').label = l10n.translate("Hide messageless plugins").fetch();
            document.getElementById('hiddenPluginsTxt').textContent = l10n.translate("Hidden plugins:").fetch();
            document.getElementById('hiddenMessagesTxt').textContent = l10n.translate("Hidden messages:").fetch();

            /* Summary */
            document.getElementById('summary').firstElementChild.textContent = l10n.translate("Summary").fetch();
            document.getElementById('masterlistRevision').previousElementSibling.textContent = l10n.translate("Masterlist Revision").fetch();
            document.getElementById('masterlistDate').previousElementSibling.textContent = l10n.translate("Masterlist Date").fetch();
            document.getElementById('totalWarningNo').previousElementSibling.textContent = l10n.translate("Warnings").fetch();
            document.getElementById('totalErrorNo').previousElementSibling.textContent = l10n.translate("Errors").fetch();
            document.getElementById('totalMessageNo').previousElementSibling.textContent = l10n.translate("Total Messages").fetch();
            document.getElementById('activePluginNo').previousElementSibling.textContent = l10n.translate("Active Plugins").fetch();
            document.getElementById('dirtyPluginNo').previousElementSibling.textContent = l10n.translate("Dirty Plugins").fetch();
            document.getElementById('totalPluginNo').previousElementSibling.textContent = l10n.translate("Total Plugins").fetch();

            /* General messages */
            document.getElementById('generalMessages').firstElementChild.textContent = l10n.translate("General Messages").fetch();

            /* Settings dialog */
            document.getElementById('settingsDialog').heading = l10n.translate("Settings").fetch();

            document.getElementById('defaultGameSelect').previousElementSibling.textContent = l10n.translate("Default Game").fetch();
            document.getElementById('defaultGameSelect').firstElementChild.textContent = l10n.translate("Autodetect").fetch();

            document.getElementById('languageSelect').previousElementSibling.textContent = l10n.translate("Language").fetch();

            document.getElementById('enableDebugLogging').label = l10n.translate("Enable debug logging").fetch();
            document.getElementById('enableDebugLogging').parentElement.label = l10n.translate("The output is logged to the LOOTDebugLog.txt file.").fetch();

            document.getElementById('updateMasterlist').label = l10n.translate("Update masterlist before sorting").fetch();

            document.getElementById('autoRefresh').label = l10n.translate("Automatically refresh content on window refocus").fetch();

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

            document.getElementById('settingsDialog').getElementsByClassName('accept')[0].textContent = l10n.translate("Apply").fetch();
            document.getElementById('settingsDialog').getElementsByClassName('cancel')[0].textContent = l10n.translate("Cancel").fetch();

            /* First-run dialog */
            var firstRun = document.getElementById('firstRun');
            firstRun.heading = l10n.translate("First-Time Tips").fetch();

            firstRun.querySelector('li:nth-child(4)').textContent = l10n.translate("Multiple metadata editors can be opened at once, and the menu bar is disabled while any editors are open.").fetch();
            firstRun.querySelector('li:last-child').textContent = l10n.translate("Many interface elements have tooltips. If you don't know what something is, try hovering your mouse over it to find out.").fetch();

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


