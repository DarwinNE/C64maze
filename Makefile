PLATFORM?=C64
TARGET=c64maze
all: options $(TARGET)

options:
ifeq ($(PLATFORM),C64)
CC=cc65
AS=ca65
LD=ld65
else
	CC=gcc
endif


$(TARGET): options
ifeq ($(PLATFORM),C64) 
	$(CC) -Oi -T -t c64 $(TARGET).c 
	$(AS) c64maze.s
	$(LD) -o $(TARGET) -t c64 $(TARGET).o c64.lib
endif

clean:
	-rm `find ./ -name *.o`
	-rm $(TARGET)
	-rm $(TARGET).s
