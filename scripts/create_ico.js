const helpers = require('./helpers');
const fs = require('fs-extra');
const path = require('path');

const svgPath = path.join('resources', 'icon.svg');
const buildDirectory = path.join('build', 'icon');
const outputPath = path.join(buildDirectory, 'icon.ico');

const sizes = [16, 20, 24, 30, 32, 36, 40, 48, 60, 64, 72, 80, 96, 128, 256];

fs.mkdirsSync(buildDirectory);

const convertArgs = [];
sizes.forEach((size) => {
  const pngFile = path.join(buildDirectory, `icon-${size}.png`);
  convertArgs.push(pngFile);

  helpers.safeExecFileSync('inkscape', [
    '-z',
    '-e',
    pngFile,
    '-w',
    size,
    '-h',
    size,
    svgPath,
  ]);
});

convertArgs.push(outputPath);
helpers.safeExecFileSync('convert', convertArgs);
