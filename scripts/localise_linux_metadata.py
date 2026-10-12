#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Write copies of the Linux desktop entry and AppStream metainfo files that
# include the translations found in LOOT's PO files.

import argparse
import os
import shutil
import subprocess
import tempfile

DESKTOP_FILENAME = 'io.github.loot.loot.desktop'
METAINFO_FILENAME = 'io.github.loot.loot.metainfo.xml'

def get_po_paths(root_path):
    l10n_path = os.path.join(root_path, 'resources', 'l10n')

    po_paths = []
    for entry in sorted(os.scandir(l10n_path), key=lambda e: e.name):
        po_path = os.path.join(entry.path, 'LC_MESSAGES', 'loot.po')
        if entry.is_dir() and os.path.exists(po_path):
            po_paths.append((entry.name, po_path))

    return po_paths

def get_msgfmt_env(root_path):
    # Make sure msgfmt can find ITS rules for the metainfo file even if
    # AppStream's rules are not installed.
    data_dirs = [os.path.join(root_path, 'resources', 'linux', 'gettext')]
    if os.environ.get('GETTEXTDATADIRS'):
        data_dirs.append(os.environ['GETTEXTDATADIRS'])

    env = os.environ.copy()
    env['GETTEXTDATADIRS'] = os.pathsep.join(data_dirs)

    return env

def merge_translations(root_path, mode_args, filename, output_path):
    # msgfmt can only merge all the translations in one go if the PO files
    # are named <locale>.po and are in a single directory. That's not how
    # LOOT's PO files are laid out, so merge one language at a time, using
    # each run's output as the template for the next run. msgfmt leaves out
    # strings that are untranslated or marked as fuzzy.
    template_path = os.path.join(root_path, 'resources', 'linux', filename)
    env = get_msgfmt_env(root_path)

    with tempfile.TemporaryDirectory() as temp_path:
        for locale, po_path in get_po_paths(root_path):
            # The filename is kept the same because it's used to find the
            # ITS rules to use.
            merged_path = os.path.join(temp_path, locale, filename)
            os.makedirs(os.path.dirname(merged_path))

            subprocess.check_call(
                ['msgfmt'] + mode_args + [
                    '--template', template_path,
                    '--locale', locale,
                    '-o', merged_path,
                    po_path
                ],
                env=env
            )

            template_path = merged_path

        shutil.copyfile(template_path, os.path.join(output_path, filename))

def localise_desktop_entry(root_path, output_path):
    # Only the desktop entry's Comment is translatable. The default keywords
    # are disabled using an empty value because msgfmt requires --keyword to
    # have a value before gettext 0.22.
    merge_translations(
        root_path,
        ['--desktop', '--keyword=', '--keyword=Comment'],
        DESKTOP_FILENAME,
        output_path
    )

def localise_metainfo(root_path, output_path):
    merge_translations(root_path, ['--xml'], METAINFO_FILENAME, output_path)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='Write translated copies of the Linux desktop entry and AppStream metainfo files'
    )
    parser.add_argument(
        'output_path',
        help='The directory to write the translated files to'
    )
    arguments = parser.parse_args()

    root_path = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

    os.makedirs(arguments.output_path, exist_ok=True)

    localise_desktop_entry(root_path, arguments.output_path)
    localise_metainfo(root_path, arguments.output_path)
