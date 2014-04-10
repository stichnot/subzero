#!/usr/bin/env python2

import argparse
import os, sys
import subprocess
import tempfile

sys.path.insert(0, '../ir_samples')
from utils import shellcmd

if __name__ == '__main__':
    argparser = argparse.ArgumentParser()
    argparser.add_argument('--test', required=True, action='append',
                           help='List of C/C++/.ll files with test functions')
    argparser.add_argument('--driver', required=True,
                           help='Driver program')
    argparser.add_argument('--target', required=False, default='x8632',
                           help='Translation target string')
    argparser.add_argument('-O', required=False, default='2', dest='optlevel',
                           choices=['m1', '-1', '0', '1', '2'],
                           help='Optimization level');
    argparser.add_argument('--prefix', required=True,
                           help='String prepended to Subzero symbol names')
    argparser.add_argument('--output', '-o', required=True,
                           help='Executable to produce')
    argparser.add_argument('--dir', required=False, default='.',
                           help='Output directory for all files')
    args = argparser.parse_args()

    objs = []
    for arg in args.test:
        base, ext = os.path.splitext(arg)
        if ext == '.ll':
            bitcode = arg
        else:
            bitcode = os.path.join(args.dir, base + '.pnacl.ll')
            shellcmd(['../ir_samples/build-pnacl-ir.py', '--dir', args.dir, arg])
            shellcmd('sed -i "s/^define internal /define /" ' + bitcode)
            shellcmd('sed -i "s/le32-unknown-nacl/i686-pc-linux-gnu/" ' + bitcode)

        asm_sz = os.path.join(args.dir, base + '.sz.s')
        obj_sz = os.path.join(args.dir, base + '.sz.o')
        obj_llc = os.path.join(args.dir, base + '.llc.o')
        shellcmd(['../llvm2ice',
                  '-O' + args.optlevel,
                  '--target=' + args.target,
                  '--prefix=' + args.prefix,
                  '-o=' + asm_sz,
                  bitcode])
        shellcmd(['$LLVM_BIN_PATH/llvm-mc',
                  '-arch=x86',
                  '-x86-asm-syntax=intel',
                  '-filetype=obj',
                  '-o=' + obj_sz,
                  asm_sz])
        shellcmd(['$LLVM_BIN_PATH/llc',
                  '-filetype=obj',
                  '-o=' + obj_llc,
                  bitcode])
        objs.append(obj_sz)
        objs.append(bitcode)
        #objs.append(obj_llc)

    shellcmd(['$LLVM_BIN_PATH/clang', '-g', '-m32', args.driver] +
             objs +
             ['-lm', '-o', os.path.join(args.dir, args.output)])
