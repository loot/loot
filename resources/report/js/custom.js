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

    showEditorTable: {
        value: function(evt) {
            var tableClass = evt.target.getAttribute('data-for');
            var tables = evt.target.parentElement.querySelectorAll('table');
            for (var i = 0; i < tables.length; ++i) {
                if (tables[i].className.indexOf(tableClass) == -1) {
                    hideElement(tables[i]);
                } else {
                    showElement(tables[i]);
                }
            }
            evt.target.parentElement.querySelector('.selected').classList.toggle('selected');
            evt.target.classList.toggle('selected');
        }
    },

    hideEditor: {
        value: function(evt) {
            var isValid = true;
            if (evt.target.className.indexOf('accept') != -1) {
                /* First validate table inputs. */
                var inputs = evt.target.parentElement.parentElement.getElementsByTagName('input');
                /* If an input is readonly, it doesn't validate, so check required / value length explicitly.
                */
                for (var i = 0; i < inputs.length; ++i) {
                    if (inputs[i].readOnly) {
                        if (inputs[i].required && inputs[i].value.length == 0) {
                            isValid = false;
                            console.log(inputs[i]);
                            inputs[i].readOnly = false;
                        }
                    } else if (!inputs[i].checkValidity()) {
                        isValid = false;
                        console.log(inputs[i]);
                    }
                }
            }
            if (isValid) {
                /* Remove drag 'n' drop event handlers. */
                var elements = document.getElementById('pluginsNav').children;
                for (var i = 0; i < elements.length; ++i) {
                    elements[i].removeAttribute('draggable', true);
                    elements[i].removeEventListener('dragstart', handlePluginDragStart, false);
                }

                /* Disable priority hover in plugins list. */
                document.getElementById('pluginsNav').classList.toggle('editMode', false);

                /* Enable header buttons. */
                document.getElementsByTagName('header')[0].classList.toggle('editMode', false);

                /* Hide editor. */
                evt.target.parentElement.parentElement.parentElement.classList.toggle('flip');
            }
        }
    },

    showEditor: {
        value: function() {

            /* Set up table tab event handlers. */
            var elements = this.shadowRoot.querySelector('#tableTabs').children;
            for (var i = 0; i < elements.length; ++i) {
                if (elements[i].hasAttribute('data-for')) {
                    elements[i].addEventListener('click', this.showEditorTable, false);
                }
            }

            /* Set up button event handlers. */
            this.shadowRoot.querySelector('#accept').addEventListener('click', this.hideEditor, false);
            this.shadowRoot.querySelector('#cancel').addEventListener('click', this.hideEditor, false);

            /* Set up drag 'n' drop event handlers. */
            elements = document.getElementById('pluginsNav').children;
            for (var i = 0; i < elements.length; ++i) {
                elements[i].setAttribute('draggable', true);
                elements[i].addEventListener('dragstart', handlePluginDragStart, false);
            }
            elements = this.shadowRoot.querySelector('table');
            for (var i = 0; i < elements.length; ++i) {
                if (elements[i].className.indexOf('loadAfter') != -1 || elements[i].className.indexOf('req') != -1 || elements[i].className.indexOf('inc') != -1) {
                    elements[i].addEventListener('drop', handlePluginDrop, false);
                    elements[i].addEventListener('dragover', handlePluginDragOver, false);
                }
            }

            /* Enable priority hover in plugins list. */
            document.getElementById('pluginsNav').classList.toggle('editMode', true);
            /* Disable header buttons. */
            document.getElementsByTagName('header')[0].classList.toggle('editMode', true);

            /* Now show editor. */
            this.classList.toggle('flip');
        }
    },


    onMenuItemClick: {
        value: function(evt) {
            if (evt.target.id == 'editMetadata') {
                /* Show editing controls. */
                evt.target.parentElement.parentElement.parentNode.host.showEditor();

            } else if (evt.target.id == 'copyMetadata') {

            } else if (evt.target.id == 'clearMetadata') {
                showMessageDialog('Clear Plugin Metadata', 'Are you sure you want to clear all existing user-added metadata from "' + evt.target.parentElement.parentElement.querySelector('h1').textContent + '"?');
            }
        }
    },

    onMenuClick: {
        value: function(evt) {
            var section = evt.currentTarget.parentElement.parentElement;
            section.querySelector('#editMetadata').addEventListener('click', section.parentNode.host.onMenuItemClick, false);
            section.querySelector('#copyMetadata').addEventListener('click', section.parentNode.host.onMenuItemClick, false);
            section.querySelector('#clearMetadata').addEventListener('click', section.parentNode.host.onMenuItemClick, false);

            section.querySelector('#menu').classList.toggle('hidden');
        }
    },

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

            var tagAdd = document.createElement('div');
            tagAdd.className = 'tag add';
            this.appendChild(tagAdd);
            var tagRemove = document.createElement('div');
            tagRemove.className = 'tag remove';
            this.appendChild(tagRemove);

            var messages = document.createElement('ul');
            this.appendChild(messages);

            this.shadowRoot.querySelector('#menuButton').addEventListener('click', this.onMenuClick, false);

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

/* Create a <message-dialog> element type that extends from <dialog>. */
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


/* Create a <editable-table> element type that extends from <table>. */
var EditableTableProto = Object.create(HTMLTableElement.prototype, {

    removeRow: {
        value: function(evt) {
            evt.target.parentElement.parentElement.removeChild(evt.target.parentElement);
        }
    },

    addEmptyRow: {
        value: function(evt) {
            /* Create new row. */
            var table = evt.currentTarget.parentElement.parentElement;
            var rowTemplateId = table.getAttribute('data-template');
            var content = document.getElementById(rowTemplateId).content;
            var row = document.importNode(content, true);
            table.getElementsByTagName('tbody')[0].insertBefore(row, evt.currentTarget);
            row = evt.currentTarget.previousElementSibling;

            /* Enable row editing. */
            var inputs = row.getElementsByTagName('input');
            for (var i = 0; i < inputs.length; ++i) {
                inputs[i].removeAttribute('readonly');
                inputs[i].addEventListener('dblclick', toggleInputRO, false);
            }
            /* Add deletion listener. */
            row.getElementsByClassName('fa-trash-o')[0].addEventListener('click', this.removeRow, false);
        }
    },

    addRow: {
        value: function(tableData) {
            var rowTemplateId = this.getAttribute('data-template');
            var content = document.getElementById(rowTemplateId).content;
            var row = document.importNode(content, true);
            var tbody = this.getElementsByTagName('tbody')[0];
            tbody.insertBefore(row, tbody.lastElementChild);
            row = tbody.lastElementChild.previousElementSibling;

            /* Data is an object with keys that match element class names. */
            for (var key in tableData) {
                row.getElementsByClassName(key)[0].value = tableData[key];
            }

            /* Enable row editing. */
            var inputs = row.getElementsByTagName('input');
            for (var i = 0; i < inputs.length; ++i) {
                inputs[i].addEventListener('dblclick', toggleInputRO, false);
            }
            /* Add deletion listener. */
            row.getElementsByClassName('fa-trash-o')[0].addEventListener('click', this.removeRow, false);
        }
    },

    createdCallback: {

        value: function() {
            /* Add new row listener. */
            this.querySelector('tbody tr:last-child').addEventListener('dblclick', this.addEmptyRow, false);
        }

    },

});
var EditableTable = document.registerElement('editable-table', {
    prototype: EditableTableProto,
    extends: 'table'
});
