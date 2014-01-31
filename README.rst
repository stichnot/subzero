Subzero - Fast code generator for PNaCl bitcode
===============================================

Building
--------

You must have LLVM trunk source code available and built.  See
http://llvm.org/docs/GettingStarted.html#getting-started-quickly-a-summary for
guidance.

Set variables ``LLVM_SRC_PATH``, ``LLVM_BUILD_PATH``, and ``LLVM_BIN_PATH`` to
point to the appropriate directories in the LLVM source and build directories.
These can be set as environment variables, or you can modify the top-level
Makefile.

Run ``make`` at the top level to build the main target ``llvm2ice``.

``llvm2ice``
------------

The ``llvm2ice`` program uses the LLVM infrastructure to parse an LLVM bitcode
file and translate it into ICE.  It then invokes ICE's translate method to lower
it to target-specific machine code, dumping the IR at various stages of the
translation.

The program can be run as follows::

    ../llvm2ice ./ir_samples/<file>.ll
    ../llvm2ice ./tests_lit/llvm2ice_tests/<file>.ll

At this time, ``llvm2ice`` accepts a few arguments:

    ``-help`` -- Show available arguments and possible values.

    ``-notranslate`` -- Suppress the ICE translation phase, which is useful if
    ICE is missing some support.

    ``-target=<TARGET>`` -- Set the target architecture (default x8632).

    ``-verbose=<list>`` -- Set verbosity flags.  This argument allows a
    comma-separated list of values.  The default is ``inst,pred`` to roughly
    match the .ll bitcode file.  Of particular use are ``all`` and ``none``.

See ir_samples/README.rst for more details.

Running the test suite
----------------------

Subzero uses the LLVM ``lit`` testing tool for its test suite, which lives in
``tests_lit``. To execute the test suite, first build Subzero, and then run::

    python <path_to_lit.py> -sv tests_lit

``path_to_lit`` is the direct path to the lit script in the LLVM source
(``$LLVM_SRC_PATH/utils/lit/lit.py``).

The above ``lit`` execution also needs the LLVM binary path in the
``LLVM_BIN_PATH`` env var.

Assuming the LLVM paths are set up, ``make check`` is a convenient way to run
the test suite.


TODO list
---------

Here is a list of TODO items.  Some might already be listed in the source code
with a ``TODO`` comment.

- Add support for not-yet-implemented LLVM bitcode instructions.  Start with
  constructors and ``dump()`` methods, then add lowering and any special needs.

- Constants.  ``i32`` sort of works now, but the rest either outright don't work
  or haven't been tested.  Floating point constants need to live in a global
  constant pool.

- Other types, especially ``i64``, ``f32``, and ``f64``.  Most of the
  requirements are probably in the lowering and register allocation.

- Global symbols.  Internal representation, and emission.

- Configurability of which passes are run.

- Other targets besides x86-32.

- Add code to LLVM so that for each method that needs translation, first offer
  Subzero the chance to translate, and if Subzero returns an error code, fall
  back to robust LLVM translation (presumably O0).

- Add code to Subzero allowing fine-grain control of which methods to attempt to
  translate and which to unconditionally reject.  The control string can come
  from the command line and/or environment.

- Refactor the IR passes using a Visitor pattern.
