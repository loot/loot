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

    this.id = this.name.replace(/\s+/g, '');

    Plugin.prototype.onMenuItemClick = function(evt) {

    }

    Plugin.prototype.onMenuClick = function(evt) {
        card.querySelector('.editMetadata').addEventListener('click', this.onMenuItemClick, false);
        card.querySelector('.editMetadata').addEventListener('click', this.onMenuItemClick, false);
        card.querySelector('.editMetadata').addEventListener('click', this.onMenuItemClick, false);
    }


    Plugin.prototype.createCard = function() {
        var card = new PluginCard();
        this.card = card;

        card.id = this.id;
        card.querySelector('h1').textContent = this.name;
        card.querySelector('.crc').textContent = this.crc;
        card.querySelector('.version').textContent = this.version;

        card.setAttribute('data-active', this.isActive);
        card.setAttribute('data-dummy', this.isDummy);
        card.setAttribute('data-bsa', this.loadsBSA);
        card.setAttribute('data-edits', this.userlist != undefined);

        card.shadowRoot.querySelector('.pluginMenu').addEventListener('click', this.onMenuClick, false);


        document.getElementById('main').appendChild(card);
    }

    Plugin.prototype.getPriorityString = function() {
        if (this.userlist) {
            var priorityText = 'Priority: ' + this.userlist.modPriority + ', Global: ';
            if (this.userlist.isGlobalPriority) {
                priorityText += '✓';
            } else {
                priorityText += '✗';
            }
            return priorityText;
        } else if (this.masterlist) {
            var priorityText = 'Priority: ' + this.masterlist.modPriority + ', Global: ';
            if (this.masterlist.isGlobalPriority) {
                priorityText += '✓';
            } else {
                priorityText += '✗';
            }
            return priorityText;
        } else {
            return 'Priority: 0, Global: ✗';
        }
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




















