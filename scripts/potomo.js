/* Convert .po files to .mo files. */
/* eslint-disable no-unused-vars */

'use strict';
const childProcess = require('child_process');
const fs = require('fs');
const path = require('path');
const helpers = require('./helpers');

function getMsgfmtPath() {
  const paths = [
    path.join('C:\\', 'Program Files (x86)', 'Poedit', 'GettextTools', 'bin', 'msgfmt.exe'),
    path.join('C:\\', 'cygwin', 'bin', 'msgfmt.exe'),
    path.join('/', 'usr', 'bin', 'msgfmt'),
  ];

  for (let i = 0; i < paths.length; i += 1) {
    if (helpers.fileExists(paths[i])) {
      return paths[i];
    }
  }

  throw new Error('No msgfmt.exe found!');
}

/* This isn't portable to Linux but it doesn't (yet?) need to be. */
const msgfmtPath = getMsgfmtPath();

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

      childProcess.execFileSync(msgfmtPath, [
        poPath,
        '-o',
        moPath,
      ]);
    } catch (error) {
      console.log(error);
    }
  }
});
