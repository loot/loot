#!/usr/bin/env node
// Archive packaging script. Takes one argument, which is the path to the
// repository's root. Requires 7-zip and Git to be installed, and Git to be
// available on the system path.

var child_process =  require('child_process');
var path = require('path');
var fs = require('fs-extra');

function vulcanize() {
    return child_process.execFileSync('node', [
        'scripts/vulcanize.js',
        root_path
    ]);
}

function getGitDescription() {
    return String(child_process.execFileSync('git', [
        'describe',
        '--tags',
        '--long'
    ])).slice(0, -1);
}

function compress(source_path, dest_path) {
    var sevenzip_path = path.join('C:\\', 'Program Files', '7-Zip', '7z.exe');

    // First remove any existing archive.
    fs.removeSync(dest_path);

    // The last argument must have a leading dot for the subdirectory not to
    // be present in the archive, but path.join removes it, so it's prefixed.
    return child_process.execFileSync(sevenzip_path, [
        'a',
        '-r',
        dest_path,
        '.' + path.sep + path.join(source_path, '*')
    ]);
}

function createAppArchive(dest_path) {
    // Ensure that the output directory is empty.
    fs.emptyDirSync(temp_path);

    // Copy LOOT exectuable and CEF files.
    [
        'LOOT.exe',
        'd3dcompiler_47.dll',
        'libEGL.dll',
        'libGLESv2.dll',
        'libcef.dll',
        'wow_helper.exe',
        'natives_blob.bin',
        'snapshot_blob.bin',
        'cef.pak',
        'cef_100_percent.pak',
        'cef_200_percent.pak',
        'devtools_resources.pak',
        'icudtl.dat'
    ].forEach(function(file){
        fs.copySync(
            path.join(root_path, 'build', 'Release', file),
            path.join(temp_path, file)
        );
    });

    // CEF locale file.
    fs.mkdirsSync(path.join(temp_path, 'resources', 'l10n'));
    fs.copySync(
        path.join(root_path, 'build', 'Release', 'resources', 'l10n', 'en-US.pak'),
        path.join(temp_path, 'resources', 'l10n', 'en-US.pak')
    );

    // Translation files.
    [
        'es', 'ru', 'fr', 'zh_CN', 'pl',
        'pt_BR', 'fi', 'de', 'da', 'ko'
    ].forEach(function(lang){
        fs.mkdirsSync(path.join(temp_path, 'resources', 'l10n', lang, 'LC_MESSAGES'));
        fs.copySync(
            path.join(root_path, 'resources', 'l10n', lang, 'LC_MESSAGES', 'loot.mo'),
            path.join(temp_path, 'resources', 'l10n', lang, 'LC_MESSAGES', 'loot.mo')
        );
    });

    // UI files.
    fs.mkdirsSync(path.join(temp_path, 'resources', 'ui', 'css'));
    fs.copySync(
        path.join(root_path, 'build', 'Release', 'resources', 'ui', 'index.html'),
        path.join(temp_path, 'resources', 'ui', 'index.html')
    );
    fs.copySync(
        path.join(root_path, 'resources', 'ui', 'css', 'dark-theme.css'),
        path.join(temp_path, 'resources', 'ui', 'css', 'dark-theme.css')
    );
    fs.copySync(
        path.join(root_path, 'resources', 'ui', 'fonts'),
        path.join(temp_path, 'resources', 'ui', 'fonts')
    );

    // Docs.
    fs.mkdirsSync(path.join(temp_path, 'docs'));
    [
        'images',
        'licenses',
        'LOOT Metadata Syntax.html',
        'LOOT Readme.html'
    ].forEach(function(item){
        fs.copySync(
            path.join(root_path, 'docs', item),
            path.join(temp_path, 'docs', item)
        );
    });

    // Now compress the folder to a 7-zip archive.
    compress(temp_path, dest_path);

    // Finally, delete the temporary folder.
    fs.removeSync(temp_path);
}

function createApiArchive(dest_path) {
    // Ensure that the output directory is empty.
    fs.emptyDirSync(temp_path);

    // API binary.
    fs.copySync(
        path.join(root_path, 'build', 'Release', 'loot32.dll'),
        path.join(temp_path, 'loot32.dll')
    );

    // API header file.
    fs.mkdirsSync(path.join(temp_path, 'include', 'loot'));
    fs.copySync(
        path.join(root_path, 'src', 'api', 'api.h'),
        path.join(temp_path, 'include', 'loot', 'api.h')
    );

    // Docs.
    fs.mkdirsSync(path.join(temp_path, 'docs'));
    fs.copySync(
        path.join(root_path, 'docs', 'latex', 'refman.pdf'),
        path.join(temp_path, 'docs', 'readme.pdf')
    );
    fs.copySync(
        path.join(root_path, 'docs', 'licenses'),
        path.join(temp_path, 'docs', 'licenses')
    );


    // Now compress the folder to a 7-zip archive.
    compress(temp_path, dest_path);

    // Finally, delete the temporary folder.
    fs.removeSync(temp_path);
}

if (process.argv.length < 3) {
    var root_path = '.';
} else {
    var root_path = process.argv[2];
}
var temp_path = path.join(root_path, 'build', 'archive.tmp');

var git_desc = getGitDescription();
vulcanize();
createAppArchive(path.join(root_path, 'build', 'LOOT ' + git_desc + '.7z'));
createApiArchive(path.join(root_path, 'build', 'LOOT API ' + git_desc + '.7z'));
