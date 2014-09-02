#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Creates an archive of a LOOT release, putting it in the 'build' folder.

# Files and folders that need to go in (relative to repository root):
#
# build/LOOT.exe
# resources/l10n/es/LC_MESSAGES/loot.mo
# resources/l10n/es/LC_MESSAGES/wxstd.mo
# resources/l10n/ru/LC_MESSAGES/loot.mo
# resources/l10n/ru/LC_MESSAGES/wxstd.mo
# resources/l10n/fr/LC_MESSAGES/loot.mo
# resources/l10n/fr/LC_MESSAGES/wxstd.mo
# resources/l10n/zh/LC_MESSAGES/loot.mo
# resources/l10n/zh/LC_MESSAGES/wxstd.mo
# resources/l10n/pl/LC_MESSAGES/loot.mo
# resources/l10n/pl/LC_MESSAGES/wxstd.mo
# resources/l10n/fi/LC_MESSAGES/loot.mo
# resources/l10n/fi/LC_MESSAGES/wxstd.mo
# resources/l10n/de/LC_MESSAGES/loot.mo
# resources/l10n/de/LC_MESSAGES/wxstd.mo
# resources/report/report.html
# resources/report/require.js
# resources/report/polyfill.js
# resources/report/script.js
# resources/report/style.css
# docs/images
# docs/licenses
# docs/LOOT Metadata Syntax.html
# docs/LOOT Readme.html

#   LOOT
#
#   A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
#   Fallout: New Vegas.
#
#   Copyright (C) 2013-2014    WrinklyNinja
#
#   This file is part of LOOT.
#
#   LOOT is free software: you can redistribute
#   it and/or modify it under the terms of the GNU General Public License
#   as published by the Free Software Foundation, either version 3 of
#   the License, or (at your option) any later version.
#
#   LOOT is distributed in the hope that it will
#   be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with LOOT.  If not, see
#   <http://www.gnu.org/licenses/>.

import sys
import os
import shutil
import zipfile

temp_path = os.path.join('..', 'build', 'archive.tmp')
archive_name = 'LOOT Archive.zip'

# Set archive name if alternative is given.
if (len(sys.argv) > 1):
    archive_name = sys.argv[1]

# First make sure that the temporary folder for the archive exists.
if not os.path.exists(temp_path):
    os.makedirs(temp_path)

# Now copy everything into the temporary folder.
shutil.copy( os.path.join('..', 'build', 'LOOT.exe'), temp_path )

for lang in ['es', 'ru', 'fr', 'zh_CN', 'pl', 'pt_BR', 'fi', 'de']:
    os.makedirs(os.path.join(temp_path, 'resources', 'l10n', lang, 'LC_MESSAGES'))
    shutil.copy( os.path.join('..', 'resources', 'l10n', lang, 'LC_MESSAGES', 'loot.mo'), os.path.join(temp_path, 'resources', 'l10n', lang, 'LC_MESSAGES') )
    shutil.copy( os.path.join('..', 'resources', 'l10n', lang, 'LC_MESSAGES', 'wxstd.mo'), os.path.join(temp_path, 'resources', 'l10n', lang, 'LC_MESSAGES') )

shutil.copytree( os.path.join('..', 'resources', 'report'), os.path.join(temp_path, 'resources', 'report') )

shutil.copytree( os.path.join('..', 'docs', 'images'), os.path.join(temp_path, 'docs', 'images') )
shutil.copytree( os.path.join('..', 'docs', 'licenses'), os.path.join(temp_path, 'docs', 'licenses') )
shutil.copy( os.path.join('..', 'docs', 'LOOT Metadata Syntax.html'), os.path.join(temp_path, 'docs') )
shutil.copy( os.path.join('..', 'docs', 'LOOT Readme.html'), os.path.join(temp_path, 'docs') )

# Now compress the temporary folder. (Creating a zip because I can't get pylzma to work...)
os.chdir(temp_path)
zip = zipfile.ZipFile( os.path.join('..', archive_name), 'w', zipfile.ZIP_DEFLATED )
for root, dirs, files in os.walk('.'):
    for file in files:
        zip.write(os.path.join(root, file))
zip.close()
os.chdir('..')

# And finally, delete the temporary folder.
shutil.rmtree('archive.tmp')
