/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceOperand.h"
#include "IceTypes.h"

static void TestSimpleLoop(void);
static void TestSimpleCond(void);

typedef void (*TestFunctionType)(void);
static struct {
  const char *TestName;
  TestFunctionType TestFunction;
} Tests[] = { { "loop", TestSimpleLoop }, { "cond", TestSimpleCond }, };
const unsigned NumTests = sizeof(Tests) / sizeof(*Tests);

static void Usage(const char *Arg) {
  printf("Usage: %s <test>\n", Arg);
  printf("where <test> is one of:\n");
  for (unsigned i = 0; i < NumTests; ++i) {
    printf("  %s\n", Tests[i].TestName);
  }
}

static TestFunctionType GetTest(const char *TestName) {
  for (unsigned i = 0; i < NumTests; ++i) {
    if (!strcmp(TestName, Tests[i].TestName))
      return Tests[i].TestFunction;
  }
  return NULL;
}

static IceValueTranslation<IceString> VariableTranslation, LabelTranslation;

static IceVariable *getVariable(IceCfg *Cfg, IceType Type, IceString Name) {
  uint32_t Index = VariableTranslation.translate(Name);
  return Cfg->makeVariable(Type, Index, Name);
}

static IceCfgNode *getNode(IceCfg *Cfg, IceString Name) {
  uint32_t Index = LabelTranslation.translate(Name);
  return Cfg->makeNode(Index, Name);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    Usage(argv[0]);
    return 1;
  }
  TestFunctionType Function = GetTest(argv[1]);
  if (!Function) {
    printf("Unknown test: %s\n", argv[1]);
    Usage(argv[0]);
    return 1;
  }
  // IceLiveRange::unitTests();
  Function();
  return 0;
}

static void TestSimpleLoop(void) {
  /*
    define internal i32 @simple_loop(i32 %a, i32 %n) {
    entry:
    %cmp4 = icmp sgt i32 %n, 0
    br i1 %cmp4, label %for.body, label %for.end

    for.body:                                 ; preds = %for.body, %entry
    %i.06 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
    %sum.05 = phi i32 [ %add, %for.body ], [ 0, %entry ]
    %gep_array = mul i32 %i.06, 4
    %gep = add i32 %a, %gep_array
    %gep.asptr = inttoptr i32 %gep to i32*
    %0 = load i32* %gep.asptr, align 1
    %add = add i32 %0, %sum.05
    %inc = add i32 %i.06, 1
    %cmp = icmp slt i32 %inc, %n
    br i1 %cmp, label %for.body, label %for.end

    for.end:                                  ; preds = %for.body, %entry
    %sum.0.lcssa = phi i32 [ 0, %entry ], [ %add, %for.body ]
    ret i32 %sum.0.lcssa
    }
  */

  // Internally omit initial '%' in variable/label names.
  // http://xkcd.com/1306/

  IceInst *Inst;
  IceInstPhi *Phi;
  IceVariable *Dest;
  IceOperand *Src1, *Src2;
  IceCfgNode *LabelId1, *LabelId2;
  IceCfgNode *Node;

  IceCfg *Cfg = new IceCfg;
  Cfg->setName("@simple_loop");
  Cfg->setReturnType(IceType_i32);
  Cfg->makeTarget(IceTarget_X8632);
  Cfg->addArg(getVariable(Cfg, IceType_i32, "a"));
  Cfg->addArg(getVariable(Cfg, IceType_i32, "n"));

  // Node = new IceCfgNode(Cfg, Cfg->translateLabel("entry"));
  Node = getNode(Cfg, "entry");
  Cfg->setEntryNode(Node);
  // %cmp4 = icmp sgt i32 %n, 0
  Dest = getVariable(Cfg, IceType_i1, "cmp4");
  Src1 = getVariable(Cfg, IceType_i32, "n");
  Src2 = Cfg->getConstant(IceType_i32, 0);
  Inst = new IceInstIcmp(Cfg, IceInstIcmp::Sgt, Dest, Src1, Src2);
  Node->appendInst(Inst);
  // br i1 %cmp4, label %for.body, label %for.end
  Src1 = getVariable(Cfg, IceType_i1, "cmp4");
  LabelId1 = getNode(Cfg, "for.body");
  LabelId2 = getNode(Cfg, "for.end");
  Inst = new IceInstBr(Cfg, Src1, LabelId1, LabelId2);
  Node->appendInst(Inst);

  // Node = new IceCfgNode(Cfg, Cfg->translateLabel("for.body"));
  Node = getNode(Cfg, "for.body");
  // %i.06 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  Dest = getVariable(Cfg, IceType_i32, "i.06");
  Phi = new IceInstPhi(Cfg, Dest);
  Src1 = getVariable(Cfg, IceType_i32, "inc");
  Phi->addArgument(Src1, getNode(Cfg, "for.body"));
  Src1 = Cfg->getConstant(IceType_i32, 0);
  Phi->addArgument(Src1, getNode(Cfg, "entry"));
  Node->addPhi(Phi);
  // %sum.05 = phi i32 [ %add, %for.body ], [ 0, %entry ]
  Dest = getVariable(Cfg, IceType_i32, "sum.05");
  Phi = new IceInstPhi(Cfg, Dest);
  Src1 = getVariable(Cfg, IceType_i32, "add");
  Phi->addArgument(Src1, getNode(Cfg, "for.body"));
  Src1 = Cfg->getConstant(IceType_i32, 0);
  Phi->addArgument(Src1, getNode(Cfg, "entry"));
  Node->addPhi(Phi);
  // %gep_array = mul i32 %i.06, 4
  Dest = getVariable(Cfg, IceType_i32, "gep_array");
  Src1 = getVariable(Cfg, IceType_i32, "i.06");
  Src2 = Cfg->getConstant(IceType_i32, 4);
  Inst = new IceInstArithmetic(Cfg, IceInstArithmetic::Mul, Dest, Src1, Src2);
  Node->appendInst(Inst);
  // %gep = add i32 %a, %gep_array
  Dest = getVariable(Cfg, IceType_i32, "gep");
  Src1 = getVariable(Cfg, IceType_i32, "a");
  Src2 = getVariable(Cfg, IceType_i32, "gep_array");
  Inst = new IceInstArithmetic(Cfg, IceInstArithmetic::Add, Dest, Src1, Src2);
  Node->appendInst(Inst);
  // %gep.asptr = inttoptr i32 %gep to i32*
  // This is a no-op, and wouldn't actually appear in the PNaCl bitcode.
  Dest = getVariable(Cfg, IceType_i32, "gep.asptr");
  Src1 = getVariable(Cfg, IceType_i32, "gep");
  Inst = new IceInstAssign(Cfg, Dest, Src1);
  Node->appendInst(Inst);
  // %0 = load i32* %gep.asptr, align 1
  Dest = getVariable(Cfg, IceType_i32, "0");
  Src1 = getVariable(Cfg, IceType_i32, "gep.asptr");
  Inst = new IceInstLoad(Cfg, Dest, Src1);
  Node->appendInst(Inst);
  // %add = add i32 %0, %sum.05
  Dest = getVariable(Cfg, IceType_i32, "add");
  Src1 = getVariable(Cfg, IceType_i32, "0");
  Src2 = getVariable(Cfg, IceType_i32, "sum.05");
  Inst = new IceInstArithmetic(Cfg, IceInstArithmetic::Add, Dest, Src1, Src2);
  Node->appendInst(Inst);
  // %inc = add i32 %i.06, 1
  Dest = getVariable(Cfg, IceType_i32, "inc");
  Src1 = getVariable(Cfg, IceType_i32, "i.06");
  Src2 = Cfg->getConstant(IceType_i32, 1);
  Inst = new IceInstArithmetic(Cfg, IceInstArithmetic::Add, Dest, Src1, Src2);
  Node->appendInst(Inst);
  // %cmp = icmp slt i32 %inc, %n
  Dest = getVariable(Cfg, IceType_i1, "cmp");
  Src1 = getVariable(Cfg, IceType_i32, "inc");
  Src2 = getVariable(Cfg, IceType_i32, "n");
  Inst = new IceInstIcmp(Cfg, IceInstIcmp::Slt, Dest, Src1, Src2);
  Node->appendInst(Inst);
  // br i1 %cmp, label %for.body, label %for.end
  Src1 = getVariable(Cfg, IceType_i1, "cmp");
  LabelId1 = getNode(Cfg, "for.body");
  LabelId2 = getNode(Cfg, "for.end");
  Inst = new IceInstBr(Cfg, Src1, LabelId1, LabelId2);
  Node->appendInst(Inst);

  // Node = new IceCfgNode(Cfg, Cfg->translateLabel("for.end"));
  Node = getNode(Cfg, "for.end");
  // %sum.0.lcssa = phi i32 [ 0, %entry ], [ %add, %for.body ]
  Dest = getVariable(Cfg, IceType_i32, "sum.0.lcssa");
  Phi = new IceInstPhi(Cfg, Dest);
  Src1 = Cfg->getConstant(IceType_i32, 0);
  Phi->addArgument(Src1, getNode(Cfg, "entry"));
  Src1 = getVariable(Cfg, IceType_i32, "add");
  Phi->addArgument(Src1, getNode(Cfg, "for.body"));
  Node->addPhi(Phi);
  // ret i32 %sum.0.lcssa
  Src1 = getVariable(Cfg, IceType_i32, "sum.0.lcssa");
  Inst = new IceInstRet(Cfg, Src1);
  Node->appendInst(Inst);

  Cfg->Str.setVerbose(IceV_All);
  Cfg->registerEdges();
  Cfg->translate();

  delete Cfg;
}

static void TestSimpleCond(void) {
  /*
    define internal i32 @simple_cond(i32 %a, i32 %n) {
    entry:
      %cmp = icmp slt i32 %n, 0
      br i1 %cmp, label %if.then, label %if.else

    if.then:                                          ; preds = %entry
      %sub = sub i32 1, %n
      br label %if.end

    if.else:                                          ; preds = %entry
      %gep_array = mul i32 %n, 4
      %gep = add i32 %a, %gep_array
      %gep.asptr = inttoptr i32 %gep to i32*
      %0 = load i32* %gep.asptr, align 1
      br label %if.end

    if.end:                                           ; preds = %if.else,
    %if.then
      %result.0 = phi i32 [ %sub, %if.then ], [ %0, %if.else ]
      ret i32 %result.0
    }
   */
  IceInst *Inst;
  IceInstPhi *Phi;
  IceVariable *Dest;
  IceOperand *Src1, *Src2;
  IceCfgNode *LabelId1, *LabelId2;
  IceCfgNode *Node;

  IceCfg *Cfg = new IceCfg;
  Cfg->setName("@simple_cond");
  Cfg->setReturnType(IceType_i32);
  Cfg->makeTarget(IceTarget_X8632);
  Cfg->addArg(getVariable(Cfg, IceType_i32, "a"));
  Cfg->addArg(getVariable(Cfg, IceType_i32, "n"));

  // Node = new IceCfgNode(Cfg, Cfg->translateLabel("entry"));
  Node = getNode(Cfg, "entry");
  Cfg->setEntryNode(Node);
  // %cmp = icmp slt i32 %n, 0
  Dest = getVariable(Cfg, IceType_i1, "cmp");
  Src1 = getVariable(Cfg, IceType_i32, "n");
  Src2 = Cfg->getConstant(IceType_i32, 0);
  Inst = new IceInstIcmp(Cfg, IceInstIcmp::Slt, Dest, Src1, Src2);
  Node->appendInst(Inst);
  // br i1 %cmp, label %if.then, label %if.else
  Src1 = getVariable(Cfg, IceType_i1, "cmp");
  LabelId1 = getNode(Cfg, "if.then");
  LabelId2 = getNode(Cfg, "if.else");
  Inst = new IceInstBr(Cfg, Src1, LabelId1, LabelId2);
  Node->appendInst(Inst);

  // Node = new IceCfgNode(Cfg, Cfg->translateLabel("if.then"));
  Node = getNode(Cfg, "if.then");
  // %sub = sub i32 1, %n
  Dest = getVariable(Cfg, IceType_i32, "sub");
  Src1 = Cfg->getConstant(IceType_i32, 1);
  Src2 = getVariable(Cfg, IceType_i32, "n");
  Inst = new IceInstArithmetic(Cfg, IceInstArithmetic::Sub, Dest, Src1, Src2);
  Node->appendInst(Inst);
  // br label %if.end
  LabelId1 = getNode(Cfg, "if.end");
  Inst = new IceInstBr(Cfg, LabelId1);
  Node->appendInst(Inst);

  // Node = new IceCfgNode(Cfg, Cfg->translateLabel("if.else"));
  Node = getNode(Cfg, "if.else");
  // %gep_array = mul i32 %n, 4
  Dest = getVariable(Cfg, IceType_i32, "gep_array");
  Src1 = getVariable(Cfg, IceType_i32, "n");
  Src2 = Cfg->getConstant(IceType_i32, 4);
  Inst = new IceInstArithmetic(Cfg, IceInstArithmetic::Mul, Dest, Src1, Src2);
  Node->appendInst(Inst);
  // %gep = add i32 %a, %gep_array
  Dest = getVariable(Cfg, IceType_i32, "gep");
  Src1 = getVariable(Cfg, IceType_i32, "a");
  Src2 = getVariable(Cfg, IceType_i32, "gep_array");
  Inst = new IceInstArithmetic(Cfg, IceInstArithmetic::Add, Dest, Src1, Src2);
  Node->appendInst(Inst);
  // %gep.asptr = inttoptr i32 %gep to i32*
  // This is a no-op, and wouldn't actually appear in the PNaCl bitcode.
  Dest = getVariable(Cfg, IceType_i32, "gep.asptr");
  Src1 = getVariable(Cfg, IceType_i32, "gep");
  Inst = new IceInstAssign(Cfg, Dest, Src1);
  Node->appendInst(Inst);
  // %0 = load i32* %gep.asptr, align 1
  Dest = getVariable(Cfg, IceType_i32, "0");
  Src1 = getVariable(Cfg, IceType_i32, "gep.asptr");
  Inst = new IceInstLoad(Cfg, Dest, Src1);
  Node->appendInst(Inst);
  // br label %if.end
  LabelId1 = getNode(Cfg, "if.end");
  Inst = new IceInstBr(Cfg, LabelId1);
  Node->appendInst(Inst);

  // Node = new IceCfgNode(Cfg, Cfg->translateLabel("if.end"));
  Node = getNode(Cfg, "if.end");
  // %result.0 = phi i32 [ %sub, %if.then ], [ %0, %if.else ]
  Dest = getVariable(Cfg, IceType_i32, "result.0");
  Phi = new IceInstPhi(Cfg, Dest);
  Src1 = getVariable(Cfg, IceType_i32, "sub");
  Phi->addArgument(Src1, getNode(Cfg, "if.then"));
  Src1 = getVariable(Cfg, IceType_i32, "0");
  Phi->addArgument(Src1, getNode(Cfg, "if.else"));
  Node->addPhi(Phi);
  // ret i32 %result.0
  Src1 = getVariable(Cfg, IceType_i32, "result.0");
  Inst = new IceInstRet(Cfg, Src1);
  Node->appendInst(Inst);

  Cfg->Str.setVerbose(IceV_All);
  Cfg->registerEdges();
  Cfg->translate();

  delete Cfg;
}
