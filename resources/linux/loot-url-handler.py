#!/usr/bin/env python
#
# Supports URLs of the form loot://launch?game=<game>&game-path=<game-path>&loot-data-path=<loot-data-path>&auto-sort=1 where all query parameters are optional. The value of the auto-sort parameter is not significant, only the presence of the parameter matters.

from urllib.parse import urlsplit, parse_qs
import subprocess
import sys

SUPPORTED_QUERY_PARAMS = ['game', 'game-path', 'loot-data-path']

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(f'Error: expected 2 argv values, got {len(sys.argv)}', file=sys.stderr)
        exit(1)

    if not sys.argv[1].startswith('loot://launch'):
        print(f'Error: unexpected value, expected a loot://launch URL, got {sys.argv[1]}', file=sys.stderr)
        exit(1)

    url_parts = urlsplit(sys.argv[1])

    if url_parts.scheme != 'loot':
        print(f'Error: unexpected URL scheme {url_parts.scheme}')
        exit(1)

    if url_parts.netloc != 'launch':
        print(f'Error: unexpected authority {url_parts.netloc}')
        exit(1)

    query_params = parse_qs(url_parts.query)

    loot_args = ['LOOT']

    for key, value in query_params.items():
        if key in ['game', 'game-path', 'loot-data-path']:
            loot_args.append(f'--{key}')
            if len(value) != 1:
                print(f'Warning: more than one value found for query parameter {key}, ignoring all except the first value', file=sys.stderr)
            loot_args.append(value[0])
        elif key == 'auto-sort':
            loot_args.append(f'--{key}')
        else:
            print(f'Warning: unrecognised query parameter {key}={value}', file=sys.stderr)

    subprocess.run(loot_args, check=True)
