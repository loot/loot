#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import polib
import re

MENU_ENTRY_REGEX = re.compile("translators: This string is an action in the ([^ ]+) menu.")
MENU_TOOLBAR_REGEX = re.compile('translators: This string is also an action in the ([^ ]+) menu.')
MENU_COMMENT = "translators: The mnemonic in this string shouldn't conflict with other menus or sidebar sections."


def extract_mnemonic(msgstr):
    index = msgstr.find('&')

    return msgstr[index + 1]

def validate_bucket(entries):
    mnemonics = {}

    for entry in entries:
        mnemonic = extract_mnemonic(entry.msgstr)
        if mnemonic not in mnemonics:
            mnemonics[mnemonic] = [entry]
        else:
            mnemonics[mnemonic].append(entry)

    for [mnemonic, entries] in mnemonics.items():
        if len(entries) > 1:
            strings = [ e.msgstr for e in entries ]
            return [mnemonic, strings]

    return None

def handle_error(error, context):
    [mnemonic, strings] = error
    print(f'ERROR: More than one translation in {po_path} uses the same mnemonic ({mnemonic}) in the {context} context: {strings}')

def validate_po_file(po_path):
    po_file = polib.pofile(po_path)

    entry_buckets = { 'menu': [], 'toolbar': [], 'menus': {} }

    for entry in po_file:
        if not entry.msgstr or not entry.comment:
            # Ignore untranslated strings and strings without a comment
            continue

        entry.comment = entry.comment.replace('\n', ' ')

        if entry.comment == MENU_COMMENT:
            entry_buckets['menu'].append(entry)
        else:
            match = MENU_TOOLBAR_REGEX.match(entry.comment)

            if match:
                entry_buckets['toolbar'].append(entry)
            else:
                match = MENU_ENTRY_REGEX.match(entry.comment)

            if match:
                menu = match.group(1)
                if menu not in entry_buckets['menus']:
                    entry_buckets['menus'][menu] = [entry]
                else:
                    entry_buckets['menus'][menu].append(entry)

    validation_failed = False

    error = validate_bucket(entry_buckets['menu'])
    if error:
        validation_failed = True
        handle_error(error, 'menu names')

    for [menu, entries] in entry_buckets['menus'].items():
        error = validate_bucket(entries)
        if error:
            validation_failed = True
            handle_error(error, menu)

    return not validation_failed

if __name__ == "__main__":
    root_path = os.path.join('.', 'resources', 'l10n')

    validation_failed = False

    locale_directories = [ f.path for f in os.scandir(root_path) if f.is_dir() ]
    for locale_directory in list(locale_directories):
        po_path = os.path.join(locale_directory, 'LC_MESSAGES', 'loot.po')

        if os.path.exists(po_path):
            print(f'Validating PO file for {os.path.basename(locale_directory)}...')
            success = validate_po_file(po_path)

            if not success:
                validation_failed = True

    if validation_failed:
        exit(1)
