#include<6502.h>
#include<conio.h>
#include <c64maze.h>
#include"vic_font.h"
#include"sid_tune.h"
#include"C64.h"
/* local definitions */
#define VIC_II_START 53248U
#define VIC_II_Y_SCROLL 53265U
#define     BMM 32  // Monochromatic high resolution mode
#define VIC_II_SCREEN_CHAR 53272U
#define SCREEN_BORDER 53280U

#define BASE 0xA000U
#define COLOR_MEM 0x8C00U


#define POKE(addr,val)     (*(unsigned char*) (addr) = (val))
#define PEEK(addr)         (*(unsigned char*) (addr))
/* local vars */
static unsigned char pix_pos[]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
unsigned int cnt;
#define STACK_SIZE 256
unsigned char stackSize[STACK_SIZE];

unsigned char *list1;
unsigned char *ptr1;
unsigned char wsh1;

unsigned char *list2;
unsigned char *ptr2;
unsigned char wsh2;

unsigned char *list3;
unsigned char *ptr3;
unsigned char wsh3;


struct font{
    unsigned char *pDesc;   /* Pointer to the character raster array */
    unsigned int pos[256]; /* Position of the symbol */
    unsigned char incX;     /* Increment in X (-1 means a proportional font) */
    unsigned char incY;     /* Increment in Y */
    unsigned char magnification;  /* Magnification */
};

struct font f;

/* local funcs defs */
static void port_process_voice(unsigned char **ptr, unsigned char *sid_pointer,
    unsigned char *wsh);
/* funcs */
void port_pset(unsigned int x, unsigned int y)
{
    static unsigned int d;
    static unsigned int e;
    static unsigned int by;
    d=y&0xFFF8;
    e=d*40;
    by=BASE+e+(x&0xFFF8)+((unsigned char)y&7);
    POKE(by, PEEK(by) | pix_pos[(unsigned char)x&7]);
}

void port_clearHGRpage(void)
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
    asm("loop:  sta $A000,y");
    asm("       sta $A100,y");
    asm("       sta $A200,y");
    asm("       sta $A300,y");
    asm("       sta $A400,y");
    asm("       sta $A500,y");
    asm("       sta $A600,y");
    asm("       sta $A700,y");
    asm("       sta $A800,y");
    asm("       sta $A900,y");
    asm("       sta $AA00,y");
    asm("       sta $AB00,y");
    asm("       sta $AC00,y");
    asm("       sta $AD00,y");
    asm("       sta $AE00,y");
    asm("       sta $AF00,y");
    asm("       sta $B000,y");
    asm("       sta $B100,y");
    asm("       sta $B200,y");
    asm("       sta $B300,y");
    asm("       sta $B400,y");
    asm("       sta $B500,y");
    asm("       sta $B600,y");
    asm("       sta $B700,y");
    asm("       sta $B800,y");
    asm("       sta $B900,y");
    asm("       sta $BA00,y");
    asm("       sta $BB00,y");
    asm("       sta $BC00,y");
    asm("       sta $BD00,y");
    asm("       sta $BE00,y");
    asm("       sta $BE40,y");
    asm("       iny");
    asm("       bne loop");

    // Setup color memory
    /*for(i=COLOR_MEM;i<COLOR_MEM+1000;++i)
        POKE(i,3);*/
    asm("       ldy #0");
    asm("       lda #3");
    asm("loop1: sta $8C00,y");
    asm("       sta $8D00,y");
    asm("       sta $8E00,y");
    asm("       sta $8F00,y");
    asm("       iny");
    asm("       bne loop1");
}

void port_clearMazeRegion(void)
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
    asm("loop2:  sta $A000,y");
    asm("       sta $A000+320,y");
    asm("       sta $A000+2*320,y");
    asm("       sta $A000+3*320,y");
    asm("       sta $A000+4*320,y");
    asm("       sta $A000+5*320,y");
    asm("       sta $A000+6*320,y");
    asm("       sta $A000+7*320,y");
    asm("       sta $A000+8*320,y");
    asm("       sta $A000+9*320,y");
    asm("       sta $A000+10*320,y");
    asm("       sta $A000+11*320,y");
    asm("       sta $A000+12*320,y");
    asm("       sta $A000+13*320,y");
    asm("       sta $A000+14*320,y");
    asm("       sta $A000+15*320,y");
    asm("       sta $A000+16*320,y");
    asm("       sta $A000+17*320,y");
    asm("       sta $A000+18*320,y");
    asm("       sta $A000+19*320,y");
    asm("       sta $A000+20*320,y");
    asm("       sta $A000+21*320,y");
    asm("       sta $A000+22*320,y");
    asm("       sta $A000+23*320,y");
    asm("       sta $A000+24*320,y");
    asm("       sta $A000+25*320,y");
    asm("       iny");
    asm("       cpy #200");
    asm("       bne loop2");

}

/** Switch on the HGR monochrome graphic mode.
*/
void port_graphics_init(void)
{
    POKE (56576U, 0x01);
    POKE (53272U, 0x38);
    POKE (53265U, 0x36);
    port_clearHGRpage();
}

void port_vert_line(unsigned short x1, unsigned short y1, unsigned short y2)
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

void port_diag_line(unsigned short x1, unsigned short y1, unsigned short ix,
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

void port_hor_line(unsigned short x1, unsigned short x2, unsigned short y1)
{
    static unsigned int d;
    static unsigned int e;
    static unsigned int by;
    d=y1&0xFFF8;
    e=d*40;
    by=BASE+e+((unsigned char)y1&7);
    for(;x1<=x2;++x1){
        if((x1&7)==0 && (x1+7)<=x2) {
            POKE(by+(x1&0xFFF8),0xFF);
            x1+=7; // because the for statement will add 1.
            continue;
        }
        POKE(by+(x1&0xFFF8), PEEK(by+(x1&0xFFF8)) |
            pix_pos[(unsigned char)x1&7]);
    }
}

void port_printat(unsigned short x, unsigned short y, char *s)
{
    unsigned char i,j,k;
    unsigned char a;
    unsigned char t;
    unsigned int p;
    unsigned char mm=f.magnification;
    unsigned char incrementx=f.incX*mm;
    unsigned char incrementy=f.incY*mm;
    unsigned int ppos;
    unsigned char q,r;

    unsigned int by;
    unsigned int d,e;
    unsigned int ix;
    unsigned int loc;
    for (i=0; s[i]!='\0';++i) {
        p=0;
        r=1;
        ppos=f.pos[s[i]];
        x+=incrementx;
        for (j=0;j<incrementy;++j) {
            a=f.pDesc[ppos+p];
            if(r==mm){
                ++p;
                r=1;
            } else {
                ++r;
            }
            t=1;
            q=y+j;
            d=q&0xFFF8;
            e=d*40;
            by=BASE+e+((unsigned char)q&7);
            for(k=0;a!=0;++k){
                if (a & 0x0001) {
                    ix=x-k;
                    loc=by+(ix&0xFFF8);
                    POKE(loc, PEEK(loc) | pix_pos[(unsigned char)ix&7]);
                } if(t==mm){
                    a>>=1;
                    t=1;
                } else {
                    ++t;
                }
            }
        }
    }
}

/* Plot a line using the Bresenham algorithm
   from Nelson Johnson, "Advanced Graphics in C"
   ed. Osborne, McGraw-Hill.
   Horizontal and vertical lines need to be considerably
   speeded up by using a direct access to video RAM.
*/
void port_line(unsigned short x1, unsigned short y1,
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
    static unsigned char style_mask;

    incx=x2>x1?1:-1;
    incy=y2>y1?1:-1;

    ix=incx>0?x2-x1:x1-x2;
    iy=incy>0?y2-y1:y1-y2;

    // If continuous lines have to be drawn, check if we can employ simplified
    // and faster code in certain particular cases.
    if(style==0x1) {
        if(ix==0) {
            if(incy>0)
                port_vert_line(x1, y1,y2);
            else
                port_vert_line(x1, y2,y1);
            return;
        }
        if(iy==0) {
            if(incx>0)
                port_hor_line(x1,x2,y1);
            else
                port_hor_line(x2,x1,y1);
            return;
        }
        if(ix==iy) {
            port_diag_line(x1,y1,ix,incx,incy);
            return;
        }
    }

    inc=a_max(ix, iy);
    style_mask=style;

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
        if (plot && (style_mask&0x0001u)) {
            plot=FALSE;
            if(changey==TRUE) {
                /* Calculations for the position of the memory location to
                   modify are more complicated in y than in x, so it is
                   worth doing them only when necessary. That greatly
                   improves speed for almost horisontal lines. */
                changey=FALSE;
                d=y1&0xFFF8;
                e=d*40;
                ypos=BASE+e+((unsigned char)y1&0x07);
            }
            by=ypos+(x1&0xFFF8);
            POKE(by, PEEK(by) | pix_pos[(unsigned char)x1&0x07]);
        }
        style_mask >>= 1;
        if(style_mask==0) style_mask=style;
    }
}

unsigned long port_get_time(void)
{
    return PEEK(160)+PEEK(161)*256;
}

void port_colour_banner(void)
{
    unsigned char x;
    unsigned char y;

    for(x=25;x<40;++x) {
        for(y=0;y<25;++y) {
            POKE(COLOR_MEM+x+y*40,0x67);
        }
    }
}

long port_get_current_time(void)
{
    long ctime=((char)PEEK(160));
    ctime<<=8;
    ctime+=((char)PEEK(161));
    ctime<<=8;
    ctime+=((char)PEEK(162));
    return ctime;
}

static void port_process_voice(unsigned char **ptr, unsigned char *sid_pointer,
    unsigned char *wsh)
{
    unsigned int timestamp=**ptr+ (*(*ptr+1)<<8);
    if(timestamp>cnt) {
        return;
    }
    ++*ptr;
    ++*ptr;
    switch(**ptr) {
        case NOTE_CODE:                 // Note event
            POKE(sid_pointer,*(++*ptr));        // Frequency lo
            POKE(sid_pointer+1,*(++*ptr));      // Frequency hi
            POKE(sid_pointer+4,0);
            POKE(sid_pointer+4,*wsh);          // Note on
            ++*ptr;
            break;
        case PAUSE_CODE:                // Pause event
            POKE(sid_pointer+4,0);             // Note off
            ++*ptr;
            break;
        case ENVELOPE_CODE:             // Change envelope
            *wsh=*(++*ptr);             // Wave shape
            POKE(sid_pointer+2,*(++*ptr));     // Duty cycle lo
            POKE(sid_pointer+3,*(++*ptr));     // Dyty cycle hi
            POKE(sid_pointer+5,*(++*ptr));     // AD
            POKE(sid_pointer+6,*(++*ptr));     // SR
            ++*ptr;
            break;
        case REWIND_CODE:
            ptr1=list1;
            ptr2=list2;
            ptr3=list3;
            cnt=0;
            break;
    }
}
unsigned char port_sound_irq(void)
{
    if(ptr1!=NULL) {
        port_process_voice(&ptr1, (unsigned char*)0xD400U, &wsh1);
    }
    if(ptr2!=NULL) {
        port_process_voice(&ptr2, (unsigned char*)0xD407U, &wsh2);
    }
    if(ptr3!=NULL) {
        port_process_voice(&ptr3, (unsigned char*)0xD40EU, &wsh3);
    }
    ++cnt;
    return (IRQ_NOT_HANDLED);
}

void port_start_sound(unsigned char *l1, unsigned char *l2, unsigned char *l3)
{
    POKE(0xD418,15);
    ptr1=list1=l1;
    ptr2=list2=l2;
    ptr3=list3=l3;
    cnt=0;

    SEI();
    set_irq(&port_sound_irq, stackSize, STACK_SIZE);
    CLI();
}

void port_loadVICFont(unsigned char magnification)
{
    unsigned int i;
    /* Load the font tables */
    for(i=0; i<256; ++i)
        f.pos[i]=0;

    for(i=' '; i<='~'; ++i)
        f.pos[i]=(i-' '+1)*8;

    f.pDesc=vic_font;
    f.incX=8;    /* Increment in X (-1 would mean a proportional font) */
    f.incY=8;    /* Increment in Y */
    f.magnification=magnification;
}

void port_font_magnification(unsigned char magnification)
{
    f.magnification = magnification;
}

char port_getch(void)
{
    return cgetc();
}

void port_fflushMazeRegion(void)
{

}

void port_music_off(void)
{
    POKE(0xD418,0);
}
void port_music_on(void)
{ 
    POKE(0xD418,15);
}
