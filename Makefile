# The following variables will likely need to be modified, depending on where
# and how you built LLVM & Clang. They can be overridden in a command-line
# invocation of make, like:
#
#   make LLVM_SRC_PATH=<path> LLVM_BIN_PATH=<path> ...
#

# LLVM_SRC_PATH is the path to the root of the checked out source code. This
# directory should contain the configure script, the include/ and lib/
# directories of LLVM, Clang in tools/clang/, etc.
# Alternatively, if you're building vs. a binary download of LLVM, then
# LLVM_SRC_PATH can point to the main untarred directory.
LLVM_SRC_PATH ?= $$HOME/llvm/llvm_svn_rw

# LLVM_BIN_PATH is the directory where binaries are placed by the LLVM build
# process. It should contain the tools like opt, llc and clang. The default
# reflects a debug build with autotools (configure & make).
LLVM_BUILD_PATH ?= $$HOME/llvm/build/svn-make-debug
LLVM_BIN_PATH ?= $(LLVM_BUILD_PATH)/Debug+Asserts/bin

$(info -----------------------------------------------)
$(info Using LLVM_SRC_PATH = $(LLVM_SRC_PATH))
$(info Using LLVM_BIN_PATH = $(LLVM_BIN_PATH))
$(info -----------------------------------------------)

LLVM_CXXFLAGS := `$(LLVM_BIN_PATH)/llvm-config --cxxflags`
#LLVM_LDFLAGS := `$(LLVM_BIN_PATH)/llvm-config --ldflags --libs --system-libs`
LLVM_LDFLAGS := `$(LLVM_BIN_PATH)/llvm-config --ldflags --libs`

# It's recommended that CXX matches the compiler you used to build LLVM itself.
OPTLEVEL := -O0
CXX := g++
CXXFLAGS := -Wall -Wextra -Werror -fno-rtti -fno-exceptions \
	$(OPTLEVEL) -g $(LLVM_CXXFLAGS)
LDFLAGS :=

SRCS= \
	IceCfg.cpp \
	IceCfgNode.cpp \
	IceGlobalContext.cpp \
	IceInst.cpp \
	IceInstX8632.cpp \
	IceLiveness.cpp \
	IceOperand.cpp \
	IceRegAlloc.cpp \
	IceRegManager.cpp \
	IceTargetLowering.cpp \
	IceTargetLoweringX8632.cpp \
	IceTypes.cpp \
	llvm2ice.cpp

OBJS=$(patsubst %.cpp, build/%.o, $(SRCS))

# Keep all the first target so it's the default.
all: llvm2ice

.PHONY: all

llvm2ice: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LLVM_LDFLAGS) -ldl -lcurses

# TODO: Be more precise than "*.h" here and elsewhere.
$(OBJS): build/%.o: src/%.cpp src/*.h src/*.def
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(OBJS): | build

build:
	@mkdir -p $@

check: llvm2ice
	LLVM_BIN_PATH=$(LLVM_BIN_PATH) \
	$(LLVM_SRC_PATH)/utils/lit/lit.py -sv tests_lit
	(cd crosstest; ./runtests.sh)

# TODO: Fix the use of wildcards.
format:
	$(LLVM_BIN_PATH)/clang-format -style=LLVM -i \
	src/Ice*.h src/Ice*.cpp src/llvm2ice.cpp

clean:
	rm -rf llvm2ice *.o build/
