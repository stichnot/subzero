/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceDefs.h"
#include "IceInst.h"
#include "IceOperand.h"

class IceConstantPool {
public:
  IceConstantPool(IceCfg *Cfg) : Cfg(Cfg) {}
  IceConstantRelocatable *getOrAddRelocatable(IceType Type, const void *Handle,
                                              int64_t Offset,
                                              const IceString &Name) {
    uint32_t Index = NameToIndex.translate(KeyType(Name, Type));
    if (Index >= RelocatablePool.size()) {
      RelocatablePool.resize(Index + 1);
      void *Handle = NULL;
      RelocatablePool[Index] = IceConstantRelocatable::create(
          Cfg, Index, Type, Handle, Offset, Name);
    }
    IceConstantRelocatable *Constant = RelocatablePool[Index];
    assert(Constant);
    return Constant;
  }
  uint32_t getSize(void) const { return RelocatablePool.size(); }
  IceConstantRelocatable *getEntry(uint32_t Index) const {
    assert(Index < RelocatablePool.size());
    return RelocatablePool[Index];
  }

private:
  typedef std::pair<IceString, IceType> KeyType;
  // TODO: Cfg is being captured primarily for arena allocation for
  // new IceConstants.  If IceConstants live beyond a function/Cfg,
  // they need to be allocated from a global arena and there needs to
  // be appropriate locking.
  IceCfg *Cfg;
  // Use IceValueTranslation<> to map (Name,Type) pairs to an index.
  IceValueTranslation<KeyType> NameToIndex;
  std::vector<IceConstantRelocatable *> RelocatablePool;
};

IceCfg::IceCfg(void)
    : Str(this), Name(""), TestPrefix(""), Type(IceType_void),
      IsInternal(false), HasError(false), ErrorMessage(""), Entry(NULL),
      NextInstNumber(1) {
  ConstantPool = new IceConstantPool(this);
}

IceCfg::~IceCfg() { delete ConstantPool; }

void IceCfg::setError(const IceString &Message) {
  HasError = true;
  ErrorMessage = Message;
  Str << "ICE translation error: " << ErrorMessage << "\n";
}

IceCfgNode *IceCfg::makeNode(const IceString &Name) {
  uint32_t LabelIndex = Nodes.size();
  IceCfgNode *Node = IceCfgNode::create(this, LabelIndex, Name);
  Nodes.push_back(Node);
  return Node;
}

// Create a new IceVariable with a particular type and an optional
// name.  The Node argument is the node where the variable is defined.
IceVariable *IceCfg::makeVariable(IceType Type, const IceCfgNode *Node,
                                  const IceString &Name) {
  uint32_t Index = Variables.size();
  Variables.push_back(IceVariable::create(this, Type, Node, Index, Name));
  return Variables[Index];
}

void IceCfg::addArg(IceVariable *Arg) {
  Arg->setIsArg(this);
  Args.push_back(Arg);
}

IceConstant *IceCfg::getConstantInt(IceType Type, uint64_t ConstantInt64) {
  return IceConstantInteger::create(this, Type, ConstantInt64);
}

// TODO: Add float and double constants to the global constant pool,
// instead of creating a new instance each time.
IceConstant *IceCfg::getConstantFloat(float ConstantFloat) {
  return IceConstantFloat::create(this, IceType_f32, ConstantFloat);
}

IceConstant *IceCfg::getConstantDouble(double ConstantDouble) {
  return IceConstantDouble::create(this, IceType_f64, ConstantDouble);
}

IceConstant *IceCfg::getConstantSym(IceType Type, const void *Handle,
                                    int64_t Offset, const IceString &Name,
                                    bool SuppressMangling) {
  IceConstantRelocatable *Const =
      ConstantPool->getOrAddRelocatable(Type, Handle, Offset, Name);
  Const->setSuppressMangling(SuppressMangling);
  return Const;
}

void IceCfg::registerEdges(void) {
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    (*I)->registerEdges();
  }
}

// ======================== Dump routines ======================== //

void IceCfg::dump(void) const {
  Str.setCurrentNode(getEntryNode());
  // Print function name+args
  if (Str.isVerbose(IceV_Instructions)) {
    Str << "define ";
    if (getInternal())
      Str << "internal ";
    Str << Type << " @" << Name << "(";
    for (uint32_t i = 0; i < Args.size(); ++i) {
      if (i > 0)
        Str << ", ";
      Str << Args[i]->getType() << " " << Args[i];
    }
    Str << ") {\n";
  }
  Str.setCurrentNode(NULL);
  // Print each basic block
  for (IceNodeList::const_iterator I = Nodes.begin(), E = Nodes.end(); I != E;
       ++I) {
    (*I)->dump(Str);
  }
  if (Str.isVerbose(IceV_Instructions)) {
    Str << "}\n";
  }
}
