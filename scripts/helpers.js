// Helper functions shared across scripts.
var path = require('path');
var fs = require('fs');

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

function isMultiArch(root_path) {
    return fileExists(path.join(root_path, 'build', '64', 'Release', 'loot64.dll'));
}

module.exports.fileExists = fileExists;
module.exports.isMultiArch = isMultiArch;
