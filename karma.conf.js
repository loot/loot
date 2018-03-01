/* eslint-disable no-console */

'use strict';

const helpers = require('./scripts/helpers');
const path = require('path');

process.env.CHROME_BIN = path.join(
  helpers.getAppReleasePaths('.')[0].path,
  'LOOT'
);
console.log(
  `Running tests using browser executable at ${process.env.CHROME_BIN}`
);

module.exports = config => {
  config.set({
    basePath: '',
    frameworks: ['mocha', 'chai'],
    files: [
      'node_modules/jed/jed.js',
      'node_modules/jed-gettext-parser/jedGettextParser.js',
      'node_modules/lodash/core.min.js',
      'src/tests/gui/html/js/mock_*.js',
      'src/gui/html/js/filters.js',
      'src/gui/html/js/plugin.js',
      'src/gui/html/js/game.js',
      'src/gui/html/js/query.js',
      'src/gui/html/js/state.js',
      'src/gui/html/js/translator.js',
      'src/gui/html/js/updateExists.js',
      'src/tests/gui/html/js/*.js'
    ],
    exclude: [],
    preprocessors: {},
    reporters: ['progress'],
    port: 9876,
    colors: true,
    logLevel: config.LOG_INFO,
    autoWatch: false,
    browsers: ['Chrome'],
    singleRun: true
  });
};
