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

    /* Converts between the LOOT metadata object for tags, and their
       editor row representation. */
    Plugin.prototype.convTagObj = function(tag) {
        if (tag.type) {
            /* Input is row data. */
            if (tag.type == 'remove') {
                tag.name = '-' + tag.name;
            }
            delete tag.type;
        } else {
            /* Input is metadata object. */
            if (tag.name[0] == '-') {
                tag.type = 'remove';
                tag.name = tag.name.substr(1);
            } else {
                tag.type = 'add';
            }
        }
        return tag;
    }

    Plugin.prototype.updateCardTags = function() {
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

        if (tagsAdded.length != 0) {
            this.card.getElementsByClassName('tag add')[0].textContent = tagsAdded.join(', ');
            this.card.getElementsByClassName('tag add')[0].classList.toggle('hidden', false);
        } else {
            this.card.getElementsByClassName('tag add')[0].classList.toggle('hidden', true);
        }
        if (tagsRemoved.length != 0) {
            this.card.getElementsByClassName('tag remove')[0].textContent = tagsRemoved.join(', ');
            this.card.getElementsByClassName('tag remove')[0].classList.toggle('hidden', false);
        } else {
            this.card.getElementsByClassName('tag remove')[0].classList.toggle('hidden', true);
        }
    }

    Plugin.prototype.getPriorityString = function() {
        if (this.modPriority != 0) {
            return 'Priority: ' + this.modPriority;
        } else {
            return '';
        }
    }

    Plugin.prototype.updateCardMessages = function() {
        var messageUL = this.card.getElementsByTagName('ul')[0];
        /* First clear any existing messages. */
        while(messageUL.firstElementChild) {
            messageUL.removeChild(messageUL.firstElementChild);
        }
        /* Now add the new messages. */
        if (this.messages && this.messages.length != 0) {
            this.messages.forEach(function(message) {
                var messageLi = document.createElement('li');
                messageLi.className = message.type;
                // Use the Marked library for Markdown formatting support.
                messageLi.innerHTML = marked(message.content[0].str);
                messageUL.appendChild(messageLi);

            });
            this.card.getElementsByTagName('ul')[0].classList.toggle('hidden', false);
        } else {
            this.card.getElementsByTagName('ul')[0].classList.toggle('hidden', true);
        }
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
        card.getElementsByTagName('h1')[0].textContent = this.name;
        card.getElementsByClassName('version')[0].textContent = this.version;
        if (this.crc != 0) {
            card.getElementsByClassName('crc')[0].textContent = this.crc.toString(16).toUpperCase();
        }

        /* Fill in Bash Tag suggestions. */
        this.updateCardTags();

        /* Fill in messages. */
        this.updateCardMessages();

        document.getElementsByTagName('main')[0].appendChild(card);
    }

    Plugin.prototype.createListItem = function() {
        var li = new PluginListItem();
        this.li = li;

        li.textContent = this.name;
        li.setLink('#' + this.id);
        li.setPriority(this.getPriorityString());

        li.setAttribute('data-dummy', this.isDummy);
        li.setAttribute('data-bsa', this.loadsBSA);
        li.setAttribute('data-edits', this.userlist != undefined);
        li.setAttribute('data-global-priority', this.isGlobalPriority);

        document.getElementById('pluginsNav').appendChild(li);
    }

    Plugin.prototype.observer = function(changes) {
        changes.forEach(function(change) {
            if (change.name == 'userlist') {
                change.object.li.setAttribute('data-edits', change.object[change.name] != undefined);
                change.object.card.setAttribute('data-edits', change.object[change.name] != undefined);
            } else if (change.name == 'modPriority') {
                change.object.li.setPriority(this.getPriorityString());
                change.object.card.shadowRoot.getElementById('priorityValue').value = change.object[change.name];
            } else if (change.name == 'isGlobalPriority') {
                change.object.li.setAttribute('data-global-priority', change.object[change.name]);
            } else if (change.name == 'messages') {
                change.object.updateCardMessages();
                /* For messages, the card's messages need updating,
                   as do the message counts. */
                var oldTotal = 0;
                var newTotal = 0;
                var oldWarns = 0;
                var newWarns = 0;
                var oldErrs = 0;
                var newErrs = 0;

                if (change.oldValue) {
                    oldTotal = change.oldValue.length;

                    change.oldValue.forEach(function(message){
                        if (message.type == 'warn') {
                            ++oldWarns;
                        } else if (message.type == 'error') {
                            ++oldErrs;
                        }
                    });
                }
                if (change.object[change.name]) {
                    newTotal = change.object[change.name].length;

                    change.object[change.name].forEach(function(message){
                        if (message.type == 'warn') {
                            ++newWarns;
                        } else if (message.type == 'error') {
                            ++newErrs;
                        }
                    });
                }

                document.getElementById('filterTotalMessageNo').textContent = parseInt(document.getElementById('filterTotalMessageNo').textContent, 10) + newTotal - oldTotal;
                document.getElementById('totalMessageNo').textContent = parseInt(document.getElementById('totalMessageNo').textContent, 10) + newTotal - oldTotal;
                document.getElementById('totalWarningNo').textContent = parseInt(document.getElementById('totalWarningNo').textContent, 10) + newWarns - oldWarns;
                document.getElementById('totalErrorNo').textContent = parseInt(document.getElementById('totalErrorNo').textContent, 10) + newErrs - oldErrs;
            } else if (change.name == 'tags') {
                change.object.updateCardTags();
            } else if (change.name == 'isDirty') {
                if (change.object[change.name]) {
                    document.getElementById('dirtyPluginNo').textContent = ++parseInt(document.getElementById('dirtyPluginNo').textContent, 10);
                } else {
                    document.getElementById('dirtyPluginNo').textContent = --parseInt(document.getElementById('dirtyPluginNo').textContent, 10);
                }
            } else if (change.name == 'crc') {
                if (change.object[change.name] != 0) {
                    change.object.card.getElementsByClassName('crc')[0].textContent = change.object[change.name].toString(16).toUpperCase();
                }
            } else if (change.name == 'isDummy') {
                change.object.li.setAttribute('data-dummy', change.object[change.name]);
                change.object.card.setAttribute('data-dummy', change.object[change.name]);
            }
        });
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




















