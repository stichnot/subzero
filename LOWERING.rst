Target-specific lowering in ICE
===============================

This document discusses several issues around generating target-specific
ICE instructions from high-level ICE instructions.

Meeting register address mode constraints
-----------------------------------------

Target-specific instructions often require specific operands to be in physical
registers.  Sometimes a specific register is required, but usually any register
in a particular register class will suffice, and that register class is defined
by the instruction/operand type.

The challenge is that ``IceVariable`` represents an operand that is either a
stack location in the current frame, or a physical register.  Register
allocation happens after target-specific lowering, so during lowering we
generally don't know whether an ``IceVariable`` operand will meet a target
instruction's physical register requirement.

Fortunately, ICE allows hints/directives to force an ``IceVariable`` to get some
physical register, to force it to get a specific physical register, and to
prefer a physical register based on another operand's physical register
assignment.

The recommended ICE lowering strategy is to generate extra assignment
instructions involving extra ``IceVariable`` operands, using the
hints/directives to force appropriate register assignments for the temporaries,
and then let the global register allocator clean things up.

Note: There is a spectrum of *implementation complexity* versus *translation
speed* versus *code quality*.  This recommended strategy picks a point on the
spectrum representing very low complexity ("splat-isel"), pretty good code
quality in terms of frame size and register shuffling/spilling, but perhaps not
the fastest translation speed since extra instructions and operands are created
up front and cleaned up at the end.

Ensuring some physical register
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The x86 instruction::

    mov dst, src

needs at least one of its operands in a physical register (ignoring the case
where ``src`` is a constant).  This can be done as follows::

    mov reg, src
    mov dst, reg

The ICE code that accomplishes this looks something like this::

    IceVariable *Reg;
    Reg = Cfg->makeVariable(Dst->getType());
    Reg->setWeightInfinite();
    NewInst = new IceInstX8632Mov(Cfg, Reg, Src);
    NewInst = new IceInstX8632Mov(Cfg, Dst, Reg);

``IceCfg::makeVariable()`` generates a new temporary, and
``IceVariable::setWeightInfinite()`` gives it infinite weight for the purpose of
register allocation.

Preferring some physical register
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

One problem with this example is that the register allocator just assigns the
first available register to a live range.  If this instruction ends the live
range of ``src``, this may lead to code like the following::

    mov reg:eax, src:esi
    mov dst:edi, reg:eax

Since the first instruction ends the live range of ``src:esi``, it would be
better to assign ``esi`` to ``reg``::

    mov reg:esi, src:esi
    mov dst:edi, reg:esi

The first instruction, "``mov esi, esi``", is a redundant assignment and can be
elided, leaving just "``mov edi, esi``".

We can tell the register allocator to prefer the register assigned to a
different ``IceVariable``, using ``IceVariable::setPreferredRegister()``::

    IceVariable *Reg;
    Reg = Cfg->makeVariable(Dst->getType());
    Reg->setWeightInfinite();
    Reg->setPreferredRegister(Src);
    NewInst = new IceInstX8632Mov(Cfg, Reg, Src);
    NewInst = new IceInstX8632Mov(Cfg, Dst, Reg);

Disabling live-range interference
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Another problem with this example happens when the instructions do *not* end the
live range of ``src``.  In this case, the live ranges of ``reg`` and ``src``
interfere, so they can't get the same physical register despite the explicit
preference.  However, ``reg`` is meant to be an alias of ``src`` so they needn't
be considered to interfere with each other.  This can be expressed via the
second argument of ``setPreferredRegister()``::

    IceVariable *Reg;
    Reg = Cfg->makeVariable(Dst->getType());
    Reg->setWeightInfinite();
    Reg->setPreferredRegister(Src, true);
    NewInst = new IceInstX8632Mov(Cfg, Reg, Src);
    NewInst = new IceInstX8632Mov(Cfg, Dst, Reg);

This should be used with caution and probably only for these short-live-range
temporaries, otherwise the classic "lost copy" or "lost swap" problem may be
encountered.

Ensuring a specific physical register
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Some instructions require operands in specific physical registers, or produce
results in specific physical registers.  For example, the 32-bit ``ret``
instruction needs its operand in ``eax``.  This can be done with
``IceVariable::setRegNum()``::

    IceVariable *Reg;
    Reg = Cfg->makeVariable(Src->getType());
    Reg->setWeightInfinite();
    Reg->setRegNum(Reg_eax);
    NewInst = new IceInstX8632Mov(Cfg, Reg, Src);
    NewInst = new IceInstX8632Ret(Cfg, Reg);

By the way, precoloring with ``IceVariable::setRegNum()`` effectively gives it
infinite weight for register allocation, so the call to
``IceVariable::setWeightInfinite()`` is unnecessary, but perhaps documents the
intention a bit more strongly.


Instructions with register side effects
---------------------------------------

Some instructions produce unwanted results in other registers, or otherwise kill
preexisting values in other registers.  For example, a ``call`` kills the
scratch registers.  Also, the x86-32 ``idiv`` instruction produces the quotient
in ``eax`` and the remainder in ``edx``, but generally only one of those is
needed in the lowering.  It's important that the register allocator doesn't
allocate that register to a live range that spans the instruction.

ICE provides the ``IceInstFakeKill`` pseudo-instruction to mark such register
kills.  For each of the instruction's source variables, a fake trivial live
range is created that begins and ends in that instruction.  The
``IceInstFakeKill`` instruction is inserted after the ``call`` instruction.  For
example::

    NewInst = new IceInstX8632Call(Cfg, ... );
    IceVarList KilledRegs;
    KilledRegs.push_back(eax);
    KilledRegs.push_back(ecx);
    KilledRegs.push_back(edx);
    NewInst = new IceInstFakeKill(Cfg, KilledRegs);

The killed register arguments need to be assigned a physical register via
``IceVarList::setRegNum()`` for this to be effective.  To avoid a massive
proliferation of ``IceVariable`` temporaries, the ``Cfg`` caches one precolored
``IceVariable`` for each physical register::

    NewInst = new IceInstX8632Call(Cfg, ... );
    IceVarList KilledRegs;
    IceVariable *eax = Cfg->getTarget()->getPhysicalRegister(Reg_eax);
    IceVariable *ecx = Cfg->getTarget()->getPhysicalRegister(Reg_ecx);
    IceVariable *edx = Cfg->getTarget()->getPhysicalRegister(Reg_edx);
    KilledRegs.push_back(eax);
    KilledRegs.push_back(ecx);
    KilledRegs.push_back(edx);
    NewInst = new IceInstFakeKill(Cfg, KilledRegs);

On first glance, it seems unnecessary to explicitly kill the register that
returns the ``call`` return value.  However, if for some reason the ``call``
result ends up being unused, dead-code elimination could remove dead assignments
and incorrectly expose the return value register to a register allocation
assignment spanning the call, which would be incorrect.

The ``IceInstFakeKill`` instruction is "linked" to the previous instruction (the
``call`` instruction in this case), such that if its linked instruction is
dead-code eliminated, the ``IceInstFakeKill`` instruction is eliminated as well.

Instructions producing multiple values
--------------------------------------

ICE instructions allow at most one destination ``IceVariable``.  Some machine
instructions produce more than one usable result.  For example, the x86-32
``call`` ABI returns a 64-bit integer result in the ``edx:eax`` register pair.
Also, x86-32 has a version of the ``imul`` instruction that produces a 64-bit
result in the ``edx:eax`` register pair.

To support multi-dest instructions, ICE provides the ``IceInstFakeDef``
pseudo-instruction.  Its destination can be precolored to the appropriate
physical register.  For example, a ``call`` returning a 64-bit result in
``edx:eax``::

    NewInst = new IceInstX8632Call(Cfg, RegLow, ... );
    ...
    NewInst = new IceInstFakeKill(Cfg, KilledRegs);
    IceVariable *RegHigh = Cfg->makeVariable(IceType_i32);
    RegHigh->setRegNum(Reg_edx);
    NewInst = new IceInstFakeDef(Cfg, RegHigh);

``RegHigh`` is then assigned into the desired ``IceVariable``.  If that
assignment ends up being dead-code eliminated, the ``IceInstFakeDef``
instruction may be eliminated as well.

Preventing dead-code elimination
--------------------------------

ICE instructions with a non-NULL ``Dest`` are subject to dead-code elimination.
However, some instructions must not be eliminated in order to preserve side
effects.  This applies to most function calls, volatile loads, and loads and
integer divisions where the underlying language and runtime are relying on
hardware exception handling.

ICE facilitates this with the ``IceInstFakeUse`` pseudo-instruction.  This
forces a use of its source ``IceVariable`` to keep that variable's definition
alive.  Since the ``IceInstFakeUse`` instruction has no ``Dest``, it will not be
eliminated.

Here is the full example of the x86-32 ``call`` returning a 32-bit integer
result::

    IceVariable *Reg = Cfg->makeVariable(IceType_i32);
    Reg->setRegNum(Reg_eax);
    NewInst = new IceInstX8632Call(Cfg, Reg, ... );
    IceVarList KilledRegs;
    KilledRegs.push_back(eax);
    KilledRegs.push_back(ecx);
    KilledRegs.push_back(edx);
    NewInst = new IceInstFakeKill(Cfg, KilledRegs);
    NewInst = new IceInstFakeUse(Cfg, Reg);
    NewInst = new IceInstX8632Mov(Cfg, Result, Reg);

Without the ``IceInstFakeUse``, the entire call sequence could be dead-code
eliminated if its result were unused.

One more note on this topic.  These tools can be used to allow a multi-dest
instruction to be dead-code eliminated only when none of its results is live.
The key is to use the optional source parameter of the ``IceInstFakeDef``
instruction.  Using pseudocode:

    t1:eax = call foo(arg1, ...)
    IceInstFakeKill(eax, ecx, edx)
    t2:edx = IceInstFakeDef(t1)
    v_result_low = t1
    v_result_high = t2

If ``v_result_high`` is live but ``v_result_low`` is dead, adding ``t1`` as an
argument to ``IceInstFakeDef`` suffices to keep the ``call`` instruction live.
