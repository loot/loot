'use strict';
const helpers = require('./helpers');
const hyd = require('hydrolysis');
const fs = require('fs-extra');
const path = require('path');
const getRobotoFiles = require('./get_roboto_files').getRobotoFiles;

function handleError(error) {
  console.error(error);
  process.exit(1);
}

function getHtmlImports(filePath) {
  return hyd.Analyzer.analyze(filePath).then((analyzer) =>
    analyzer._getDependencies(filePath)
  );
}

function isString(variable) {
  return typeof variable === 'string' || variable instanceof String;
}

function flattenUnique(array) {
  const results = new Set();
  array.forEach((element) => {
    if (isString(element)) {
      results.add(element);
    } else if (element) {
      flattenUnique(element).forEach((subelement) => {
        results.add(subelement);
      });
    }
  });
  return results;
}

function getRecursiveHtmlImports(filePath, imports) {
  return getHtmlImports(filePath).then((paths) =>
    Promise.all(paths.map((dependency) => {
      if (imports.has(dependency)) {
        return null;
      }
      imports.add(dependency);
      return getRecursiveHtmlImports(dependency, imports);
    }))
  ).then((results) => {
    flattenUnique(results).forEach((dependency) => {
      imports.add(dependency);
    });

    return imports;
  });
}

function getJavaScriptSources(filePath) {
  return hyd.Analyzer.analyze(filePath).then((analyzer) =>
    Object.keys(analyzer.parsedScripts).filter((script) =>
      script.endsWith('.js')
    )
  );
}

function getRelativePath(filePath) {
  if (filePath.startsWith('src/gui/html/')) {
    return filePath.substring(13);
  }
  return filePath;
}

function normalisePaths(html) {
  return html.replace(/href="(\.\.\/){3}/g, 'href="')
    .replace(/src="(\.\.\/){3}/g, 'src="');
}

function copyNormalisedFile(sourceFile, destinationFile) {
  const html = fs.readFileSync(sourceFile, { encoding: 'utf8' });
  fs.mkdirsSync(path.dirname(destinationFile));
  fs.writeFileSync(destinationFile, normalisePaths(html));
}

function copyFiles(pathsPromise, destinationRootPath) {
  pathsPromise.then((paths) => {
    paths.forEach((filePath) => {
      const destinationPath = `${destinationRootPath}/${getRelativePath(filePath)}`;
      if (filePath.includes('bower_components')) {
        fs.copySync(filePath, destinationPath);
      } else {
        copyNormalisedFile(filePath, destinationPath);
      }
    });
  }).catch(handleError);
}

const url = 'https://github.com/google/roboto/releases/download/v2.135/roboto-hinted.zip';
const fontsPath = 'build/fonts';

Promise.resolve().then(() => {
  if (!fs.existsSync(fontsPath)) {
    return getRobotoFiles(url, fontsPath);
  }

  return '';
}).then(() => {
  helpers.getAppReleasePaths('.').forEach(releasePath => {
    const index = 'src/gui/html/index.html';
    const destinationRootPath = `${releasePath.path}/resources/ui`;
    const imports = new Set();

    copyFiles(getRecursiveHtmlImports(index, imports), destinationRootPath);
    copyFiles(getJavaScriptSources(index), destinationRootPath);
    fs.copySync('src/gui/html/css', `${destinationRootPath}/css`);
    fs.copySync('resources/ui/css/dark-theme.css', `${destinationRootPath}/css/dark-theme.css`);
    fs.copySync(fontsPath, `${destinationRootPath}/fonts`);
    copyNormalisedFile(index, `${destinationRootPath}/index.html`);

    // This is the only JS file referenced by a HTML import (neon-animation),
    // so just hardcode it instead of recursively searching for it.
    const webAnimationsJs = 'bower_components/web-animations-js/web-animations-next-lite.min.js';
    fs.copySync(webAnimationsJs, `${destinationRootPath}/${webAnimationsJs}`);
  });
}).catch(handleError);
