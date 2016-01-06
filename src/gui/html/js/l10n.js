'use strict';

(function exportModule(root, factory) {
  if (typeof define === 'function' && define.amd) {
    // AMD. Register as an anonymous module.
    define([], factory);
  } else {
    // Browser globals
    root.loot = root.loot || {};
    root.loot.translateStaticText = factory();
  }
}(this, () => {
  function translatePluginCardTemplate(l10n) {
    /* Plugin card template. */
    let pluginCard = document.querySelector('link[rel="import"][href$="loot-plugin-card.html"]');
    if (pluginCard) {
      pluginCard = pluginCard.import.querySelector('template').content;
    } else {
      pluginCard = document.querySelector('polymer-element[name="loot-plugin-card"]').querySelector('template').content;
    }

    pluginCard.getElementById('activeTick').setAttribute('label', l10n.translate('Active Plugin'));
    pluginCard.getElementById('isMaster').setAttribute('label', l10n.translate('Master File'));
    pluginCard.getElementById('emptyPlugin').setAttribute('label', l10n.translate('Empty Plugin'));
    pluginCard.getElementById('loadsArchive').setAttribute('label', l10n.translate('Loads Archive'));
    pluginCard.getElementById('hasUserEdits').setAttribute('label', l10n.translate('Has User Metadata'));

    pluginCard.getElementById('showOnlyConflicts').previousElementSibling.textContent = l10n.translate('Show Only Conflicts');
    pluginCard.getElementById('editMetadata').lastChild.textContent = l10n.translate('Edit Metadata');
    pluginCard.getElementById('copyMetadata').lastChild.textContent = l10n.translate('Copy Metadata');
    pluginCard.getElementById('clearMetadata').lastChild.textContent = l10n.translate('Clear User Metadata');
  }

  function translatePluginEditorTemplate(l10n) {
    /* Plugin editor template. */
    let pluginEditor = document.querySelector('link[rel="import"][href$="loot-plugin-card.html"]');
    if (pluginEditor) {
      pluginEditor = pluginEditor.import.querySelector('link[rel="import"][href$="loot-plugin-editor.html"]').import.querySelector('template').content;
    } else {
      pluginEditor = document.querySelector('polymer-element[name="loot-plugin-editor"]').querySelector('template').content;
    }

    pluginEditor.getElementById('activeTick').setAttribute('label', l10n.translate('Active Plugin'));
    pluginEditor.getElementById('isMaster').setAttribute('label', l10n.translate('Master File'));
    pluginEditor.getElementById('emptyPlugin').setAttribute('label', l10n.translate('Empty Plugin'));
    pluginEditor.getElementById('loadsArchive').setAttribute('label', l10n.translate('Loads Archive'));

    pluginEditor.getElementById('enableEdits').previousElementSibling.textContent = l10n.translate('Enable Edits');
    pluginEditor.getElementById('globalPriority').parentElement.parentElement.setAttribute('label', l10n.translate('Global priorities are compared against all other plugins. Normal priorities are compared against only conflicting plugins.'));
    pluginEditor.getElementById('globalPriority').previousElementSibling.textContent = l10n.translate('Global Priority');
    pluginEditor.getElementById('priorityValue').parentElement.previousElementSibling.textContent = l10n.translate('Priority Value');

    pluginEditor.getElementById('tableTabs').querySelector('[data-for=main]').textContent = l10n.translate('Main');
    pluginEditor.getElementById('tableTabs').querySelector('[data-for=loadAfter]').textContent = l10n.translate('Load After');
    pluginEditor.getElementById('tableTabs').querySelector('[data-for=req]').textContent = l10n.translate('Requirements');
    pluginEditor.getElementById('tableTabs').querySelector('[data-for=inc]').textContent = l10n.translate('Incompatibilities');
    pluginEditor.getElementById('tableTabs').querySelector('[data-for=message]').textContent = l10n.translate('Messages');
    pluginEditor.getElementById('tableTabs').querySelector('[data-for=tags]').textContent = l10n.translate('Bash Tags');
    pluginEditor.getElementById('tableTabs').querySelector('[data-for=dirty]').textContent = l10n.translate('Dirty Info');
    pluginEditor.getElementById('tableTabs').querySelector('[data-for=locations]').textContent = l10n.translate('Locations');

    pluginEditor.getElementById('loadAfter').querySelector('th:first-child').textContent = l10n.translate('Filename');
    pluginEditor.getElementById('loadAfter').querySelector('th:nth-child(2)').textContent = l10n.translate('Display Name');
    pluginEditor.getElementById('loadAfter').querySelector('th:nth-child(3)').textContent = l10n.translate('Condition');

    pluginEditor.getElementById('req').querySelector('th:first-child').textContent = l10n.translate('Filename');
    pluginEditor.getElementById('req').querySelector('th:nth-child(2)').textContent = l10n.translate('Display Name');
    pluginEditor.getElementById('req').querySelector('th:nth-child(3)').textContent = l10n.translate('Condition');

    pluginEditor.getElementById('inc').querySelector('th:first-child').textContent = l10n.translate('Filename');
    pluginEditor.getElementById('inc').querySelector('th:nth-child(2)').textContent = l10n.translate('Display Name');
    pluginEditor.getElementById('inc').querySelector('th:nth-child(3)').textContent = l10n.translate('Condition');

    pluginEditor.getElementById('message').querySelector('th:first-child').textContent = l10n.translate('Type');
    pluginEditor.getElementById('message').querySelector('th:nth-child(2)').textContent = l10n.translate('Content');
    pluginEditor.getElementById('message').querySelector('th:nth-child(3)').textContent = l10n.translate('Condition');
    pluginEditor.getElementById('message').querySelector('th:nth-child(4)').textContent = l10n.translate('Language');

    pluginEditor.getElementById('tags').querySelector('th:first-child').textContent = l10n.translate('Add/Remove');
    pluginEditor.getElementById('tags').querySelector('th:nth-child(2)').textContent = l10n.translate('Bash Tag');
    pluginEditor.getElementById('tags').querySelector('th:nth-child(3)').textContent = l10n.translate('Condition');

    pluginEditor.getElementById('dirty').querySelector('th:first-child').textContent = l10n.translate('CRC');
    pluginEditor.getElementById('dirty').querySelector('th:nth-child(2)').textContent = l10n.translate('ITM Count');
    pluginEditor.getElementById('dirty').querySelector('th:nth-child(3)').textContent = l10n.translate('Deleted References');
    pluginEditor.getElementById('dirty').querySelector('th:nth-child(4)').textContent = l10n.translate('Deleted Navmeshes');
    pluginEditor.getElementById('dirty').querySelector('th:nth-child(5)').textContent = l10n.translate('Cleaning Utility');

    pluginEditor.getElementById('locations').querySelector('th:first-child').textContent = l10n.translate('URL');
    pluginEditor.getElementById('locations').querySelector('th:nth-child(2)').textContent = l10n.translate('Name');

    pluginEditor.getElementById('accept').parentElement.setAttribute('label', l10n.translate('Apply'));
    pluginEditor.getElementById('cancel').parentElement.setAttribute('label', l10n.translate('Cancel'));
  }

  function translatePluginListItemTemplate(l10n) {
    /* Plugin List Item Template */
    let pluginItem = document.querySelector('link[rel="import"][href$="loot-plugin-item.html"]');
    if (pluginItem) {
      pluginItem = pluginItem.import.querySelector('template').content;
    } else {
      pluginItem = document.querySelector('polymer-element[name="loot-plugin-item"]').querySelector('template').content;
    }
    pluginItem.querySelector('#secondary core-tooltip').setAttribute('label', l10n.translate('Global Priority'));
    pluginItem.getElementById('hasUserEditsTooltip').textContent = l10n.translate('Has User Metadata');
    pluginItem.getElementById('editorIsOpenTooltip').textContent = l10n.translate('Editor Is Open');
  }

  function translateMessageDialogTemplate(l10n) {
    /* Plugin List Item Template */
    let messageDialog = document.querySelector('link[rel="import"][href$="loot-message-dialog.html"]');
    if (messageDialog) {
      messageDialog = messageDialog.import.querySelector('template').content;
    } else {
      messageDialog = document.querySelector('polymer-element[name="loot-message-dialog"]').querySelector('template').content;
    }
    messageDialog.getElementById('confirm').textContent = l10n.translate('OK');
    messageDialog.getElementById('dismiss').textContent = l10n.translate('Cancel');
  }

  function translateFileRowTemplate(l10n) {
    /* File row template */
    let fileRow = document.querySelector('link[rel="import"][href$="editable-table.html"]');
    if (fileRow) {
      fileRow = fileRow.import.querySelector('#fileRow').content;
    } else {
      fileRow = document.querySelector('#fileRow').content;
    }
    fileRow.querySelector('loot-validated-input').setAttribute('error', l10n.translate('A filename is required.'));
    fileRow.querySelector('core-tooltip').setAttribute('label', l10n.translate('Delete Row'));
  }

  function translateMessageRowTemplate(l10n) {
    /* Message row template */
    let messageRow = document.querySelector('link[rel="import"][href$="editable-table.html"]');
    if (messageRow) {
      messageRow = messageRow.import.querySelector('#messageRow').content;
    } else {
      messageRow = document.querySelector('#messageRow').content;
    }
    messageRow.querySelector('.type').children[0].textContent = l10n.translate('Note');
    messageRow.querySelector('.type').children[1].textContent = l10n.translate('Warning');
    messageRow.querySelector('.type').children[2].textContent = l10n.translate('Error');
    messageRow.querySelector('loot-validated-input').setAttribute('error', l10n.translate('A content string is required.'));
    messageRow.querySelector('core-tooltip').setAttribute('label', l10n.translate('Delete Row'));
  }

  function translateTagRowTemplate(l10n) {
    /* Tag row template */
    let tagRow = document.querySelector('link[rel="import"][href$="editable-table.html"]');
    if (tagRow) {
      tagRow = tagRow.import.querySelector('#tagRow').content;
    } else {
      tagRow = document.querySelector('#tagRow').content;
    }
    tagRow.querySelector('.type').children[0].textContent = l10n.translate('Add');
    tagRow.querySelector('.type').children[1].textContent = l10n.translate('Remove');
    tagRow.querySelector('loot-validated-input').setAttribute('error', l10n.translate('A name is required.'));
    tagRow.querySelector('core-tooltip').setAttribute('label', l10n.translate('Delete Row'));
  }

  function translateDirtyInfoRowTemplate(l10n) {
    /* Dirty Info row template */
    let dirtyInfoRow = document.querySelector('link[rel="import"][href$="editable-table.html"]');
    if (dirtyInfoRow) {
      dirtyInfoRow = dirtyInfoRow.import.querySelector('#dirtyInfoRow').content;
    } else {
      dirtyInfoRow = document.querySelector('#dirtyInfoRow').content;
    }

    dirtyInfoRow.querySelector('loot-validated-input.crc').setAttribute('error', l10n.translate('A CRC is required.'));
    dirtyInfoRow.querySelector('loot-validated-input.itm').setAttribute('error', l10n.translate('Values must be integers.'));
    dirtyInfoRow.querySelector('loot-validated-input.udr').setAttribute('error', l10n.translate('Values must be integers.'));
    dirtyInfoRow.querySelector('loot-validated-input.nav').setAttribute('error', l10n.translate('Values must be integers.'));
    dirtyInfoRow.querySelector('loot-validated-input.util').setAttribute('error', l10n.translate('A utility name is required.'));
    dirtyInfoRow.querySelector('core-tooltip').setAttribute('label', l10n.translate('Delete Row'));
  }

  function translateLocationRowTemplate(l10n) {
    /* Location row template */
    let locationRow = document.querySelector('link[rel="import"][href$="editable-table.html"]');
    if (locationRow) {
      locationRow = locationRow.import.querySelector('#locationRow').content;
    } else {
      locationRow = document.querySelector('#locationRow').content;
    }
    locationRow.querySelector('loot-validated-input').setAttribute('error', l10n.translate('A link is required.'));
    locationRow.querySelector('core-tooltip').setAttribute('label', l10n.translate('Delete Row'));
  }

  function translateGameRowTemplate(l10n) {
    /* Game row template */
    let gameRow = document.querySelector('link[rel="import"][href$="editable-table.html"]');
    if (gameRow) {
      gameRow = gameRow.import.querySelector('#gameRow').content;
    } else {
      gameRow = document.querySelector('#gameRow').content;
    }
    gameRow.querySelector('loot-validated-input.name').setAttribute('error', l10n.translate('A name is required.'));
    gameRow.querySelector('loot-validated-input.folder').setAttribute('error', l10n.translate('A folder is required.'));
    gameRow.querySelector('core-tooltip').setAttribute('label', l10n.translate('Delete Row'));
  }

  function translateNewRowTemplate(l10n) {
    /* New row template */
    let newRow = document.querySelector('link[rel="import"][href$="editable-table.html"]');
    if (newRow) {
      newRow = newRow.import.querySelector('#newRow').content;
    } else {
      newRow = document.querySelector('#newRow').content;
    }
    newRow.querySelector('core-tooltip').setAttribute('label', l10n.translate('Add New Row'));
  }

  function translateMainToolbar(l10n) {
    /* Main toolbar */
    document.getElementById('jumpToGeneralInfo').parentElement.label = l10n.translate('Jump To General Information');
    document.getElementById('sortButton').parentElement.label = l10n.translate('Sort Plugins');
    document.getElementById('updateMasterlistButton').parentElement.label = l10n.translate('Update Masterlist');
    document.getElementById('applySortButton').textContent = l10n.translate('Apply');
    document.getElementById('cancelSortButton').textContent = l10n.translate('Cancel');
    document.getElementById('showSearch').parentElement.label = l10n.translate('Search Cards');

    /* Toolbar menu */
    document.getElementById('redatePluginsButton').lastElementChild.textContent = l10n.translate('Redate Plugins');
    document.getElementById('openLogButton').lastElementChild.textContent = l10n.translate('Open Debug Log Location');
    document.getElementById('wipeUserlistButton').lastElementChild.textContent = l10n.translate('Clear All User Metadata');
    document.getElementById('copyLoadOrderButton').lastElementChild.textContent = l10n.translate('Copy Load Order');
    document.getElementById('copyContentButton').lastElementChild.textContent = l10n.translate('Copy Content');
    document.getElementById('refreshContentButton').lastElementChild.textContent = l10n.translate('Refresh Content');
    document.getElementById('helpButton').lastElementChild.textContent = l10n.translate('View Documentation');
    document.getElementById('aboutButton').lastElementChild.textContent = l10n.translate('About');
    document.getElementById('settingsButton').lastElementChild.textContent = l10n.translate('Settings');
    document.getElementById('quitButton').lastElementChild.textContent = l10n.translate('Quit');

    /* Search bar */
    document.getElementById('searchBar').shadowRoot.getElementById('search').label = l10n.translate('Search cards');
  }

  function translateSidebar(l10n) {
    /* Nav items */
    document.getElementById('sidebarTabs').firstElementChild.textContent = l10n.translate('Plugins');
    document.getElementById('sidebarTabs').firstElementChild.nextElementSibling.textContent = l10n.translate('Filters');
    document.getElementById('contentFilter').parentElement.label = l10n.translate('Press Enter or click outside the input to set the filter.');
    document.getElementById('contentFilter').label = l10n.translate('Filter content');

    /* Filters */
    document.getElementById('hideVersionNumbers').label = l10n.translate('Hide version numbers');
    document.getElementById('hideCRCs').label = l10n.translate('Hide CRCs');
    document.getElementById('hideBashTags').label = l10n.translate('Hide Bash Tags');
    document.getElementById('hideNotes').label = l10n.translate('Hide notes');
    document.getElementById('hideDoNotCleanMessages').label = l10n.translate('Hide \'Do not clean\' messages');
    document.getElementById('hideAllPluginMessages').label = l10n.translate('Hide all plugin messages');
    document.getElementById('hideInactivePlugins').label = l10n.translate('Hide inactive plugins');
    document.getElementById('hideMessagelessPlugins').label = l10n.translate('Hide messageless plugins');
    document.getElementById('hiddenPluginsTxt').textContent = l10n.translate('Hidden plugins:');
    document.getElementById('hiddenMessagesTxt').textContent = l10n.translate('Hidden messages:');
  }

  function translateSummaryCard(l10n) {
    /* Summary */
    document.getElementById('summary').firstElementChild.textContent = l10n.translate('General Information');
    document.getElementById('masterlistRevision').previousElementSibling.textContent = l10n.translate('Masterlist Revision');
    document.getElementById('masterlistDate').previousElementSibling.textContent = l10n.translate('Masterlist Date');
    document.getElementById('totalWarningNo').previousElementSibling.textContent = l10n.translate('Warnings');
    document.getElementById('totalErrorNo').previousElementSibling.textContent = l10n.translate('Errors');
    document.getElementById('totalMessageNo').previousElementSibling.textContent = l10n.translate('Total Messages');
    document.getElementById('activePluginNo').previousElementSibling.textContent = l10n.translate('Active Plugins');
    document.getElementById('dirtyPluginNo').previousElementSibling.textContent = l10n.translate('Dirty Plugins');
    document.getElementById('totalPluginNo').previousElementSibling.textContent = l10n.translate('Total Plugins');
  }

  function translateSettingsDialog(l10n) {
    /* Settings dialog */
    document.getElementById('settingsDialog').heading = l10n.translate('Settings');

    const defaultGameSelect = document.getElementById('defaultGameSelect');
    defaultGameSelect.previousElementSibling.textContent = l10n.translate('Default Game');
    defaultGameSelect.firstElementChild.textContent = l10n.translate('Autodetect');
    /* The selected text doesn't update, so force that translation. */
    defaultGameSelect.shadowRoot.querySelector('paper-dropdown-menu').selectedItemLabel = defaultGameSelect.shadowRoot.querySelector('core-menu').selectedItem.textContent;

    document.getElementById('languageSelect').previousElementSibling.textContent = l10n.translate('Language');
    document.getElementById('languageSelect').previousElementSibling.label = l10n.translate('Language changes will be applied after LOOT is restarted.');

    document.getElementById('enableDebugLogging').previousElementSibling.textContent = l10n.translate('Enable debug logging');
    document.getElementById('enableDebugLogging').parentElement.label = l10n.translate('The output is logged to the LOOTDebugLog.txt file.');

    document.getElementById('updateMasterlist').previousElementSibling.textContent = l10n.translate('Update masterlist before sorting');

    const gameTable = document.getElementById('gameTable');
    gameTable.querySelector('th:first-child').textContent = l10n.translate('Name');
    gameTable.querySelector('th:nth-child(2)').textContent = l10n.translate('Base Game');
    gameTable.querySelector('th:nth-child(3)').textContent = l10n.translate('LOOT Folder');
    gameTable.querySelector('th:nth-child(4)').textContent = l10n.translate('Master File');
    gameTable.querySelector('th:nth-child(5)').textContent = l10n.translate('Masterlist Repository URL');
    gameTable.querySelector('th:nth-child(6)').textContent = l10n.translate('Masterlist Repository Branch');
    gameTable.querySelector('th:nth-child(7)').textContent = l10n.translate('Install Path');
    gameTable.querySelector('th:nth-child(8)').textContent = l10n.translate('Install Path Registry Key');

    /* As the game table is attached on launch, its "Add New Row"
       tooltip doesn't benefit from the template translation above. */
    gameTable.querySelector('tr:last-child core-tooltip').setAttribute('label', l10n.translate('Add New Row'));

    document.getElementById('settingsDialog').getElementsByClassName('accept')[0].textContent = l10n.translate('Apply');
    document.getElementById('settingsDialog').getElementsByClassName('cancel')[0].textContent = l10n.translate('Cancel');
  }

  function translateFirstRunDialog(l10n) {
    /* First-run dialog */
    const firstRun = document.getElementById('firstRun');
    firstRun.heading = l10n.translate('First-Time Tips');

    firstRun.querySelector('li:nth-child(3)').textContent = l10n.translate('CRCs are only displayed after plugins have been loaded, either by conflict filtering, or by sorting.');
    firstRun.querySelector('li:nth-child(4)').textContent = l10n.translate('Double-click a plugin in the sidebar to quickly open its metadata editor. Multiple metadata editors can be opened at once.');
    firstRun.querySelector('li:nth-child(5)').textContent = l10n.translate('Plugins can be drag and dropped from the sidebar into editors\' "load after", "requirements" and "incompatibility" tables.');
    firstRun.querySelector('li:nth-child(6)').textContent = l10n.translate('Some features are disabled while there is an editor open, or while there is a sorted load order that has not been applied or discarded.');
    firstRun.querySelector('li:last-child').textContent = l10n.translate('Many interface elements have tooltips. If you don\'t know what something is, try hovering your mouse over it to find out. Otherwise, LOOT\'s documentation can be accessed through the main menu.');

    firstRun.getElementsByTagName('paper-button')[0].textContent = l10n.translate('OK');
  }

  return (l10n) => {
    translatePluginCardTemplate(l10n);
    translatePluginEditorTemplate(l10n);
    translatePluginListItemTemplate(l10n);
    translateMessageDialogTemplate(l10n);

    translateFileRowTemplate(l10n);
    translateMessageRowTemplate(l10n);
    translateTagRowTemplate(l10n);
    translateDirtyInfoRowTemplate(l10n);
    translateLocationRowTemplate(l10n);
    translateGameRowTemplate(l10n);
    translateNewRowTemplate(l10n);

    translateMainToolbar(l10n);
    translateSidebar(l10n);

    translateSummaryCard(l10n);
    translateSettingsDialog(l10n);
    translateFirstRunDialog(l10n);
  };
}));
