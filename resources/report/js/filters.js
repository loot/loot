var filters = {
    /* Filter functions return true if the given plugin passes the filter and
       should be displayed, otherwise false. */

    searchFilter: function(plugin, needle) {
        if (needle.length == 0) {
            return true;
        }

        if (plugin.name.toLowerCase().indexOf(needle) != -1
            || plugin.getCrcString().toLowerCase().indexOf(needle) != -1
            || plugin.version.toLowerCase().indexOf(needle) != -1) {

            return true;
        }

        var tags = plugin.getTagStrings();
        if (tags.added.toLowerCase().indexOf(needle) != -1
            || tags.removed.toLowerCase().indexOf(needle) != -1) {

            return true;
        }

        for (var i = 0; i < plugin.messages.length; ++i) {
            if (plugin.messages[i].content[0].str.toLowerCase().indexOf(needle) != -1) {
                return true;
            }
        }
    },

    messagelessFilter: function(plugin) {
        if (document.getElementById('hideMessagelessPlugins').checked) {
            /* If any messages exist, check if they are hidden or not. Note
               that the messages may not be present as elements, so the check
               is actually if they would be hidden according to the message
               filters. */

            if (this.allMessageFilter()) {
                for (var i = 0; i < plugin.messages.length; ++i) {
                    if (this.noteFilter(plugin.messages[i])
                        && this.doNotCleanFilter(plugin.messages[i])) {

                        return true;
                    }
                }
            }

            return false;
        } else {
            return true;
        }
    },

    applyPluginFilters: function(plugins) {
        var search = document.getElementById('searchBox').value.toLowerCase();
        var hiddenPluginNo = 0;
        var filteredPlugins = [];

        plugins.forEach(function(plugin){
            if (this.messagelessFilter(plugin)
                && this.searchFilter(plugin, search)) {

                filteredPlugins.push(plugin);
                return;
            }
            ++hiddenPluginNo;
        }, this);

        document.getElementById('hiddenPluginNo').textContent = hiddenPluginNo;

        return filteredPlugins;
    },

    /* Message filter functions are run from within plugin cards, when the card's
       messages are to be added as elements. Each filter should return true if the
       message is to be displayed. */

    noteFilter: function(message) {
        if (document.getElementById('hideNotes').checked) {
            return message.type != 'say';
        } else {
            return true;
        }
    },

    doNotCleanFilter: function(message) {
        if (document.getElementById('hideDoNotCleanMessages').checked) {
            return message.content[0].str.indexOf(l10n.jed.translate("Do not clean").fetch()) == -1;
        } else {
            return true;
        }
    },

    allMessageFilter: function() {
        return !document.getElementById('hideAllPluginMessages').checked;
    },

    applyMessageFilters: function(messages) {
        var hiddenMessageNo = 0;
        var filteredMessages = [];

        if (this.allMessageFilter()) {
            messages.forEach(function(message){
                if (this.noteFilter(message)
                    && this.doNotCleanFilter(message)) {

                    filteredMessages.push(message);
                    return;
                }
                ++hiddenMessageNo;
            }, this);
        }

        document.getElementById('hiddenMessageNo').textContent = hiddenMessageNo;

        return filteredMessages;
    },
};

function setFilteredUIData(evt) {
    var filtered = filters.applyPluginFilters(loot.game.plugins);
    document.getElementById('cardsNav').lastElementChild.data = filtered;
    document.getElementById('main').lastElementChild.data = filtered;
}