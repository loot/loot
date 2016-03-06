#!/usr/bin/env node
// Archive packaging script. Takes one argument, which is the path to the
// repository's root. Requires 7-zip and Git to be installed, and Git to be
// available on the system path.
'use strict';
const childProcess = require('child_process');
const path = require('path');
const fs = require('fs-extra');
const os = require('os');
const helpers = require('./helpers');

function vulcanize(rootPath) {
  return childProcess.execFileSync('node', [
    'scripts/vulcanize.js',
    rootPath,
  ]);
}

function getGitDescription() {
  return String(childProcess.execFileSync('git', [
    'describe',
    '--tags',
    '--long',
  ])).slice(0, -1);
}

function compress(sourcePath, destPath) {
  let sevenzipPath = '';
  if (os.platform() === 'win32') {
    sevenzipPath = path.join('C:\\', 'Program Files', '7-Zip', '7z.exe');
  } else {
    sevenzipPath = '/usr/bin/7z';
  }

  // First remove any existing archive.
  fs.removeSync(destPath);

  // The last argument must have a leading dot for the subdirectory not to
  // be present in the archive, but path.join removes it, so it's prefixed.
  return childProcess.execFileSync(sevenzipPath, [
    'a',
    '-r',
    destPath,
    `.${path.sep}${path.join(sourcePath, '*')}`,
  ]);
}

function createAppArchive(rootPath, releasePath, tempPath, destPath) {
  // Ensure that the output directory is empty.
  fs.emptyDirSync(tempPath);

  // Copy LOOT exectuable and CEF files.
  let binaries = [];
  if (os.platform() === 'win32') {
    binaries = [
      'LOOT.exe',
      'd3dcompiler_47.dll',
      'libEGL.dll',
      'libGLESv2.dll',
      'libcef.dll',
      'natives_blob.bin',
      'snapshot_blob.bin',
      'cef.pak',
      'cef_100_percent.pak',
      'cef_200_percent.pak',
      'devtools_resources.pak',
      'icudtl.dat',
    ];

    if (helpers.fileExists(path.join(releasePath, 'wow_helper.exe'))) {
      binaries.push('wow_helper.exe');
    }
  } else {
    binaries = [
      'LOOT',
      'chrome-sandbox',
      'libcef.so',
      'natives_blob.bin',
      'snapshot_blob.bin',
      'cef.pak',
      'cef_100_percent.pak',
      'cef_200_percent.pak',
      'devtools_resources.pak',
      'icudtl.dat',
    ];
  }
  binaries.forEach((file) => {
    fs.copySync(
      path.join(releasePath, file),
      path.join(tempPath, file)
    );
  });

  // CEF locale file.
  fs.mkdirsSync(path.join(tempPath, 'resources', 'l10n'));
  fs.copySync(
    path.join(releasePath, 'resources', 'l10n', 'en-US.pak'),
    path.join(tempPath, 'resources', 'l10n', 'en-US.pak')
  );

  // Translation files.
  [
    'es', 'ru', 'fr', 'zh_CN', 'pl', 'pt_BR', 'fi', 'de', 'da', 'ko',
  ].forEach((lang) => {
    fs.mkdirsSync(path.join(tempPath, 'resources', 'l10n', lang, 'LC_MESSAGES'));
    fs.copySync(
      path.join(rootPath, 'resources', 'l10n', lang, 'LC_MESSAGES', 'loot.mo'),
      path.join(tempPath, 'resources', 'l10n', lang, 'LC_MESSAGES', 'loot.mo')
    );
  });

  // UI files.
  fs.mkdirsSync(path.join(tempPath, 'resources', 'ui', 'css'));
  fs.copySync(
    path.join(releasePath, 'resources', 'ui', 'index.html'),
    path.join(tempPath, 'resources', 'ui', 'index.html')
  );
  fs.copySync(
    path.join(rootPath, 'resources', 'ui', 'css', 'dark-theme.css'),
    path.join(tempPath, 'resources', 'ui', 'css', 'dark-theme.css')
  );
  fs.copySync(
    path.join(rootPath, 'resources', 'ui', 'fonts'),
    path.join(tempPath, 'resources', 'ui', 'fonts')
  );

  // Docs.
  fs.mkdirsSync(path.join(tempPath, 'docs'));
  [
    'images',
    'licenses',
    'LOOT Metadata Syntax.html',
    'LOOT Readme.html',
  ].forEach((item) => {
    fs.copySync(
      path.join(rootPath, 'docs', item),
      path.join(tempPath, 'docs', item)
    );
  });

  // Now compress the folder to a 7-zip archive.
  compress(tempPath, destPath);

  // Finally, delete the temporary folder.
  fs.removeSync(tempPath);
}

function createApiArchive(rootPath, binaryPath, tempPath, destPath) {
  // Ensure that the output directory is empty.
  fs.emptyDirSync(tempPath);

  // API binary/binaries.
  fs.copySync(
    binaryPath,
    path.join(tempPath, path.basename(binaryPath))
  );

  // API header file.
  fs.mkdirsSync(path.join(tempPath, 'include', 'loot'));
  fs.copySync(
    path.join(rootPath, 'include', 'loot', 'api.h'),
    path.join(tempPath, 'include', 'loot', 'api.h')
  );

  // Docs.
  fs.mkdirsSync(path.join(tempPath, 'docs'));
  fs.copySync(
    path.join(rootPath, 'docs', 'latex', 'refman.pdf'),
    path.join(tempPath, 'docs', 'readme.pdf')
  );
  fs.copySync(
    path.join(rootPath, 'docs', 'licenses'),
    path.join(tempPath, 'docs', 'licenses')
  );

  // Now compress the folder to a 7-zip archive.
  compress(tempPath, destPath);

  // Finally, delete the temporary folder.
  fs.removeSync(tempPath);
}

let rootPath = '.';
if (process.argv.length > 2) {
  rootPath = process.argv[2];
}
const tempPath = path.join(rootPath, 'build', 'archive.tmp');

const gitDesc = getGitDescription();
vulcanize(rootPath);

const releasePaths = helpers.getAppReleasePaths(rootPath);
for (let i = 0; i < releasePaths.length; ++i) {
  if (releasePaths[i].label) {
    const filename = `LOOT ${gitDesc} (${releasePaths[i].label}).7z`;
    createAppArchive(rootPath,
                     releasePaths[i].path,
                     tempPath,
                     path.join(rootPath, 'build', filename));
  } else {
    createAppArchive(releasePaths[i].path,
                     path.join(rootPath, 'build', `LOOT ${gitDesc}.7z`));
  }
}

const binaryPaths = helpers.getApiBinaryPaths(rootPath);
for (let i = 0; i < binaryPaths.length; ++i) {
  const filename = `LOOT API ${gitDesc} (${binaryPaths[i].label}).7z`;
  createApiArchive(rootPath,
                   binaryPaths[i].path,
                   tempPath,
                   path.join(rootPath, 'build', filename));
}
