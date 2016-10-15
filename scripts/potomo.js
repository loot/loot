/* Convert .po files to .mo files. */
/* eslint-disable no-unused-vars */

'use strict';
const childProcess = require('child_process');
const fs = require('fs');
const path = require('path');
const helpers = require('./helpers');

let rootPath = '.';
if (process.argv.length > 2) {
  rootPath = process.argv[2];
}

const l10nPath = path.join(rootPath, 'resources', 'l10n');
fs.readdirSync(l10nPath).forEach((file) => {
  if (fs.statSync(path.join(l10nPath, file)).isDirectory()) {
    try {
      const poPath = path.join(l10nPath, file, 'LC_MESSAGES', 'loot.po');
      const moPath = path.join(l10nPath, file, 'LC_MESSAGES', 'loot.mo');

      fs.accessSync(poPath, fs.R_OK);

      childProcess.execFileSync('msgfmt', [
        poPath,
        '-o',
        moPath,
      ]);
    } catch (error) {
      console.log(error);
    }
  }
});
