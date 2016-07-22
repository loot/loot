// Helper functions shared across scripts.
'use strict';
const path = require('path');
const fs = require('fs');
const os = require('os');

function fileExists(filePath) {
  try {
    // Query the entry
    const stats = fs.lstatSync(filePath);

    // Is it a directory?
    if (stats.isFile()) {
      return true;
    }
  } catch (e) {
    /* Don't do anything, it's not an error. */
  }

  return false;
}

function getAppReleasePaths(rootPath) {
  const paths = [];
  const pathsToTry = [
    {
      path: path.join(rootPath, 'build'),
      label: null,
    },
    {
      path: path.join(rootPath, 'build', '32'),
      label: '32 bit',
    },
    {
      path: path.join(rootPath, 'build', '64'),
      label: '64 bit',
    },
  ];

  let file = 'LOOT';
  if (os.platform() === 'win32') {
    file += '.exe';
  }

  for (let i = 0; i < pathsToTry.length; ++i) {
    if (os.platform() === 'win32') {
      pathsToTry[i].path = path.join(pathsToTry[i].path, 'Release');
    }

    if (fileExists(path.join(pathsToTry[i].path, file))) {
      paths.push(pathsToTry[i]);
    }
  }

  return paths;
}

function getApiBinaryPaths(rootPath) {
  const paths = [];
  const pathsToTry = [
    {
      path: path.join(rootPath, 'build'),
      label: null,
    },
    {
      path: path.join(rootPath, 'build', '32'),
      label: '32 bit',
    },
    {
      path: path.join(rootPath, 'build', '64'),
      label: '64 bit',
    },
  ];

  let file = 'loot_api';
  if (os.platform() === 'win32') {
    file += '.dll';
  } else {
    file = `lib${file}.so`;
  }

  for (let i = 0; i < pathsToTry.length; ++i) {
    if (os.platform() === 'win32') {
      pathsToTry[i].path = path.join(pathsToTry[i].path, 'Release');
    }

    if (fileExists(path.join(pathsToTry[i].path, file))) {
      pathsToTry[i].path = path.join(pathsToTry[i].path, file);
      paths.push(pathsToTry[i]);
    }
  }

  return paths;
}

module.exports.fileExists = fileExists;
module.exports.getAppReleasePaths = getAppReleasePaths;
module.exports.getApiBinaryPaths = getApiBinaryPaths;
