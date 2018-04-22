import initialise from './initialise.js';
import { showProgress } from './dialog.js';
import { onQuit } from './events.js';

/* Also import the custom elements here so they're bundled by webpack */

import '../elements/editable-table.js';
import '../elements/loot-dropdown-menu.js';
import '../elements/loot-groups-editor.js';
import '../elements/loot-menu.js';
import '../elements/loot-message-dialog.js';
import '../elements/loot-plugin-card.js';
import '../elements/loot-plugin-editor.js';
import '../elements/loot-plugin-item.js';
import '../elements/loot-search-toolbar.js';

window.loot = window.loot || {};
// These are assumed to exist by C++ callbacks.
window.loot.showProgress = showProgress;
window.loot.onQuit = onQuit;

initialise(loot);
