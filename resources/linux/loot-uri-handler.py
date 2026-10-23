#!/usr/bin/env python
#
# Supports URIs of the form loot://launch?game=<game>&game-path=<game-path>&loot-data-path=<loot-data-path>&auto-sort=1 where all query parameters are optional. The value of the auto-sort parameter is not significant, only the presence of the parameter matters.

from urllib.parse import urlsplit, parse_qs
import subprocess
import sys

class ValidationError(ValueError):
    pass

def validate_uri_parts(uri_parts):
    if uri_parts.scheme != 'loot':
        raise ValidationError(f'unexpected URI scheme "{uri_parts.scheme}"')

    if uri_parts.netloc != 'launch':
        raise ValidationError(f'unexpected authority "{uri_parts.netloc}"')

    if uri_parts.path != '':
        raise ValidationError(f'unexpected path "{uri_parts.path}"')

    if uri_parts.fragment != '':
        raise ValidationError(f'unexpected fragment "{uri_parts.fragment}"')

def parse_query_params(query):
    SUPPORTED_QUERY_PARAMS = ['game', 'game-path', 'loot-data-path', 'auto-sort']

    query_params = parse_qs(query,
        strict_parsing=True,
        max_num_fields=len(SUPPORTED_QUERY_PARAMS),
        errors='strict')

    for key, value in query_params.items():
        if key not in SUPPORTED_QUERY_PARAMS:
            raise ValidationError(f'unrecognised query parameter {key}={value}')

        if len(value) != 1:
            raise ValidationError(f'more than one value found for query parameter {key}')

    return query_params

if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise ValidationError(f'expected 2 argv values, got {len(sys.argv)}')

    uri_parts = urlsplit(sys.argv[1])
    validate_uri_parts(uri_parts)

    query_params = parse_query_params(uri_parts.query)

    loot_args = ['LOOT']

    for key, value in query_params.items():
        if key in ['game', 'game-path', 'loot-data-path']:
            loot_args.append(f'--{key}')
            loot_args.append(value[0])
        elif key == 'auto-sort':
            loot_args.append(f'--{key}')

    subprocess.run(loot_args, check=True)
