#!/usr/bin/env python2

import argparse
import os, sys
from subprocess import Popen, PIPE
import tempfile

def shellcmd(command, echo=True):
    if echo: print '[cmd]', command
    sb = Popen(command, stdout=PIPE, shell=True)
    stdout_result = sb.communicate()[0]
    if echo: sys.stdout.write(stdout_result)
    return stdout_result

def find_llvm_bin():
    if os.path.exists(os.path.expandvars('$LLVMSVN/clang')):
        return os.path.expandvars('$LLVMSVN')
    elif os.path.exists(os.path.expandvars('$LLVM_BIN_PATH/clang')):
        return os.path.expandvars('$LLVM_BIN_PATH')
    else:
        return ''

if __name__ == '__main__':
    argparser = argparse.ArgumentParser()
    argparser.add_argument('cfile', nargs='+', type=str)
    argparser.add_argument('--llvmbin', nargs='?', type=str)
    args = argparser.parse_args()

    if args.llvmbin:
        llvm_bin_path = args.llvmbin
    else:
        llvm_bin_path = find_llvm_bin()
    print('LLVM binary path set to: "%s"' % llvm_bin_path)

    def toolpath(toolname):
        return os.path.join(llvm_bin_path, toolname)

    tempdir = tempfile.mkdtemp()

    for cname in args.cfile:
        basename = os.path.splitext(cname)[0]
        llname = os.path.join(tempdir, basename + '.ll')
        optllname = basename + '-opt.ll'

        shellcmd(toolpath('clang') + ' -cc1 -O3 -emit-llvm {0} -o {1}'.format(
            cname, llname))
        shellcmd(toolpath('opt') + ' -O3 -S {0} > {1}'.format(
            llname, optllname))


