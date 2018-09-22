#!/usr/bin/env node
// Archive packaging script. Takes one argument, which is the path to the
// repository's root. Requires 7-zip and Git to be installed, and Git to be
// available on the system path.

const childProcess = require('child_process');
const path = require('path');
const fs = require('fs-extra');
const os = require('os');
const helpers = require('./helpers');

function getGitDescription() {
  const describe = String(
    helpers.safeExecFileSync('git', [
      'describe',
      '--tags',
      '--long',
      '--abbrev=7'
    ])
  ).slice(0, -1);
  let branch = String(
    helpers.safeExecFileSync('git', ['rev-parse', '--abbrev-ref', 'HEAD'])
  ).slice(0, -1);

  /* On AppVeyor and Travis CI, a specific commit is checked out, so the branch
     is HEAD. Use their stored branch value instead. */
  if (branch === 'HEAD') {
    if (process.env.APPVEYOR_REPO_BRANCH) {
      branch = process.env.APPVEYOR_REPO_BRANCH;
    } else if (process.env.TRAVIS_BRANCH) {
      branch = process.env.TRAVIS_BRANCH;
    }
  }

  return `${describe}_${branch}`;
}

function getLanguageFolders() {
  return [
    'cs',
    'es',
    'ru',
    'fr',
    'zh_CN',
    'pl',
    'pt_BR',
    'fi',
    'de',
    'da',
    'ko',
    'sv',
    'ja'
  ];
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
      'natives_blob.bin',
      'snapshot_blob.bin',
      'v8_context_snapshot.bin',
      'cef.pak',
      'cef_100_percent.pak',
      'cef_200_percent.pak',
      'devtools_resources.pak',
      'icudtl.dat'
    ];
  } else {
    binaries = [
      'LOOT',
      'libloot.so',
      'chrome-sandbox',
      'libcef.so',
      'natives_blob.bin',
      'snapshot_blob.bin',
      'v8_context_snapshot.bin',
      'cef.pak',
      'cef_100_percent.pak',
      'cef_200_percent.pak',
      'devtools_resources.pak',
      'icudtl.dat'
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
  getLanguageFolders().forEach(lang => {
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
}

function replaceInvalidFilenameCharacters(filename) {
  return filename.replace(/[/<>"|]/g, '-');
}

function getFilenameSuffix(label, gitDescription) {
  if (label) {
    return replaceInvalidFilenameCharacters(`${gitDescription}_${label}`);
  }

  return replaceInvalidFilenameCharacters(gitDescription);
}

function getArchiveFileExtension() {
  if (os.platform() === 'win32') {
    return '.7z';
  }

  return '.tar.xz';
}

const [, , rootPath = '.'] = process.argv;
const gitDesc = getGitDescription();
const fileExtension = getArchiveFileExtension();

helpers.getAppReleasePaths(rootPath).forEach(releasePath => {
  const filename = `loot_${getFilenameSuffix(releasePath.label, gitDesc)}`;
  createAppArchive(
    rootPath,
    releasePath.path,
    path.join(rootPath, 'build', filename),
    path.join(rootPath, 'build', filename + fileExtension)
  );
});
