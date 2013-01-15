#! /usr/bin/env python3
#  BOSS
#
#  A plugin load order optimiser for games that use the esp/esm plugin system.
#
#  Copyright (C) 2012    WrinklyNinja
#
#  This file is part of BOSS.
#
#  BOSS is free software: you can redistribute
#  it and/or modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation, either version 3 of
#  the License, or (at your option) any later version.
#
#  BOSS is distributed in the hope that it will
#  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with BOSS.  If not, see
#  <http://www.gnu.org/licenses/>.

# This is a script that converts a MF 2.3 masterlist to a MF 3 masterlist. It's
# a bit hacky, but the YAML parser accepts it. Limitations are:
#   - Doesn't strip plugins that no longer belong in the masterlist, ie. without
#     messages or positioning comments.
#   - Requirements and incompatibilities do not use their new data structures,
#     and are just converted to 'say' messages.
#   - No extraneous comments/messages are removed.
#   - VAR conditions are not replaced with their conditional statements. SET lines
#     are stripped, so the old masterlist must be used to check what they are.
#   - Each message or list of tags comes under a different 'msg' or 'tag' object.
#   - Tag message text, aside from the tags themselves, is lost.
#   - Conditional plugin positions are all made unconditional, so there will be
#     duplicate entries.

class Plugin:


def escapeYAMLStr(s):
    special = ['-', '?', ':', ',', '[', ']', '{', '}', '&', '*', '!', '|', '>', '\'', '"', '%', '@', '`']
    if ('\'' in s):
        return '"' + s.replace('\\', '\\\\').replace('"', '\\"').strip() + '"'
    elif (1 in [c in s for c in special]):
        return '\'' + s.strip() + '\''
    else:
        return s;

def mContent(line):
    content = line[line.find(':')+1:]
    return escapeYAMLStr(content)

def convertCondition(condition):
    #IF no longer exists.
    #IFNOT is now 'not'.
    #The VAR condition no longer exists.
    #The '=' comparator is now '=='.
    #The LANG condition no longer exists.
    #'&&' is now 'and'.
    #'||' is now 'or'.

    #Really, these will probably have to be done manually.

    parts = condition.split('"')  # Every odd index is quoted and shouldn't be lowercased.

    for i in range(len(parts)):
        if (i % 2 == 0):
            parts[i] = parts[i].lower()

    condition = '"'.join(parts)

    condition.replace(' && ', ' and ').replace(' || ', ' or ').replace(' =)', ' ==)')

    return escapeYAMLStr(condition)

inFile = open('../../BOSS/data/boss-oblivion/masterlist.txt', 'r')
outFile = open('../build/masterlist.yaml', 'w')

outFile.write('---\n');

isInComment = False
lastLine = ''
lastLineType = 0
for line in inFile:

    line = line.replace('\n', '')

    if (len(line.strip()) == 0):
        continue

    # Skip group lines.
    if ("BEGINGROUP:" in line or "ENDGROUP:" in line):
        continue

    # Skip var lines.
    if ('SET:' in line):
        continue

    # Write comments.
    if (line[0:2] == '//'):
        outFile.write('#' + line[2:] + '\n')
        continue

    if ('/*' in line):
        isInComment = True
        outFile.write('#' + line + '\n')

    if ('*/' in line):
        isInComment = False
        outFile.write('#' + line + '\n')

    if ('*/' in line or '/*' in line):
        continue

    if (isInComment):
        outFile.write('#' + line + '\n')
        continue

    # Write plugin lines.
    if (':' not in line):
        outFile.write('  - name: ' + escapeYAMLStr(line) + '\n')
    else:
        pos = line.find(':')
        if ('MOD:' in line[:pos+1]):
            outFile.write('  - name: ' + escapeYAMLStr(line[pos+1:]) + '\n')
        elif ('REGEX:' in line[:pos+1]):
            outFile.write('  - name: ' + escapeYAMLStr(line[pos+1:]) + '\n')

    # Extract conditional from line.
    if ('IF' in line and ':' in line):
        key = line[:line.find(':')]
        key = key.replace('GLOBAL', '')
        condition = key[:key.rfind(' ')].strip()
    elif ('ELSE' in line and ':' in line):
        if (condition[0] != 'n'):
            condition = 'not ' + condition
        condition = condition.replace('not ', '`~||~`')
        condition = condition.replace('d f', 'd not f').replace('d c', 'd not c', ', 'and
    else:
        condition = ''
    condition = convertCondition(condition);

    # Write tag lines.
    if ('TAG:' in line):
        if ('{{BASH' in line):
            tags = line[line.find('{{BASH:') + 7:line.find('}}') ]
            tags = tags.split(',')
            outFile.write('    tag:\n')
            for tag in tags:
                if (condition):
                    outFile.write('      - condition: ' + condition + '\n')
                    outFile.write('        name: ' + tag.strip(' ') + '\n')
                else:
                    outFile.write('      - ' + tag.strip(' ') + '\n')
        if ('[' in line):
            tags = line[line.find('[')+1:line.find(']')]
            tags = tags.split(',')
            outFile.write('    tag:\n')
            for tag in tags:
                if (condition):
                    outFile.write('      - condition: ' + condition + '\n')
                    outFile.write('        name: -' + tag.strip(' ') + '\n')
                else:
                    outFile.write('      - -' + tag.strip(' ') + '\n')

    # Write 'say' messages.
    if ('SAY:' in line or 'INC:' in line or 'REQ:' in line):
        if ('GLOBAL ' not in line):
            outFile.write('    msg:\n')
            indent = '      '
        else:
            indent = '  '
        prefix = ''
        if ('INC:' in line):
            line = 'Incompatible with ' + line.split(':')[1]
        elif ('REQ:' in line):
            line = 'Requires ' + line.split(':')[1]
        else:
            line = line.split(':')[1]
        outFile.write(indent + '- type: say\n')
        if (condition):
            outFile.write(indent + '  condition: ' + condition + '\n')
        outFile.write(indent + '  content: ' + escapeYAMLStr(line) + '\n')

    # Write 'warn' messages.
    if ('DIRTY:' in line or 'WARN:' in line):
        if ('GLOBAL ' not in line):
            outFile.write('    msg:\n')
            indent = '      '
        else:
            indent = '  '
        outFile.write(indent + '- type: warn\n')
        if (condition):
            outFile.write(indent + '  condition: ' + condition + '\n')
        outFile.write(indent + '  content: ' + mContent(line) + '\n')

    # Write 'error' messages.
    if ('ERROR:' in line):
        if ('GLOBAL ' not in line):
            outFile.write('    msg:\n')
            indent = '      '
        else:
            indent = '  '
        outFile.write(indent + '- type: error\n')
        if (condition):
            outFile.write(indent + '  condition: ' + condition + '\n')
        outFile.write(indent + '  content: ' + mContent(line) + '\n')


inFile.close();
outFile.close();
