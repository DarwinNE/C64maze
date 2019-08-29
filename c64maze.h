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
#define SIZEX 128
#define SIZEY 64
#define STEPSIZEX 5
#define STEPSIZEY 5
#elif PLATFORM == STM_128x64
#define SIZEX 128
#define SIZEY 64
#define STEPSIZEX  5
#define STEPSIZEY  5
#endif


#define labyrinthSizeX 40
#define labyrinthSizeY 17
extern unsigned char style;

/* dynamic display bounds */
typedef struct {
    unsigned short szx;
    unsigned short szy;
    unsigned short stepszx;
    unsigned short stepszy;
} display_bounds_t;
extern display_bounds_t disp_bounds;

#endif /* C64MAZE_H */
