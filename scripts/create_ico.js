const helpers = require('./helpers');
const fs = require('fs-extra');
const path = require('path');

const svgPath = path.join('resources', 'icon.svg');
const buildDirectory = path.join('build', 'icon');
const outputPath = path.join(buildDirectory, 'icon.ico');

const svgSize = 192;
const svgDPI = 90;

const sizes = [16, 20, 24, 30, 32, 36, 40, 48, 60, 64, 72, 80, 96, 128, 256];

fs.mkdirsSync(buildDirectory);

const convertArgs = [];
sizes.forEach((size) => {
  const pngFile = path.join(buildDirectory, `icon-${size}.png`);
  convertArgs.push(pngFile);

  helpers.safeExecFileSync('convert', [
    '-density',
    (size / svgSize) * svgDPI,
    '-background',
    'none',
    svgPath,
    pngFile,
  ]);
});

convertArgs.push(outputPath);
helpers.safeExecFileSync('convert', convertArgs);
