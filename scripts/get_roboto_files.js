const fs = require('fs-extra');
const path = require('path');
const request = require('request');
const decompress = require('decompress');

function getRobotoFiles(url, destinationPath) {
  const extractPath = path.dirname(destinationPath);
  const downloadPath = path.join(extractPath, 'roboto-hinted.zip');

  return new Promise((resolve, reject) => {
    request(url)
    .pipe(fs.createWriteStream(downloadPath))
    .on('close', (err) => {
      if (err) {
        reject(err);
      }
      resolve();
    });
  }).then(() => decompress(downloadPath, extractPath)).then(() => {
    fs.renameSync(path.join(extractPath, 'roboto-hinted'), destinationPath);
    fs.removeSync(downloadPath);
  });
}

module.exports.getRobotoFiles = getRobotoFiles;
