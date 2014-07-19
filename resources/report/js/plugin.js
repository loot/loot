/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013-2014    WrinklyNinja

    This file is part of LOOT.

    LOOT is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    LOOT is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with LOOT.  If not, see
    <http://www.gnu.org/licenses/>.
*/
'use strict';
/* Plugin object for managing data and UI interaction. */
function Plugin(obj) {
    this.name = obj.name;
    this.crc = obj.crc;
    this.version = obj.version;
    this.isActive = obj.isActive;
    this.isDummy = obj.isDummy;
    this.loadsBSA = obj.loadsBSA;

    this.masterlist = obj.masterlist;
    this.userlist = obj.userlist;

    this.modPriority = obj.modPriority;
    this.isGlobalPriority = obj.isGlobalPriority;
    this.messages = obj.messages;
    this.tags = obj.tags;
    this.isDirty = obj.isDirty;

    this.id = this.name.replace(/\s+/g, '');

    Plugin.prototype.getTagsStrings = function () {
        var tagsAdded = [];
        var tagsRemoved = [];

        if (this.tags) {
            for (var i = 0; i < this.tags.length; ++i) {
                if (this.tags[i].name[0] == '-') {
                    tagsRemoved.push(this.tags[i].name.substr(1));
                } else {
                    tagsAdded.push(this.tags[i].name);
                }
            }
        }
        /* Now make sure that the same tag doesn't appear in both arrays.
           Prefer the removed list. */
        for (var i = 0; i < tagsAdded.length; ++i) {
            for (var j = 0; j < tagsRemoved.length; ++j) {
                if (tagsRemoved[j].toLowerCase() == tagsAdded[i].toLowerCase()) {
                    /* Remove tag from the tagsAdded array. */
                    tagsAdded.splice(i, 1);
                    --i;
                }
            }
        }

        return {
          tagsAdded: tagsAdded.join(', '),
          tagsRemoved: tagsRemoved.join(', ')
        };

    }

    Plugin.prototype.getPriorityString = function() {
        var priorityText = 'Priority: ' + this.modPriority + ', Global: ';
        if (this.isGlobalPriority) {
            priorityText += '✓';
        } else {
            priorityText += '✗';
        }
        return priorityText;
    }

    Plugin.prototype.getConflictingPlugins = function() {
        var request = {
            name: 'getConflictingPlugins',
            args: [
                this.name
            ]
        };

        var request_id = window.cefQuery({
            request: JSON.stringify(request),
            persistent: false,
            onSuccess: function(response) {},
            onFailure: function(error_code, error_message) {
                showMessageBox('error', "Error", "Error code: " + error_code + "; " + error_message);
            }
        });
    }

    Plugin.prototype.createCard = function() {
        var card = new PluginCard();
        this.card = card;

        card.id = this.id;

        card.setAttribute('data-active', this.isActive);
        card.setAttribute('data-dummy', this.isDummy);
        card.setAttribute('data-bsa', this.loadsBSA);
        card.setAttribute('data-edits', this.userlist != undefined);

        /* Fill in name, version, CRC. */
        card.querySelector('h1').textContent = this.name;
        card.querySelector('.version').textContent = this.version;
        if (this.crc != '0') {
            card.querySelector('.crc').textContent = this.crc;
        }

        /* Fill in Bash Tag suggestions. */
        var tags = this.getTagsStrings();
        if (tags.tagsAdded) {
            card.getElementsByClassName('tag add')[0].textContent = tags.tagsAdded;
        } else {
            card.getElementsByClassName('tag add')[0].classList.toggle('hidden');
        }
        if (tags.tagsRemoved) {
            card.getElementsByClassName('tag remove')[0].textContent = tags.tagsRemoved;
        } else {
            card.getElementsByClassName('tag remove')[0].classList.toggle('hidden');
        }

        /* Fill in messages. */
        if (this.messages && this.messages.length != 0) {
            this.messages.forEach(function(message) {
                var messageLi = document.createElement('li');
                messageLi.className = message.type;
                // Use the Marked library for Markdown formatting support.
                messageLi.innerHTML = marked(message.content[0].str);
                card.getElementsByTagName('ul')[0].appendChild(messageLi);

            });
        } else {
            card.getElementsByTagName('ul')[0].classList.toggle('hidden');
        }

        /* The content elements steal the name, CRC and version, so they
           don't get distributed into the editor part of the shadow DOM.
           Update the editor elements manually. */

        card.shadowRoot.querySelector('#editor h1').textContent = this.name;
        card.shadowRoot.querySelector('#editor .version').textContent = this.version;
        if (this.crc != '0') {
            card.shadowRoot.querySelector('#editor .crc').textContent = this.crc;
        }

        /* Fill in editor table data. */
        var tables = card.shadowRoot.getElementsByTagName('editable-table');
        for (var i = 0; i < tables.length; ++i) {

        }

        document.getElementById('main').appendChild(card);
    }

    Plugin.prototype.createListItem = function() {
        var li = new PluginListItem();

        li.shadowRoot.querySelector('a').href = '#' + this.id;

        li.querySelector('.name').textContent = this.name;
        li.querySelector('.priority').textContent = this.getPriorityString();

        li.setAttribute('data-dummy', this.isDummy);
        li.setAttribute('data-bsa', this.loadsBSA);
        li.setAttribute('data-edits', this.userlist != undefined);

        document.getElementById('pluginsNav').appendChild(li);
    }

    Plugin.prototype.changeAction = function(change) {
        console.log(change);
    }

    Plugin.prototype.observer = function(changes) {
        //console.log(changes);
        changes.forEach(function(change) {
        //for (var i = 0; i < changes.length; ++i) {

            if (change.name == 'name') {
                change.object.card.querySelector('h1').textContent = change.object[change.name];
            }
        });
        //changes.forEach(changeAction);
    }

    this.createCard();
    this.createListItem();
    Object.observe(this, this.observer);
}

function jsonToPlugin(key, value) {
    if (value !== null && value.__type === 'Plugin') {
        var p = new Plugin(value);
        return p;
    }
    return value;
}




















