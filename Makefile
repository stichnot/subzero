CC=g++
# TODO: INCLUDEPATH is meant to point into LLVM ADT, Support, etc.
INCLUDEPATH=.
CFLAGS=-g -Wall -I$(INCLUDEPATH)
LDFLAGS=

OBJS= \
	IceCfg.o \
	IceCfgNode.o \
	IceInst.o \
	IceInstX8632.o \
	IceOperand.o \
	IceRegManager.o \
	IceTypes.o \
	IceTest.o

subzerotest: $(OBJS)
	$(CC) $(LDFLAGS) -o subzerotest $(OBJS)

$(OBJS): %.o: %.cpp *.h
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f subzerotest *.o
