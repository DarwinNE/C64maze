PLATFORM?=C64
TARGET=c64maze
all: options $(TARGET)

options:
	#include files
CFLAGS += -I ./ 
ifeq ($(PLATFORM),C64)
CC=cc65
AS=ca65
LD=ld65
CFLAGS += -DPLATFORM_MAZE=C64
else
	CC=gcc
endif


$(TARGET): options
ifeq ($(PLATFORM),C64) 
	$(CC) $(CFLAGS) -Oi -T -t c64 $(TARGET).c 
	$(AS) c64maze.s
	$(CC) $(CFLAGS) -Oi -T -t c64 ports/$(PLATFORM).c 
	$(AS) ports/$(PLATFORM).s
	$(LD) -o $(TARGET) -t c64 $(TARGET).o ports/$(PLATFORM).o c64.lib
endif

clean:
	-rm `find ./ -name *.o`
	-rm $(TARGET)
	-rm $(TARGET).s
