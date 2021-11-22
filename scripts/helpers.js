// Helper functions shared across scripts.

const childProcess = require('child_process');
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

function getAppReleasePath(rootPath) {
  if (os.platform() === 'win32') {
    return path.join(rootPath, 'build', 'Release');
  }

  return path.join(rootPath, 'build');
}

function safeExecFileSync(file, args, options) {
  try {
    return childProcess.execFileSync(file, args, options);
  } catch (error) {
    throw new Error(error.message);
  }
}

module.exports.fileExists = fileExists;
module.exports.getAppReleasePath = getAppReleasePath;
module.exports.safeExecFileSync = safeExecFileSync;
