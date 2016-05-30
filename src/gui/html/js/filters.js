'use strict';
(function exportModule(root, factory) {
  if (typeof define === 'function' && define.amd) {
    // AMD. Register as an anonymous module.
    define([], factory);
  } else {
    // Browser globals
    root.loot = root.loot || {};
    root.loot.Filters = factory();
  }
}(this, () => class {
  constructor(l10n) {
    /* Plugin filters */
    this.hideMessagelessPlugins = false;
    this.hideInactivePlugins = false;
    this.conflictingPluginNames = [];
    this.contentSearchString = '';

    /* Plugin content filters */
    this.hideVersionNumbers = false;
    this.hideCRCs = false;
    this.hideBashTags = false;
    this.hideAllPluginMessages = false;
    this.hideNotes = false;
    this.hideDoNotCleanMessages = false;

    this._doNotCleanString = l10n.translate('Do not clean').toLowerCase();
  }

  pluginFilter(plugin) {
    if (this.hideInactivePlugins && !plugin.isActive) {
      return false;
    }

    if (this.conflictingPluginNames.length !== 0
        && this.conflictingPluginNames.indexOf(plugin.name) === -1) {
      return false;
    }

    if (this.hideMessagelessPlugins
        && plugin.getCardContent(this).messages.length === 0) {
      return false;
    }

    if (this.contentSearchString.length !== 0
        && !plugin.getCardContent(this).containsText(this.contentSearchString)) {
      return false;
    }

    return true;
  }

  messageFilter(message) {
    if (this.hideAllPluginMessages) {
      return false;
    }

    if (this.hideNotes && message.type === 'say') {
      return false;
    }

    if (this.hideDoNotCleanMessages
        && message.content.toLowerCase().indexOf(this._doNotCleanString) !== -1) {
      return false;
    }

    return true;
  }
}));
