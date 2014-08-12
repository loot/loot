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

    Plugin.prototype.getTagObj = function(tag) {
        var data = {
            type: '',
            name: '',
            condition: tag.condition
        };
        if (tag.name[0] == '-') {
            data.type = 'remove';
            data.name = tag.name.substr(1);
        } else {
            data.type = 'add';
            data.name = tag.name;
        }
        return data;
    }

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

        /* Fill in the editor input values. */
        if (this.userlist && !this.userlist.enabled) {
            card.shadowRoot.querySelector('#enableEdits').checked = false;
        } else {
            card.shadowRoot.querySelector('#enableEdits').checked = true;
        }
        card.shadowRoot.querySelector('#globalPriority').value = this.globalPriority;
        card.shadowRoot.querySelector('#priorityValue').value = this.modPriority;

        /* Fill in editor table data. Masterlist-originated rows should have
           their contents made read-only, and be unremovable. */
        var tables = card.shadowRoot.getElementsByTagName('table');
        for (var i = 0; i < tables.length; ++i) {
            if (tables[i].id == 'loadAfter') {

                if (this.masterlist && this.masterlist.after) {
                    this.masterlist.after.forEach(function(file) {
                        var row = tables[i].addRow(file);
                        row.querySelector('.fa-trash-o').classList.toggle('hidden');

                        var inputs = row.getElementsByTagName('input');
                        for (var j = 0; j < inputs.length; ++j) {
                            inputs[j].setAttribute('readonly', true);
                        }
                    });
                }
                if (this.userlist && this.userlist.after) {
                    this.userlist.after.forEach(function(file) {
                        tables[i].addRow(file);
                    });
                }

            } else if (tables[i].id == 'req') {

                if (this.masterlist && this.masterlist.req) {
                    this.masterlist.req.forEach(function(file) {
                        var row = tables[i].addRow(file);
                        row.querySelector('.fa-trash-o').classList.toggle('hidden');

                        var inputs = row.getElementsByTagName('input');
                        for (var j = 0; j < inputs.length; ++j) {
                            inputs[j].setAttribute('readonly', true);
                        }
                    });
                }
                if (this.userlist && this.userlist.req) {
                    this.userlist.req.forEach(function(file) {
                        tables[i].addRow(file);
                    });
                }

            } else if (tables[i].id == 'inc') {

                if (this.masterlist && this.masterlist.inc) {
                    this.masterlist.inc.forEach(function(file) {
                        var row = tables[i].addRow(file);
                        row.querySelector('.fa-trash-o').classList.toggle('hidden');

                        var inputs = row.getElementsByTagName('input');
                        for (var j = 0; j < inputs.length; ++j) {
                            inputs[j].setAttribute('readonly', true);
                        }
                    });
                }
                if (this.userlist && this.userlist.inc) {
                    this.userlist.inc.forEach(function(file) {
                        tables[i].addRow(file);
                    });
                }

            } else if (tables[i].id == 'message') {

                if (this.masterlist && this.masterlist.msg) {
                    this.masterlist.msg.forEach(function(message) {
                        var data = {
                            type: message.type,
                            content: message.content[0].str,
                            condition: message.condition,
                            language: message.content[0].lang
                        };
                        var row = tables[i].addRow(data);
                        row.querySelector('.fa-trash-o').classList.toggle('hidden');

                        var inputs = row.getElementsByTagName('input');
                        for (var j = 0; j < inputs.length; ++j) {
                            inputs[j].setAttribute('readonly', true);
                        }
                        var select = row.getElementsByTagName('select')[0].setAttribute('disabled', true);

                    });
                }
                if (this.userlist && this.userlist.msg) {
                    this.userlist.msg.forEach(function(message) {
                        var data = {
                            type: message.type,
                            content: message.content[0].str,
                            condition: message.condition,
                            language: message.content[0].lang
                        };
                        tables[i].addRow(data);
                    });
                }

            } else if (tables[i].id == 'tags') {

                if (this.masterlist && this.masterlist.tag) {
                    this.masterlist.tag.forEach(function(tag) {
                        var data = this.getTagObj(tag);
                        var row = tables[i].addRow(data);
                        row.querySelector('.fa-trash-o').classList.toggle('hidden');

                        var inputs = row.getElementsByTagName('input');
                        for (var j = 0; j < inputs.length; ++j) {
                            inputs[j].setAttribute('readonly', true);
                        }
                        var select = row.getElementsByTagName('select')[0].setAttribute('disabled', true);
                    }, this);
                }
                if (this.userlist && this.userlist.tag) {
                    this.userlist.tag.forEach(function(tag) {
                        var data = this.getTagObj(tag);
                        tables[i].addRow(data);
                    }, this);
                }

            } else if (tables[i].id == 'dirty') {

                if (this.masterlist && this.masterlist.dirty) {
                    this.masterlist.dirty.forEach(function(info) {
                        info.crc = info.crc.toString(16);
                        var row = tables[i].addRow(info);
                        row.querySelector('.fa-trash-o').classList.toggle('hidden');

                        var inputs = row.getElementsByTagName('input');
                        for (var j = 0; j < inputs.length; ++j) {
                            inputs[j].setAttribute('readonly', true);
                        }
                    });
                }
                if (this.userlist && this.userlist.dirty) {
                    this.userlist.dirty.forEach(function(info) {
                        info.crc = info.crc.toString(16);
                        tables[i].addRow(info);
                    });
                }

            }
        }

        document.getElementById('main').appendChild(card);
    }

    Plugin.prototype.createListItem = function() {
        var li = new PluginListItem();
        this.li = li;

        li.shadowRoot.querySelector('a').href = '#' + this.id;

        li.querySelector('.name').textContent = this.name;
        li.querySelector('.priority').textContent = this.getPriorityString();

        li.setAttribute('data-dummy', this.isDummy);
        li.setAttribute('data-bsa', this.loadsBSA);
        li.setAttribute('data-edits', this.userlist != undefined);

        document.getElementById('pluginsNav').appendChild(li);
    }

    Plugin.prototype.observer = function(changes) {
        changes.forEach(function(change) {
            if (change.name == 'userlist') {
                change.object.li.setAttribute('data-edits', change.object[change.name] != undefined);
                change.object.card.setAttribute('data-edits', change.object[change.name] != undefined);
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




















