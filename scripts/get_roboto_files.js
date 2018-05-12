/* eslint-disable no-console */

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
      .on('close', err => {
        if (err) {
          reject(err);
        }
        resolve();
      });
  })
    .then(() => decompress(downloadPath, extractPath))
    .then(() => {
      fs.renameSync(path.join(extractPath, 'roboto-hinted'), destinationPath);
      fs.removeSync(downloadPath);
    });
}

function handleError(error) {
  console.error(error);
  process.exit(1);
}

const url =
  'https://github.com/google/roboto/releases/download/v2.135/roboto-hinted.zip';
const fontsPath = 'build/fonts';

Promise.resolve()
  .then(() => {
    if (!fs.existsSync(fontsPath)) {
      return getRobotoFiles(url, fontsPath);
    }

    return '';
  })
  .catch(handleError);
