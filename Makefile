PLATFORM?=C64
TARGET=c64maze
all: $(TARGET)

CFLAGS += -I ./ -g
ifeq ($(PLATFORM),C64)
	#commodore 64 platform
CC=cc65
AS=ca65
LD=ld65
CFLAGS += -DPLATFORM_MAZE=C64
endif
ifeq ($(PLATFORM),UNIX)
	#unix platform
CC=gcc
AS=as
LD=ld
CFLAGS += -DPLATFORM_MAZE=UNIX
endif


$(TARGET): $(TARGET).c ports/$(PLATFORM).c
ifeq ($(PLATFORM),C64) 
	$(CC) $(CFLAGS) -Oi -T -t c64 $(TARGET).c 
	$(AS) $(TARGET).s
	$(CC) $(CFLAGS) -Oi -T -t c64 ports/$(PLATFORM).c 
	$(AS) ports/$(PLATFORM).s
	$(LD) -o $(TARGET) -t c64 $(TARGET).o ports/$(PLATFORM).o c64.lib
endif
ifeq ($(PLATFORM),UNIX)
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c ports/$(PLATFORM).c -lSDL2
endif

clean:
	-rm `find ./ -name *.o`
	-rm $(TARGET)
	-rm $(TARGET).s
	-rm ports/$(PLATFORM).s
