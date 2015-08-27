#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Creates an archive of a LOOT release, putting it in the 'build' folder.

#   LOOT
#
#   A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
#   Fallout: New Vegas.
#
#   Copyright (C) 2013-2015    WrinklyNinja
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
#
# The current path throughout the script is the `src` folder, where this script
# is located.

# Find an executable's path from its name. Like Python 3.3's shutil.which, but
# also checks likely paths for hardcoded programs.
def which(cmd):
    if 'which' in dir(shutil):
        exe_path = shutil.which(cmd)
    else:
        # Check the PATH environmental variable manually.
        exe_path = None
        path = os.getenv('PATH')
        for p in path.split(os.path.pathsep):
            p = os.path.join(p, cmd)
            if os.path.exists(p) and os.access(p, os.X_OK):
                exe_path = p

    if not exe_path:
        if cmd == '7z.exe':
            sevenzip_path = os.path.join('C:\\', 'Program Files', '7-Zip', '7z.exe')
            if os.path.exists(sevenzip_path):
                exe_path = sevenzip_path
        elif cmd == 'vulcanize.cmd':
            npm = os.path.join( os.getenv('APPDATA'), 'npm' )
            if (os.path.exists(npm, 'vulcanize.cmd')):
                exe_path = os.path.join(npm, 'vulcanize.cmd')

    return exe_path

def getNameSuffix():
    # Check for a Git install, and get the revision info if found.
    git = which('git.exe')
    if git:
        args = [git, 'describe', '--tags', '--long']
        output = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
        if len(output[1]) == 0:
            return output[0].decode('ascii')[:-1]

    return ''

def buildUIFiles():
    # Vulcanize the UI's files.
    subprocess.call(['node', './vulcanize.js', '..']);

def createArchive(folder_path, archive_path):
    sevenzip_path = os.path.join('C:\\', 'Program Files', '7-Zip', '7z.exe')
    if os.path.exists(sevenzip_path):
        args = [sevenzip_path, 'a', '-r', archive_path + '.7z', os.path.join(folder_path, '*')]
        subprocess.call(args)
    else:
        zip = zipfile.ZipFile( archive_path + '.zip', 'w', zipfile.ZIP_DEFLATED )
        for root, dirs, files in os.walk(folder_path):
            for file in files:
                zip.write(os.path.join(root, file))
        zip.close()

def createAppArchive(archive_path):
    temp_path = os.path.join('..', 'build', 'archive.tmp')

    # Delete the temporary folder if it already exists, then create it.
    if os.path.exists(temp_path):
        shutil.rmtree(temp_path)
    os.makedirs(temp_path)

    # Now copy everything into the temporary folder.
    # LOOT executable and CEF files.
    shutil.copy( os.path.join('..', 'build', 'Release', 'LOOT.exe'), temp_path )
    shutil.copy( os.path.join('..', 'build', 'Release', 'd3dcompiler_47.dll'), temp_path )
    shutil.copy( os.path.join('..', 'build', 'Release', 'libEGL.dll'), temp_path )
    shutil.copy( os.path.join('..', 'build', 'Release', 'libGLESv2.dll'), temp_path )
    shutil.copy( os.path.join('..', 'build', 'Release', 'libcef.dll'), temp_path )
    shutil.copy( os.path.join('..', 'build', 'Release', 'wow_helper.exe'), temp_path )
    shutil.copy( os.path.join('..', 'build', 'Release', 'natives_blob.bin'), temp_path )
    shutil.copy( os.path.join('..', 'build', 'Release', 'snapshot_blob.bin'), temp_path )
    shutil.copy( os.path.join('..', 'build', 'Release', 'cef.pak'), temp_path )
    shutil.copy( os.path.join('..', 'build', 'Release', 'cef_100_percent.pak'), temp_path )
    shutil.copy( os.path.join('..', 'build', 'Release', 'cef_200_percent.pak'), temp_path )
    shutil.copy( os.path.join('..', 'build', 'Release', 'devtools_resources.pak'), temp_path )
    shutil.copy( os.path.join('..', 'build', 'Release', 'icudtl.dat'), temp_path )

    # Translation files.
    os.makedirs(os.path.join(temp_path, 'resources', 'l10n'))
    shutil.copy( os.path.join('..', 'build', 'Release', 'resources', 'l10n', 'en-US.pak'), os.path.join(temp_path, 'resources', 'l10n') )
    for lang in ['es', 'ru', 'fr', 'zh_CN', 'pl', 'pt_BR', 'fi', 'de', 'da', 'ko']:
        os.makedirs(os.path.join(temp_path, 'resources', 'l10n', lang, 'LC_MESSAGES'))
        shutil.copy( os.path.join('..', 'resources', 'l10n', lang, 'LC_MESSAGES', 'loot.mo'), os.path.join(temp_path, 'resources', 'l10n', lang, 'LC_MESSAGES') )

    # UI files.
    os.makedirs( os.path.join(temp_path, 'resources', 'ui', 'css') )
    shutil.copy( os.path.join('..', 'build', 'Release', 'resources', 'ui', 'index.html'), os.path.join(temp_path, 'resources', 'ui') )
    shutil.copy( os.path.join('..', 'resources', 'ui', 'css', 'dark-theme.css'), os.path.join(temp_path, 'resources', 'ui', 'css') )
    shutil.copytree( os.path.join('..', 'resources', 'ui', 'fonts'), os.path.join(temp_path, 'resources', 'ui', 'fonts') )

    # Docs.
    shutil.copytree( os.path.join('..', 'docs', 'images'), os.path.join(temp_path, 'docs', 'images') )
    shutil.copytree( os.path.join('..', 'docs', 'licenses'), os.path.join(temp_path, 'docs', 'licenses') )
    shutil.copy( os.path.join('..', 'docs', 'LOOT Metadata Syntax.html'), os.path.join(temp_path, 'docs') )
    shutil.copy( os.path.join('..', 'docs', 'LOOT Readme.html'), os.path.join(temp_path, 'docs') )

    # Now compress the temporary folder.
    createArchive(temp_path, archive_path);

    # And finally, delete the temporary folder.
    shutil.rmtree(temp_path)

def createApiArchive(archive_path):
    temp_path = os.path.join('..', 'build', 'archive.tmp')

    # Delete the temporary folder if it already exists, then create it.
    if os.path.exists(temp_path):
        shutil.rmtree(temp_path)
    os.makedirs(temp_path)

    # Create directories in temporary folder.
    os.makedirs( os.path.join(temp_path, 'include', 'loot') )
    os.makedirs( os.path.join(temp_path, 'docs') )

    # Now copy everything into the temporary folder.
    shutil.copy( os.path.join('..', 'build', 'Release', 'loot32.dll'), temp_path )
    shutil.copy( os.path.join('..', 'src', 'api', 'api.h'), os.path.join(temp_path, 'include', 'loot') )
    shutil.copy( os.path.join('..', 'docs', 'latex', 'refman.pdf'), os.path.join(temp_path, 'docs', 'readme.pdf') )

    shutil.copytree( os.path.join('..', 'docs', 'licenses'), os.path.join(temp_path, 'docs', 'licenses') )

    # Now compress the temporary folder.
    createArchive(temp_path, archive_path);

    # And finally, delete the temporary folder.
    shutil.rmtree(temp_path)

# Create the archives.
archive_suffix = getNameSuffix()
buildUIFiles();
createAppArchive( os.path.join('..', 'build', 'LOOT ' + archive_suffix) )
createApiArchive( os.path.join('..', 'build', 'LOOT API ' + archive_suffix) )
