#!/usr/bin/env python2

import argparse
import itertools
import subprocess
import re

if __name__ == '__main__':
    """Runs llvm2ice on an input .ll file, and compares the output
    against the input.

    Before comparing, the input file is massaged to remove comments,
    blank lines, global variable definitions, external function
    declarations, and possibly other patterns that llvm2ice does not
    handle.

    The output file and the massaged input file are compared line by
    line for differences.  However, there is a regex defined such that
    if the regex matches a line in the input file, that line and the
    corresponding line in the output file are ignored.  This lets us
    ignore minor differences such as inttoptr and ptrtoint, and
    printing of floating-point constants.

    On success, no output is produced.  On failure, each mismatch is
    printed as two lines, one starting with 'SZ' and one starting with
    'LL'.
    """
    desc = 'Compare llvm2ice output against bitcode input.'
    argparser = argparse.ArgumentParser(description=desc)
    argparser.add_argument(
        'llfile', nargs='?', default='-',
        type=argparse.FileType('r'), metavar='FILE',
        help='Textual bitcode file [default stdin]')
    argparser.add_argument(
        '--llvm2ice', required=False, default='./llvm2ice', metavar='LLVM2ICE',
        help='Path to llvm2ice driver program [default ./llvm2ice]')
    args = argparser.parse_args()
    bitcode = args.llfile.readlines()

    # Run llvm2ice and collect its output lines into sz_out.
    command = [args.llvm2ice, '-verbose', 'inst', '-notranslate', '-']
    p = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    sz_out = p.communicate(input=''.join(bitcode))[0].splitlines()

    # Filter certain lines and patterns from the input, and collect
    # the remainder into llc_out.
    llc_out = []
    trailing_comment = re.compile(';.*')
    ignore_pattern = re.compile('^ *$|^declare|^@')
    for line in bitcode:
        # Remove trailing comments and spaces.
        line = trailing_comment.sub('', line).rstrip()
        # Ignore blanks lines, forward declarations, and variable definitions.
        if not ignore_pattern.search(line):
            llc_out.append(line)

    # Compare sz_out and llc_out line by line, but ignore pairs of
    # lines where the llc line matches a certain pattern.
    return_code = 0
    lines_total = 0
    lines_diff = 0
    ignore_pattern = re.compile(
        '|'.join([' -[0-9]',                # negative constants
                  ' (float|double) [-0-9]', # FP constants
                  ' inttoptr ',             # inttoptr pointer types
                  ' ptrtoint '              # ptrtoint pointer types
                  ]))
    for (sz_line, llc_line) in itertools.izip_longest(sz_out, llc_out):
        lines_total += 1
        if sz_line == llc_line:
            continue
        if llc_line and ignore_pattern.search(llc_line):
            lines_diff += 1
            continue
        if sz_line: print 'SZ>' + sz_line
        if llc_line: print 'LL>' + llc_line
        return_code = 1

    if return_code == 0:
        message = 'Success (ignored %d diffs out of %d lines)'
        print message % (lines_diff, lines_total)
    exit(return_code)
