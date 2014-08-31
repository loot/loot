#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Creates an archive of a LOOT release, putting it in the 'build' folder.

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
import subprocess

temp_path = os.path.join('..', 'build', 'archive.tmp')
archive_name = 'LOOT Archive.zip'

# Some initial setup.
#
# There are two compression methods available:
#
#  * Zip (Deflate)
#  * 7-Zip (LZMA)
#
# Python 3.3+ can also do BZIP2 and LZMA zip archives, but they don't have good
# Windows OS support, so people think they should be able to open them without
# an archiving utility, find they can't, and think the archive is broken. So
# they won't be used.
#
# Look for 7-Zip in its default install location, and use it if it is found.
# Fall back to zip if not.
#
# Archives are named using the output of `git describe --tags --long`, if Git
# is found in the current PATH. Otherwise, they will simply be named
# 'LOOT Archive'.

# Python 3.3+ has shutil.which, but 2.7 doesn't.
if 'which' in dir(shutil):
    git = shutil.which('git.exe')
else:
    git = None
    path = os.getenv('PATH')
    for p in path.split(os.path.pathsep):
        p = os.path.join(p, 'git.exe')
        if os.path.exists(p) and os.access(p, os.X_OK):
            git = p

if not git:
    # Git wasn't found in PATH, do a search (in GitHub install location).
    github = os.path.join( os.getenv('LOCALAPPDATA'), 'GitHub' )
    for folder, subdirs, files in os.walk(github):
        for subdir in subdirs:
            # Check for a <github>\<subdir>\cmd\git.exe
            if os.path.exists( os.path.join( folder, subdir, 'cmd', 'git.exe') ):
                git = os.path.join( folder, subdir, 'cmd', 'git.exe')


if git:
    args = [git, 'describe', '--tags', '--long']
    output = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    if len(output[1]) == 0:
        archive_name = 'LOOT ' + output[0].decode('ascii')[:-1]
    else:
        archive_name = 'LOOT Archive'
else:
    archive_name = 'LOOT Archive'

# Delete the temporary folder if it already exists, then create it.
if os.path.exists(temp_path):
    shutil.rmtree(temp_path)
os.makedirs(temp_path)

# Now copy everything into the temporary folder.
shutil.copy( os.path.join('..', 'build', 'Release', 'LOOT.exe'), temp_path )
shutil.copy( os.path.join('..', 'build', 'Release', 'd3dcompiler_43.dll'), temp_path )
shutil.copy( os.path.join('..', 'build', 'Release', 'd3dcompiler_46.dll'), temp_path )
shutil.copy( os.path.join('..', 'build', 'Release', 'libEGL.dll'), temp_path )
shutil.copy( os.path.join('..', 'build', 'Release', 'libGLESv2.dll'), temp_path )
shutil.copy( os.path.join('..', 'build', 'Release', 'libcef.dll'), temp_path )
shutil.copy( os.path.join('..', 'build', 'Release', 'cef.pak'), temp_path )
shutil.copy( os.path.join('..', 'build', 'Release', 'cef_100_percent.pak'), temp_path )
shutil.copy( os.path.join('..', 'build', 'Release', 'cef_200_percent.pak'), temp_path )
shutil.copy( os.path.join('..', 'build', 'Release', 'devtools_resources.pak'), temp_path )
shutil.copy( os.path.join('..', 'build', 'Release', 'icudtl.dat'), temp_path )

for lang in ['es', 'ru', 'fr', 'zh_CN', 'pl', 'pt_BR', 'fi', 'de']:
    os.makedirs(os.path.join(temp_path, 'resources', 'l10n', lang, 'LC_MESSAGES'))
    shutil.copy( os.path.join('..', 'resources', 'l10n', lang, 'LC_MESSAGES', 'loot.mo'), os.path.join(temp_path, 'resources', 'l10n', lang, 'LC_MESSAGES') )

shutil.copytree( os.path.join('..', 'resources', 'report'), os.path.join(temp_path, 'resources', 'report') )
shutil.copy( os.path.join('..', 'resources', 'icon.ico'), os.path.join(temp_path, 'resources') )

shutil.copytree( os.path.join('..', 'docs', 'images'), os.path.join(temp_path, 'docs', 'images') )
shutil.copytree( os.path.join('..', 'docs', 'licenses'), os.path.join(temp_path, 'docs', 'licenses') )
shutil.copy( os.path.join('..', 'docs', 'LOOT Metadata Syntax.html'), os.path.join(temp_path, 'docs') )
shutil.copy( os.path.join('..', 'docs', 'LOOT Readme.html'), os.path.join(temp_path, 'docs') )

# Now compress the temporary folder. (Creating a zip because I can't get pylzma to work...)
os.chdir(temp_path)

sevenzip_path = os.path.join('C:\\', 'Program Files', '7-Zip', '7z.exe')
if os.path.exists(sevenzip_path):
    out_path = os.path.join('..', archive_name + '.7z')
    args = [sevenzip_path, 'a', '-r', out_path]
    subprocess.call(args)
else:
    zip = zipfile.ZipFile( os.path.join('..', archive_name + '.zip'), 'w', zipfile.ZIP_DEFLATED )
    for root, dirs, files in os.walk('.'):
        for file in files:
            zip.write(os.path.join(root, file))
    zip.close()
os.chdir('..')

# And finally, delete the temporary folder.
shutil.rmtree('archive.tmp')
