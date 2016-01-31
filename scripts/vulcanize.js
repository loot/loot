#!/usr/bin/env node
// Build the UI's index.html file. Takes one argument, which is the path to the
// repository's root.
'use strict';
const path = require('path');
const fs = require('fs');
const helpers = require('./helpers');
const vulcanize = require('vulcanize');

function runVulcanize(err) {
  if (err) {
    console.error(err);
    process.exit(1);
  }
  vulcanize.processDocument();
}

let rootPath = '.';
if (process.argv.length > 2) {
  rootPath = process.argv[2];
}

helpers.getAppReleasePaths(rootPath).forEach((releasePath) => {
  const outputPath = path.join(releasePath.path, 'resources', 'ui');

  // Makes sure output directory exists first.
  try {
    fs.mkdirSync(outputPath);
  } catch (e) {
    if (e.code !== 'EEXIST') {
      console.log(e);
    }
  }

  vulcanize.setOptions({
    inline: true,
    excludes: {
      styles: ['css/theme.css'],
    },
    output: path.join(outputPath, 'index.html'),
    input: path.join(rootPath, 'src', 'gui', 'html', 'index.html'),
  }, runVulcanize);
});
