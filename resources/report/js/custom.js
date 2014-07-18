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
/* Create a <plugin-card> element type. */
var pluginCardProto = Object.create(HTMLElement.prototype, {

    createdCallback: {

        value: function() {

            var template = document.getElementById('pluginCard');
            var clone = document.importNode(template.content, true);

            this.createShadowRoot().appendChild(clone);

            var h1 = document.createElement('h1');
            this.appendChild(h1);
            var crc = document.createElement('div');
            crc.className = 'crc';
            this.appendChild(crc);
            var version = document.createElement('div');
            version.className = 'version';
            this.appendChild(version);

        }

    }

});
var PluginCard = document.registerElement('plugin-card', {prototype: pluginCardProto});

/* Create a <plugin-li> element type that extends from <li>. */
var pluginLIProto = Object.create(HTMLLIElement.prototype, {

    createdCallback: {

        value: function() {

            var template = document.getElementById('pluginLI');
            var clone = document.importNode(template.content, true);

            this.createShadowRoot().appendChild(clone);

            var name = document.createElement('span');
            name.className = 'name';
            this.appendChild(name);
            var priority = document.createElement('span');
            priority.className = 'priority';
            this.appendChild(priority);

        }

    }

});
var PluginListItem = document.registerElement('plugin-li', {
    prototype: pluginLIProto,
    extends: 'li'
});

/* Create a <message-dialog> element type that extends from <dialog>.
   Use a data-type member on the element to style its type. */
var messageDialogProto = Object.create(HTMLDialogElement.prototype, {

    onButtonClick: {
        value: function(evt) {
            evt.currentTarget.parentElement.parentElement.close( evt.target.className == 'accept' );
        }
    },

    showModal: {
        /* A type of 'error' or 'info' produces an 'OK'-only dialog box. */
        value: function(type, title, text, onClose) {

            this.querySelector('h1').textContent = title;
            this.querySelector('p').textContent = text;

            this.querySelector('.accept').addEventListener('click', this.onButtonClick, false);
            this.querySelector('.cancel').addEventListener('click', this.onButtonClick, false);

            if (type == 'error' || type == 'info') {
                this.querySelector('.accept').textContent = 'OK';
                this.querySelector('.cancel').style.display = 'hidden';
            }

            HTMLDialogElement.prototype.showModal.call(this);
        }
    },

    createdCallback: {

        value: function() {

            var icon = document.createElement('span');
            icon.className = 'fa fa-exclamation-circle';
            this.appendChild(icon);

            var h1 = document.createElement('h1');
            this.appendChild(h1);

            var message = document.createElement('p');
            this.appendChild(message);

            var buttons = document.createElement('div');
            buttons.className = 'buttons';
            this.appendChild(buttons);

            var accept = document.createElement('button');
            accept.className = 'accept';
            accept.textContent = 'Yes';
            buttons.appendChild(accept);

            var cancel = document.createElement('button');
            cancel.className = 'cancel';
            cancel.textContent = 'Cancel';
            buttons.appendChild(cancel);
        }

    },

    detachedCallback: {
        value: function() {

            this.removeEventListener('click', this.onButtonClick, false);
        }
    }

});
var MessageDialog = document.registerElement('message-dialog', {
    prototype: messageDialogProto,
    extends: 'dialog'
});
