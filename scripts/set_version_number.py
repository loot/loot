#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import os
import re

def replace_in_file(path, regex, replacement):
    regex = re.compile(regex)

    lines = []
    with open(path, encoding='utf8') as infile:
        for line in infile:
            lines.append(re.sub(regex, replacement, line))

    with open(path, 'w', encoding='utf8') as outfile:
        for line in lines:
            outfile.write(line)

def update_pre_commit_hook_script(path, version):
    replace_in_file(path, '--package-version=[\\d.]+', '--package-version={}'.format(version))

def update_installer_script(path, version):
    replace_in_file(path, '#define MyAppVersion "[\\d.]+"', '#define MyAppVersion "{}"'.format(version))

def update_cpp_file(path, version):
    version_parts = version.split('.')

    replace_in_file(path, 'LOOT_VERSION_MAJOR = \\d+;', 'LOOT_VERSION_MAJOR = {};'.format(version_parts[0]))
    replace_in_file(path, 'LOOT_VERSION_MINOR = \\d+;', 'LOOT_VERSION_MINOR = {};'.format(version_parts[1]))
    replace_in_file(path, 'LOOT_VERSION_PATCH = \\d+;', 'LOOT_VERSION_PATCH = {};'.format(version_parts[2]))

def update_resource_file(path, version):
    comma_separated_version = version.replace('.', ', ')

    replace_in_file(path, 'VERSION \\d+, \\d+, \\d+', 'VERSION {}'.format(comma_separated_version))
    replace_in_file(path, 'Version", "\\d+\\.\\d+\\.\\d+"', 'Version", "{}"'.format(version))

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = 'Set the LOOT version number')
    parser.add_argument('version', nargs='+')

    arguments = parser.parse_args()

    if len(arguments.version) != 1:
        raise RuntimeError('Invalid number of arguments given. Only one argument (the new version number) is expected.')

    if len(arguments.version[0].split('.')) != 3:
        raise RuntimeError('The version number must be a three-part semantic version.')

    version = arguments.version[0]

    update_pre_commit_hook_script(
        os.path.join('scripts', 'git', 'hooks', 'pre-commit'),
        version
    )
    update_installer_script(
        os.path.join('scripts', 'installer.iss'),
        version
    )
    update_cpp_file(
        os.path.join('src', 'gui', 'version.h'),
        version
    )
    update_resource_file(
        os.path.join('src', 'gui', 'resource.rc'),
        version
    )
