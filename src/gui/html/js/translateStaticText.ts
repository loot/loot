import { PaperInputElement } from '@polymer/paper-input/paper-input';
import Translator from './translator';
import LootGroupsEditor from '../elements/loot-groups-editor';
import LootDropdownMenu from '../elements/loot-dropdown-menu';
import { LootVersion } from './interfaces';
import {
  getElementById,
  querySelector,
  getShadowElementById
} from './dom/helpers';
import EditableTable from '../elements/editable-table';

function getFirstElementChildById(elementId: string): Element {
  const sibling = getElementById(elementId).firstElementChild;
  if (sibling === null) {
    throw new Error(
      `Expected element with ID "${elementId}" to have an element child`
    );
  }

  return sibling;
}

function getPreviousElementSiblingById(elementId: string): Element {
  const sibling = getElementById(elementId).previousElementSibling;
  if (sibling === null) {
    throw new Error(
      `Expected element with ID "${elementId}" to have a previous element sibling`
    );
  }

  return sibling;
}

function getNextElementSiblingById(elementId: string): Element {
  const sibling = getElementById(elementId).nextElementSibling;
  if (sibling === null) {
    throw new Error(
      `Expected element with ID "${elementId}" to have a next element sibling`
    );
  }

  return sibling;
}

function getLastChildById(elementId: string): ChildNode {
  const child = getElementById(elementId).lastChild;
  if (child === null) {
    throw new Error(
      `Expected element with ID "${elementId}" to have a child element`
    );
  }

  return child;
}

function getPreviousElementSibling(element: Element): Element {
  const sibling = element.previousElementSibling;
  if (sibling === null) {
    throw new Error('Expected element to have a previous element sibling');
  }

  return sibling;
}

function getTemplate(templateId: string): DocumentFragment {
  return (getElementById(templateId) as HTMLTemplateElement).content;
}

function translatePluginCard(l10n: Translator, element: ShadowRoot): void {
  querySelector(element, 'paper-tooltip[for=activeTick]').textContent =
    l10n.translate('Active Plugin');
  querySelector(element, 'paper-tooltip[for=isMaster]').textContent =
    l10n.translate('Master File');
  querySelector(element, 'paper-tooltip[for=isLightPlugin]').textContent =
    l10n.translate('Light Plugin');
  querySelector(element, 'paper-tooltip[for=isEmpty]').textContent =
    l10n.translate('Empty Plugin');
  querySelector(element, 'paper-tooltip[for=loadsArchive]').textContent =
    // translators: Tooltip text for an icon that may be displayed on plugin cards. The icon indicates that the plugin in question loads a .bsa or .ba2 (depending on the game) archive.
    l10n.translate('Loads Archive');
  querySelector(element, 'paper-tooltip[for=hasUserEdits]').textContent =
    l10n.translate('Has User Metadata');

  querySelector(element, '#editMetadata > :last-child').textContent =
    l10n.translate('Edit Metadata');
  querySelector(element, '#copyMetadata > :last-child').textContent =
    l10n.translate('Copy Metadata');
  querySelector(element, '#clearMetadata > :last-child').textContent =
    l10n.translate('Clear User Metadata');
}

function translatePluginCardInstance(l10n: Translator): void {
  const elements = getElementById('pluginCardList').children;

  if (elements.length > 1 && elements[1].tagName === 'LOOT-PLUGIN-CARD') {
    if (elements[1].shadowRoot !== null) {
      translatePluginCard(l10n, elements[1].shadowRoot);
    }
  }
}

function translatePluginEditor(l10n: Translator): void {
  /* Plugin editor template. */
  const pluginEditor = getElementById('editor');
  const pluginEditorShadow = pluginEditor.shadowRoot;
  if (pluginEditorShadow === null) {
    throw new Error('Expected plugin editor to have a shadow root');
  }

  getPreviousElementSibling(
    getShadowElementById(pluginEditorShadow, 'group')
  ).textContent = l10n.translate('Group');

  querySelector(pluginEditorShadow, '#tableTabs [data-for=main]').textContent =
    l10n.translate('Main');
  querySelector(pluginEditorShadow, '#tableTabs [data-for=after]').textContent =
    l10n.translate('Load After');
  querySelector(pluginEditorShadow, '#tableTabs [data-for=req]').textContent =
    l10n.translate('Requirements');
  querySelector(pluginEditorShadow, '#tableTabs [data-for=inc]').textContent =
    l10n.translate('Incompatibilities');
  querySelector(pluginEditorShadow, '#tableTabs [data-for=msg]').textContent =
    l10n.translate('Messages');
  querySelector(pluginEditorShadow, '#tableTabs [data-for=tag]').textContent =
    // translators: Title of a tab in the metadata editor.
    l10n.translate('Bash Tags');
  querySelector(pluginEditorShadow, '#tableTabs [data-for=dirty]').textContent =
    l10n.translate('Dirty Plugin Info');
  querySelector(pluginEditorShadow, '#tableTabs [data-for=clean]').textContent =
    l10n.translate('Clean Plugin Info');
  querySelector(pluginEditorShadow, '#tableTabs [data-for=url]').textContent =
    l10n.translate('Locations');

  const editableTables = pluginEditor.getElementsByTagName('editable-table');
  for (const editableTable of editableTables) {
    (editableTable as EditableTable).localise(l10n);
  }

  querySelector(pluginEditor, '[slot=after] th:first-child').textContent =
    l10n.translate('Filename');

  querySelector(pluginEditor, '[slot=after] th:nth-child(2)').textContent =
    l10n.translate('Display Name');

  querySelector(pluginEditor, '[slot=after] th:nth-child(3)').textContent =
    l10n.translate('Condition');

  querySelector(pluginEditor, '[slot=req] th:first-child').textContent =
    l10n.translate('Filename');
  querySelector(pluginEditor, '[slot=req] th:nth-child(2)').textContent =
    l10n.translate('Display Name');
  querySelector(pluginEditor, '[slot=req] th:nth-child(3)').textContent =
    l10n.translate('Condition');

  querySelector(pluginEditor, '[slot=inc] th:first-child').textContent =
    l10n.translate('Filename');
  querySelector(pluginEditor, '[slot=inc] th:nth-child(2)').textContent =
    l10n.translate('Display Name');
  querySelector(pluginEditor, '[slot=inc] th:nth-child(3)').textContent =
    l10n.translate('Condition');

  querySelector(pluginEditor, '[slot=msg] th:first-child').textContent =
    l10n.translate('Type');
  querySelector(pluginEditor, '[slot=msg] th:nth-child(2)').textContent =
    l10n.translate('Content');
  querySelector(pluginEditor, '[slot=msg] th:nth-child(3)').textContent =
    l10n.translate('Condition');
  querySelector(pluginEditor, '[slot=msg] th:nth-child(4)').textContent =
    l10n.translate('Language');

  querySelector(pluginEditor, '[slot=tag] th:first-child').textContent =
    l10n.translate('Add/Remove');
  querySelector(pluginEditor, '[slot=tag] th:nth-child(2)').textContent =
    l10n.translate('Bash Tag');
  querySelector(pluginEditor, '[slot=tag] th:nth-child(3)').textContent =
    l10n.translate('Condition');

  querySelector(pluginEditor, '[slot=dirty] th:first-child').textContent =
    l10n.translate('CRC');
  querySelector(pluginEditor, '[slot=dirty] th:nth-child(2)').textContent =
    l10n.translate('ITM Count');
  querySelector(pluginEditor, '[slot=dirty] th:nth-child(3)').textContent =
    l10n.translate('Deleted References');
  querySelector(pluginEditor, '[slot=dirty] th:nth-child(4)').textContent =
    l10n.translate('Deleted Navmeshes');
  querySelector(pluginEditor, '[slot=dirty] th:nth-child(5)').textContent =
    l10n.translate('Cleaning Utility');

  querySelector(pluginEditor, '[slot=clean] th:first-child').textContent =
    l10n.translate('CRC');
  querySelector(pluginEditor, '[slot=clean] th:nth-child(2)').textContent =
    l10n.translate('Cleaning Utility');

  querySelector(pluginEditor, '[slot=url] th:first-child').textContent =
    l10n.translate('URL');
  querySelector(pluginEditor, '[slot=url] th:nth-child(2)').textContent =
    l10n.translate('Name');

  querySelector(pluginEditorShadow, 'paper-tooltip[for=accept]').textContent =
    l10n.translate('Save Metadata');
  querySelector(pluginEditorShadow, 'paper-tooltip[for=cancel]').textContent =
    l10n.translate('Cancel');
}

function translatePluginListItem(l10n: Translator, element: ShadowRoot): void {
  querySelector(element, '#groupTooltip').textContent = l10n.translate('Group');
  querySelector(element, 'paper-tooltip[for=hasUserEdits]').textContent =
    l10n.translate('Has User Metadata');
  querySelector(element, 'paper-tooltip[for=editorIsOpen]').textContent =
    l10n.translate('Editor Is Open');
}

function translatePluginListItemInstance(l10n: Translator): void {
  const elements = getElementById('cardsNav').children;

  if (elements.length > 1 && elements[1].tagName === 'LOOT-PLUGIN-ITEM') {
    if (elements[1].shadowRoot !== null) {
      translatePluginListItem(l10n, elements[1].shadowRoot);
    }
  }
}

function translateFileRowTemplate(l10n: Translator): void {
  /* File row template */
  const fileRow = getTemplate('fileRow');

  querySelector(fileRow, '.name').setAttribute(
    'error-message',
    l10n.translate('A filename is required.')
  );
  querySelector(fileRow, 'paper-tooltip').textContent =
    l10n.translate('Delete Row');
}

function translateMessageRowTemplate(l10n: Translator): void {
  /* Message row template */
  const messageRow = getTemplate('messageRow');

  querySelector(messageRow, '.type').children[0].textContent =
    l10n.translate('Note');
  querySelector(messageRow, '.type').children[1].textContent =
    l10n.translate('Warning');
  querySelector(messageRow, '.type').children[2].textContent =
    l10n.translate('Error');
  querySelector(messageRow, '.text').setAttribute(
    'error-message',
    // translators: The error message displayed if you add a message with no content in the metadata editor.
    l10n.translate('A content string is required.')
  );
  querySelector(messageRow, 'paper-tooltip').textContent =
    l10n.translate('Delete Row');
}

function translateTagRowTemplate(l10n: Translator): void {
  /* Tag row template */
  const tagRow = getTemplate('tagRow');

  querySelector(tagRow, '.type').children[0].textContent =
    l10n.translate('Add');
  querySelector(tagRow, '.type').children[1].textContent =
    l10n.translate('Remove');
  querySelector(tagRow, '.name').setAttribute(
    'error-message',
    l10n.translate('A name is required.')
  );
  querySelector(tagRow, 'paper-tooltip').textContent =
    l10n.translate('Delete Row');
}

function translateDirtyInfoRowTemplate(l10n: Translator): void {
  /* Dirty Info row template */
  const dirtyInfoRow = getTemplate('dirtyInfoRow');

  querySelector(dirtyInfoRow, '.crc').setAttribute(
    'error-message',
    l10n.translate('A CRC is required.')
  );

  querySelector(dirtyInfoRow, '.itm').setAttribute(
    'error-message',
    l10n.translate('Values must be integers.')
  );

  querySelector(dirtyInfoRow, '.udr').setAttribute(
    'error-message',
    l10n.translate('Values must be integers.')
  );

  querySelector(dirtyInfoRow, '.nav').setAttribute(
    'error-message',
    l10n.translate('Values must be integers.')
  );

  querySelector(dirtyInfoRow, '.utility').setAttribute(
    'error-message',
    l10n.translate('A utility name is required.')
  );
  querySelector(dirtyInfoRow, 'paper-tooltip').textContent =
    l10n.translate('Delete Row');
}

function translateCleanInfoRowTemplate(l10n: Translator): void {
  /* Dirty Info row template */
  const cleanInfoRow = getTemplate('cleanInfoRow');

  querySelector(cleanInfoRow, '.crc').setAttribute(
    'error-message',
    l10n.translate('A CRC is required.')
  );
  querySelector(cleanInfoRow, '.utility').setAttribute(
    'error-message',
    l10n.translate('A utility name is required.')
  );
  querySelector(cleanInfoRow, 'paper-tooltip').textContent =
    l10n.translate('Delete Row');
}

function translateLocationRowTemplate(l10n: Translator): void {
  /* Location row template */
  const locationRow = getTemplate('locationRow');

  querySelector(locationRow, '.link').setAttribute(
    'error-message',
    l10n.translate('A link is required.')
  );
  querySelector(locationRow, 'paper-tooltip').textContent =
    l10n.translate('Delete Row');
}

function translateMainToolbar(l10n: Translator): void {
  /* Main toolbar */
  const mainToolbar = getElementById('mainToolbar');
  querySelector(
    mainToolbar,
    'paper-tooltip[for=jumpToGeneralInfo]'
  ).textContent = l10n.translate('Jump To General Information');
  querySelector(mainToolbar, 'paper-tooltip[for=sortButton]').textContent =
    l10n.translate('Sort Plugins');
  querySelector(
    mainToolbar,
    'paper-tooltip[for=updateMasterlistButton]'
  ).textContent = l10n.translate('Update Masterlist');
  getElementById('applySortButton').textContent = l10n.translate('Apply');
  getElementById('cancelSortButton').textContent = l10n.translate('Cancel');
  querySelector(mainToolbar, 'paper-tooltip[for=showSearch]').textContent =
    // translators: The tooltip for the search icon that reveals the content search input when clicked.
    l10n.translate('Search Cards');

  /* Toolbar menu */
  getLastChildById('redatePluginsButton').textContent =
    // translators: Text of menu option that triggers display of the Redate Plugins dialog.
    l10n.translate('Redate Plugins');
  getLastChildById('openLogButton').textContent = l10n.translate(
    'Open Debug Log Location'
  );
  getLastChildById('groupsEditorButton').textContent =
    l10n.translate('Open Groups Editor');
  getLastChildById('wipeUserlistButton').textContent = l10n.translate(
    'Clear All User Metadata'
  );
  getLastChildById('copyLoadOrderButton').textContent =
    l10n.translate('Copy Load Order');
  getLastChildById('copyContentButton').textContent =
    l10n.translate('Copy Content');
  getLastChildById('refreshContentButton').textContent =
    l10n.translate('Refresh Content');
  getLastChildById('helpButton').textContent =
    l10n.translate('View Documentation');
  getLastChildById('aboutButton').textContent = l10n.translate('About');
  getLastChildById('settingsButton').textContent = l10n.translate('Settings');
  getLastChildById('quitButton').textContent = l10n.translate('Quit');

  /* Search bar */
  const shadowRoot = getElementById('searchBar').shadowRoot;
  if (shadowRoot === null) {
    throw new Error('Expected search bar to have a shadow root');
  }

  (getShadowElementById(shadowRoot, 'search') as PaperInputElement).label =
    // translators: The label on the content search text input.
    l10n.translate('Search cards');
}

function updateDropdownSelectedItemText(dropdownElement: HTMLElement): void {
  if (dropdownElement.shadowRoot === null) {
    throw new Error('Expected dropdown element to have a shadow root');
  }

  const paperDropdownMenu = dropdownElement.shadowRoot.querySelector(
    'paper-dropdown-menu'
  );
  if (paperDropdownMenu === null) {
    throw new Error(
      'Expected dropdown element to contain a paper-dropdown-menu element'
    );
  }

  if (paperDropdownMenu.selectedItem) {
    if (paperDropdownMenu.shadowRoot === null) {
      throw new Error(
        'Expected paper-dropdown-menu element to have a shadow root'
      );
    }

    const paperInput =
      paperDropdownMenu.shadowRoot.querySelector('paper-input');

    if (paperInput === null) {
      throw new Error(
        'Expected paper-dropdown-menu element shadow root to contain a paper-input element'
      );
    }

    paperInput.value = (
      paperDropdownMenu.selectedItem as HTMLElement
    ).textContent;
  }
}

function translateSidebar(l10n: Translator): void {
  /* Nav items */
  getFirstElementChildById('sidebarTabs').textContent =
    l10n.translate('Plugins');
  const secondSidebarTab =
    getFirstElementChildById('sidebarTabs').nextElementSibling;

  if (secondSidebarTab === null) {
    throw new Error(
      'Expected sidebar tabs to contain at least two child elements'
    );
  }
  secondSidebarTab.textContent = l10n.translate('Filters');

  querySelector(
    getElementById('filters'),
    'paper-tooltip[for=contentFilter]'
  ).textContent = l10n.translate(
    'Press Enter or click outside the input to set the filter.'
  );
  const contentFilter = getElementById('contentFilter') as PaperInputElement;
  contentFilter.label = l10n.translate(
    'Show only plugins with cards that contain'
  );
  contentFilter.placeholder = l10n.translate('No text specified');

  /* Filters */
  getElementById('hideVersionNumbers').textContent = l10n.translate(
    'Hide version numbers'
  );
  getElementById('hideCRCs').textContent = l10n.translate('Hide CRCs');
  getElementById('hideBashTags').textContent =
    // translators: Text of a filter in the sidebar.
    l10n.translate('Hide Bash Tags');
  getElementById('hideNotes').textContent = l10n.translate('Hide notes');
  getElementById('hideAllPluginMessages').textContent = l10n.translate(
    'Hide all plugin messages'
  );
  getElementById('hideInactivePlugins').textContent = l10n.translate(
    'Hide inactive plugins'
  );
  getElementById('hideMessagelessPlugins').textContent = l10n.translate(
    'Hide messageless plugins'
  );

  const conflictsFilter = getElementById('conflictsFilter') as LootDropdownMenu;
  conflictsFilter.label = l10n.translate('Show only conflicting plugins for');
  getFirstElementChildById('conflictsFilter').textContent =
    l10n.translate('No plugin selected');
  /* The selected text doesn't update, so force that translation. */
  updateDropdownSelectedItemText(conflictsFilter);

  getElementById('hiddenPluginsTxt').textContent =
    l10n.translate('Hidden plugins:');
  getElementById('hiddenMessagesTxt').textContent =
    l10n.translate('Hidden messages:');
}

function translateSummaryCard(l10n: Translator): void {
  /* Summary */
  getFirstElementChildById('summary').textContent = l10n.translate(
    'General Information'
  );
  getPreviousElementSiblingById('masterlistRevision').textContent =
    l10n.translate('Masterlist Revision');
  getPreviousElementSiblingById('masterlistDate').textContent =
    l10n.translate('Masterlist Date');
  getPreviousElementSiblingById('totalWarningNo').textContent =
    // translators: The number of warning messages present. This text appears in the General Information card.
    l10n.translate('Warnings');
  getPreviousElementSiblingById('totalErrorNo').textContent =
    // translators: The number of error messages present. This text appears in the General Information card.
    l10n.translate('Errors');
  getPreviousElementSiblingById('totalMessageNo').textContent =
    l10n.translate('Total Messages');
  getPreviousElementSiblingById('activePluginNo').textContent =
    l10n.translate('Active Plugins');
  getPreviousElementSiblingById('dirtyPluginNo').textContent =
    l10n.translate('Dirty Plugins');
  getPreviousElementSiblingById('totalPluginNo').textContent =
    l10n.translate('Total Plugins');
}

function translateSettingsDialog(l10n: Translator): void {
  /* Settings dialog */
  querySelector(getElementById('settingsDialog'), 'h2').textContent =
    l10n.translate('Settings');

  getElementById('settingsGeneralItem').textContent = l10n.translate('General');

  const defaultGameSelect = getElementById('defaultGameSelect');
  getPreviousElementSiblingById('defaultGameSelect').textContent =
    l10n.translate('Default Game');
  getFirstElementChildById('defaultGameSelect').textContent =
    l10n.translate('Autodetect');
  /* The selected text doesn't update, so force that translation. */
  updateDropdownSelectedItemText(defaultGameSelect);

  getElementById('languageLabel').textContent = l10n.translate('Language');
  getNextElementSiblingById('languageLabel').textContent = l10n.translate(
    'Language changes will be applied after LOOT is restarted.'
  );

  getElementById('themeLabel').textContent = l10n.translate('Theme');
  getNextElementSiblingById('themeLabel').textContent = l10n.translate(
    'Theme changes will be applied after LOOT is restarted.'
  );
  getFirstElementChildById('themeSelect').textContent =
    l10n.translate('default');
  /* The selected text doesn't update, so force that translation. */
  updateDropdownSelectedItemText(getElementById('themeSelect'));

  getPreviousElementSiblingById('enableDebugLogging').textContent =
    l10n.translate('Enable debug logging');
  getNextElementSiblingById('enableDebugLogging').textContent = l10n.translate(
    'The output is logged to the LOOTDebugLog.txt file.'
  );

  getPreviousElementSiblingById('updateMasterlist').textContent =
    l10n.translate('Update masterlist before sorting');

  getPreviousElementSiblingById('enableLootUpdateCheck').textContent =
    l10n.translate('Check for LOOT updates on startup');

  getElementById('settingsGameName').setAttribute(
    'error-message',
    l10n.translate('A name is required.')
  );
  getElementById('settingsGameName').setAttribute(
    'label',
    l10n.translate('Name')
  );

  getElementById('settingsGameTypeDropdown').setAttribute(
    'label',
    l10n.translate('Base Game')
  );

  getElementById('settingsGameFolder').setAttribute(
    'error-message',
    l10n.translate('A folder is required.')
  );
  getElementById('settingsGameFolder').setAttribute(
    'label',
    l10n.translate('LOOT Folder')
  );

  getElementById('settingsGameMaster').setAttribute(
    'label',
    l10n.translate('Master File')
  );

  getElementById('settingsGameMasterlistUrl').setAttribute(
    'label',
    l10n.translate('Masterlist Repository URL')
  );

  getElementById('settingsGameMasterlistBranch').setAttribute(
    'label',
    l10n.translate('Masterlist Repository Branch')
  );

  getElementById('settingsGamePath').setAttribute(
    'label',
    l10n.translate('Install Path')
  );

  getElementById('settingsGameRegistry').setAttribute(
    'label',
    l10n.translate('Install Path Registry Key')
  );

  getElementById('settingsGameLocalPath').setAttribute(
    'label',
    l10n.translate('Local Data Path')
  );

  getElementById('addGameButton').textContent = l10n.translate('Add new game');
  getElementById('deleteGameButton').textContent =
    l10n.translate('Delete game');

  getElementById('settingsDialog').getElementsByClassName(
    'accept'
  )[0].textContent = l10n.translate('Apply');
  getElementById('settingsDialog').getElementsByClassName(
    'cancel'
  )[0].textContent = l10n.translate('Cancel');
}

function translateGroupsEditorDialog(l10n: Translator): void {
  const dialog = getElementById('groupsEditorDialog');

  dialog.getElementsByTagName('h2')[0].textContent =
    l10n.translate('Groups Editor');

  dialog.getElementsByClassName('accept')[0].textContent =
    l10n.translate('Apply');
  dialog.getElementsByClassName('cancel')[0].textContent =
    l10n.translate('Cancel');
}

function translateFirstRunDialog(l10n: Translator, version: LootVersion): void {
  /* First-run dialog */
  const firstRun = getElementById('firstRun');
  querySelector(firstRun, 'h2').textContent = l10n.translate('First-Time Tips');

  querySelector(firstRun, 'p:first-of-type').innerHTML =
    l10n.translateFormatted(
      'This appears to be the first time you have run LOOT v%s. Here are some tips to help you get started with the interface.',
      version.release
    );

  querySelector(firstRun, 'li:nth-child(1)').innerHTML =
    // translators: The %(menu_icon)s placeholder is replaced with the menu icon (three vertical dots).
    l10n.translateFormatted('Click %(menu_icon)s buttons to open menus.', {
      // eslint-disable-next-line @typescript-eslint/naming-convention
      menu_icon: '<iron-icon icon="more-vert"></iron-icon>'
    });
  querySelector(firstRun, 'li:nth-child(2)').innerHTML = l10n.translate(
    'As well as messages, LOOT displays plugin <span class="version">version numbers</span>, <span class="crc">CRCs</span> and Bash Tag suggestions for <span class="tag add">addition</span> and <span class="tag remove">removal</span>.'
  );
  querySelector(firstRun, 'li:nth-child(3)').textContent = l10n.translate(
    'CRCs are only displayed after plugins have been loaded, either by conflict filtering, or by sorting.'
  );
  querySelector(firstRun, 'li:nth-child(4)').textContent = l10n.translate(
    'Double-click a plugin in the sidebar to quickly open it in the metadata editor.'
  );
  querySelector(firstRun, 'li:nth-child(5)').textContent = l10n.translate(
    'Plugins can be drag and dropped from the sidebar into the metadata editor\'s "load after", "requirements" and "incompatibility" tables.'
  );
  querySelector(firstRun, 'li:nth-child(6)').textContent = l10n.translate(
    'Some features are disabled while the metadata editor is open, or while there is a sorted load order that has not been applied or discarded.'
  );
  querySelector(firstRun, 'li:last-child').textContent = l10n.translate(
    "Many interface elements have tooltips. If you don't know what something is, try hovering your mouse over it to find out. Otherwise, LOOT's documentation can be accessed through the main menu."
  );

  querySelector(firstRun, 'p:last-of-type').innerHTML = l10n.translateFormatted(
    "LOOT is free, but if you want to show your appreciation with some money, donations may be made to WrinklyNinja (LOOT's creator and main developer) using %s.",
    '<a href="https://www.paypal.me/OliverHamlet">PayPal</a>'
  );

  firstRun.getElementsByTagName('paper-button')[0].textContent =
    l10n.translate('OK');
}

function translateAboutDialog(l10n: Translator, version: LootVersion): void {
  const about = getElementById('about');
  querySelector(about, 'h2').textContent = l10n.translate('About LOOT');

  querySelector(about, 'p:nth-child(1)').innerHTML = l10n.translateFormatted(
    'Version %s (build %s)',
    version.release,
    version.build
  );
  querySelector(about, 'p:nth-child(2)').textContent = l10n.translate(
    'Load order optimisation for Morrowind, Oblivion, Nehrim, Skyrim, Enderal, Skyrim Special Edition, Enderal Special Edition, Skyrim VR, Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.'
  );

  querySelector(about, 'p:nth-child(4)').innerHTML = l10n.translateFormatted(
    "LOOT is free, but if you want to show your appreciation with some money, donations may be made to WrinklyNinja (LOOT's creator and main developer) using %s.",
    '<a href="https://www.paypal.me/OliverHamlet">PayPal</a>'
  );
}

export default function translateStaticText(
  l10n: Translator,
  version: LootVersion
): void {
  translatePluginCardInstance(l10n);
  translatePluginEditor(l10n);
  translatePluginListItemInstance(l10n);
  (getElementById('groupsEditor') as LootGroupsEditor).localise(l10n);

  translateFileRowTemplate(l10n);
  translateMessageRowTemplate(l10n);
  translateTagRowTemplate(l10n);
  translateDirtyInfoRowTemplate(l10n);
  translateCleanInfoRowTemplate(l10n);
  translateLocationRowTemplate(l10n);

  translateMainToolbar(l10n);
  translateSidebar(l10n);

  translateSummaryCard(l10n);
  translateSettingsDialog(l10n);
  translateGroupsEditorDialog(l10n);
  translateFirstRunDialog(l10n, version);
  translateAboutDialog(l10n, version);
}
