/* Convert .po files to .mo files. */
/* eslint-disable no-console */

const fs = require('fs');
const path = require('path');
const helpers = require('./helpers');

const [, , rootPath = '.'] = process.argv;

const l10nPath = path.join(rootPath, 'resources', 'l10n');
fs.readdirSync(l10nPath).forEach(file => {
  if (fs.statSync(path.join(l10nPath, file)).isDirectory()) {
    try {
      const poPath = path.join(l10nPath, file, 'LC_MESSAGES', 'loot.po');
      const moPath = path.join(l10nPath, file, 'LC_MESSAGES', 'loot.mo');

      fs.accessSync(poPath, fs.R_OK);

      helpers.safeExecFileSync('msgfmt', [poPath, '-o', moPath]);
    } catch (error) {
      console.error(error);
    }
  }
});
