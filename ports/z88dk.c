
// ZX 81 (with 32K + WRX mod)
//  zcc +zx81 -clib=wrx -subtype=_wrx -pragma-define:hrgpage=40000 -create-app -DSZ=7 -DGFXSCALEX=4/5 -DGFXSCALEY=4/5 -DNO_SOUND -lm -O3 -DPLATFORM_MAZE=z88dk c64maze.c ports/z88dk.c

// ZX Spectrum
// zcc +zx -lndos -create-app -DSZ=7 -DGFXSCALEX=4/5 -DGFXSCALEY=4/5 -DNO_SOUND -lm -DPLATFORM_MAZE=z88dk c64maze.c ports/z88dk.c

// TS 2068
// zcc +ts2068 -pragma-define:CLIB_DEFAULT_SCREEN_MODE=0x3e -lm -create-app -DPLATFORM_MAZE=z88dk -DSZ=7 -DGFXSCALEX=5/6 -DGFXSCALEY=5/6 -DNO_SOUND c64maze.c ports/z88dk.c

// TRS80 Model4 (Montezuma CP/M, Graphyx Solution High Resolution Board)
// zcc +cpm -subtype=montezuma -lm -create-app -DPLATFORM_MAZE=z88dk -DSZ=8 -DGFXSCALEX=2 -DGFXSCALEY=1 -DNO_SOUND -lgrafyx4 -DUSE_SPRITES c64maze.c ports/z88dk.c

// Commodore 128
//  zcc +c128 -lgfx128hr -lm -create-app -DPLATFORM_MAZE=z88dk -DSZ=7 -DGFXSCALEX=9/4 -DGFXSCALEY=1 c64maze.c ports/z88dk.c

// Microbee, 640px Ultra high resolution  (add -subtype=microbee40 or microbee80 for Foppy Disk image file)
// zcc +cpm -lm -create-app -DPLATFORM_MAZE=z88dk -DSZ=8 -DGFXSCALEX=2 -DGFXSCALEY=1 -DNO_SOUND -lgfxbee640 c64maze.c ports/z88dk.c

// Microbee, 512px high resolution
// zcc +cpm -lm -create-app -DPLATFORM_MAZE=z88dk -DSZ=7 -DGFXSCALEX=7/4 -DGFXSCALEY=1 -DNO_SOUND -lgfxbee512 c64maze.c ports/z88dk.c

// Microbee, 320px high resolution
//  zcc +cpm -lm -create-app -DPLATFORM_MAZE=z88dk -DSZ=8 -DGFXSCALEX=1 -DGFXSCALEY=1 -DNO_SOUND -lgfxbee320 c64maze.c ports/z88dk.c



#include <stdio.h>
#include <graphics.h>
#include <games.h>
#include <time.h>
#include <conio.h>
#include <ctype.h>
#include <bgi.h>

//#define	clear_area(a,b,c,d)	stencil_init(bgi_stencil);stencil_add_side((a),(b),(a),(d),bgi_stencil);stencil_add_side((c),(b),(c),(d),bgi_stencil);stencil_render(bgi_stencil,0)

//#define	clear_area(a,b,c,d) clga((a)*GFXSCALEX,(b)*GFXSCALEY,((c)-(a))*GFXSCALEX,((d)-(b))*GFXSCALEY)

#include "z88dk.h"
#include "../c64maze.h"
#include <stdbool.h>

#ifdef __C128__
#include <c128/cia.h>
#include <c128/sid.h>
// 4 bytes: Hrs, Min, Sec, Ten
unsigned char appTOD[4];
#endif


/* local funcs defs */
static void port_process_voice(unsigned char **ptr, unsigned char *sid_pointer,
    unsigned char *wsh);

/* funcs */
void port_pset(unsigned int x, unsigned int y)
{
    plot(x, y);
}

void port_clearHGRpage(void)
{
	clg();
}

#ifdef USE_SPRITES
char square[]={8,8,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
#endif

void port_clearMazeRegion(void)
{
	// in Turbo C you should use the histogram bar() element
	//clear_area(0,0,disp_bounds.labyrinthx,disp_bounds.labyrinthy);

	int x,y,maxx,maxy;

	maxx=disp_bounds.labyrinthx*GFXSCALEX;
	maxy=disp_bounds.labyrinthy*GFXSCALEY;

#ifdef USE_SPRITES
	for (x=0;x<maxx;x+=8)
		for (y=0;y<maxy;y+=8)
			putsprite (spr_and,x,y,square);
#else

//	for (x=0;x<maxx/2;x++)
//		for (y=0;y<maxy/2;y++)
//			undrawb (x,y,maxx-x,maxy-y);
		
	clga(0,0,maxx,maxy);

//	stencil_init(bgi_stencil);
//	stencil_add_side(0,0,0,maxy,bgi_stencil);
//	stencil_add_side(maxx,0,maxx,maxy,bgi_stencil);
//	stencil_render(bgi_stencil,0);
#endif
}

/** Switch on the HGR monochrome graphic mode.
*/
int port_graphics_init(void)
{
    initgraph(0,0,0);
	
    if(getmaxx() < getmaxy()) {
        disp_bounds.szx = getmaxx();
        disp_bounds.szy = getmaxx() * SIZEY / SIZEX;
        /* calculate labyrinth size */
        disp_bounds.labyrinthx = getmaxx() * 2 / 3;
        disp_bounds.labyrinthy = disp_bounds.labyrinthx / SIZEX * SIZEY;
    } else {
        disp_bounds.szy = getmaxy();
        disp_bounds.szx = getmaxy() * SIZEX / SIZEY;
        disp_bounds.labyrinthy = getmaxy() * 2 / 3;
        disp_bounds.labyrinthx = disp_bounds.labyrinthy * SIZEY / SIZEX;
    }
    disp_bounds.stepszx = disp_bounds.labyrinthx * STEPSIZEX / SIZEX;
    disp_bounds.stepszy = disp_bounds.labyrinthx * STEPSIZEY / SIZEX;

#ifdef __C128__
  // An initial timer setup kicks it off
  settodcia(cia2,appTOD);
#endif

}

void port_vert_line(unsigned short x1, unsigned short y1, unsigned short y2)
{
    line(x1,y1,x1,y2);
}

void port_diag_line(unsigned short x1, unsigned short y1, unsigned short ix,
    short incx, short incy)
{
    //putpixel(x1,y1,1);
	line(x1, y1, x1+incx, y1+incy);
}

void port_hor_line(unsigned short x1, unsigned short x2, unsigned short y1)
{
    line(x1, y1, x2, y1);
}

void port_printat(unsigned short x, unsigned short y, char *s)
{
//	gotoxy(x,y);
//	printf("%s",s);
	outtextxy(x,y,s);

}

void port_line(unsigned short x1, unsigned short y1,
        unsigned short x2, unsigned short y2)
{
    line(x1, y1, x2, y2);
}

unsigned long port_get_time(void)
{
	#ifdef __C128__
		gettodcia(cia2,appTOD);
		// 4 bytes: Hrs, Min, Sec, Ten
		return (appTOD[2]+60*appTOD[1]+3600*appTOD[0]);
	#else
		return time(NULL);
	#endif
}

void port_colour_banner(void)
{
}

long port_get_current_time(void)
{
	#ifdef __C128__
		gettodcia(cia2,appTOD);
		return (appTOD[2]+60*appTOD[1]+3600*appTOD[0]);
	#else
		return time(NULL);
	#endif
}

static void port_process_voice(unsigned char **ptr, unsigned char *sid_pointer,
    unsigned char *wsh)
{
    /* TODO */
}
unsigned char port_sound_irq(void)
{
    /* TODO */
    return 0;
}

void port_start_sound(unsigned char *l1, unsigned char *l2, unsigned char *l3)
{

	#ifdef __C128__
	volumesid(15,0);
	
	//setintctrlcia(cia2,ciaClearIcr); /* disable all cia 2 interrupts */
	#endif

    /* TODO */
//    ptr1=list1=l1;
//    ptr2=list2=l2;
//    ptr3=list3=l3;
//    cnt=0;
//
//    SEI();
//    set_irq(&port_sound_irq, stackSize, STACK_SIZE);
//    CLI();
	
}

void port_loadVICFont(unsigned char magnification)
{
	settextstyle(DEFAULT_FONT, HORIZ_DIR, magnification*11);
	
}

char port_getch(void)
{
    return tolower(fgetc_cons());
}

void port_fflushMazeRegion(void)
{
}
void port_music_on(void){
	#ifdef __C128__
	volumesid(0,0);
	#endif
}

void port_music_off(void){
	#ifdef __C128__
	volumesid(15,0);
	#endif
}

void port_font_magnification(unsigned char magnification)
{
//    (void)magnification;
}
void port_exit(void)
{

	exit(0);
}
