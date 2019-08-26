#ifndef _C64MAZE_H
#define _C64MAZE_H


#define a_abs(a) ((a)>0 ? (a):(-a))
#define a_max(a,b) (((a)>(b))?(a):(b))
#define a_sign(a) ((a)>0?1:((a)==0?0:(-1)))
#define TRUE  0xFF
#define FALSE 0x00

#define SIZEX 200
#define SIZEY 199
#define STEPSIZEX  15
#define STEPSIZEY  15


#define labyrinthSizeX 40
#define labyrinthSizeY 17
extern unsigned char style;

#endif /* C64MAZE_H */
