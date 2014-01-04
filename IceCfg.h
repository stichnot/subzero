// -*- Mode: c++ -*-
#ifndef _IceCfg_h
#define _IceCfg_h

#include <stdint.h>
#include <ostream>
#include <vector>
#include "IceDefs.h"
#include "IceTypes.h"

class IceCfg {
public:
  IceCfg(void);
  ~IceCfg();
  void setName(const IceString &FunctionName);
  void setReturnType(IceType ReturnType);
  void addArg(IceVariable *Arg);
  void setEntryNode(IceCfgNode *EntryNode);
  void addNode(IceCfgNode *Node, uint32_t LabelIndex);
  IceCfgNode *getNode(uint32_t LabelIndex) const;
  // getConstant() is not const because it might add something to the
  // constant pool.
  IceConstant *getConstant(IceType Type, const void *ConstantBits);
  IceConstant *getConstant(IceType Type, int32_t ConstantInt32);
  // Look up a variable used as an rvalue.  The variable might not yet
  // have a definition if its definition is in a predecessor block
  // that hasn't yet been processed, e.g. via a phi for a loopback
  // edge.
  IceVariable *getVariable(IceType Type, uint32_t Index);
  IceVariable *getVariable(IceType Type, const IceString &Name) {
    return getVariable(Type, translateVariable(Name));
  }

  uint32_t translateVariable(const IceString &Name);
  uint32_t translateLabel(const IceString &Name);
  IceString variableName(uint32_t VariableIndex) const;
  IceString labelName(uint32_t LabelIndex) const;
  void translate(void);
  void dump(void) const;
  void markLastUse(IceOperand *Operand, const IceInst *Inst);
  bool isLastUse(const IceInst *Inst, IceOperand *Operand) const;

  mutable IceOstream Str;
private:
  IceString Name; // function name
  IceType Type; // return type
  IceCfgNode *Entry; // entry basic block
  std::vector<IceCfgNode *> Nodes; // linearization; Entry must be first
  std::vector<IceVariable *> Variables;
  std::vector<const IceInst *> LastUses; // instruction ending each variable's live range
  std::vector<IceVariable *> Args; // densely packed vector, subset of Variables

  class NameTranslation *VariableTranslation;
  class NameTranslation *LabelTranslation;
  // arena allocator for the function
  // list of exit IceCfgNode* for liveness analysis
  // operand pool - set of IceOperand
  // whether there is an alloca (frame ptr optimization)
  void registerInEdges(void);
  void findAddressOpt(void);
  void markLastUses(void);
  void placePhiLoads(void);
  void placePhiStores(void);
  void deletePhis(void);
  void genCodeX8632(void);
  void simpleDCE(void);
};

#endif // _IceCfg_h
