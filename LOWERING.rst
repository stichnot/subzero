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


Instructions producing multiple values
--------------------------------------

Instructions with register side effects
---------------------------------------

Preventing dead-code elimination
--------------------------------

