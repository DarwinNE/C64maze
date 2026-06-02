#include<6502.h>
#include<conio.h>
#include <c64maze.h>
#include"vic_font.h"
#include"sid_tune.h"
#include"C64.h"
/* local definitions */
#define VDC_REGISTERN  0xD600
#define VDC_DATA       0xD601
#define BASE        0

typedef unsigned char byte;

byte bmp_hdr[8];
byte save_VDC[37];


void write_VDC(byte reg, byte data)
{
    (*(byte *)VDC_REGISTERN)=reg;
    asm("@wait: bit $D600");
    asm("       bpl @wait");
    (*(byte *)VDC_DATA)=data;
}

byte read_VDC(byte reg)
{
    (*(byte *)VDC_REGISTERN)=reg;
    asm("@wait: bit $D600");
    asm("       bpl @wait");
    return (*(byte *)VDC_DATA);
}

unsigned int C64_freq_table[]={
    278, 295, 313, 331, 351, 372, 394, 417, 442, 468, 496, 526, 557, 590, 625,
    662, 702, 743, 788, 834, 884, 937, 992, 1051, 1114, 1180, 1250, 1325, 1403,
    1487, 1575, 1669, 1768, 1873, 1985, 2103, 2228, 2360, 2500, 2649, 2807,
    2973, 3150, 3338, 3536, 3746, 3969, 4205, 4455, 4720, 5001, 5298, 5613,
    5947, 6300, 6675, 7072, 7493, 7938, 8410, 8910, 9440, 10001, 10596, 11226,
    11894, 12601, 13350, 14144, 14985, 15876, 16820, 17820, 18880, 20003, 21192,
    22452, 23787, 25202, 26700, 28288, 29970, 31752, 33640, 35641, 37760, 40005,
    42384, 44904, 47574, 50403, 53401, 56576, 59940, 63504}; 



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


void gr_mode(void)
{
    byte c,i;
    int iy;
    asm("lda $D011");    // Disable the VIC-II
    asm("and #$6F");
    asm("sta $D011");
    asm("lda #$01");    // Set FAST mode
    asm("sta $D030");
    
    for(i=0; i<37; ++i)
        save_VDC[i]=read_VDC(i);

    c=read_VDC(25);
    write_VDC(25, 128+(c & 0x0F));  // Set bitmap mode
    write_VDC(26,14*16+0);

    write_VDC(18,0);
    write_VDC(19,0);
    for(iy=0;iy<16383/256;++iy) {
        write_VDC(31,0);
        write_VDC(30,255);
    }
}

void back_to_txt(void)
{
    byte i;
    for(i=0;i<16383/256;++i) {
        write_VDC(31,0);
        write_VDC(30,255);
    }
    for(i=0; i<37; ++i)
        write_VDC(i,save_VDC[i]);
    asm("jsr $ce0c");
    asm("lda #$93");     // Clear the screen
    asm("jsr $ffd2");

}


/* funcs */
void port_pset(unsigned int x, unsigned int y)
{
    static unsigned int by;
    static unsigned char oep;
    static unsigned char ll,hh;
    

    //by=(x>>3)+y*80;
    by=x>>3;
    by+=y<<6;
    by+=y<<4;
    
    (*(byte *)VDC_REGISTERN)=18;
    hh=(by&0xFF00)>>8;
    (*(byte *)VDC_DATA)=hh;
    (*(byte *)VDC_REGISTERN)=19;
    ll=by&0x00FF;
    (*(byte *)VDC_DATA)=ll;
    (*(byte *)VDC_REGISTERN)=31;
    oep= (*(byte *)VDC_DATA);

    oep |= pix_pos[(unsigned char)x&7];

    (*(byte *)VDC_REGISTERN)=18;
    (*(byte *)VDC_DATA)=hh;
    (*(byte *)VDC_REGISTERN)=19;
    (*(byte *)VDC_DATA)=ll;  
    (*(byte *)VDC_REGISTERN)=31;
    (*(byte *)VDC_DATA)=oep;
}

void port_clearHGRpage(void)
{
    unsigned char x;
    unsigned char y;
    static unsigned int by;
    for(y=0;y<200;++y) {
        by=y*80;
        write_VDC(18,(by&0xFF00)>>8);    // HI
        write_VDC(19,by&0x00FF);         // LO    
        write_VDC(30, 80); // DATA
        write_VDC(31, 0); // DATA
    }
}

void port_clearMazeRegion(void)
{
    unsigned char x;
    unsigned char y;
    static unsigned int by;
    for(y=0;y<200;++y) {
        by=y*80;
        write_VDC(18,(by&0xFF00)>>8);    // HI
        write_VDC(19,by&0x00FF);         // LO    
        write_VDC(30, 400/8); // DATA
        write_VDC(31, 0); // DATA
    }
}

/** Switch on the HGR monochrome graphic mode.
*/
void port_graphics_init(void)
{
    gr_mode();
    //port_clearHGRpage();
}

void port_vert_line(unsigned short x1, unsigned short y1, unsigned short y2)
{
    static unsigned int y;

    if(y2<y1) {
        y=y1;
        y1=y2;
        y2=y;
    }

    if(x1>=640)
        return;
    if(y2>=200) y2=199;
    
    for(y=y1;y<y2;++y)
        port_pset(x1,y);
}

void port_diag_line(unsigned short x1, unsigned short y1, unsigned short ix,
    short incx, short incy)
{
    static unsigned int d;
    static unsigned int e;
    static unsigned int by;
    static int i;
    for(i=0;i<=ix;++i){
        port_pset(x1,y1);
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
    static unsigned int x;
    unsigned char i;
    signed char ttl;

    static unsigned int by;
    static unsigned char oep;
    unsigned short x1a;
    unsigned short x2a;
    
    if(x2<x1) {
        x=x1;
        x1=x2;
        x2=x;
    }

    if(y1>=200)
        return;
    if(x2>=640)
        x2=639;

    x1a=x1&0xFFF8;
    x2a=x2&0xFFF8;
    if(x2<x1a+8) {
        for(x=x1;x<=x2;++x)
            port_pset(x,y1);
        return;
    }

    for(x=x1;x<x1a+8;++x){
        port_pset(x,y1);
    }

    ttl=(x2a-x1a)>>3;
    --ttl;
    --ttl;
    if(ttl>0) {
        by=((x1a>>3)+1)+y1*80;
    
        write_VDC(18,(by&0xFF00)>>8);    // HI
        write_VDC(19,by&0x00FF);         // LO
        write_VDC(31, 0xFF); // DATA
        write_VDC(30, ttl); // REP
    }
    
    for(x=x2a;x<=x2;++x){
        port_pset(x,y1);
    }
    
}

/* printat prints a text at the given location unsing the font described
   by the structure font */
void port_printat(unsigned short x, unsigned short y, char *s)
{
    short i,j,k;
    unsigned short a;
    short t;
    
    for (i=0; s[i]!='\0';++i) {
        for (j=0;j<f.incY*f.magnification;++j) {
            a=f.pDesc[f.pos[s[i]]+j/f.magnification];
            t=1;
            for(k=0;a!=0;++k){
                if (a & 0x0001)
                    port_pset(x+f.incX*f.magnification-k, y+j);
                if(t==f.magnification){
                    a>>=1;
                    t=1;
                } else {
                    ++t;
                }
            }
        }
        x+=f.incX*f.magnification;
    }
}

/*
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
//                    POKE(loc, PEEK(loc) | pix_pos[(unsigned char)ix&7]);
                } if(t==mm){
                    a>>=1;
                    t=1;
                } else {
                    ++t;
                }
            }
        }
    }
}*/

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
    static unsigned char oep;
    static unsigned char ll,hh;
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

    /* Plot the first pixel */
    port_pset(x1,y1);

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

            by=x1>>3;
            by+=y1<<6;
            by+=y1<<4;
            
            (*(byte *)VDC_REGISTERN)=18;
            hh=(by&0xFF00)>>8;
            (*(byte *)VDC_DATA)=hh;
            (*(byte *)VDC_REGISTERN)=19;
            ll=by&0x00FF;
            (*(byte *)VDC_DATA)=ll;
            (*(byte *)VDC_REGISTERN)=31;
            oep= (*(byte *)VDC_DATA);
        
            oep |= pix_pos[(unsigned char)x1&7];
        
            (*(byte *)VDC_REGISTERN)=18;
            (*(byte *)VDC_DATA)=hh;
            (*(byte *)VDC_REGISTERN)=19;
            (*(byte *)VDC_DATA)=ll;  
            (*(byte *)VDC_REGISTERN)=31;
            (*(byte *)VDC_DATA)=oep;
        }
        style_mask >>= 1;
        if(style_mask==0) style_mask=style;
    }
}

unsigned long port_get_time(void)
{
    return 0;//PEEK(160)+PEEK(161)*256;
}

void port_colour_banner(void)
{
    unsigned char x;
    unsigned char y;

    for(x=25;x<40;++x) {
        for(y=0;y<25;++y) {
            //POKE(COLOR_MEM+x+y*40,0x67);
        }
    }
}

long port_get_current_time(void)
{
    /*long ctime=((char)PEEK(160));
    ctime<<=8;
    ctime+=((char)PEEK(161));
    ctime<<=8;
    ctime+=((char)PEEK(162));
    return ctime;*/
    return 0;
}

static void port_process_voice(unsigned char **ptr, unsigned char *sid_pointer,
    unsigned char *wsh)
{
    unsigned char freq;
    unsigned int timestamp=**ptr+ (*(*ptr+1)<<8);
    return;
    if(timestamp>cnt) {
        return;
    }
    ++*ptr;
    ++*ptr;
    switch(**ptr) {
        case NOTE_CODE:                 // Note event
            freq=*(++*ptr)-24;
            POKE(sid_pointer,C64_freq_table[freq]&0xFF);          // Freq. lo
            POKE(sid_pointer+1,(C64_freq_table[freq]&0xFF00)>>8); // Freq. hi
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
    //POKE(0xD418,15);
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
    //POKE(0xD418,0);
}
void port_music_on(void)
{ 
    //POKE(0xD418,15);
}

void port_exit(void)
{
    f.magnification = 1;
    port_printat(0,60, "We cant exit on this port!");
    f.magnification = 2;
}
