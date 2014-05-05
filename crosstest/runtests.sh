#!/bin/sh

# TODO: Retire this script and move the individual tests into the lit
# framework, to leverage parallel testing and other lit goodness.

set -eux

OPTLEVELS="m1 2"
OUTDIR=Output
# Clean the output directory to avoid reusing stale results.
rm -rf "${OUTDIR}"
mkdir -p "${OUTDIR}"

for optlevel in ${OPTLEVELS} ; do

    ./crosstest.py -O${optlevel} --prefix=Subzero_ --target=x8632 \
        --dir="${OUTDIR}" \
        --llvm-bin-path="${LLVM_BIN_PATH}" \
        --test=simple_loop.c \
        --driver=simple_loop_main.c \
        --output=simple_loop_O${optlevel}

    ./crosstest.py -O${optlevel} --prefix=Subzero_ --target=x8632 \
        --dir="${OUTDIR}" \
        --llvm-bin-path="${LLVM_BIN_PATH}" \
        --test=test_cast.cpp --test=test_cast_to_u1.ll \
        --driver=test_cast_main.cpp \
        --output=test_cast_O${optlevel}

    ./crosstest.py -O${optlevel} --prefix=Subzero_ --target=x8632 \
        --dir="${OUTDIR}" \
        --llvm-bin-path="${LLVM_BIN_PATH}" \
        --test=test_fcmp.pnacl.ll \
        --driver=test_fcmp_main.cpp \
        --output=test_fcmp_O${optlevel}

    ./crosstest.py -O${optlevel} --prefix=Subzero_ --target=x8632 \
        --dir="${OUTDIR}" \
        --llvm-bin-path="${LLVM_BIN_PATH}" \
        --test=test_icmp.cpp \
        --driver=test_icmp_main.cpp \
        --output=test_icmp_O${optlevel}

    ./crosstest.py -O${optlevel} --prefix=Subzero_ --target=x8632 \
        --dir="${OUTDIR}" \
        --llvm-bin-path="${LLVM_BIN_PATH}" \
        --test=test_arith.cpp --test=test_arith_frem.ll \
        --driver=test_arith_main.cpp \
        --output=test_arith_O${optlevel}

done

for optlevel in ${OPTLEVELS} ; do
    "${OUTDIR}"/simple_loop_O${optlevel}
    "${OUTDIR}"/test_cast_O${optlevel}
    "${OUTDIR}"/test_fcmp_O${optlevel}
    "${OUTDIR}"/test_icmp_O${optlevel}
    "${OUTDIR}"/test_arith_O${optlevel}
done
