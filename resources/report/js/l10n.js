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
            pluginCard.getElementById('cancel').textContent = l10n.translate("Apply").fetch();

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


            document.getElementById('hideVersionNumbers').nextElementSibling.textContent = l10n.translate("Hide version numbers").fetch();
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


