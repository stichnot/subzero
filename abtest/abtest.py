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

if __name__ == '__main__':
    argparser = argparse.ArgumentParser()
    argparser.add_argument('--test', required=True,
                           help='C/C++/.ll file with test functions')
    argparser.add_argument('--driver', required=True,
                           help='Driver program')
    argparser.add_argument('--prefix', required=True,
                           help='String prepended to Subzero symbol names')
    argparser.add_argument('--output', '-o', required=True,
                           help='Executable to produce')
    args = argparser.parse_args()

    (base, ext) = os.path.splitext(args.test)
    if ext == '.ll':
        bitcode = args.test
    else:
        bitcode = base + '.pnacl.ll'
        shellcmd(' '.join(['../ir_samples/build-pnacl-ir.py', args.test]))
        shellcmd('sed -i "s/^define internal /define /" ' + bitcode)

    asm_sz = base + '.sz.s'
    obj_sz = base + '.sz.o'
    obj_llc = base + '.llc.o'
    shellcmd(' '.join(['../llvm2ice',
                       '--prefix=' + args.prefix,
                       '-o',
                       asm_sz,
                       bitcode]))
    shellcmd(' '.join(['$LLVM_BIN_PATH/llvm-mc',
                       '-arch=x86',
                       '-x86-asm-syntax=intel',
                       '-filetype=obj',
                       '-o=' + obj_sz,
                       asm_sz]))
    shellcmd(' '.join(['$LLVM_BIN_PATH/llc',
                       '-march=x86',
                       '-filetype=obj',
                       '-o',
                       obj_llc,
                       bitcode]))
    shellcmd(' '.join(['$LLVM_BIN_PATH/clang',
                       '-g',
                       '-m32',
                       args.driver,
                       obj_llc,
                       obj_sz,
                       '-lm',
                       '-o',
                       args.output]))
