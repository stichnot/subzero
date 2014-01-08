/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "llvm/Support/raw_ostream.h"

using namespace llvm;


int main(int argc, char** argv) {
  errs() << "x\n";
  return 0;
}

