#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import glob
import os
import re
import shutil
import subprocess

INVALID_FILENAME_CHARACTERS = re.compile('[/<>"|]')

def get_git_description(given_branch):
    result = subprocess.run(
        ['git', 'describe', '--tags', '--long', '--abbrev=7'],
        capture_output=True,
        text=True,
        check=True
    )

    describe = result.stdout[:-1]

    branch = given_branch
    if (not branch):
        result = subprocess.run(
            ['git', 'rev-parse', '--abbrev-ref', 'HEAD'],
            capture_output=True,
            text=True,
            check=True
        )

        branch = result.stdout[:-1]

    return describe + '_' + branch

def get_archive_file_extension():
    if os.name == 'nt':
        return '.7z'

    return '.tar.xz'

def get_app_release_path(root_path):
    if os.name == 'nt':
        return os.path.join(root_path, 'build', 'Release')

    return os.path.join(root_path, 'build')

def replace_invalid_filename_characters(filename):
    return INVALID_FILENAME_CHARACTERS.sub('-', filename)

def copy_qt_resources(executable_path, output_path, qt_root_path):
    if os.name == 'nt':
        subprocess.run(
            ['windeployqt', '--release', '--dir', output_path, executable_path],
            check=True
        )

        # windeployqt copies a few DLLs that aren't actually used by LOOT, so
        # delete them from the output path.
        unnecessary_dlls = [
            'D3Dcompiler_47.dll',
            'opengl32sw.dll'
        ]
        for unnecessary_dll in unnecessary_dlls:
            dll_path = os.path.join(output_path, unnecessary_dll)
            if os.path.exists(dll_path):
                os.remove(dll_path)

        return

    libraries = [
        'libicudata.so.56',
        'libicudata.so.56.1',
        'libicui18n.so.56',
        'libicui18n.so.56.1',
        'libicuuc.so.56',
        'libicuuc.so.56.1',
        'libQt6Core.so.6',
        'libQt6Core.so.6.4.2',
        'libQt6DBus.so.6',
        'libQt6DBus.so.6.4.2',
        'libQt6Gui.so.6',
        'libQt6Gui.so.6.4.2',
        'libQt6Network.so.6',
        'libQt6Network.so.6.4.2',
        'libQt6Widgets.so.6',
        'libQt6Widgets.so.6.4.2',

        # For xcb platform plugin.
        'libQt6XcbQpa.so.6',
        'libQt6XcbQpa.so.6.4.2',
        'libQt6OpenGL.so.6',
        'libQt6OpenGL.so.6.4.2'
    ]

    for library in libraries:
        shutil.copy2(
            os.path.join(qt_root_path, 'lib', library),
            os.path.join(output_path, library)
        )

    plugins_folders = [
        'iconengines',
        'imageformats',
        'networkinformation',
        'tls'
    ]

    for plugin_folder in plugins_folders:
        shutil.copytree(
            os.path.join(qt_root_path, 'plugins', plugin_folder),
            os.path.join(output_path, plugin_folder)
        )

    os.makedirs(os.path.join(output_path, 'platforms'))
    shutil.copy2(
        os.path.join(qt_root_path, 'plugins', 'platforms', 'libqxcb.so'),
        os.path.join(output_path, 'platforms', 'libqxcb.so')
    )

    os.makedirs(os.path.join(output_path, 'translations'))
    qt_translations = glob.glob(
        os.path.join(qt_root_path, 'translations', 'qt_*.qm')
    )

    for translation in qt_translations:
        shutil.copy2(
            translation,
            os.path.join(output_path, 'translations', os.path.basename(translation))
        )

def get_language_folders(root_path):
    l10n_path = os.path.join(root_path, 'resources', 'l10n')

    return [ f.name for f in os.scandir(l10n_path) if f.is_dir() ]

def compress(source_path, destination_path):
    # Ensure that the output directory is empty.
    if os.path.exists(destination_path):
        shutil.rmtree(destination_path)

    filename = os.path.basename(destination_path)
    root_folder = os.path.basename(source_path)
    working_directory = os.path.dirname(source_path)

    if os.name == 'nt':
        seven_zip_path = 'C:\\Program Files\\7-Zip\\7z.exe'
        if not os.path.exists(seven_zip_path):
            seven_zip_path = '7z'

        subprocess.run(
            [seven_zip_path, 'a', '-r', filename, root_folder],
            cwd=working_directory,
            check=True
        )
    else:
        subprocess.run(
            ['tar', '-cJf', filename, root_folder],
            cwd=working_directory,
            check=True
        )

def create_app_archive(root_path, release_path, temp_path, destination_path, qt_root_path):
    # Ensure that the output directory is empty.
    if os.path.exists(temp_path):
        shutil.rmtree(temp_path)

    os.makedirs(temp_path)

    # Copy LOOT exectuable and other binaries.
    binaries = []
    if os.name == 'nt':
        binaries = ['LOOT.exe', 'loot.dll']
    else:
        binaries = ['LOOT', 'libloot.so']

    for binary in binaries:
        shutil.copy2(
            os.path.join(release_path, binary),
            os.path.join(temp_path, binary)
        )

    copy_qt_resources(
        os.path.join(release_path, 'LOOT.exe'),
        temp_path,
        qt_root_path
    )

    # Translation files.
    for folder_name in get_language_folders(root_path):
        l10n_path = os.path.join(temp_path, 'resources', 'l10n', folder_name, 'LC_MESSAGES')

        os.makedirs(l10n_path)
        shutil.copy2(
            os.path.join(root_path, 'resources', 'l10n', folder_name, 'LC_MESSAGES', 'loot.mo'),
            os.path.join(l10n_path, 'loot.mo')
        )

    # Documentation.
    shutil.copytree(
        os.path.join(root_path, 'build', 'docs', 'html'),
        os.path.join(temp_path, 'docs')
    )

    compress(temp_path, destination_path)

    shutil.rmtree(temp_path)

    print(destination_path)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = 'Create an archive artifact')
    parser.add_argument('root-path', nargs=1)
    parser.add_argument('given-branch', nargs=1)
    parser.add_argument('qt-root-path', nargs='?')
    parser.print_help()

    arguments = vars(parser.parse_args())

    given_branch = arguments['given-branch'][0]
    root_path = arguments['root-path'][0]
    qt_root_path = arguments['qt-root-path']

    git_description = get_git_description(given_branch)
    file_extension = get_archive_file_extension()
    release_path = get_app_release_path(root_path)
    filename = 'loot_{}'.format(replace_invalid_filename_characters(git_description))

    create_app_archive(
        root_path,
        release_path,
        os.path.join(root_path, 'build', filename),
        os.path.join(root_path, 'build', filename + file_extension),
        qt_root_path
    )
