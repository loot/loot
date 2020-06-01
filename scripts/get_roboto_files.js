/* eslint-disable no-console */

const fs = require('fs');
const path = require('path');
const request = require('request');
const yauzl = require('yauzl');
const { promisify } = require('util');

const mkdir = promisify(fs.mkdir);

function downloadRobotoZip(url) {
  return new Promise((resolve, reject) => {
    request(url, { encoding: null }, (err, response, body) => {
      if (err) {
        reject(err);
        return;
      }
      if (response.statusCode < 200 || response.statusCode >= 300) {
        reject(
          new Error(`Received a HTTP status code of ${response.statusCode}`)
        );
        return;
      }

      resolve(body);
    });
  });
}

// The archive is expected to contain a single 'roboto-hinted' directory that
// contains all the font files.
async function unzip(zipBuffer, destinationPath) {
  const expectedRootDir = 'roboto-hinted/';

  await mkdir(destinationPath, { recursive: true });

  return new Promise((resolve, reject) => {
    yauzl.fromBuffer(zipBuffer, { lazyEntries: true }, (err, zipFile) => {
      if (err) {
        reject(err);
        return;
      }

      zipFile.on('entry', entry => {
        if (!entry.fileName.startsWith(expectedRootDir)) {
          reject(new Error(`Unexpected zip entry: "${entry.fileName}".`));
          return;
        }

        if (entry.fileName.endsWith('/')) {
          if (entry.fileName !== expectedRootDir) {
            reject(err);
            return;
          }
          zipFile.readEntry();
          return;
        }

        zipFile.openReadStream(entry, (streamErr, stream) => {
          if (streamErr) {
            reject(streamErr);
            return;
          }

          stream.on('end', () => zipFile.readEntry());

          const destinationFilename = entry.fileName.substr(
            expectedRootDir.length
          );
          stream.pipe(
            fs.createWriteStream(
              path.join(destinationPath, destinationFilename)
            )
          );
        });
      });

      zipFile.on('end', resolve);
      zipFile.on('error', reject);

      zipFile.readEntry();
    });
  });
}

async function getRobotoFiles(url, destinationPath) {
  const zipBuffer = await downloadRobotoZip(url);

  await unzip(zipBuffer, destinationPath);
}

function handleError(error) {
  console.error(error);
  process.exit(1);
}

const url =
  'https://github.com/google/roboto/releases/download/v2.135/roboto-hinted.zip';
const fontsPath = path.join(process.cwd(), 'build', 'fonts');

Promise.resolve()
  .then(() => {
    if (!fs.existsSync(fontsPath)) {
      return getRobotoFiles(url, fontsPath);
    }

    return '';
  })
  .catch(handleError);
