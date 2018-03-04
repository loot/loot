/* eslint-disable no-console */

const helpers = require('./helpers');
const { Analyzer, FSUrlLoader } = require('polymer-analyzer');
const fs = require('fs-extra');
const path = require('path');
const { getRobotoFiles } = require('./get_roboto_files');

function handleError(error) {
  console.error(error);
  process.exit(1);
}

function getFeatureURLs(filePath, featureTypes) {
  const analyzer = new Analyzer({
    urlLoader: new FSUrlLoader(process.cwd())
  });
  return analyzer.analyze([filePath]).then(analysis => {
    const featureUrls = {};
    featureTypes.forEach(featureType => {
      featureUrls[featureType] = new Set();
      analysis
        .getFeatures({
          kind: featureType,
          imported: true,
          externalPackages: true
        })
        .forEach(feature => {
          featureUrls[featureType].add(feature.url);
        });
    });

    return featureUrls;
  });
}

function getRelativePath(filePath) {
  if (filePath.startsWith('src/gui/html/')) {
    return filePath.substring(13);
  }
  return filePath;
}

function normalisePaths(html) {
  return html
    .replace(/href="(\.\.\/){3}/g, 'href="')
    .replace(/src="(\.\.\/){3}/g, 'src="');
}

function resolveBowerPaths(sourceFile, destinationFile) {
  const html = fs
    .readFileSync(sourceFile, { encoding: 'utf8' })
    .replace(/bower_components/g, '../../../bower_components');

  fs.mkdirsSync(path.dirname(destinationFile));
  fs.writeFileSync(destinationFile, html);
}

function copyNormalisedFile(sourceFile, destinationFile) {
  const html = fs.readFileSync(sourceFile, { encoding: 'utf8' });
  fs.mkdirsSync(path.dirname(destinationFile));
  fs.writeFileSync(destinationFile, normalisePaths(html));
}

function copyFiles(pathsPromise, destinationRootPath) {
  return pathsPromise
    .then(paths => {
      paths.forEach(filePath => {
        const destinationPath = `${destinationRootPath}/${getRelativePath(
          filePath
        )}`;
        if (filePath.includes('bower_components')) {
          fs.copySync(filePath, destinationPath);
        } else {
          copyNormalisedFile(filePath, destinationPath);
        }
      });
    })
    .catch(handleError);
}

const index = 'src/gui/html/index.html';
const tempIndex = 'src/gui/html/index.bower.html';
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
  .then(() => resolveBowerPaths(index, tempIndex))
  .then(() => {
    const promises = [];
    helpers.getAppReleasePaths('.').forEach(releasePath => {
      const destinationRootPath = `${releasePath.path}/resources/ui`;

      const urls = getFeatureURLs(tempIndex, ['html-import', 'html-script']);
      const htmlImportUrls = urls.then(features => features['html-import']);
      const scriptUrls = urls.then(features => features['html-script']);

      promises.push(copyFiles(htmlImportUrls, destinationRootPath));
      promises.push(copyFiles(scriptUrls, destinationRootPath));
    });
    return Promise.all(promises);
  })
  .then(() => fs.unlinkSync(tempIndex))
  .catch(handleError);
