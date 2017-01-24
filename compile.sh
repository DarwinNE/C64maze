#!/bin/bash
cc65 -Oi -T -t c64 c64maze.c
ca65 c64maze.s
ld65 -o c64maze -t c64 c64maze.o c64.lib
