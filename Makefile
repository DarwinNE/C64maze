PLATFORM?=C64
TARGET=c64maze
all: $(TARGET)

CFLAGS += -I ./ -g -DP_C64=1 -DP_UNIX=2 
ifeq ($(PLATFORM),C64)
	#commodore 64 platform
CC=cc65
AS=ca65
LD=ld65
CFLAGS += -Os -DPLATFORM_MAZE=C64 -DP_CURRENT=P_C64
endif
ifeq ($(PLATFORM),UNIX)
	#unix platform
CC=gcc
AS=as
LD=ld
CFLAGS += -DPLATFORM_MAZE=UNIX  -DP_CURRENT=P_UNIX
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
	$(CC) $(CFLAGS) -o SDL_FontCache.o -c ports/SDL_FontCache/SDL_FontCache.c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c ports/$(PLATFORM).c SDL_FontCache.o -lSDL2 -lSDL2_ttf
endif

clean:
	-rm `find ./ -name *.o`
	-rm $(TARGET)
	-rm $(TARGET).s
	-rm ports/$(PLATFORM).s
