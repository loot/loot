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

/* Create a <plugin-menu> element type. */
var pluginMenuProto = Object.create(HTMLElement.prototype, {

    onMenuItemClick: {
        value: function(evt) {

            var pluginID = evt.target.parentNode.host.getAttribute('data-for');
            var pluginCard = document.getElementById(pluginID);

            if (evt.target.id == 'editMetadata') {
                /* Show editing controls. */
                pluginCard.showEditor();

            } else if (evt.target.id == 'copyMetadata') {
                var request = JSON.stringify({
                    name: 'copyMetadata',
                    args: [
                        pluginCard.getElementsByTagName('h1')[0].textContent
                    ]
                });

                loot.query(request).catch(processCefError);
            } else if (evt.target.id == 'clearMetadata') {
                showMessageDialog('Clear Plugin Metadata', 'Are you sure you want to clear all existing user-added metadata from "' + pluginCard.querySelector('h1').textContent + '"?', function(result){
                    if (result) {
                        var request = JSON.stringify({
                            name: 'clearPluginMetadata',
                            args: [
                                pluginCard.getElementsByTagName('h1')[0].textContent
                            ]
                        });

                        loot.query(request).then(JSON.parse).then(function(result){
                            /* Need to empty the UI-side user metadata. */
                            for (var i = 0; i < loot.game.plugins.length; ++i) {
                                if (loot.game.plugins[i].id == pluginID) {
                                    loot.game.plugins[i].userlist = undefined;

                                    loot.game.plugins[i].modPriority = result.modPriority;
                                    loot.game.plugins[i].isGlobalPriority = result.isGlobalPriority;
                                    loot.game.plugins[i].messages = result.messages;
                                    loot.game.plugins[i].tags = result.tags;
                                    loot.game.plugins[i].isDirty = result.isDirty;

                                    break;
                                }
                            }
                        }).catch(processCefError);
                    }
                });
            }
        }
    },

    onClick: {
        value: function(evt) {
            var menus = document.getElementsByTagName('plugin-menu');

            for (var i = 0; i < menus.length; ++i) {
                if (!evt.newMenu) {
                    menus[i].parentElement.removeChild(menus[i]);
                }
            }
        }
    },

    createdCallback: {
        value: function() {

            var template = document.getElementById('pluginMenu');
            var clone = document.importNode(template.content, true);

            this.createShadowRoot().appendChild(clone);

            this.id = 'activePluginMenu';

            /* Add an event listener so that the menu gets closed. */
            main.addEventListener('click', this.onClick, false);

            /* Add event listeners for the menu items. */
            this.shadowRoot.querySelector('#editMetadata').addEventListener('click', this.onMenuItemClick, false);
            this.shadowRoot.querySelector('#copyMetadata').addEventListener('click', this.onMenuItemClick, false);
            this.shadowRoot.querySelector('#clearMetadata').addEventListener('click', this.onMenuItemClick, false);
        }
    },

    detachedCallback: {
        value: function() {
            /* Remove menu close listener. */
            document.getElementById('main').removeEventListener('click', this.onClick, false);

            /* Remove event listeners for the menu items. */
            this.shadowRoot.querySelector('#editMetadata').removeEventListener('click', this.onMenuItemClick, false);
            this.shadowRoot.querySelector('#copyMetadata').removeEventListener('click', this.onMenuItemClick, false);
            this.shadowRoot.querySelector('#clearMetadata').removeEventListener('click', this.onMenuItemClick, false);

        }
    }


});
var PluginMenu = document.registerElement('plugin-menu', {prototype: pluginMenuProto});

/* Create a <plugin-card> element type. */
var pluginCardProto = Object.create(HTMLElement.prototype, {

    showEditorTable: {
        value: function(evt) {
            var tableId = evt.target.getAttribute('data-for');
            var tables = evt.target.parentElement.parentElement.querySelectorAll('table');
            for (var i = 0; i < tables.length; ++i) {
                if (tables[i].id != tableId) {
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
                /* If an input is readonly, it doesn't validate, but that's ok
                   because the read-only inputs must be valid.
                */
                for (var i = 0; i < inputs.length; ++i) {
                    if (!inputs[i].checkValidity()) {
                        isValid = false;
                        console.log(inputs[i]);
                    }
                }
            }
            if (isValid) {
                var card = evt.target.parentElement.parentElement.parentNode.host;

                /* Set up table tab event handlers. */
                var elements = card.shadowRoot.querySelector('#tableTabs').children;
                for (var i = 0; i < elements.length; ++i) {
                    if (elements[i].hasAttribute('data-for')) {
                        elements[i].removeEventListener('click', card.showEditorTable, false);
                    }
                }

                /* Set up button event handlers. */
                card.shadowRoot.querySelector('#accept').removeEventListener('click', card.hideEditor, false);
                card.shadowRoot.querySelector('#cancel').removeEventListener('click', card.hideEditor, false);


                /* Remove drag 'n' drop event handlers. */
                var elements = document.getElementById('pluginsNav').children;
                for (var i = 0; i < elements.length; ++i) {
                    elements[i].removeAttribute('draggable', true);
                    elements[i].removeEventListener('dragstart', handlePluginDragStart, false);
                }
                elements = card.shadowRoot.querySelector('table');
                for (var i = 0; i < elements.length; ++i) {
                    if (elements[i].className.indexOf('loadAfter') != -1 || elements[i].className.indexOf('req') != -1 || elements[i].className.indexOf('inc') != -1) {
                        elements[i].removeEventListener('drop', handlePluginDrop, false);
                        elements[i].removeEventListener('dragover', handlePluginDragOver, false);
                    }
                }

                /* Disable priority hover in plugins list and enable header
                   buttons if this is the only editor instance. */
                var numEditors = parseInt(document.body.getAttribute('data-editors'), 10);
                --numEditors;

                if (numEditors == 0) {
                    document.body.classList.remove('editMode');
                }
                document.body.setAttribute('data-editors', numEditors);

                /* Hide editor. */
                card.classList.toggle('flip');
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

            /* Enable priority hover in plugins list and enable header
               buttons if this is the only editor instance. */
            var numEditors = 0;
            if (document.body.hasAttribute('data-editors')) {
                numEditors = parseInt(document.body.getAttribute('data-editors'), 10);
            }
            ++numEditors;

            if (numEditors == 1) {
                document.body.classList.add('editMode');
            }
            document.body.setAttribute('data-editors', numEditors);

            /* Now show editor. */
            this.classList.toggle('flip');
        }
    },

    onMenuButtonClick: {
        value: function(evt) {

            /* Open a new plugin menu. */
            var menu = new PluginMenu();
            var card = evt.currentTarget.parentElement.parentElement.parentNode.host;

            menu.setAttribute('data-for', card.id);


            var main = document.getElementById('main');
            main.appendChild(menu);

            /* Set page position of menu. */

            function getOffset( el, stopEl ) {
                var _x = 0;
                var _y = 0;
                while( el && el != stopEl ) {
                    _x += el.offsetLeft;
                    _y += el.offsetTop;
                    el = el.offsetParent;
                }
                return { top: _y, left: _x };
            }
            var offset = getOffset(evt.target, main);

            menu.style.top = (offset.top + evt.target.offsetHeight + 10) + 'px';
            menu.style.right = (main.offsetWidth - offset.left - evt.target.offsetWidth - 10) + 'px';

            evt.stopPropagation();

            /* To prevent the click event closing this menu just after it was
               opened, stop the current event and send off a new one. */
            menu.dispatchEvent(new CustomEvent('click', { newMenu: true }));
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

            this.shadowRoot.querySelector('#menuButton').addEventListener('click', this.onMenuButtonClick, false);

            var hoverTargets = this.shadowRoot.querySelectorAll('[title]');
            for (var i = 0; i < hoverTargets.length; ++i) {
                hoverTargets[i].addEventListener('mouseenter', showHoverText, false);
                hoverTargets[i].addEventListener('mouseleave', hideHoverText, false);
            }

        }

    },

    detachedCallback: {
        value: function() {
            this.shadowRoot.querySelector('#menuButton').removeEventListener('click', this.onMenuButtonClick, false);

            var hoverTargets = this.shadowRoot.querySelectorAll('[title]');
            for (var i = 0; i < hoverTargets.length; ++i) {
                hoverTargets[i].removeEventListener('mouseenter', showHoverText, false);
                hoverTargets[i].removeEventListener('mouseleave', hideHoverText, false);
            }
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

            var hoverTargets = this.shadowRoot.querySelectorAll('[title]');
            for (var i = 0; i < hoverTargets.length; ++i) {
                hoverTargets[i].addEventListener('mouseenter', showHoverText, false);
                hoverTargets[i].addEventListener('mouseleave', hideHoverText, false);
            }

        }

    },

    detachedCallback: {
        value: function() {
            var hoverTargets = this.shadowRoot.querySelectorAll('[title]');
            for (var i = 0; i < hoverTargets.length; ++i) {
                hoverTargets[i].removeEventListener('mouseenter', showHoverText, false);
                hoverTargets[i].removeEventListener('mouseleave', hideHoverText, false);
            }
        }
    }

});
var PluginListItem = document.registerElement('plugin-li', {
    prototype: pluginLIProto,
    extends: 'li'
});

/* Create a <message-dialog> element type that extends from <dialog>. */
var messageDialogProto = Object.create(HTMLDialogElement.prototype, {

    onClose: {
        value: function(evt) {
            var ret = evt.target.returnValue == 'true';

            evt.target.parentElement.removeChild(evt.target);

            if (evt.target.closeCallback != undefined) {
                evt.target.closeCallback(ret);
            }
        }
    },

    onButtonClick: {
        value: function(evt) {
            var dialog = evt.currentTarget.parentElement.parentElement;

            dialog.querySelector('.accept').removeEventListener('click', dialog.onButtonClick, false);
            dialog.querySelector('.cancel').removeEventListener('click', dialog.onButtonClick, false);

            dialog.close( evt.target.className == 'accept' );
        }
    },

    showModal: {
        /* A type of 'error' or 'info' produces an 'OK'-only dialog box. */
        value: function(type, title, text, closeCallback) {

            this.closeCallback = closeCallback;

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

            this.addEventListener('close', this.onClose, false);
        }
    },

    detachedCallback: {
        value: function() {
            this.removeEventListener('close', this.onClose, false);
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
            var tr = evt.target.parentElement;
            var tbody = tr.parentElement
            var table = tbody.parentElement;

            /* Remove deletion listener. */
            evt.target.removeEventListener('click', table.removeRow, false);

            /* Now remove row. */
            tbody.removeChild(tr);
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

            /* Add deletion listener. */
            row.getElementsByClassName('fa-trash-o')[0].addEventListener('click', table.removeRow, false);
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

            /* Add deletion listener. */
            row.getElementsByClassName('fa-trash-o')[0].addEventListener('click', this.removeRow, false);

            return row;
        }
    },

    createdCallback: {

        value: function() {
            /* Add new row listener. */
            this.querySelector('tbody tr:last-child').addEventListener('dblclick', this.addEmptyRow, false);
        }

    },

    detachedCallback: {
        value: function() {
            /* Remove event listeners. */
            var tbody = this.getElementsByTagName('tbody')[0];

            /* Remove deletion listener. */
            var icons = tbody.getElementsByClassName('fa-trash-o');
            for (var i = 0; i < icons.length; ++i) {
                icons[i].removeEventListener('click', this.removeRow, false);
            }

            /* Remove new row listener. */
            this.querySelector('tbody tr:last-child').removeEventListener('dblclick', this.addEmptyRow, false);

        }
    },

});
var EditableTable = document.registerElement('editable-table', {
    prototype: EditableTableProto,
    extends: 'table'
});
