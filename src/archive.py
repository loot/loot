#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Creates an archive of a BOSS release, putting it in the 'build' folder.

# Files and folders that need to go in (relative to repository root):
#
# build/BOSS.exe
# resources/graphvis
# resources/svgweb
# resources/svn
# resources/icon.ico
# resources/polyfill.js
# resources/script.js
# resources/style.css
# docs/images
# docs/licenses
# docs/BOSS Metadata Syntax.html
# docs/BOSS Readme.html

import sys
import os
import shutil
import zipfile

temp_path = os.path.join('..', 'build', 'archive.tmp')
archive_name = 'archive.zip'

# Set archive name if alternative is given.
if (len(sys.argv) > 1):
    archive_name = sys.argv[1]

# First make sure that the temporary folder for the archive exists.
if not os.path.exists(temp_path):
    os.makedirs(temp_path)


# Now copy everything into the temporary folder.
shutil.copy( os.path.join('..', 'build', 'BOSS.exe'), temp_path )

shutil.copytree( os.path.join('..', 'resources', 'graphvis'), os.path.join(temp_path, 'resources', 'graphvis') )
shutil.copytree( os.path.join('..', 'resources', 'svgweb'), os.path.join(temp_path, 'resources', 'svgweb') )
shutil.copytree( os.path.join('..', 'resources', 'svn'), os.path.join(temp_path, 'resources', 'svn') )
shutil.copy( os.path.join('..', 'resources', 'icon.ico'), os.path.join(temp_path, 'resources') )
shutil.copy( os.path.join('..', 'resources', 'polyfill.js'), os.path.join(temp_path, 'resources') )
shutil.copy( os.path.join('..', 'resources', 'script.js'), os.path.join(temp_path, 'resources') )
shutil.copy( os.path.join('..', 'resources', 'style.css'), os.path.join(temp_path, 'resources') )

shutil.copytree( os.path.join('..', 'docs', 'images'), os.path.join(temp_path, 'docs', 'images') )
shutil.copytree( os.path.join('..', 'docs', 'licenses'), os.path.join(temp_path, 'docs', 'licenses') )
shutil.copy( os.path.join('..', 'docs', 'BOSS Metadata Syntax.html'), os.path.join(temp_path, 'docs') )
shutil.copy( os.path.join('..', 'docs', 'BOSS Readme.html'), os.path.join(temp_path, 'docs') )


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
