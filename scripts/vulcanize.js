#!/usr/bin/env node
// Build the UI's index.html file. Takes one argument, which is the path to the
// repository's root.
'use strict';
const path = require('path');
const fs = require('fs');
const helpers = require('./helpers');
const vulcanize = require('vulcanize');

let rootPath = '.';
if (process.argv.length > 2) {
  rootPath = process.argv[2];
}

function mkdir(dir) {
  try {
    fs.mkdirSync(dir);
  } catch (e) {
    if (e.code !== 'EEXIST') {
      console.log(e);
    }
  }
}

function runVulcanize(err) {
  if (err) {
    console.error(err);
    process.exit(1);
  }
  vulcanize.processDocument();
}

function vulcanizeRelease(releasePath) {
  const inputPath = path.join(rootPath, 'src', 'gui', 'html', 'index.html');
  const outputPath = path.join(releasePath.path, 'resources', 'ui', 'index.html');

  // Make sure output directory exists first.
  mkdir(path.dirname(outputPath));

  vulcanize.setOptions({
    inline: true,
    excludes: {
      styles: ['css/theme.css'],
    },
    output: outputPath,
    input: inputPath,
  }, runVulcanize);
}

helpers.getAppReleasePaths(rootPath).forEach(vulcanizeRelease);
