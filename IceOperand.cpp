/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceCfg.h"
#include "IceInst.h"
#include "IceOperand.h"

void IceVariable::setUse(const IceInst *Inst, const IceCfgNode *Node) {
  if (DefNode == NULL)
    return;
  if (llvm::isa<IceInstPhi>(Inst) || Node != DefNode)
    DefNode = NULL;
}

void IceVariable::setDefinition(IceInst *Inst, const IceCfgNode *Node) {
  if (DefNode == NULL)
    return;
  // Can first check preexisting DefInst if we care about multi-def vars.
  DefInst = Inst;
  if (Node != DefNode)
    DefNode = NULL;
}

void IceVariable::replaceDefinition(IceInst *Inst, const IceCfgNode *Node) {
  DefInst = NULL;
  setDefinition(Inst, Node);
}

void IceVariable::setIsArg(IceCfg *Cfg) {
  IsArgument = true;
  if (DefNode == NULL)
    return;
  IceCfgNode *Entry = Cfg->getEntryNode();
  if (DefNode == Entry)
    return;
  DefNode = NULL;
}

IceString IceVariable::getName(void) const {
  if (Name != "")
    return Name;
  char buf[30];
  sprintf(buf, "__%u", getIndex());
  return buf;
}

// ======================== dump routines ======================== //

IceOstream &operator<<(IceOstream &Str, const IceOperand *O) {
  if (O)
    O->dump(Str);
  else
    Str << "<NULL>";
  return Str;
}

void IceVariable::dump(IceOstream &Str) const {
  const IceCfgNode *CurrentNode = Str.getCurrentNode();
  (void)CurrentNode;
  assert(CurrentNode == NULL || DefNode == NULL || DefNode == CurrentNode);
  Str << "%" << getName();
}

void IceOperand::dump(IceOstream &Str) const { Str << "IceOperand<?>"; }

void IceConstantRelocatable::dump(IceOstream &Str) const {
  Str << "@" << Name;
  if (Offset)
    Str << "+" << Offset;
}
