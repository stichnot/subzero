CC=g++
CFLAGS=-c -g -Wall

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
	$(CC) -o subzerotest $(OBJS)

clean:
	rm -f subzerotest *.o
