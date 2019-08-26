#ifndef _C64MAZE_H
#define _C64MAZE_H


#define a_abs(a) ((a)>0 ? (a):(-a))
#define a_max(a,b) (((a)>(b))?(a):(b))
#define a_sign(a) ((a)>0?1:((a)==0?0:(-1)))
#define TRUE  0xFF
#define FALSE 0x00

#if PLATFORM == C64
#define SIZEX 200
#define SIZEY 199
#define STEPSIZEX 15
#define STEPSIZEY 15
#elif PLATFORM == UNIX
#define SIZEX 1024
#define SIZEY 768
#define STEPSIZEX 70
#define STEPSIZEY 70
#elif PLATFORM == STM_128x64
#define SIZEX 128
#define SIZEY 64
#define STEPSIZEX  5
#define STEPSIZEY  5
#endif


#define labyrinthSizeX 40
#define labyrinthSizeY 17
extern unsigned char style;

#endif /* C64MAZE_H */
