#!/bin/sh

set -x

./abtest.py --target=x8632 --test=simple_loop.c --driver=simple_loop_main.c --prefix=Subzero_ --output=simple_loop || exit 1
./abtest.py --target=x8632 --test=test_cast.cpp --test=test_cast_to_u1.ll --driver=test_cast_main.cpp --prefix=Subzero_ --output=test_cast || exit 1
./abtest.py --target=x8632 --test=test_fcmp.pnacl.ll --driver=test_fcmp_main.cpp --prefix=Subzero_ --output=test_fcmp || exit 1
./abtest.py --target=x8632 --test=test_icmp.cpp --driver=test_icmp_main.cpp --prefix=Subzero_ --output=test_icmp || exit 1

./abtest.py --target=x8632fast --test=simple_loop.c --driver=simple_loop_main.c --prefix=Subzero_ --output=simple_loop_fast || exit 1
./abtest.py --target=x8632fast --test=test_cast.cpp --test=test_cast_to_u1.ll --driver=test_cast_main.cpp --prefix=Subzero_ --output=test_cast_fast || exit 1
./abtest.py --target=x8632fast --test=test_fcmp.pnacl.ll --driver=test_fcmp_main.cpp --prefix=Subzero_ --output=test_fcmp_fast || exit 1
./abtest.py --target=x8632fast --test=test_icmp.cpp --driver=test_icmp_main.cpp --prefix=Subzero_ --output=test_icmp_fast || exit 1

./simple_loop
./test_cast
./test_fcmp
./test_icmp

./simple_loop_fast
./test_cast_fast
./test_fcmp_fast
./test_icmp_fast
