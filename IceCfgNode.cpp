/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceOperand.h"

IceCfgNode::IceCfgNode(IceCfg *Cfg, uint32_t LabelNumber, IceString Name)
    : Cfg(Cfg), Number(LabelNumber), Name(Name) {}

// Returns the name the node was created with.  If no name was given,
// it synthesizes a (hopefully) unique name.
IceString IceCfgNode::getName(void) const {
  if (Name != "")
    return Name;
  char buf[30];
  sprintf(buf, "__%u", getIndex());
  return buf;
}

// Adds an instruction to either the Phi list or the regular
// instruction list.  Validates that all Phis are added before all
// regular instructions.
void IceCfgNode::appendInst(IceInst *Inst) {
  if (IceInstPhi *Phi = llvm::dyn_cast<IceInstPhi>(Inst)) {
    if (!Insts.empty()) {
      Cfg->setError("Phi instruction added to the middle of a block");
      return;
    }
    Phis.push_back(Phi);
  } else {
    Insts.push_back(Inst);
  }
  Inst->updateVars(this);
}

// When a node is created, the OutEdges are immediately knows, but the
// InEdges have to be built up incrementally.  After the CFG has been
// constructed, the registerEdges() pass finalizes it by creating the
// InEdges list.
void IceCfgNode::registerEdges(void) {
  OutEdges = (*Insts.rbegin())->getTerminatorEdges();
  for (IceNodeList::const_iterator I = OutEdges.begin(), E = OutEdges.end();
       I != E; ++I) {
    IceCfgNode *Node = *I;
    Node->InEdges.push_back(this);
  }
}

// ======================== Dump routines ======================== //

void IceCfgNode::dump(IceOstream &Str) const {
  Str.setCurrentNode(this);
  if (Str.isVerbose(IceV_Instructions)) {
    Str << getName() << ":\n";
  }
  // Dump list of predecessor nodes.
  if (Str.isVerbose(IceV_Preds) && !InEdges.empty()) {
    Str << "    // preds = ";
    for (IceNodeList::const_iterator I = InEdges.begin(), E = InEdges.end();
         I != E; ++I) {
      if (I != InEdges.begin())
        Str << ", ";
      Str << "%" << (*I)->getName();
    }
    Str << "\n";
  }
  // Dump each instruction.
  if (Str.isVerbose(IceV_Instructions)) {
    for (IcePhiList::const_iterator I = Phis.begin(), E = Phis.end(); I != E;
         ++I) {
      Str << (*I);
    }
    IceInstList::const_iterator I = Insts.begin(), E = Insts.end();
    while (I != E) {
      IceInst *Inst = *I++;
      Str << Inst;
    }
  }
  // Dump list of successor nodes.
  if (Str.isVerbose(IceV_Succs)) {
    Str << "    // succs = ";
    for (IceNodeList::const_iterator I = OutEdges.begin(), E = OutEdges.end();
         I != E; ++I) {
      if (I != OutEdges.begin())
        Str << ", ";
      Str << "%" << (*I)->getName();
    }
    Str << "\n";
  }
}
