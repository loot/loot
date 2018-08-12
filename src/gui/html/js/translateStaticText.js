function getTemplate(templateId) {
  return document.getElementById(templateId).content;
}

function translatePluginCard(l10n, element) {
  element.querySelector(
    'paper-tooltip[for=activeTick]'
  ).textContent = l10n.translate('Active Plugin');
  element.querySelector(
    'paper-tooltip[for=isMaster]'
  ).textContent = l10n.translate('Master File');
  element.querySelector(
    'paper-tooltip[for=isLightMaster]'
  ).textContent = l10n.translate('Light Master File');
  element.querySelector(
    'paper-tooltip[for=isEmpty]'
  ).textContent = l10n.translate('Empty Plugin');
  element.querySelector(
    'paper-tooltip[for=loadsArchive]'
  ).textContent = l10n.translate('Loads Archive');
  element.querySelector(
    'paper-tooltip[for=hasUserEdits]'
  ).textContent = l10n.translate('Has User Metadata');

  element.querySelector('#editMetadata').lastChild.textContent = l10n.translate(
    'Edit Metadata'
  );
  element.querySelector('#copyMetadata').lastChild.textContent = l10n.translate(
    'Copy Metadata'
  );
  element.querySelector(
    '#clearMetadata'
  ).lastChild.textContent = l10n.translate('Clear User Metadata');
}

function translatePluginCardInstance(l10n) {
  const elements = document.getElementById('pluginCardList').children;

  if (elements.length > 1 && elements[1].tagName === 'LOOT-PLUGIN-CARD') {
    translatePluginCard(l10n, elements[1].shadowRoot);
  }
}

function translatePluginEditor(l10n) {
  /* Plugin editor template. */
  const pluginEditor = document.getElementById('editor');
  const pluginEditorShadow = pluginEditor.shadowRoot;

  pluginEditorShadow.getElementById(
    'enableEdits'
  ).previousElementSibling.textContent = l10n.translate('Enable Edits');
  pluginEditorShadow.getElementById(
    'group'
  ).previousElementSibling.textContent = l10n.translate('Group');

  pluginEditorShadow
    .getElementById('tableTabs')
    .querySelector('[data-for=main]').textContent = l10n.translate('Main');
  pluginEditorShadow
    .getElementById('tableTabs')
    .querySelector('[data-for=after]').textContent = l10n.translate(
    'Load After'
  );
  pluginEditorShadow
    .getElementById('tableTabs')
    .querySelector('[data-for=req]').textContent = l10n.translate(
    'Requirements'
  );
  pluginEditorShadow
    .getElementById('tableTabs')
    .querySelector('[data-for=inc]').textContent = l10n.translate(
    'Incompatibilities'
  );
  pluginEditorShadow
    .getElementById('tableTabs')
    .querySelector('[data-for=message]').textContent = l10n.translate(
    'Messages'
  );
  pluginEditorShadow
    .getElementById('tableTabs')
    .querySelector('[data-for=tags]').textContent = l10n.translate('Bash Tags');
  pluginEditorShadow
    .getElementById('tableTabs')
    .querySelector('[data-for=dirty]').textContent = l10n.translate(
    'Dirty Plugin Info'
  );
  pluginEditorShadow
    .getElementById('tableTabs')
    .querySelector('[data-for=clean]').textContent = l10n.translate(
    'Clean Plugin Info'
  );
  pluginEditorShadow
    .getElementById('tableTabs')
    .querySelector('[data-for=url]').textContent = l10n.translate('Locations');

  pluginEditor
    .querySelector('[slot=after]')
    .querySelector('th:first-child').textContent = l10n.translate('Filename');
  pluginEditor
    .querySelector('[slot=after]')
    .querySelector('th:nth-child(2)').textContent = l10n.translate(
    'Display Name'
  );
  pluginEditor
    .querySelector('[slot=after]')
    .querySelector('th:nth-child(3)').textContent = l10n.translate('Condition');

  pluginEditor
    .querySelector('[slot=req]')
    .querySelector('th:first-child').textContent = l10n.translate('Filename');
  pluginEditor
    .querySelector('[slot=req]')
    .querySelector('th:nth-child(2)').textContent = l10n.translate(
    'Display Name'
  );
  pluginEditor
    .querySelector('[slot=req]')
    .querySelector('th:nth-child(3)').textContent = l10n.translate('Condition');

  pluginEditor
    .querySelector('[slot=inc]')
    .querySelector('th:first-child').textContent = l10n.translate('Filename');
  pluginEditor
    .querySelector('[slot=inc]')
    .querySelector('th:nth-child(2)').textContent = l10n.translate(
    'Display Name'
  );
  pluginEditor
    .querySelector('[slot=inc]')
    .querySelector('th:nth-child(3)').textContent = l10n.translate('Condition');

  pluginEditor
    .querySelector('[slot=message]')
    .querySelector('th:first-child').textContent = l10n.translate('Type');
  pluginEditor
    .querySelector('[slot=message]')
    .querySelector('th:nth-child(2)').textContent = l10n.translate('Content');
  pluginEditor
    .querySelector('[slot=message]')
    .querySelector('th:nth-child(3)').textContent = l10n.translate('Condition');
  pluginEditor
    .querySelector('[slot=message]')
    .querySelector('th:nth-child(4)').textContent = l10n.translate('Language');

  pluginEditor
    .querySelector('[slot=tags]')
    .querySelector('th:first-child').textContent = l10n.translate('Add/Remove');
  pluginEditor
    .querySelector('[slot=tags]')
    .querySelector('th:nth-child(2)').textContent = l10n.translate('Bash Tag');
  pluginEditor
    .querySelector('[slot=tags]')
    .querySelector('th:nth-child(3)').textContent = l10n.translate('Condition');

  pluginEditor
    .querySelector('[slot=dirty]')
    .querySelector('th:first-child').textContent = l10n.translate('CRC');
  pluginEditor
    .querySelector('[slot=dirty]')
    .querySelector('th:nth-child(2)').textContent = l10n.translate('ITM Count');
  pluginEditor
    .querySelector('[slot=dirty]')
    .querySelector('th:nth-child(3)').textContent = l10n.translate(
    'Deleted References'
  );
  pluginEditor
    .querySelector('[slot=dirty]')
    .querySelector('th:nth-child(4)').textContent = l10n.translate(
    'Deleted Navmeshes'
  );
  pluginEditor
    .querySelector('[slot=dirty]')
    .querySelector('th:nth-child(5)').textContent = l10n.translate(
    'Cleaning Utility'
  );

  pluginEditor
    .querySelector('[slot=clean]')
    .querySelector('th:first-child').textContent = l10n.translate('CRC');
  pluginEditor
    .querySelector('[slot=clean]')
    .querySelector('th:nth-child(2)').textContent = l10n.translate(
    'Cleaning Utility'
  );

  pluginEditor
    .querySelector('[slot=url]')
    .querySelector('th:first-child').textContent = l10n.translate('URL');
  pluginEditor
    .querySelector('[slot=url]')
    .querySelector('th:nth-child(2)').textContent = l10n.translate('Name');

  pluginEditorShadow.querySelector(
    'paper-tooltip[for=accept]'
  ).textContent = l10n.translate('Save Metadata');
  pluginEditorShadow.querySelector(
    'paper-tooltip[for=cancel]'
  ).textContent = l10n.translate('Cancel');
}

function translatePluginListItem(l10n, element) {
  element.querySelector('#groupTooltip').textContent = l10n.translate('Group');
  element.querySelector(
    'paper-tooltip[for=hasUserEdits]'
  ).textContent = l10n.translate('Has User Metadata');
  element.querySelector(
    'paper-tooltip[for=editorIsOpen]'
  ).textContent = l10n.translate('Editor Is Open');
}

function translatePluginListItemInstance(l10n) {
  const elements = document.getElementById('cardsNav').children;

  if (elements.length > 1 && elements[1].tagName === 'LOOT-PLUGIN-ITEM') {
    translatePluginListItem(l10n, elements[1].shadowRoot);
  }
}

function translateFileRowTemplate(l10n) {
  /* File row template */
  const fileRow = getTemplate('fileRow', 'editable-table-rows');

  fileRow
    .querySelector('.name')
    .setAttribute('error-message', l10n.translate('A filename is required.'));
  fileRow.querySelector('paper-tooltip').textContent = l10n.translate(
    'Delete Row'
  );
}

function translateMessageRowTemplate(l10n) {
  /* Message row template */
  const messageRow = getTemplate('messageRow', 'editable-table-rows');

  messageRow.querySelector('.type').children[0].textContent = l10n.translate(
    'Note'
  );
  messageRow.querySelector('.type').children[1].textContent = l10n.translate(
    'Warning'
  );
  messageRow.querySelector('.type').children[2].textContent = l10n.translate(
    'Error'
  );
  messageRow
    .querySelector('.text')
    .setAttribute(
      'error-message',
      l10n.translate('A content string is required.')
    );
  messageRow.querySelector('paper-tooltip').textContent = l10n.translate(
    'Delete Row'
  );
}

function translateTagRowTemplate(l10n) {
  /* Tag row template */
  const tagRow = getTemplate('tagRow', 'editable-table-rows');

  tagRow.querySelector('.type').children[0].textContent = l10n.translate('Add');
  tagRow.querySelector('.type').children[1].textContent = l10n.translate(
    'Remove'
  );
  tagRow
    .querySelector('.name')
    .setAttribute('error-message', l10n.translate('A name is required.'));
  tagRow.querySelector('paper-tooltip').textContent = l10n.translate(
    'Delete Row'
  );
}

function translateDirtyInfoRowTemplate(l10n) {
  /* Dirty Info row template */
  const dirtyInfoRow = getTemplate('dirtyInfoRow', 'editable-table-rows');

  dirtyInfoRow
    .querySelector('.crc')
    .setAttribute('error-message', l10n.translate('A CRC is required.'));
  dirtyInfoRow
    .querySelector('.itm')
    .setAttribute('error-message', l10n.translate('Values must be integers.'));
  dirtyInfoRow
    .querySelector('.udr')
    .setAttribute('error-message', l10n.translate('Values must be integers.'));
  dirtyInfoRow
    .querySelector('.nav')
    .setAttribute('error-message', l10n.translate('Values must be integers.'));
  dirtyInfoRow
    .querySelector('.utility')
    .setAttribute(
      'error-message',
      l10n.translate('A utility name is required.')
    );
  dirtyInfoRow.querySelector('paper-tooltip').textContent = l10n.translate(
    'Delete Row'
  );
}

function translateCleanInfoRowTemplate(l10n) {
  /* Dirty Info row template */
  const cleanInfoRow = getTemplate('cleanInfoRow', 'editable-table-rows');

  cleanInfoRow
    .querySelector('.crc')
    .setAttribute('error-message', l10n.translate('A CRC is required.'));
  cleanInfoRow
    .querySelector('.utility')
    .setAttribute(
      'error-message',
      l10n.translate('A utility name is required.')
    );
  cleanInfoRow.querySelector('paper-tooltip').textContent = l10n.translate(
    'Delete Row'
  );
}

function translateLocationRowTemplate(l10n) {
  /* Location row template */
  const locationRow = getTemplate('locationRow', 'editable-table-rows');

  locationRow
    .querySelector('.link')
    .setAttribute('error-message', l10n.translate('A link is required.'));
  locationRow.querySelector('paper-tooltip').textContent = l10n.translate(
    'Delete Row'
  );
}

function translateGameRowTemplate(l10n) {
  /* Game row template */
  const gameRow = getTemplate('gameRow', 'editable-table-rows');

  gameRow
    .querySelector('.name')
    .setAttribute('error-message', l10n.translate('A name is required.'));
  gameRow
    .querySelector('.folder')
    .setAttribute('error-message', l10n.translate('A folder is required.'));
  gameRow.querySelector('paper-tooltip').textContent = l10n.translate(
    'Delete Row'
  );
}

function translateNewRowTemplate(l10n) {
  /* New row template */
  const newRow = getTemplate('newRow', 'editable-table-rows');

  newRow.querySelector('paper-tooltip').textContent = l10n.translate(
    'Add New Row'
  );
}

function translateMainToolbar(l10n) {
  /* Main toolbar */
  const mainToolbar = document.getElementById('mainToolbar');
  mainToolbar.querySelector(
    'paper-tooltip[for=jumpToGeneralInfo]'
  ).textContent = l10n.translate('Jump To General Information');
  mainToolbar.querySelector(
    'paper-tooltip[for=sortButton]'
  ).textContent = l10n.translate('Sort Plugins');
  mainToolbar.querySelector(
    'paper-tooltip[for=updateMasterlistButton]'
  ).textContent = l10n.translate('Update Masterlist');
  document.getElementById('applySortButton').textContent = l10n.translate(
    'Apply'
  );
  document.getElementById('cancelSortButton').textContent = l10n.translate(
    'Cancel'
  );
  mainToolbar.querySelector(
    'paper-tooltip[for=showSearch]'
  ).textContent = l10n.translate('Search Cards');

  /* Toolbar menu */
  document.getElementById(
    'redatePluginsButton'
  ).lastChild.textContent = l10n.translate('Redate Plugins');
  document.getElementById(
    'openLogButton'
  ).lastChild.textContent = l10n.translate('Open Debug Log Location');
  document.getElementById(
    'wipeUserlistButton'
  ).lastChild.textContent = l10n.translate('Clear All User Metadata');
  document.getElementById(
    'copyLoadOrderButton'
  ).lastChild.textContent = l10n.translate('Copy Load Order');
  document.getElementById(
    'copyContentButton'
  ).lastChild.textContent = l10n.translate('Copy Content');
  document.getElementById(
    'refreshContentButton'
  ).lastChild.textContent = l10n.translate('Refresh Content');
  document.getElementById('helpButton').lastChild.textContent = l10n.translate(
    'View Documentation'
  );
  document.getElementById('aboutButton').lastChild.textContent = l10n.translate(
    'About'
  );
  document.getElementById(
    'settingsButton'
  ).lastChild.textContent = l10n.translate('Settings');
  document.getElementById('quitButton').lastChild.textContent = l10n.translate(
    'Quit'
  );

  /* Search bar */
  document
    .getElementById('searchBar')
    .shadowRoot.getElementById('search').label = l10n.translate('Search cards');
}

function updateDropdownSelectedItemText(dropdownElement) {
  const paperDropdownMenu = dropdownElement.shadowRoot.querySelector(
    'paper-dropdown-menu'
  );
  if (paperDropdownMenu.selectedItem) {
    paperDropdownMenu.shadowRoot.querySelector('paper-input').value =
      paperDropdownMenu.selectedItem.textContent;
  }
}

function translateSidebar(l10n) {
  /* Nav items */
  document.getElementById(
    'sidebarTabs'
  ).firstElementChild.textContent = l10n.translate('Plugins');
  document.getElementById(
    'sidebarTabs'
  ).firstElementChild.nextElementSibling.textContent = l10n.translate(
    'Filters'
  );
  document
    .getElementById('filters')
    .querySelector(
      'paper-tooltip[for=contentFilter]'
    ).textContent = l10n.translate(
    'Press Enter or click outside the input to set the filter.'
  );
  document.getElementById('contentFilter').label = l10n.translate(
    'Show only plugins with cards that contain'
  );
  document.getElementById('contentFilter').placeholder = l10n.translate(
    'No text specified'
  );

  /* Filters */
  document.getElementById('hideVersionNumbers').textContent = l10n.translate(
    'Hide version numbers'
  );
  document.getElementById('hideCRCs').textContent = l10n.translate('Hide CRCs');
  document.getElementById('hideBashTags').textContent = l10n.translate(
    'Hide Bash Tags'
  );
  document.getElementById('hideNotes').textContent = l10n.translate(
    'Hide notes'
  );
  document.getElementById(
    'hideDoNotCleanMessages'
  ).textContent = l10n.translate("Hide 'Do not clean' messages");
  document.getElementById('hideAllPluginMessages').textContent = l10n.translate(
    'Hide all plugin messages'
  );
  document.getElementById('hideInactivePlugins').textContent = l10n.translate(
    'Hide inactive plugins'
  );
  document.getElementById(
    'hideMessagelessPlugins'
  ).textContent = l10n.translate('Hide messageless plugins');

  const conflictsFilter = document.getElementById('conflictsFilter');
  conflictsFilter.label = l10n.translate('Show only conflicting plugins for');
  conflictsFilter.firstElementChild.textContent = l10n.translate(
    'No plugin selected'
  );
  /* The selected text doesn't update, so force that translation. */
  updateDropdownSelectedItemText(conflictsFilter);

  document.getElementById('hiddenPluginsTxt').textContent = l10n.translate(
    'Hidden plugins:'
  );
  document.getElementById('hiddenMessagesTxt').textContent = l10n.translate(
    'Hidden messages:'
  );
}

function translateSummaryCard(l10n) {
  /* Summary */
  document.getElementById(
    'summary'
  ).firstElementChild.textContent = l10n.translate('General Information');
  document.getElementById(
    'masterlistRevision'
  ).previousElementSibling.textContent = l10n.translate('Masterlist Revision');
  document.getElementById(
    'masterlistDate'
  ).previousElementSibling.textContent = l10n.translate('Masterlist Date');
  document.getElementById(
    'totalWarningNo'
  ).previousElementSibling.textContent = l10n.translate('Warnings');
  document.getElementById(
    'totalErrorNo'
  ).previousElementSibling.textContent = l10n.translate('Errors');
  document.getElementById(
    'totalMessageNo'
  ).previousElementSibling.textContent = l10n.translate('Total Messages');
  document.getElementById(
    'activePluginNo'
  ).previousElementSibling.textContent = l10n.translate('Active Plugins');
  document.getElementById(
    'dirtyPluginNo'
  ).previousElementSibling.textContent = l10n.translate('Dirty Plugins');
  document.getElementById(
    'totalPluginNo'
  ).previousElementSibling.textContent = l10n.translate('Total Plugins');
}

function translateSettingsDialog(l10n) {
  /* Settings dialog */
  document
    .getElementById('settingsDialog')
    .querySelector('h2').textContent = l10n.translate('Settings');

  const defaultGameSelect = document.getElementById('defaultGameSelect');
  defaultGameSelect.previousElementSibling.textContent = l10n.translate(
    'Default Game'
  );
  defaultGameSelect.firstElementChild.textContent = l10n.translate(
    'Autodetect'
  );
  /* The selected text doesn't update, so force that translation. */
  updateDropdownSelectedItemText(defaultGameSelect);

  document.getElementById('languageLabel').textContent = l10n.translate(
    'Language'
  );
  document.getElementById(
    'languageLabel'
  ).nextElementSibling.textContent = l10n.translate(
    'Language changes will be applied after LOOT is restarted.'
  );

  document.getElementById(
    'enableDebugLogging'
  ).previousElementSibling.textContent = l10n.translate('Enable debug logging');
  document.getElementById(
    'enableDebugLogging'
  ).nextElementSibling.textContent = l10n.translate(
    'The output is logged to the LOOTDebugLog.txt file.'
  );

  document.getElementById(
    'updateMasterlist'
  ).previousElementSibling.textContent = l10n.translate(
    'Update masterlist before sorting'
  );

  document.getElementById(
    'enableLootUpdateCheck'
  ).previousElementSibling.textContent = l10n.translate(
    'Check for LOOT updates on startup'
  );

  const gameTable = document.getElementById('gameTable');
  gameTable.querySelector('th:first-child').textContent = l10n.translate(
    'Name'
  );
  gameTable.querySelector('th:nth-child(2)').textContent = l10n.translate(
    'Base Game'
  );
  gameTable.querySelector('th:nth-child(3)').textContent = l10n.translate(
    'LOOT Folder'
  );
  gameTable.querySelector('th:nth-child(4)').textContent = l10n.translate(
    'Master File'
  );
  gameTable.querySelector('th:nth-child(5)').textContent = l10n.translate(
    'Masterlist Repository URL'
  );
  gameTable.querySelector('th:nth-child(6)').textContent = l10n.translate(
    'Masterlist Repository Branch'
  );
  gameTable.querySelector('th:nth-child(7)').textContent = l10n.translate(
    'Install Path'
  );
  gameTable.querySelector('th:nth-child(8)').textContent = l10n.translate(
    'Install Path Registry Key'
  );

  /* As the game table is attached on launch, its "Add New Row"
      tooltip doesn't benefit from the template translation above. */
  gameTable.querySelector(
    'tr:last-child paper-tooltip'
  ).textContent = l10n.translate('Add New Row');

  document
    .getElementById('settingsDialog')
    .getElementsByClassName('accept')[0].textContent = l10n.translate('Apply');
  document
    .getElementById('settingsDialog')
    .getElementsByClassName('cancel')[0].textContent = l10n.translate('Cancel');
}

function translateFirstRunDialog(l10n, version) {
  /* First-run dialog */
  const firstRun = document.getElementById('firstRun');
  firstRun.querySelector('h2').textContent = l10n.translate('First-Time Tips');

  firstRun.querySelector('p:first-of-type').innerHTML = l10n.translateFormatted(
    'This appears to be the first time you have run LOOT v%s. Here are some tips to help you get started with the interface.',
    version.release
  );

  firstRun.querySelector('li:nth-child(1)').innerHTML = l10n.translateFormatted(
    'Click %(menu_icon)s buttons to open menus.',
    {
      menu_icon: '<iron-icon icon="more-vert"></iron-icon>'
    }
  );
  firstRun.querySelector('li:nth-child(2)').innerHTML = l10n.translate(
    'As well as messages, LOOT displays plugin <span class="version">version numbers</span>, <span class="crc">CRCs</span> and Bash Tag suggestions for <span class="tag add">addition</span> and <span class="tag remove">removal</span>.'
  );
  firstRun.querySelector('li:nth-child(3)').textContent = l10n.translate(
    'CRCs are only displayed after plugins have been loaded, either by conflict filtering, or by sorting.'
  );
  firstRun.querySelector('li:nth-child(4)').textContent = l10n.translate(
    'Double-click a plugin in the sidebar to quickly open it in the metadata editor.'
  );
  firstRun.querySelector('li:nth-child(5)').textContent = l10n.translate(
    'Plugins can be drag and dropped from the sidebar into the metadata editor\'s "load after", "requirements" and "incompatibility" tables.'
  );
  firstRun.querySelector('li:nth-child(6)').textContent = l10n.translate(
    'Some features are disabled while the metadata editor is open, or while there is a sorted load order that has not been applied or discarded.'
  );
  firstRun.querySelector('li:last-child').textContent = l10n.translate(
    "Many interface elements have tooltips. If you don't know what something is, try hovering your mouse over it to find out. Otherwise, LOOT's documentation can be accessed through the main menu."
  );

  firstRun.querySelector('p:last-of-type').innerHTML = l10n.translateFormatted(
    "LOOT is free, but if you want to show your appreciation with some money, donations may be made to WrinklyNinja (LOOT's creator and main developer) using %s.",
    '<a href="https://www.paypal.me/OliverHamlet">PayPal</a>'
  );

  firstRun.getElementsByTagName('paper-button')[0].textContent = l10n.translate(
    'OK'
  );
}

function translateAboutDialog(l10n, version) {
  const about = document.getElementById('about');
  about.querySelector('h2').textContent = l10n.translate('About LOOT');

  about.querySelector('p:nth-child(1)').innerHTML = l10n.translateFormatted(
    'Version %s (build %s)',
    version.release,
    version.build
  );
  about.querySelector('p:nth-child(2)').textContent = l10n.translate(
    'Load order optimisation for Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR, Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.'
  );

  about.querySelector('p:nth-child(4)').innerHTML = l10n.translateFormatted(
    "LOOT is free, but if you want to show your appreciation with some money, donations may be made to WrinklyNinja (LOOT's creator and main developer) using %s.",
    '<a href="https://www.paypal.me/OliverHamlet">PayPal</a>'
  );
}

export default function translateStaticText(l10n, version) {
  translatePluginCardInstance(l10n);
  translatePluginEditor(l10n);
  translatePluginListItemInstance(l10n);
  document.getElementById('groupsEditor').localise(l10n);

  translateFileRowTemplate(l10n);
  translateMessageRowTemplate(l10n);
  translateTagRowTemplate(l10n);
  translateDirtyInfoRowTemplate(l10n);
  translateCleanInfoRowTemplate(l10n);
  translateLocationRowTemplate(l10n);
  translateGameRowTemplate(l10n);
  translateNewRowTemplate(l10n);

  translateMainToolbar(l10n);
  translateSidebar(l10n);

  translateSummaryCard(l10n);
  translateSettingsDialog(l10n);
  translateFirstRunDialog(l10n, version);
  translateAboutDialog(l10n, version);
}
