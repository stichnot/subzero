# The following variables will likely need to be modified, depending on where
# and how you built LLVM & Clang. They can be overridden in a command-line
# invocation of make, like:
#
#   make LLVM_SRC_PATH=<path> LLVM_BUILD_PATH=<path> ...
#

# LLVM_SRC_PATH is the path to the root of the checked out source code. This
# directory should contain the configure script, the include/ and lib/
# directories of LLVM, Clang in tools/clang/, etc.
# Alternatively, if you're building vs. a binary download of LLVM, then
# LLVM_SRC_PATH can point to the main untarred directory.
LLVM_SRC_PATH ?= $$HOME/llvm/llvm_svn_rw

# LLVM_BUILD_PATH is the directory in which you built LLVM - where you ran
# configure or cmake.
# For linking vs. a binary build of LLVM, point to the main untarred directory.
# LLVM_BIN_PATH is the directory where binaries are placed by the LLVM build
# process. It should contain the tools like opt, llc and clang. The default
# reflects a debug build with autotools (configure & make).
LLVM_BUILD_PATH ?= $$HOME/llvm/build/svn-make-debug
LLVM_BIN_PATH ?= $(LLVM_BUILD_PATH)/Debug+Asserts/bin

$(info -----------------------------------------------)
$(info Using LLVM_SRC_PATH = $(LLVM_SRC_PATH))
$(info Using LLVM_BUILD_PATH = $(LLVM_BUILD_PATH))
$(info Using LLVM_BIN_PATH = $(LLVM_BIN_PATH))
$(info -----------------------------------------------)

LLVM_CXXFLAGS := `$(LLVM_BIN_PATH)/llvm-config --cxxflags`
LLVM_LDFLAGS := `$(LLVM_BIN_PATH)/llvm-config --ldflags --libs --system-libs`

# It's recommended that CXX matches the compiler you used to build LLVM itself.
CXX := g++
CXXFLAGS := -Wall -Werror -fno-rtti -O0 -g $(LLVM_CXXFLAGS)
LDFLAGS :=

OBJS= \
	IceCfg.o \
	IceCfgNode.o \
	IceInst.o \
	IceInstX8632.o \
	IceOperand.o \
	IceRegAlloc.o \
	IceRegManager.o \
	IceTargetLowering.o \
	IceTypes.o

# Keep all the first target so it's the default.
all: subzerotest llvm2ice

.PHONY: all

subzerotest: $(OBJS) IceTest.o
	$(CXX) $(LDFLAGS) -o $@ $^

llvm2ice: $(OBJS) PNaClABITypeChecker.o llvm2ice.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LLVM_LDFLAGS)

PNaClABITypeChecker.o: PNaClABITypeChecker.cpp PNaClABITypeChecker.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

# Compiling driver files (with a 'main' function) separately, so they don't
# get included in OBJS.
llvm2ice.o: llvm2ice.cpp *.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

IceTest.o: IceTest.cpp *.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(OBJS): %.o: %.cpp *.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	rm -f llvm2ice subzerotest *.o
