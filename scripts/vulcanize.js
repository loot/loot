#!/usr/bin/env node
// Build the UI's index.html file. Takes one argument, which is the path to the
// repository's root.
var child_process =  require('child_process');
var path = require('path');
var fs = require('fs');

if (process.argc < 3) {
    var root_path = process.argv[2];
} else {
    var root_path = '.';
}
var output_path = path.join(root_path, 'build', 'Release', 'resources', 'ui');

// Makes sure output directory exists first.
try {
    fs.mkdirSync(output_path);
} catch (e) {
    if (e.code != 'EEXIST') {
        console.log(e);
    }
}

child_process.execFileSync('vulcanize.cmd', [
    '--inline',
    '--strip',
    '--config',
    path.join(root_path, 'scripts', 'vulcanize.config.json'),
    '-o',
    path.join(output_path, 'index.html'),
    path.join(root_path, 'src', 'gui', 'html', 'index.html')
]);
