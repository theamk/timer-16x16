#!/usr/bin/python3 -B

import sys
import os
import argparse
import subprocess


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('input')

    args = parser.parse_args();
    pbm = subprocess.check_output(
        ['convert', args.input] + '-flatten -compress none pbm:-'.split()
    ).decode('latin1')
    assert pbm.startswith('P1\n'), f'Invalid format: {pbm[:30]!a}'
    _, sizeline, rest = pbm.split("\n", 2)
    assert sizeline == "16 16", f'Invalid image size: {sizeline!a}'
    pixels = rest.split()
    assert len(pixels) == 256
    

if __name__ == '__main__':
    main()

