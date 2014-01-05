/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include <assert.h>
#include <stdint.h>
#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceOperand.h"
#include "IceRegManager.h"

IceCfgNode::IceCfgNode(IceCfg *Cfg, uint32_t LabelIndex) :
  NameIndex(LabelIndex), ArePhiLoadsPlaced(false), ArePhiStoresPlaced(false),
  RegManager(NULL) {
  Cfg->addNode(this, LabelIndex);
}

void IceCfgNode::appendInst(IceInst *Inst) {
  Insts.push_back(Inst);
  Inst->updateVars(this);
}

void IceCfgNode::addPhi(IceInstPhi *Inst) {
  assert(!ArePhiLoadsPlaced);
  assert(!ArePhiStoresPlaced);
  Phis.push_back(Inst);
  Inst->updateVars(this);
}

void IceCfgNode::addFallthrough(uint32_t TargetLabel) {
  OutEdges.insert(OutEdges.begin(), TargetLabel);
}

void IceCfgNode::addNonFallthrough(uint32_t TargetLabel) {
  OutEdges.push_back(TargetLabel);
}

void IceCfgNode::registerInEdges(IceCfg *Cfg) {
  for (IceEdgeList::const_iterator I = OutEdges.begin(), E = OutEdges.end();
       I != E; ++I) {
    IceCfgNode *Node = Cfg->getNode(*I);
    Node->InEdges.push_back(NameIndex);
  }
}

void IceCfgNode::findAddressOpt(IceCfg *Cfg) {
  // No need to check the Phi instructions.
  IceInstList::const_iterator I = Insts.begin(), E = Insts.end();
  while (I != E) {
    IceInst *Inst = *I++;
    if (Inst->isDeleted())
      continue;
    Inst->findAddressOpt(Cfg, this);
  }
}

void IceCfgNode::markLastUses(IceCfg *Cfg) {
  // No need to check the Phi instructions.
  IceInstList::const_iterator I = Insts.begin(), E = Insts.end();
  while (I != E) {
    IceInst *Inst = *I++;
    if (Inst->isDeleted())
      continue;
    Inst->markLastUses(Cfg);
  }
}

static IceInst *getNextInst(IceInstList::iterator I,
                            const IceInstList::iterator &E) {
  while (I != E && (*I)->isDeleted())
    ++I;
  if (I == E)
    return NULL;
  return *I;
}

void IceCfgNode::placePhiLoads(IceCfg *Cfg) {
  assert(!ArePhiLoadsPlaced);
  ArePhiLoadsPlaced = true;
  // Create the phi version of each destination and add it to the phi
  // instruction's Srcs list.
  IceInstList NewPhiLoads;
  for (unsigned i = 0; i < Phis.size(); ++i) {
    // Change "a=phi(...)" to "a_phi=phi(...); a=a_phi".
    IceInstPhi *Phi = Phis[i];
    IceInst *NewPhi = Phi->lower(Cfg, this);
    NewPhiLoads.push_back(NewPhi);
  }
  if (NewPhiLoads.empty())
    return;
  // TODO: Insert each phi right before its destination's first use in
  // the block, but before any instruction with an implicit use of the
  // destination such as a call or integer divide instruction when the
  // function uses setjmp().
  //Insts.insert(Insts.begin(), NewPhiLoads.begin(), NewPhiLoads.end());
  insertInsts(Insts.begin(), NewPhiLoads);
}

void IceCfgNode::placePhiStores(IceCfg *Cfg) {
  assert(ArePhiLoadsPlaced);
  assert(!ArePhiStoresPlaced);
  ArePhiStoresPlaced = true;

  IceInstList NewPhiStores;
  for (IceEdgeList::const_iterator I1 = OutEdges.begin(), E1 = OutEdges.end();
       I1 != E1; ++I1) {
    IceCfgNode *Target = Cfg->getNode(*I1);
    assert(Target);
    if (Target == NULL)
      continue;
    for (std::vector<IceInstPhi *>::const_iterator I2 = Target->Phis.begin(), E2 = Target->Phis.end();
         I2 != E2; ++I2) {
      IceOperand *Operand = (*I2)->getOperandForTarget(NameIndex);
      assert(Operand);
      if (Operand == NULL)
        continue;
      IceType Type = (*I2)->getDest(0)->getType();
      IceInstAssign *NewInst =
        new IceInstAssign(Type, (*I2)->getDest(0), Operand);
      NewPhiStores.push_back(NewInst);
    }
  }
  if (NewPhiStores.empty())
    return;
  IceInstList::iterator InsertionPoint = Insts.end();
  if (InsertionPoint != Insts.begin()) {
    --InsertionPoint;
    if ((*InsertionPoint)->getKind() == IceInst::Br) {
      if (InsertionPoint != Insts.begin()) {
        --InsertionPoint;
        if ((*InsertionPoint)->getKind() != IceInst::Icmp &&
            (*InsertionPoint)->getKind() != IceInst::Fcmp) {
          ++InsertionPoint;
        }
      }
    }
  }
  //Insts.insert(InsertionPoint, NewPhiStores.begin(), NewPhiStores.end());
  insertInsts(InsertionPoint, NewPhiStores);
}

void IceCfgNode::deletePhis(IceCfg *Cfg) {
  for (unsigned i = 0; i < Phis.size(); ++i) {
    Phis[i]->setDeleted();
  }
}

void IceCfgNode::genCodeX8632(IceCfg *Cfg) {
  const unsigned NumScratchReg = 3; // eax, ecx, edx
  // TODO: Copy predecessor RegManager for extended basic block.
  RegManager = new IceRegManager(Cfg, this, NumScratchReg);
  // Defer the Phi instructions.
  IceInstList::iterator I = Insts.begin(), E = Insts.end();
  while (I != E) {
    IceInst *Inst = *I++;
    IceInst *Next = getNextInst(I, E);
    if (Inst->isDeleted())
      continue;
    bool DeleteCurInst = false, DeleteNextInst = false;
    IceInstList NewInsts = Inst->genCodeX8632(Cfg, RegManager, Next,
                                              DeleteCurInst, DeleteNextInst);
    //Insts.insert(I, NewInsts.begin(), NewInsts.end());
    insertInsts(I, NewInsts);
    if (DeleteCurInst)
      Inst->setDeleted();
    if (DeleteNextInst)
      Next->setDeleted();
  }
}

void IceCfgNode::insertInsts(IceInstList::iterator Location,
                             const IceInstList &NewInsts) {
  Insts.insert(Location, NewInsts.begin(), NewInsts.end());
  IceInstList::const_iterator I = NewInsts.begin(), E = NewInsts.end();
  while (I != E) {
    IceInst *Inst = *I++;
    if (Inst->isDeleted())
      continue;
    Inst->updateVars(this);
  }
}

// ======================== Dump routines ======================== //

void IceCfgNode::dump(IceOstream &Str) const {
  IceString Name = Str.Cfg->labelName(getIndex());
  Str << Name << ":\n";
  if (Str.isVerbose()) {
    Str << "// preds = ";
    for (IceEdgeList::const_iterator I = InEdges.begin(), E = InEdges.end();
         I != E; ++I) {
      if (I != InEdges.begin())
        Str << ", ";
      Str << "%" << Str.Cfg->labelName(*I);
    }
    Str << "\n";
  }
  for (unsigned i = 0; i < Phis.size(); ++i) {
    Str << Phis[i];
  }
  IceInstList::const_iterator I = Insts.begin(), E = Insts.end();
  while (I != E) {
    IceInst *Inst = *I++;
    Str << Inst;
  }
}
