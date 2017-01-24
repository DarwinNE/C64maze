#include<conio.h>
#include"vic_font.h"

typedef unsigned char byte;

#define POKE(addr,val)     (*(unsigned char*) (addr) = (val))
#define PEEK(addr)         (*(unsigned char*) (addr))

#define VIC_II_START 53248U
#define VIC_II_Y_SCROLL 53265U
#define     BMM 32  // Monochromatic high resolution mode
#define VIC_II_SCREEN_CHAR 53272U

#define BASE 0x6000U
#define COLOR_MEM 0x4400U

#define a_abs(a) ((a)>0 ? (a):(-a))
#define a_max(a,b) (((a)>(b))?(a):(b))
#define a_sign(a) ((a)>0?1:((a)==0?0:(-1)))
#define TRUE  0xFF
#define FALSE 0x00

#define SIZEX 200
#define SIZEY 199
#define STEPSIZEX  15
#define STEPSIZEY  15


static unsigned char pix_pos[]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

char labyrinth[] =  "****************************************"
                    "*      *    *     * *   *        *     *"
                    "* **** * ******** * *** * ****** * ** **"
                    "* * *  *   *  *   *     *   * *  *  *  *"
                    "*   * ** ** * *** ***** *** * * *** ** *"
                    "***** ***** *            ****** * * ****"
                    "*             * ** **** * *     *  *   *"
                    "* * *********** *     * * * ****** * * *"
                    "* *         *    * ** *   * *     *  * *"
                    "***** ***** * * *  *  * * * **** *  *  *"
                    "* *       * ******** ** **     * ** ****"
                    "* * *** * * *     *  *    * **** *     *"
                    "*     *** * ***** * ** **** *  * **** **"
                    "** ** *   *     * * *   *   * **    *  *"
                    "*  * ** ******* * * *** * *** *  ** ** *"
                    "**   *  *          *     *     * *      *"
                    "****************************************";
unsigned char labyrinthSizeX=40;
unsigned char labyrinthSizeY=17;
char startx=6;
char starty=16;
char positionx=6;
char positiony=16;
unsigned int style=0x1;

char exitx=13;
char exity=1;
    /*  0 = north
        1 = west
        2 = sud
        3 = east
    */
char orientation=0;

struct font{
    unsigned char *pDesc;   /* Pointer to the character raster array */
    unsigned int  pos[256]; /* Position of the symbol */
    unsigned char incX;     /* Increment in X (-1 means a proportional font) */
    unsigned char incY;     /* Increment in Y */
    unsigned char magnification;  /* Magnification */
};

struct font f;

/* Turn on a pixel by accessing directly to the video RAM */
void pset(unsigned int x, unsigned int y)
{
    static unsigned int d;
    static unsigned int e;
    static unsigned int by;
    d=y&0xFFF8;
    e=d*40;
    by=BASE+e+(x&0xFFF8)+((unsigned char)y&7);
    POKE(by, PEEK(by) | pix_pos[(unsigned char)x&7]);
}

void clearHGRpage(void)
{
    //unsigned int i;
    // Clear HGR page (too slow!)
    /*for(i=BASE;i<BASE+8000;++i)
        POKE(i,0);*/
    /* The fastest version you can possibly imagine, with an unrolled loop .
       Do not forget to change the addresses if you need to modify the base
       address of the HGR page */
    asm("       ldy #0");
    asm("       tya");
    asm("loop:  sta $6000,y"); 
    asm("       sta $6100,y");
    asm("       sta $6200,y");
    asm("       sta $6300,y");
    asm("       sta $6400,y");
    asm("       sta $6500,y");
    asm("       sta $6600,y");
    asm("       sta $6700,y");
    asm("       sta $6800,y");
    asm("       sta $6900,y");
    asm("       sta $6A00,y");
    asm("       sta $6B00,y");
    asm("       sta $6C00,y");
    asm("       sta $6D00,y");
    asm("       sta $6E00,y");
    asm("       sta $6F00,y");
    asm("       sta $7000,y");
    asm("       sta $7100,y");
    asm("       sta $7200,y");
    asm("       sta $7300,y");
    asm("       sta $7400,y");
    asm("       sta $7500,y");
    asm("       sta $7600,y");
    asm("       sta $7700,y");
    asm("       sta $7800,y");
    asm("       sta $7900,y");
    asm("       sta $7A00,y");
    asm("       sta $7B00,y");
    asm("       sta $7C00,y");
    asm("       sta $7D00,y");
    asm("       sta $7E00,y");
    asm("       sta $7F00,y");
    asm("       iny");
    asm("       bne loop");
    
    // Setup color memory
    /*for(i=COLOR_MEM;i<COLOR_MEM+1000;++i)
        POKE(i,3);*/
    asm("       ldy #0");
    asm("       lda #3");
    asm("loop1: sta $4400,y"); 
    asm("       sta $4500,y");
    asm("       sta $4600,y");
    asm("       sta $4700,y");
    asm("       iny");
    asm("       bne loop1");
}
/* printat prints a text at the given location unsing the font described
   by the structure font */
void printat(unsigned short x, unsigned short y, char *s)
{
    unsigned short i,j,k;
    unsigned char a;
    unsigned char t;
    unsigned char mm=f.magnification;
    unsigned char incrementx=f.incX*mm;
    unsigned char incrementy=f.incX*mm;
    

    for (i=0; s[i]!='\0';++i) {
        for (j=0;j<incrementy;++j) {
            a=f.pDesc[f.pos[s[i]]+j/mm];
            t=1;
            for(k=0;a!=0;++k){
                if (a & 0x0001)
                    pset(x+incrementx-k, y+j);
                if(t==mm){
                    a>>=1;
                    t=1;
                } else {
                    ++t;
                }
            }
        }
        x+=incrementx;
    }
}

void clearMazeRegion(void)
{
    /*unsigned char x;
    unsigned char y;
    unsigned int by=BASE;
    // Clear the leftmost part of the screen
    
    for(y=0;y<25;++y) {
        for(x=0;x<200;++x)
            POKE(by++,0);
        by+=120;
    }*/
    // Very fast version of the routine.
    asm("       ldy #0");
    asm("       tya");
    asm("loop2:  sta $6000,y"); 
    asm("       sta $6000+320,y");
    asm("       sta $6000+2*320,y");
    asm("       sta $6000+3*320,y");
    asm("       sta $6000+4*320,y");
    asm("       sta $6000+5*320,y");
    asm("       sta $6000+6*320,y");
    asm("       sta $6000+7*320,y");
    asm("       sta $6000+8*320,y");
    asm("       sta $6000+9*320,y");
    asm("       sta $6000+10*320,y");
    asm("       sta $6000+11*320,y");
    asm("       sta $6000+12*320,y");
    asm("       sta $6000+13*320,y");
    asm("       sta $6000+14*320,y");
    asm("       sta $6000+15*320,y");
    asm("       sta $6000+16*320,y");
    asm("       sta $6000+17*320,y");
    asm("       sta $6000+18*320,y");
    asm("       sta $6000+19*320,y");
    asm("       sta $6000+20*320,y");
    asm("       sta $6000+21*320,y");
    asm("       sta $6000+22*320,y");
    asm("       sta $6000+23*320,y");
    asm("       sta $6000+24*320,y");
    asm("       sta $6000+25*320,y");
    asm("       iny");
    asm("       cpy #200");
    asm("       bne loop2");
    
}

/** Switch on the HGR monochrome graphic mode.
*/
void graphics_monochrome(void)
{
    POKE(56576U,(PEEK(56576U) & 0xFC)|2);    // Second bank for the VIC II
    POKE(VIC_II_SCREEN_CHAR,(PEEK(VIC_II_SCREEN_CHAR)&240)|12);
    POKE(VIC_II_Y_SCROLL,   PEEK(VIC_II_Y_SCROLL)|BMM);     // HGR on
    clearHGRpage();
}

void vert_line(unsigned short x1, unsigned short y1, unsigned short y2)
{
    static unsigned int d;
    static unsigned int e;
    static unsigned int by;
    static char v;
    static unsigned int cc;

    cc=BASE+(x1&0xFFF8);
    v=pix_pos[(unsigned char)x1&7];
    for(;y1<=y2;++y1){
        if((y1&7)==0 && (y1+8)<=y2) {
            d=y1&0xFFF8;
            e=d*40;
            by=cc+e;
            POKE(by, PEEK(by++) | v);
            POKE(by, PEEK(by++) | v);
            POKE(by, PEEK(by++) | v);
            POKE(by, PEEK(by++) | v);
            POKE(by, PEEK(by++) | v);
            POKE(by, PEEK(by++) | v);
            POKE(by, PEEK(by++) | v);
            POKE(by, PEEK(by++) | v);
            y1+=7; // because the for statement will add 1.
            continue;
        }
        d=y1&0xFFF8;
        e=d*40;
        by=cc+e+((unsigned char)y1&7);
        POKE(by, PEEK(by) | v);
    }
}

void diag_line(unsigned short x1, unsigned short y1, unsigned short ix,
    short incx, short incy)
{
    static unsigned int d;
    static unsigned int e;
    static unsigned int by;
    static int i;
    for(i=0;i<=ix;++i){
        d=y1&0xFFF8;
        e=d*40;
        by=BASE+e+(x1&0xFFF8)+((unsigned char)y1&7);
        POKE(by, PEEK(by) | pix_pos[(unsigned char)x1&7]);
        if(incx>0)
            ++x1;
        else
            --x1;
        if(incy>0)
            ++y1;
        else
            --y1;
    }
}

void hor_line(unsigned short x1, unsigned short x2, unsigned short y1)
{
    static unsigned int d;
    static unsigned int e;
    static unsigned int by;
    d=y1&0xFFF8;
    e=d*40;
    by=BASE+e+((unsigned char)y1&7);
    for(;x1<=x2;++x1){
        if((x1&7)==0 && (x1+8)<=x2) {
            POKE(by+(x1&0xFFF8),0xFF);
            x1+=7; // because the for statement will add 1.
            continue;
        }
        POKE(by+(x1&0xFFF8), PEEK(by+(x1&0xFFF8)) | 
            pix_pos[(unsigned char)x1&7]);
    }
}
/* Plot a line using the Bresenham algorithm
   from Nelson Johnson, "Advanced Graphics in C"
   ed. Osborne, McGraw-Hill.
   Horizontal and vertical lines need to be considerably
   speeded up by using a direct access to video RAM.
*/
void line_br(unsigned short x1, unsigned short y1,
        unsigned short x2, unsigned short y2)
{
    static short dx;
    static short dy;
    static short incx;
    static short incy;
    static unsigned short ix;
    static unsigned short iy;
    static unsigned short inc;

    static unsigned short plotx;
    static unsigned short ploty;
    static unsigned char changey;
    static short x, y;
    static unsigned short style_mask;
    static unsigned char plot;

    static unsigned char d;
    static unsigned int e;
    static unsigned int by;
    static unsigned int ypos;
    static unsigned short i;
    
    dx=x2-x1;
    dy=y2-y1;
    incx=a_sign(dx);
    incy=a_sign(dy);
    ix=a_abs(dx);
    iy=a_abs(dy);
    inc=a_max(ix, iy);
    plotx=x1;
    ploty=y1;
    changey=TRUE;
    x=0;
    y=0;
    style_mask=style;
    plot=FALSE;
    d=y&0xFFF8;
    e=d*40;
    by=BASE+e+(x&0xFFF8)+((unsigned char)y&7);
    

    /* Plot the first pixel */
    POKE(by, PEEK(by) | pix_pos[(unsigned char)x&7]);

    /* Slower version with line styles */
    for(i=0; i<=inc; ++i) {
        x += ix;
        y += iy;
        if (x>inc) {
            plot=TRUE;
            x-=inc;
            plotx +=incx;
        }
        if (y>inc) {
            plot=TRUE;
            y-=inc;
            ploty +=incy;
            changey=TRUE;
        }
        if (plot && style_mask & 0x001) {
            plot=FALSE;
            if(changey==TRUE) {
                /* Calculations for the position of the memory location to
                   modify are more complicated in y than in x, so it is
                   worth doing them only when necessary. That greatly
                   improves speed for horisontal lines. */
                changey=FALSE;
                d=ploty&0xFFF8;
                e=d*40;
                ypos=BASE+e+((unsigned char)ploty&0x07);
            }
            by=ypos+(plotx&0xFFF8);
            POKE(by, PEEK(by) | pix_pos[(unsigned char)plotx&0x07]);
        }
        style_mask >>= 1;
        if(style_mask==0) style_mask=style;
    }
}
/* Plot a line using the Bresenham algorithm
   from Nelson Johnson, "Advanced Graphics in C"
   ed. Osborne, McGraw-Hill.
   Horizontal and vertical lines need to be considerably
   speeded up by using a direct access to video RAM.
*/
void line(unsigned short x1, unsigned short y1,
        unsigned short x2, unsigned short y2)
{
    static short incx;
    static short incy;
    
    static unsigned short ix;
    static unsigned short iy;
    static unsigned short inc;

    static unsigned char changey;
    static short x, y;
    static unsigned char plot;

    static unsigned int d;
    static unsigned int e;
    static unsigned int by;
    static unsigned int ypos;
    static unsigned int i;

    if(style!=0x1) {
        line_br(x1,y1,x2,y2);
        return;
    }

    incx=x2>x1?1:-1;
    incy=y2>y1?1:-1;
    
    ix=((x2-x1)*incx);
    iy=((y2-y1)*incy);

    if(ix==0) {
        if(y1<y2)
            vert_line(x1, y1,y2);
        else
            vert_line(x1, y2,y1);
        return;
    }
    if(iy==0) {
        if(x1<x2)
            hor_line(x1,x2,y1);
        else
            hor_line(x2,x1,y1);
        return;
    }
    if(ix==iy) {
        diag_line(x1,y1,ix,incx,incy);
        return;
    }

    inc=a_max(ix, iy);

    changey=TRUE;
    x=0;
    y=0;
    plot=FALSE;

    d=y1&0xFFF8;
    e=d*40;
    by=BASE+e+(x1&0xFFF8)+((unsigned char)y1&7);


    /* Plot the first pixel */
    POKE(by, PEEK(by) | pix_pos[(unsigned char)x1&7]);

    /* Faster version with continuous line */
    for(i=0; i<=inc; ++i) {
        x += ix;
        y += iy;
        if (x>inc) {
            plot=TRUE;
            x-=inc;
            x1 +=incx;
        }
        if (y>inc) {
            plot=TRUE;
            y-=inc;
            y1 +=incy;
            changey=TRUE;
        }
        if (plot) {
            plot=FALSE;
            if(changey==TRUE) {
                /* Calculations for the position of the memory location to
                   modify are more complicated in y than in x, so it is 
                   worth doing them only when necessary. That greatly 
                   improves speed for horisontal lines. */
                changey=FALSE;
                d=y1&0xFFF8;
                e=d*40;
                ypos=BASE+e+((unsigned char)y1&0x07);
            }
            by=ypos+(x1&0xFFF8);
            POKE(by, PEEK(by) | pix_pos[(unsigned char)x1&0x07]);
        }
    }
}


/* box draws a box given the coordinates of the two
   diagonal corners. From Nelson Johnson, "Advanced
   Graphics in C" ed. Osborne, McGraw-Hill
 */
void box(unsigned short x1, unsigned short y1, unsigned short x2,
    unsigned short y2)
{
    static unsigned short xul;
    static unsigned short yul;
    static unsigned short xlr;
    static unsigned short ylr;

    xul=(x2>x1) ? x1 : x2;
    yul=(y2>y1) ? y1 : y2;
    xlr=(x2>x1) ? x2 : x1;
    ylr=(y2>y1) ? y2 : y1;
    if(style==0x1) {
        hor_line(xul, xlr, yul);
        hor_line(xul, xlr, ylr);
        vert_line(xul, yul, ylr);
        vert_line(xlr, yul, ylr);
    } else {
        line_br(xul, yul, xlr, yul);
        line_br(xlr, yul, xlr, ylr);
        line_br(xlr, ylr, xul, ylr);
        line_br(xul, ylr, xul, yul);
    }
}

void loadVICFont(unsigned char magnification)
{
    unsigned int i;
    /* Load the font tables */
    for(i=0; i<256; ++i)
        f.pos[i]=0;

    for(i=' '; i<='~'; ++i)
        f.pos[i]=(i-' '+1)*8;

    f.pDesc=vic_font;
    f.incX=8;     /* Increment in X (-1 would mean a proportional font) */
    f.incY=8;    /* Increment in Y */
    f.magnification=magnification;
}

int leftx;
int lefty;
int rightx;
int righty;
int advancex;
int advancey;

void set_orientation(void)
{
    switch(orientation) {
        case 0:
            leftx=-1;
            lefty=0;
            rightx=1;
            righty=0;
            advancex=0;
            advancey=-1;
            break;
        case 1:
            leftx=0;
            lefty=1;
            rightx=0;
            righty=-1;
            advancex=-1;
            advancey=0;
            break;
        case 2:
            leftx=1;
            lefty=0;
            rightx=-1;
            righty=0;
            advancex=0;
            advancey=1;
            break;
        case 3:
            leftx=0;
            lefty=-1;
            rightx=0;
            righty=1;
            advancex=1;
            advancey=0;
            break;
    }
}

void drawLabyrinthView()
{
    unsigned char posx=positionx;
    unsigned char posy=positiony;
    unsigned char step=0;
    unsigned char wall=FALSE;
    unsigned char wayout=FALSE;
    
    style=0x1;
    set_orientation();
    for(step=0;(wall==FALSE)&&(step<6);++step) {
        switch(step) {
            case 3:
                style=0x2;
                break;
            case 4:
                style=0x8;
                break;
            case 5:
                style=0x20;
                break;
            default:
                style=0x1;
                break;
        }
        if(labyrinth[posx+posy*labyrinthSizeX]=='*') {
            break;
        }
        // Wall on the right?
        if(labyrinth[posx+rightx+(posy+righty)*labyrinthSizeX]=='*') {
            line(SIZEX-step*STEPSIZEX,step*STEPSIZEY,
                 SIZEX-(step+1)*STEPSIZEX,(step+1)*STEPSIZEY);
            line(SIZEX-step*STEPSIZEX,SIZEY-step*STEPSIZEY,
                 SIZEX-(step+1)*STEPSIZEX,SIZEY-(step+1)*STEPSIZEY);
        } else {
            // Closer vertical line
            line(SIZEX-step*STEPSIZEX,step*STEPSIZEY,
                 SIZEX-step*STEPSIZEX,SIZEY-step*STEPSIZEY);
            // Farther vertical line
            if(labyrinth[posx+advancex+(posy+advancey)*labyrinthSizeX]!='*') {
                line(SIZEX-(step+1)*STEPSIZEX,(step+1)*STEPSIZEY,
                    SIZEX-(step+1)*STEPSIZEX,SIZEY-(step+1)*STEPSIZEY);
            }
            // Upper horisontal line
            line(SIZEX-(step+1)*STEPSIZEX,(step+1)*STEPSIZEY,
                 SIZEX-(step)*STEPSIZEX,(step+1)*STEPSIZEY);
            // Lower horisontal line
            line(SIZEX-(step+1)*STEPSIZEX,SIZEY-(step+1)*STEPSIZEY,
                 SIZEX-(step)*STEPSIZEX,SIZEY-(step+1)*STEPSIZEY);
        }
        // Wall on the left?
        if(labyrinth[posx+leftx+(posy+lefty)*labyrinthSizeX]=='*') {
            line(step*STEPSIZEX,step*STEPSIZEY,
                 (step+1)*STEPSIZEX,(step+1)*STEPSIZEY);
            line(step*STEPSIZEX,SIZEY-step*STEPSIZEY,
                 (step+1)*STEPSIZEX,SIZEY-(step+1)*STEPSIZEY);
        } else {
            // Closer vertical line
            line(step*STEPSIZEX,step*STEPSIZEY,
                 step*STEPSIZEX,SIZEY-step*STEPSIZEY);
            // Farter vertical line
            if(labyrinth[posx+advancex+(posy+advancey)*labyrinthSizeX]!='*') {
                line((step+1)*STEPSIZEX,(step+1)*STEPSIZEY,
                 (step+1)*STEPSIZEX,SIZEY-(step+1)*STEPSIZEY);
            }
            // Upper horisontal line
            line((step+1)*STEPSIZEX,(step+1)*STEPSIZEY,
                 (step)*STEPSIZEX,(step+1)*STEPSIZEY);
            // Lower horisontal line
            line((step+1)*STEPSIZEX,SIZEY-(step+1)*STEPSIZEY,
                 (step)*STEPSIZEX,SIZEY-(step+1)*STEPSIZEY);
        }
        posx+=advancex;
        posy+=advancey;
        // Exit is in sight!
        if (posx==exitx && posy==exity) {
            wall=TRUE;
            wayout=TRUE;
        }
        // A wall is in sight!
        if(labyrinth[posx+posy*labyrinthSizeX]=='*') {
            wall=TRUE;
        }
    }
    if(wall==TRUE) 
        box(step*STEPSIZEX,step*STEPSIZEY,
            SIZEX-step*STEPSIZEX,SIZEY-step*STEPSIZEY);
    if(wayout) {
        ++step;
        box(step*STEPSIZEX,step*STEPSIZEY,
            SIZEX-step*STEPSIZEX,SIZEY-step*STEPSIZEY);
        line(step*STEPSIZEX,step*STEPSIZEY,
            SIZEX-step*STEPSIZEX,SIZEY-step*STEPSIZEY);
        line(SIZEX-step*STEPSIZEX,step*STEPSIZEY,
            step*STEPSIZEX,SIZEY-step*STEPSIZEY);
    }
        
}
void validate_data()
{
    if(positionx>=labyrinthSizeX)
        positionx=labyrinthSizeX-1;
    if(positiony>=labyrinthSizeY)
        positiony=labyrinthSizeY-1;
    if(positiony<=0)
        positiony=0;
    if(positiony<=0)
        positiony=0;
}

void move_forward()
{
    switch(orientation) {
        case 0:
            --positiony;
            break;
        case 1:
            --positionx;
            break;
        case 2:
            ++positiony;
            break;
        case 3:
            ++positionx;
            break;
    }
    validate_data();
}

void move_backwards()
{
    switch(orientation) {
        case 0:
            ++positiony;
            break;
        case 1:
            ++positionx;
            break;
        case 2:
            --positiony;
            break;
        case 3:
            --positionx;
            break;
    }
    validate_data();
}

void colour_banner(void)
{
    unsigned char x;
    unsigned char y;
    
    for(x=25;x<40;++x) {
        for(y=0;y<25;++y) {
            POKE(COLOR_MEM+x+y*40,0x67);
        }
    }
}

void main(void)
{
    unsigned char oldx=0;
    unsigned char oldy=0;
    char oldo=0;
    char c;
    char iv=TRUE;

    graphics_monochrome();
    colour_banner();
    positionx=6;
    positiony=16;
    loadVICFont(2);
    printat(204,17,"c64maze");
    loadVICFont(1);
    box(251,53,278,84);
    printat(260,55,"t");
    printat(252,65,"f+g");
    printat(260,75,"v");
    printat(207,170,"d. bucci 2017");
    loadVICFont(2);
    line(200,0,200,199);
    while(TRUE) {
        if(oldx!=positionx || oldy!=positiony || oldo!=orientation) {
            oldx=positionx;
            oldy=positiony;
            oldo=orientation;
            clearMazeRegion();
            drawLabyrinthView();
            if (positionx==startx && positiony==starty)
                printat(40,100,"step in!");
            if (positionx==exitx && positiony==exity) {
                printat(40,100,"way out!");
            }
            POKE(53280U,4);
        }
        do {
            c=cgetc();
            iv=FALSE;
            switch(c) {
                case 't':
                    move_forward();
                    break;
                case 'v':
                    move_backwards();
                    break;
                case 'f':
                    if(orientation==3)
                        orientation=0;
                    else
                        ++orientation;
                    break;
                case 'g':
                    if(orientation==0)
                        orientation=3;
                    else
                        --orientation;
                    break;
                default:
                    iv=TRUE;
            };
        } while(iv==TRUE);
        if(labyrinth[positionx+positiony*labyrinthSizeX]=='*') {
            POKE(53280U,1);
            positionx=oldx;
            positiony=oldy;
        }
    }
}