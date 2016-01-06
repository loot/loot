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

  for (let i = 0; i < pathsToTry.length; ++i) {
    let files = [];
    if (os.platform() === 'win32') {
      pathsToTry[i].path = path.join(pathsToTry[i].path, 'Release');

      files = [
        {
          name: 'loot32.dll',
          label: '32 bit',
        },
        {
          name: 'loot64.dll',
          label: '64 bit',
        },
      ];
    } else {
      files = [
        {
          name: 'libloot32.so',
          label: '32 bit',
        },
        {
          name: 'libloot64.so',
          label: '64 bit',
        },
      ];
    }

    for (let j = 0; j < files.length; ++j) {
      if (fileExists(path.join(pathsToTry[i].path, files[j].name))) {
        pathsToTry[i].path = path.join(pathsToTry[i].path, files[j].name);
        pathsToTry[i].label = files[j].label;
        paths.push(pathsToTry[i]);
        break;
      }
    }
  }

  return paths;
}

module.exports.fileExists = fileExists;
module.exports.getAppReleasePaths = getAppReleasePaths;
module.exports.getApiBinaryPaths = getApiBinaryPaths;
