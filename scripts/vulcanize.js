#!/usr/bin/env node
// Build the UI's index.html file. Takes one argument, which is the path to the
// repository's root.
'use strict';
const childProcess = require('child_process');
const path = require('path');
const fs = require('fs');
const os = require('os');
const helpers = require('./helpers');

let rootPath = '.';
if (process.argv.length > 2) {
  rootPath = process.argv[2];
}

const releasePaths = helpers.getAppReleasePaths(rootPath);

for (let i = 0; i < releasePaths.length; ++i) {
  const outputPath = path.join(releasePaths[i].path, 'resources', 'ui');

  // Makes sure output directory exists first.
  try {
    fs.mkdirSync(outputPath);
  } catch (e) {
    if (e.code !== 'EEXIST') {
      console.log(e);
    }
  }

  let vulcanize = path.join(rootPath, 'node_modules', '.bin', 'vulcanize');
  if (os.platform() === 'win32') {
    vulcanize += '.cmd';
  }

  childProcess.execFileSync(vulcanize, [
    '--inline',
    '--config',
    path.join(rootPath, 'scripts', 'vulcanize.config.json'),
    '-o',
    path.join(outputPath, 'index.html'),
    path.join(rootPath, 'src', 'gui', 'html', 'index.html'),
  ]);
}
