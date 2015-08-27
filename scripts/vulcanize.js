#!/usr/bin/env node
// Build the UI's index.html file. Takes one argument, which is the path to the
// repository's root.
'use strict';
const path = require('path');
const fs = require('fs');
const helpers = require('./helpers');
const Vulcanize = require('vulcanize');

let rootPath = '.';
if (process.argv.length > 2) {
  rootPath = process.argv[2];
}

const vulcanize = new Vulcanize({
  inlineScripts: true,
  inlineCss: true,
  excludes: [
    'css/theme.css',
  ],
});

function mkdir(dir) {
  try {
    fs.mkdirSync(dir);
  } catch (e) {
    if (e.code !== 'EEXIST') {
      console.log(e);
    }
  }
}

function vulcanizeFile(inputPath, outputPath) {
  // Make sure output directory exists first.
  mkdir(path.dirname(outputPath));

  vulcanize.process(inputPath, (err, inlinedHtml) => {
    if (err) {
      console.error(err);
      process.exit(1);
    }

    fs.writeFileSync(outputPath, inlinedHtml);
  });
}

function vulcanizeRelease(releasePath) {
  const inputPath = path.join(rootPath, 'src', 'gui', 'html', 'index.html');
  const outputPath = path.join(releasePath.path, 'resources', 'ui', 'index.html');

  vulcanizeFile(inputPath, outputPath);
}

helpers.getAppReleasePaths(rootPath).forEach(vulcanizeRelease);
