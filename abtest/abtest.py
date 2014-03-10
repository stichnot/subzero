#!/usr/bin/env python2

import argparse
import os, sys
import subprocess
import tempfile

sys.path.insert(0, '../ir_samples')
from utils import shellcmd

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

    base, ext = os.path.splitext(args.test)
    if ext == '.ll':
        bitcode = args.test
    else:
        bitcode = base + '.pnacl.ll'
        shellcmd(['../ir_samples/build-pnacl-ir.py', args.test])
        shellcmd('sed -i "s/^define internal /define /" ' + bitcode)
        # Leaving the 'target' lines sometimes causes llc assertion failures.
        shellcmd('sed -i "s/^target /;target /" ' + bitcode)

    asm_sz = base + '.sz.s'
    obj_sz = base + '.sz.o'
    obj_llc = base + '.llc.o'
    shellcmd(['../llvm2ice',
              '--prefix=' + args.prefix,
              '-o',
              asm_sz,
              bitcode])
    shellcmd(['$LLVM_BIN_PATH/llvm-mc',
             '-arch=x86',
             '-x86-asm-syntax=intel',
             '-filetype=obj',
             '-o=' + obj_sz,
             asm_sz])
    shellcmd(['$LLVM_BIN_PATH/llc',
             '-march=x86',
             '-filetype=obj',
             '-o',
             obj_llc,
             bitcode])
    shellcmd(['$LLVM_BIN_PATH/clang',
             '-g',
             '-m32',
             args.driver,
             bitcode,#obj_llc,
             obj_sz,
             '-lm',
             '-o',
             args.output])
