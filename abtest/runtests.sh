#!/bin/sh

set -x

./abtest.py --target=X8632 -O2 --test=simple_loop.c --driver=simple_loop_main.c --prefix=Subzero_ --output=simple_loop_O2 || exit 1
./abtest.py --target=X8632 -O2 --test=test_cast.cpp --test=test_cast_to_u1.ll --driver=test_cast_main.cpp --prefix=Subzero_ --output=test_cast_O2 || exit 1
./abtest.py --target=X8632 -O2 --test=test_fcmp.pnacl.ll --driver=test_fcmp_main.cpp --prefix=Subzero_ --output=test_fcmp_O2 || exit 1
./abtest.py --target=X8632 -O2 --test=test_icmp.cpp --driver=test_icmp_main.cpp --prefix=Subzero_ --output=test_icmp_O2 || exit 1

./abtest.py --target=X8632 -Om1 --test=simple_loop.c --driver=simple_loop_main.c --prefix=Subzero_ --output=simple_loop_Om1 || exit 1
./abtest.py --target=X8632 -Om1 --test=test_cast.cpp --test=test_cast_to_u1.ll --driver=test_cast_main.cpp --prefix=Subzero_ --output=test_cast_Om1 || exit 1
./abtest.py --target=X8632 -Om1 --test=test_fcmp.pnacl.ll --driver=test_fcmp_main.cpp --prefix=Subzero_ --output=test_fcmp_Om1 || exit 1
./abtest.py --target=X8632 -Om1 --test=test_icmp.cpp --driver=test_icmp_main.cpp --prefix=Subzero_ --output=test_icmp_Om1 || exit 1

./simple_loop_O2
./test_cast_O2
./test_fcmp_O2
./test_icmp_O2

./simple_loop_Om1
./test_cast_Om1
./test_fcmp_Om1
./test_icmp_Om1
