// Helper functions shared across scripts.
var path = require('path');
var fs = require('fs');
var os = require('os');

function fileExists(file_path) {
    try {
        // Query the entry
        stats = fs.lstatSync(file_path);

        // Is it a directory?
        if (stats.isFile()) {
            return true;
        }
    } catch (e) {}

    return false;
}

function getAppReleasePaths(root_path) {
    var paths = [];
    var file = 'LOOT';
    var paths_to_try = [
        {
            path: path.join(root_path, 'build'),
            label: null
        },
        {
            path: path.join(root_path, 'build', '32'),
            label: '32 bit'
        },
        {
            path: path.join(root_path, 'build', '64'),
            label: '64 bit'
        }
    ];

    if (os.platform() == 'win32') {
        file += '.exe';
    }

    for (var i = 0; i < paths_to_try.length; ++i) {
        if (os.platform() == 'win32') {
            paths_to_try[i].path = path.join(paths_to_try[i].path, 'Release');
        }

        if (fileExists(path.join(paths_to_try[i].path, file))) {
            paths.push(paths_to_try[i]);
        }
    }

    return paths;
}

function getApiBinaryPaths(root_path) {
    var paths = [];
    var files = [];
    var paths_to_try = [
        {
            path: path.join(root_path, 'build'),
            label: null
        },
        {
            path: path.join(root_path, 'build', '32'),
            label: '32 bit'
        },
        {
            path: path.join(root_path, 'build', '64'),
            label: '64 bit'
        }
    ];

    for (var i = 0; i < paths_to_try.length; ++i) {
        if (os.platform() == 'win32') {
            paths_to_try[i].path = path.join(paths_to_try[i].path, 'Release');

            files = [
                {
                    name: 'loot32.dll',
                    label: '32 bit'
                },
                {
                    name: 'loot64.dll',
                    label: '64 bit'
                }
            ];
        } else {
            files = [
                {
                    name: 'libloot32.so',
                    label: '32 bit'
                },
                {
                    name: 'libloot64.so',
                    label: '64 bit'
                }
            ];
        }

        for (var j = 0; j < files.length; ++j) {
            if (fileExists(path.join(paths_to_try[i].path, files[j].name))) {
                paths_to_try[i].path = path.join(paths_to_try[i].path, files[j].name);
                paths_to_try[i].label = files[j].label;
                paths.push(paths_to_try[i]);
                break;
            }
        }
    }

    return paths;
}

module.exports.fileExists = fileExists;
module.exports.getAppReleasePaths = getAppReleasePaths;
module.exports.getApiBinaryPaths = getApiBinaryPaths;
