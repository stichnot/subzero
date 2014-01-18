Subzero - Fast code generator for PNaCl bitcode
===============================================

Building
--------

You must have LLVM trunk source code available and built.  See
http://llvm.org/docs/GettingStarted.html#getting-started-quickly-a-summary
for guidance.

Set variables ``LLVM_SRC_PATH``, ``LLVM_BUILD_PATH``, and
``LLVM_BIN_PATH`` to point to the appropriate directories in the LLVM
source and build directories.  These can be set as environment
variables, or you can modify the top-level Makefile.

Run ``make`` at the top level to build targets ``subzerotest`` and ``llvm2ice``.

``subzerotest``
---------------

The ``subzerotest`` program uses ``IceTest.cpp`` as a driver.  It
hand-constructs the ICE IR for a small number of tests and runs the
various translation phases on the IR, dumping the IR at various points.

The test is run as follows::

    ./subzerotest cond
    ./subzerotest loop

The ``cond`` test is a simple if-then-else conditional, and the
``loop`` test is a simple loop that sums the elements of an array.

``llvm2ice``
------------

The ``llvm2ice`` program uses the LLVM infrastructure to parse an LLVM
bitcode file, translate it into ICE, and dump the ICE IR.  At this
time, no Subzero translation is performed.  The goal at this point is
to ensure that the LLVM and Subzero IR dumps agree with each other.

The test is run as follows::

    ../llvm2ice ./ir_samples/<file>.ll

See ir_samples/README.rst for more details.
