/* eslint-disable no-console */

'use strict';

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

function copyNormalisedFile(sourceFile, destinationFile) {
  const html = fs.readFileSync(sourceFile, { encoding: 'utf8' });
  fs.mkdirsSync(path.dirname(destinationFile));
  fs.writeFileSync(destinationFile, normalisePaths(html));
}

function copyFiles(pathsPromise, destinationRootPath) {
  pathsPromise
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
  .then(() => {
    helpers.getAppReleasePaths('.').forEach(releasePath => {
      const index = 'src/gui/html/index.html';
      const destinationRootPath = `${releasePath.path}/resources/ui`;

      const urls = getFeatureURLs(index, ['html-import', 'html-script', 'js-import']);
      const htmlImportUrls = urls.then(features => features['html-import']);
      const scriptUrls = urls.then(features => features['html-script']);
      const moduleUrls = urls.then(features => features['js-import']);

      copyFiles(htmlImportUrls, destinationRootPath);
      copyFiles(scriptUrls, destinationRootPath);
      copyFiles(moduleUrls, destinationRootPath);
      fs.copySync('src/gui/html/css', `${destinationRootPath}/css`);
      fs.copySync(
        'resources/ui/css/dark-theme.css',
        `${destinationRootPath}/css/dark-theme.css`
      );
      fs.copySync(fontsPath, `${destinationRootPath}/fonts`);
      copyNormalisedFile(index, `${destinationRootPath}/index.html`);
    });
  })
  .catch(handleError);
