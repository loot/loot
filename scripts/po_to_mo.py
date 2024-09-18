#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import subprocess

def run_msgfmt(locale_directory):
    po_path = os.path.join(locale_directory, 'LC_MESSAGES', 'loot.po')
    mo_path = os.path.join(locale_directory, 'LC_MESSAGES', 'loot.mo')

    if (os.path.exists(po_path)):
        subprocess.check_call(['msgfmt', po_path, '-o', mo_path])

if __name__ == "__main__":
    root_path = os.path.join('.', 'resources', 'l10n')

    locale_directories = [ f.path for f in os.scandir(root_path) if f.is_dir() ]
    for local_directory in list(locale_directories):
        run_msgfmt(local_directory)
