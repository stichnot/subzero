#!/usr/bin/env python2

# Runs llvm2ice on an input .ll file, and compares the output against
# the input.
#
# Before comparing, the input file is massaged to remove comments,
# blank lines, global variable definitions, external function
# declarations, and possibly other patterns that llvm2ice does not
# handle.
#
# The output file and the massaged input file are compared line by
# line for differences.  However, there is a regex defined such that
# if the regex matches a line in the input file, that line and the
# corresponding line in the output file are ignored.  This lets us
# ignore minor differences such as inttoptr and ptrtoint, and printing
# of floating-point constants.
#
# On success, no output is produced.  On failure, each mismatch is
# printed as two lines, one starting with "SZ" and one starting with
# "LL".

import argparse
import subprocess
import re

if __name__ == '__main__':
    argparser = argparse.ArgumentParser()
    argparser.add_argument('llfile')
    argparser.add_argument('--llvm2ice')
    args = argparser.parse_args()

    infile = args.llfile
    llvm2ice = args.llvm2ice if args.llvm2ice else './llvm2ice'
    command = [llvm2ice, '-verbose', 'inst', '-notranslate', infile]
    p = subprocess.Popen(command, stdout=subprocess.PIPE)
    sz_out, stderr = p.communicate()
    sed_command = ['sed', '-e', 's/;.*//', '-e', 's/ *$//', infile]
    grep1_command = ['grep', '-v', '^ *$']
    grep2_command = ['grep', '-v', '^declare']
    grep3_command = ['grep', '-v', '^@']
    sed = subprocess.Popen(sed_command, stdout=subprocess.PIPE)
    grep1 = subprocess.Popen(grep1_command, stdin=sed.stdout,
                             stdout=subprocess.PIPE)
    grep2 = subprocess.Popen(grep2_command, stdin=grep1.stdout,
                             stdout=subprocess.PIPE)
    grep3 = subprocess.Popen(grep3_command, stdin=grep2.stdout,
                             stdout=subprocess.PIPE)
    llc_out = grep3.communicate()[0]
    match = re.compile('|'.join([' -[0-9]+',     # negative constants
                                 'float [-0-9]', # FP constants
                                 'inttoptr',     # inttoptr pointer types
                                 'ptrtoint'      # ptrtoint pointer types
                                 ]))
    return_code = 0
    for (sz_line, llc_line) in zip(sz_out.splitlines(), llc_out.splitlines()):
        if not match.search(llc_line) and sz_line != llc_line:
            print 'SZ>' + sz_line
            print 'LL>' + llc_line
            return_code = 1
    if return_code == 0: print 'Success'
    exit(return_code)
