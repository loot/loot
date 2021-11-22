#!/usr/bin/env node
// Archive packaging script. Takes one argument, which is the path to the
// repository's root. Requires 7-zip and Git to be installed, and Git to be
// available on the system path.

const childProcess = require('child_process');
const path = require('path');
const fs = require('fs-extra');
const os = require('os');
const helpers = require('./helpers');

function getGitDescription(givenBranch) {
  const describe = String(
    helpers.safeExecFileSync('git', [
      'describe',
      '--tags',
      '--long',
      '--abbrev=7'
    ])
  ).slice(0, -1);

  const branch =
    givenBranch ||
    String(
      helpers.safeExecFileSync('git', ['rev-parse', '--abbrev-ref', 'HEAD'])
    ).slice(0, -1);

  return `${describe}_${branch}`;
}

function getLanguageFolders(rootPath) {
  return fs
    .readdirSync(path.join(rootPath, 'resources', 'l10n'), {
      withFileTypes: true
    })
    .filter(dirent => dirent.isDirectory())
    .map(dirent => dirent.name);
}

function compress(sourcePath, destPath) {
  // First remove any existing archive.
  fs.removeSync(destPath);

  const filename = path.basename(destPath);
  const rootFolder = path.basename(sourcePath);
  const workingDirectory = path.dirname(sourcePath);

  if (os.platform() === 'win32') {
    let sevenzipPath = path.join('C:\\', 'Program Files', '7-Zip', '7z.exe');
    if (!helpers.fileExists(sevenzipPath)) {
      sevenzipPath = '7z';
    }

    // The last argument must have a leading dot for the subdirectory not to
    // be present in the archive, but path.join removes it, so it's prefixed.
    return helpers.safeExecFileSync(
      sevenzipPath,
      ['a', '-r', filename, rootFolder],
      {
        cwd: workingDirectory
      }
    );
  }

  return childProcess.execSync(`tar -cJf ${filename} ${rootFolder}`, {
    cwd: workingDirectory
  });
}

function createAppArchive(rootPath, releasePath, tempPath, destPath) {
  // Ensure that the output directory is empty.
  fs.emptyDirSync(tempPath);

  // Copy LOOT exectuable and CEF files.
  let binaries = [];
  if (os.platform() === 'win32') {
    binaries = [
      'LOOT.exe',
      'loot.dll',
      'chrome_elf.dll',
      'd3dcompiler_47.dll',
      'libEGL.dll',
      'libGLESv2.dll',
      'libcef.dll',
      'snapshot_blob.bin',
      'v8_context_snapshot.bin',
      'chrome_100_percent.pak',
      'chrome_200_percent.pak',
      'icudtl.dat',
      'resources.pak'
    ];
  } else {
    binaries = [
      'LOOT',
      'libloot.so',
      'chrome-sandbox',
      'libcef.so',
      'snapshot_blob.bin',
      'v8_context_snapshot.bin',
      'chrome_100_percent.pak',
      'chrome_200_percent.pak',
      'icudtl.dat',
      'resources.pak'
    ];
  }
  binaries.forEach(file => {
    fs.copySync(path.join(releasePath, file), path.join(tempPath, file));
  });

  // CEF locale file.
  fs.mkdirsSync(path.join(tempPath, 'resources', 'l10n'));
  fs.copySync(
    path.join(releasePath, 'resources', 'l10n', 'en-US.pak'),
    path.join(tempPath, 'resources', 'l10n', 'en-US.pak')
  );

  // Translation files.
  getLanguageFolders(rootPath).forEach(lang => {
    fs.mkdirsSync(
      path.join(tempPath, 'resources', 'l10n', lang, 'LC_MESSAGES')
    );
    fs.copySync(
      path.join(rootPath, 'resources', 'l10n', lang, 'LC_MESSAGES', 'loot.mo'),
      path.join(tempPath, 'resources', 'l10n', lang, 'LC_MESSAGES', 'loot.mo')
    );
  });

  // UI files.
  fs.copySync(
    path.join(releasePath, 'resources', 'ui'),
    path.join(tempPath, 'resources', 'ui')
  );

  // Documentation.
  fs.copySync(
    path.join(rootPath, 'build', 'docs', 'html'),
    path.join(tempPath, 'docs')
  );

  // Now compress the folder to a 7-zip archive.
  compress(tempPath, destPath);

  // Finally, delete the temporary folder.
  fs.removeSync(tempPath);

  // eslint-disable-next-line no-console
  console.log(destPath);
}

function replaceInvalidFilenameCharacters(filename) {
  return filename.replace(/[/<>"|]/g, '-');
}

function getArchiveFileExtension() {
  if (os.platform() === 'win32') {
    return '.7z';
  }

  return '.tar.xz';
}

const [, , rootPath = '.', givenBranch = undefined] = process.argv;
const gitDesc = getGitDescription(givenBranch);
const fileExtension = getArchiveFileExtension();

const releasePath = helpers.getAppReleasePath(rootPath);
const filename = `loot_${replaceInvalidFilenameCharacters(gitDesc)}`;
createAppArchive(
  rootPath,
  releasePath,
  path.join(rootPath, 'build', filename),
  path.join(rootPath, 'build', filename + fileExtension)
);
