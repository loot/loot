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

            pluginCard.getElementById('activeTick').label = l10n.translate("Active Plugin").fetch();
            pluginCard.getElementById('dummyPlugin').label = l10n.translate("Dummy Plugin").fetch();
            pluginCard.getElementById('loadsBSA').label = l10n.translate("Loads BSA").fetch();
            pluginCard.getElementById('hasUserEdits').label = l10n.translate("Has User Metadata").fetch();

            pluginCard.getElementById('showOnlyConflicts').label = l10n.translate("Show Only Conflicts").fetch();
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

            pluginEditor.getElementById('activeTick').label = l10n.translate("Active Plugin").fetch();
            pluginEditor.getElementById('dummyPlugin').label = l10n.translate("Dummy Plugin").fetch();
            pluginEditor.getElementById('loadsBSA').label = l10n.translate("Loads BSA").fetch();

            pluginEditor.getElementById('enableEdits').label = l10n.translate("Enable Edits").fetch();
            pluginEditor.getElementById('globalPriority').parentElement.label = l10n.translate("Global priorities are compared against all other plugins. Normal priorities are compared against only conflicting plugins.").fetch();
            pluginEditor.getElementById('globalPriority').label = l10n.translate("Global Priority").fetch();
            pluginEditor.getElementById('priorityValue').parentElement.previousElementSibling.textContent = l10n.translate("Priority Value").fetch();

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
            pluginEditor.getElementById('loadAfter').querySelector('td:first-child').textContent = l10n.translate("Add new row...").fetch();

            pluginEditor.getElementById('req').querySelector('th:first-child').textContent = l10n.translate("Filename").fetch();
            pluginEditor.getElementById('req').querySelector('th:nth-child(2)').textContent = l10n.translate("Display Name").fetch();
            pluginEditor.getElementById('req').querySelector('th:nth-child(3)').textContent = l10n.translate("Condition").fetch();
            pluginEditor.getElementById('req').querySelector('td:first-child').textContent = l10n.translate("Add new row...").fetch();

            pluginEditor.getElementById('inc').querySelector('th:first-child').textContent = l10n.translate("Filename").fetch();
            pluginEditor.getElementById('inc').querySelector('th:nth-child(2)').textContent = l10n.translate("Display Name").fetch();
            pluginEditor.getElementById('inc').querySelector('th:nth-child(3)').textContent = l10n.translate("Condition").fetch();
            pluginEditor.getElementById('inc').querySelector('td:first-child').textContent = l10n.translate("Add new row...").fetch();

            pluginEditor.getElementById('message').querySelector('th:first-child').textContent = l10n.translate("Type").fetch();
            pluginEditor.getElementById('message').querySelector('th:nth-child(2)').textContent = l10n.translate("Content").fetch();
            pluginEditor.getElementById('message').querySelector('th:nth-child(3)').textContent = l10n.translate("Condition").fetch();
            pluginEditor.getElementById('message').querySelector('th:nth-child(4)').textContent = l10n.translate("Language").fetch();
            pluginEditor.getElementById('message').querySelector('td:first-child').textContent = l10n.translate("Add new row...").fetch();

            pluginEditor.getElementById('tags').querySelector('th:first-child').textContent = l10n.translate("Add/Remove").fetch();
            pluginEditor.getElementById('tags').querySelector('th:nth-child(2)').textContent = l10n.translate("Bash Tag").fetch();
            pluginEditor.getElementById('tags').querySelector('th:nth-child(3)').textContent = l10n.translate("Condition").fetch();
            pluginEditor.getElementById('tags').querySelector('td:first-child').textContent = l10n.translate("Add new row...").fetch();

            pluginEditor.getElementById('dirty').querySelector('th:first-child').textContent = l10n.translate("CRC").fetch();
            pluginEditor.getElementById('dirty').querySelector('th:nth-child(2)').textContent = l10n.translate("ITM Count").fetch();
            pluginEditor.getElementById('dirty').querySelector('th:nth-child(3)').textContent = l10n.translate("Deleted References").fetch();
            pluginEditor.getElementById('dirty').querySelector('th:nth-child(4)').textContent = l10n.translate("Deleted Navmeshes").fetch();
            pluginEditor.getElementById('dirty').querySelector('th:nth-child(5)').textContent = l10n.translate("Cleaning Utility").fetch();
            pluginEditor.getElementById('dirty').querySelector('td:first-child').textContent = l10n.translate("Add new row...").fetch();

            pluginEditor.getElementById('locations').querySelector('th:first-child').textContent = l10n.translate("URL").fetch();
            pluginEditor.getElementById('locations').querySelector('th:nth-child(2)').textContent = l10n.translate("Version").fetch();
            pluginEditor.getElementById('locations').querySelector('td:first-child').textContent = l10n.translate("Add new row...").fetch();

            pluginEditor.getElementById('accept').textContent = l10n.translate("Apply").fetch();
            pluginEditor.getElementById('cancel').textContent = l10n.translate("Cancel").fetch();

            /* Plugin List Item Template */
            var pluginItem = document.querySelector('link[rel="import"][href$="loot-plugin-item.html"]');
            if (pluginItem) {
                pluginItem = pluginItem.import.querySelector('template').content;
            } else {
                pluginItem = document.querySelector('polymer-element[name="loot-plugin-item"]').querySelector('template').content;
            }
            pluginItem.getElementById('loadsBSA').label = l10n.translate("Loads BSA").fetch();
            pluginItem.getElementById('hasUserEdits').label = l10n.translate("Has User Metadata").fetch();

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
            document.getElementById('copyContentButton').lastChild.textContent = l10n.translate("Copy Content").fetch();
            document.getElementById('refreshContentButton').lastChild.textContent = l10n.translate("Refresh Content").fetch();
            document.getElementById('helpButton').lastChild.textContent = l10n.translate("View Documentation").fetch();
            document.getElementById('aboutButton').lastChild.textContent = l10n.translate("About").fetch();
            document.getElementById('settingsButton').lastChild.textContent = l10n.translate("Settings").fetch();
            document.getElementById('applySortButton').textContent = l10n.translate("Apply").fetch();
            document.getElementById('cancelSortButton').textContent = l10n.translate("Cancel").fetch();

            /* Nav items */
            document.getElementById('sidebarTabs').firstElementChild.textContent = l10n.translate("Plugins").fetch();
            document.getElementById('sidebarTabs').firstElementChild.nextElementSibling.textContent = l10n.translate("Filters").fetch();
            document.getElementById('searchBox').parentElement.label = l10n.translate("Press Enter or click outside the input to set the filter.").fetch();

            /* Filters */
            document.getElementById('hideVersionNumbers').label = l10n.translate("Hide version numbers").fetch();
            document.getElementById('hideCRCs').label = l10n.translate("Hide CRCs").fetch();
            document.getElementById('hideBashTags').label = l10n.translate("Hide Bash Tags").fetch();
            document.getElementById('hideNotes').label = l10n.translate("Hide notes").fetch();
            document.getElementById('hideDoNotCleanMessages').label = l10n.translate("Hide 'Do not clean' messages").fetch();
            document.getElementById('hideInactivePlugins').label = l10n.translate("Hide inactive plugin messages").fetch();
            document.getElementById('hideAllPluginMessages').label = l10n.translate("Hide all plugin messages").fetch();
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

            document.getElementById('defaultGameSelect').previousElementSibling.textContent = l10n.translate("Default Game").fetch();
            document.getElementById('defaultGameSelect').firstElementChild.textContent = l10n.translate("Autodetect").fetch();

            document.getElementById('languageSelect').previousElementSibling.textContent = l10n.translate("Language").fetch();
            document.getElementById('languageSelect').parentElement.label = l10n.translate("Language changes will be applied after LOOT is restarted.").fetch();

            document.getElementById('enableDebugLogging').label = l10n.translate("Enable debug logging").fetch();
            document.getElementById('enableDebugLogging').parentElement.label = l10n.translate("The output is logged to the LOOTDebugLog.txt file.").fetch();

            document.getElementById('updateMasterlist').label = l10n.translate("Update masterlist before sorting").fetch();

            var gameTable = document.getElementById('gameTable');
            gameTable.querySelector('th:first-child').textContent = l10n.translate("Name").fetch();
            gameTable.querySelector('th:nth-child(2)').textContent = l10n.translate("Base Game").fetch();
            gameTable.querySelector('th:nth-child(3)').textContent = l10n.translate("LOOT Folder").fetch();
            gameTable.querySelector('th:nth-child(4)').textContent = l10n.translate("Master File").fetch();
            gameTable.querySelector('th:nth-child(5)').textContent = l10n.translate("Masterlist Repository URL").fetch();
            gameTable.querySelector('th:nth-child(6)').textContent = l10n.translate("Masterlist Repository Branch").fetch();
            gameTable.querySelector('th:nth-child(7)').textContent = l10n.translate("Install Path").fetch();
            gameTable.querySelector('th:nth-child(8)').textContent = l10n.translate("Install Path Registry Key").fetch();
            gameTable.querySelector('tr:last-child td:first-child').textContent = l10n.translate("Add new row...").fetch();

            document.getElementById('settingsDialog').getElementsByClassName('accept')[0].textContent = l10n.translate("Apply").fetch();
            document.getElementById('settingsDialog').getElementsByClassName('cancel')[0].textContent = l10n.translate("Cancel").fetch();

            /* First-run dialog */
            var firstRun = document.getElementById('firstRun');
            firstRun.heading = l10n.translate("First-Time Tips").fetch();

            firstRun.querySelector('li:nth-child(4)').textContent = l10n.translate("Multiple metadata editors can be opened at once, and many options in the header bar are disabled while any editors are open.").fetch();
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
