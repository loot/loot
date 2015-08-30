#!/usr/bin/env node
// Build the UI's index.html file. Takes one argument, which is the path to the
// repository's root.
'use strict';
const path = require('path');
const fs = require('fs');
const helpers = require('./helpers');
const Vulcanize = require('vulcanize');
const mkdirp = require('mkdirp');

// Initialise from command line parameters.
let rootPath = '.';
let buildType = 'ui';
if (process.argv.length > 3) {
  rootPath = process.argv[2];
  buildType = process.argv[3];
} else if (process.argv.length > 2) {
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
    mkdirp.sync(dir);
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

function vulcanizeTests(releasePath) {
  const inputPath = path.join(rootPath, 'src', 'tests', 'gui', 'html', 'elements');
  const outputPath = path.join(releasePath.path, 'html_tests', 'elements');

  const tests = [
    'test_editable-table.html',
    'test_loot-custom-icons.html',
    'test_loot-dropdown-menu.html',
    'test_loot-message-dialog.html',
    'test_loot-plugin-editor.html',
    'test_loot-plugin-item.html',
    'test_loot-search-toolbar.html',
  ];

  tests.forEach((test) => {
    vulcanizeFile(path.join(inputPath, test), path.join(outputPath, test));
  });
}

// Run the appropriate build(s).
const releasePaths = helpers.getAppReleasePaths(rootPath);

if (buildType === 'ui' || buildType === 'all') {
  releasePaths.forEach(vulcanizeRelease);
}

if (buildType === 'tests' || buildType === 'all') {
  releasePaths.forEach(vulcanizeTests);
}
