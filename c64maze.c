#include<stdlib.h>
#include "c64maze.h"



char labyrinth[] =  "****************************************"
                    "*      *    *     * *   *        *     *"
                    "* **** * ******** * *** * *** **   ** **"
                    "* * ** *   *  *   *     *   * *  *  *  *"
                    "*   *  * *  * *** ***** * * * * ***  * *"
                    "***** ** ** *             *** *   **** *"
                    "*             * ** **** * *     *  *   *"
                    "* * *********** *     * * * ****** * * *"
                    "* *         *   ** ** *   * *        * *"
                    "***** ***** * * *  *  * * * **** ** *  *"
                    "* *       * *** **** ** **     *  * ****"
                    "* * *** * * *     *  *    * ** * *     *"
                    "*     *** * ***** * ** **** *  * **** **"
                    "** ** *   *     * * *   *   * **    *  *"
                    "*  * ** ******* * * *** * *** *  ** ** *"
                    "**   *  *         * *   *      * *     *"
                    "****************************************";

char startx=1;
char starty=1;
char positionx;
char positiony;
unsigned char style=0x1;

#if (P_CURRENT== P_C64)
    #include "ports/C64.h"
    #define LABYRINTHSZX_DYN    SIZEX
    #define LABYRINTHSZY_DYN    SIZEY
    #define LABSTEPSZX_DYN  STEPSIZEX
    #define LABSTEPSZY_DYN  STEPSIZEY
    #define MULT 1
#elif (P_CURRENT== P_C128)
    #include "ports/C128.h"
    #define LABYRINTHSZX_DYN    400
    #define LABYRINTHSZY_DYN    200
    #define LABSTEPSZX_DYN      24
    #define LABSTEPSZY_DYN      14
    #define MULT 2
#else
    #include "ports/UNIX.h"
    display_bounds_t disp_bounds;
    #define LABYRINTHSZX_DYN	disp_bounds.labyrinthx
    #define LABYRINTHSZY_DYN	disp_bounds.labyrinthy
    #define LABSTEPSZX_DYN	disp_bounds.stepszx
    #define LABSTEPSZY_DYN 	disp_bounds.stepszy
    #define MULT 1
#endif


char exitx=13;
char exity=1;


    /*  0 = north
        1 = west
        2 = sud
        3 = east
    */
char orientation=0;

int leftx;
int lefty;
int rightx;
int righty;
int advancex;
int advancey;

long start_time;


void random_exit(void)
{
    do {
        exitx = (rand()>>8)*labyrinthSizeX/(RAND_MAX>>8);
        exity = (rand()>>8)*labyrinthSizeY/(RAND_MAX>>8);
    } while(labyrinth[exitx+exity*labyrinthSizeX]!=' ');
}

void flipx(void)
{
    int x,y;
    char temp;
    
    for (x=0; x<labyrinthSizeX/2; ++x) {
        for (y=0; y<labyrinthSizeY; ++y) {
            temp=labyrinth[x+y*labyrinthSizeX];
            labyrinth[x+y*labyrinthSizeX]=
                labyrinth[(labyrinthSizeX-x-1)+y*labyrinthSizeX];
            labyrinth[(labyrinthSizeX-x-1)+y*labyrinthSizeX]=temp;
        }
    }
}

void flipy(void)
{
    int x,y;
    char temp;
    
    for (x=0; x<labyrinthSizeX; ++x) {
        for (y=0; y<labyrinthSizeY/2; ++y) {
            temp=labyrinth[x+y*labyrinthSizeX];
            labyrinth[x+y*labyrinthSizeX]=
                labyrinth[x+(labyrinthSizeY-y-1)*labyrinthSizeX];
            labyrinth[x+(labyrinthSizeY-y-1)*labyrinthSizeX]=temp;
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
    // The simplified code does not handle every style lines, so one must check
    // for it.
    if(style==0x1) {
        port_hor_line(xul, xlr, yul);
        port_hor_line(xul, xlr, ylr);
        port_vert_line(xul, yul, ylr);
        port_vert_line(xlr, yul, ylr);
    } else {
        port_line(xul, yul, xlr, yul);
        port_line(xlr, yul, xlr, ylr);
        port_line(xlr, ylr, xul, ylr);
        port_line(xul, ylr, xul, yul);
    }
}

/** Choose randomly the starting position in the maze.
*/
void choose_start_position()
{
    unsigned int time=port_get_time();
    srand(time);
    do {
        startx=(labyrinthSizeX*(rand()/(RAND_MAX/100)))/100;
        starty=(labyrinthSizeY*(rand()/(RAND_MAX/100)))/100;
    } while(labyrinth[startx+starty*labyrinthSizeX]!=' ');
    positionx=startx;
    positiony=starty;
}

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

//#include <stdio.h>

/** In this function, the maze view is shown from the current point and
    orientation of the player (those are specified in global variables).
*/
void drawLabyrinthView()
{
    unsigned char posx=positionx;
    unsigned char posy=positiony;
    unsigned char step=0;
    unsigned int sszx;
    unsigned int sszy;
    unsigned int sszxp1;
    unsigned int sszyp1;
    unsigned char wall=FALSE;
    unsigned char wayout=FALSE;

    unsigned long tt=port_get_current_time();
    style=0x1;
    set_orientation();

    /* Draw the maze in isometric perspective starting from the position of
       the player and going progressively farther from them, until a wall is
       hit or until the distance becomes greater than 5 steps.
    */
    for(step=0;(wall==FALSE)&&(step<6);++step) {
        /* Specify the line style so that walls and corners far from the
           player somewhat appears to be less definite, thus adding a certain
           3D effect.
        */
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
        // We hit a wall!
        if(labyrinth[posx+posy*labyrinthSizeX]=='*') {
            break;
        }
        // Some pre-calculated data for wall and corner drawing.
        sszx=step*LABSTEPSZX_DYN;
        sszy=step*LABSTEPSZY_DYN;
        sszxp1=(step+1)*LABSTEPSZX_DYN;
        sszyp1=(step+1)*LABSTEPSZY_DYN;
        // Wall on the right?
        if(labyrinth[posx+rightx+(posy+righty)*labyrinthSizeX]=='*') {
            port_line(LABYRINTHSZX_DYN-sszx,sszy,
                 LABYRINTHSZX_DYN-sszxp1,sszyp1);
            port_line(LABYRINTHSZX_DYN-sszx,LABYRINTHSZY_DYN-sszy,
                 LABYRINTHSZX_DYN-sszxp1,LABYRINTHSZY_DYN-sszyp1);
        } else {
            // Closer vertical line
            port_line(LABYRINTHSZX_DYN-sszx,sszy,
                 LABYRINTHSZX_DYN-sszx,LABYRINTHSZY_DYN-sszy);
            // Farther vertical line
            if(labyrinth[posx+advancex+(posy+advancey)*labyrinthSizeX]!='*') {
                port_line(LABYRINTHSZX_DYN-sszxp1,sszyp1,
                    LABYRINTHSZX_DYN-sszxp1,LABYRINTHSZY_DYN-sszyp1);
            }
            // Upper horisontal line
            port_line(LABYRINTHSZX_DYN-sszxp1,sszyp1,
                 LABYRINTHSZX_DYN-sszx,sszyp1);
            // Lower horisontal line
            port_line(LABYRINTHSZX_DYN-sszxp1,LABYRINTHSZY_DYN-sszyp1,
                 LABYRINTHSZX_DYN-sszx,LABYRINTHSZY_DYN-sszyp1);
        }
        // Wall on the left?
        if(labyrinth[posx+leftx+(posy+lefty)*labyrinthSizeX]=='*') {
            port_line(sszx,sszy,
                 sszxp1,sszyp1);
            port_line(sszx,LABYRINTHSZY_DYN-sszy,
                 sszxp1,LABYRINTHSZY_DYN-sszyp1);
        } else {
            // Closer vertical line
            port_line(sszx,sszy,
                 sszx,LABYRINTHSZY_DYN-sszy);
            // Farter vertical line
            if(labyrinth[posx+advancex+(posy+advancey)*labyrinthSizeX]!='*') {
                port_line(sszxp1,sszyp1,
                    sszxp1,LABYRINTHSZY_DYN-sszyp1);
            }
            // Upper horisontal line
            port_line(sszxp1,sszyp1,
                 sszx,sszyp1);
            // Lower horisontal line
            port_line(sszxp1,LABYRINTHSZY_DYN-sszyp1,
                 sszx,LABYRINTHSZY_DYN-sszyp1);
        }
        // Advance one step farther from the player.
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
    // We have a wall at the end of our sight
    if(wall==TRUE)
        box(step*LABSTEPSZX_DYN,step*LABSTEPSZY_DYN,
            LABYRINTHSZX_DYN-step*LABSTEPSZX_DYN,
            LABYRINTHSZY_DYN-step*LABSTEPSZY_DYN);
    // The exit is in sight!
    if(wayout) {
        ++step;
        box(step*LABSTEPSZX_DYN,step*LABSTEPSZY_DYN,
            LABYRINTHSZX_DYN-step*LABSTEPSZX_DYN,
            LABYRINTHSZY_DYN-step*LABSTEPSZY_DYN);
        port_line(step*LABSTEPSZX_DYN,step*LABSTEPSZY_DYN,
            LABYRINTHSZX_DYN-step*LABSTEPSZX_DYN,
            LABYRINTHSZY_DYN-step*LABSTEPSZY_DYN);
        port_line(LABYRINTHSZX_DYN-step*LABSTEPSZX_DYN,step*LABSTEPSZY_DYN,
            step*LABSTEPSZX_DYN,LABYRINTHSZY_DYN-step*LABSTEPSZY_DYN);
    }
    
    //printf("Elapsed: %d\n",(int)(port_get_current_time()-tt));
}

/** Be sure that the current player position is correct.
*/
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

/** Advance in the forward direction, following the current orientation.
*/
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

/** Go backwards one step, following the current orientation.
*/
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

/** Colour the right side banner in the screen view.
*/
void colour_banner(void)
{
    port_colour_banner();
}

/** Draw the banner on the right side of the screen.
*/
void draw_banner(void)
{
    unsigned long tt=port_get_current_time();
    colour_banner();
#if P_CURRENT == P_C128
    port_loadVICFont(2);
    port_printat(204*2+50,17,"c128maze");
    port_loadVICFont(1);
    box(251*2,53,281*2+1,84);
    port_printat(260*2,55," t");
    port_printat(252*2,65,"f  +  g");
    port_printat(260*2,75," v");
    port_printat(207*2+50,100,"[p] view maze");
#ifndef NO_SOUND
    port_printat(207*2+50,110,"[m] music 1/0");
#endif
    port_printat(207*2+50,120,"[a] restart  ");
    
    port_printat(207*2+50,170,"d. bucci 2017-2026");
    port_printat(207*2+50,160,"igor1101 2019");
    port_loadVICFont(2);
    //port_line(200*2,0,200*2,199);
    
#endif
#if P_CURRENT == P_C64
    port_loadVICFont(2);
    port_printat(204,17,"c64maze");
    port_loadVICFont(1);
    box(251,53,278,84);
    port_printat(260,55,"t");
    port_printat(252,65,"f+g");
    port_printat(260,75,"v");
    port_printat(207,100,"[p] view maze");
#ifndef NO_SOUND
    port_printat(207,110,"[m] music 1/0");
#endif
    port_printat(207,120,"[a] restart  ");
    
    port_printat(207,170,"d. bucci 2017");
    port_printat(207,160,"igor1101 2019");
    port_loadVICFont(2);
    port_line(200,0,200,199);
#elif (P_CURRENT == P_UNIX)
    unsigned short xoffset = disp_bounds.bannerx;
    unsigned short xbannersz = disp_bounds.bannerx_end - disp_bounds.bannerx;
    unsigned short xbannerhalfsz = xbannersz / 2;
    unsigned short xbanner13sz = xbannersz / 3;
    unsigned short xbanner23sz = xbannersz * 2 / 3;
    unsigned short xmiddle = xoffset + xbannerhalfsz;
    unsigned short xmiddletxt = xoffset + xbanner13sz;
    unsigned short xmiddletxt_end = xoffset + xbanner23sz;
    unsigned short ymiddle = disp_bounds.bannery_end / 2;
    unsigned short yend = disp_bounds.bannery_end;
    port_printat(xmiddletxt,17,"c64maze");
    port_loadVICFont(1);
    box(xoffset, 0, xoffset + xbannersz, yend);
    box(xmiddletxt - 2,ymiddle - 40 ,xmiddletxt_end + 2,ymiddle + 40);
    port_printat(xmiddle,ymiddle - 20,"t");
    port_printat(xmiddle - 18,ymiddle,"f+g");
    port_printat(xmiddle,ymiddle + 20,"v");
    port_printat(xoffset,ymiddle + 40,"[p] view maze");
#ifndef NO_SOUND
    port_printat(xoffset,ymiddle + 60,"[m] music 1/0");
#endif
    port_printat(xoffset,ymiddle + 80,"[a] restart  ");
    port_printat(xoffset,ymiddle - 60,"d. bucci 2017");
    port_printat(xoffset,ymiddle - 80,"igor1101 2019");
    port_loadVICFont(2);
    port_line(xoffset,0,200,199);
#endif 
    //printf("(banner) elapsed: %d\n",(int)(port_get_current_time()-tt));

}


char *write_time(char *message, unsigned char start)
{
    long approx=0;
    char thousands=0;
    char hundreds=0;
    char tens=0;
    char seconds=0;
    approx=port_get_current_time()-start_time;
    if(approx>60*1000) {
        thousands = approx/60l/1000l;
        approx-=thousands*60l*1000l;
    }
    if(approx>60*100) {
        hundreds= approx/60l/100l;
        approx-=hundreds*60l*100l;
    }
    if(approx>60*10) {
        tens= approx/60l/10l;
        approx-=tens*60l*10l;
    }
    if(approx>60) {
        seconds=approx/60l;
        approx-=seconds*60l;
    }

    message[start++]= thousands+'0';
    message[start++]= hundreds+'0';
    message[start++]= tens + '0';
    message[start++]= seconds + '0';
    return message;
}

/** Show the maze with the current position and orientation.
*/
void show_maze()
{
    unsigned char x = 0;
    unsigned char y = 0;
    unsigned int by = 0;
    const char *pt = 0;

    char message[]="elapsed:      s";

    style=0x1;

    start_time-=60l*30l; // 30 seconds penalty.

    port_clearHGRpage();
    /* findout appropriate box size */
    for(y=0; y<labyrinthSizeY;++y) {
        //by=COLOR_MEM+y*40;
        pt=labyrinth+y*labyrinthSizeX;
        for(x=0; x<labyrinthSizeX;++x) {
            if(pt[x]=='*') {
#ifndef SZ
#define SZ	8
#endif
                box(x*SZ,y*SZ, x*SZ+SZ, y*SZ+SZ);
            } else if(positiony==y && positionx==x) {
                box(x*SZ+2,y*SZ+2,x*SZ+5,y*SZ+5);
                switch(orientation) {
                    case 0:
                        port_line(x*SZ+3,y*SZ,x*SZ+3,y*SZ+2);
                        port_line(x*SZ+4,y*SZ,x*SZ+4,y*SZ+2);
                        break;
                    case 1:
                        port_line(x*SZ,y*SZ+3,x*SZ+2,y*SZ+3);
                        port_line(x*SZ,y*SZ+4,x*SZ+2,y*SZ+4);
                        break;
                    case 2:
                        port_line(x*SZ+3,y*SZ+5,x*SZ+3,y*SZ+7);
                        port_line(x*SZ+4,y*SZ+5,x*SZ+4,y*SZ+7);
                        break;
                    case 3:
                        port_line(x*SZ+5,y*SZ+3,x*SZ+7,y*SZ+3);
                        port_line(x*SZ+5,y*SZ+4,x*SZ+7,y*SZ+4);
                        break;
                }
            } else if(x==exitx && y==exity) {
                box(x*SZ+2,y*SZ+2,x*SZ+5,y*SZ+5);
            }
            ++by;
        }
    }
    port_font_magnification(1);
    port_printat(15,150,"https://github.com/darwinne/c64maze");

    port_font_magnification(2);
    write_time(message,9);
    port_printat(40,170,message);
    port_fflushMazeRegion();
    port_getch();
    port_clearHGRpage();
    draw_banner();
}

// byte 1: lo byte timestamp in 1/60's of second
// byte 2: hi byte timestamp in 1/60's of second
// byte 3: message
// byte 4: first byte of the message (if applicable)
// byte 5: second byte of the message (if applicable)
//

#ifndef NO_SOUND



#endif

void start_game(void)
{
    int flip;
    flip=rand();
    if(flip & 0x01) flipx();
    if(flip & 0x02) flipy();

    choose_start_position();
    random_exit();
    draw_banner();

    start_time=port_get_current_time();
}

/** Starting point of the program.
*/
void main(void)
{
    unsigned char oldx=0;
    unsigned char oldy=0;
    char oldo=0;
    char c;
    char iv=TRUE;
    unsigned char music=TRUE;
    char time_spent[]="     s";
    int t;
    _randomize();       // CC65 initialization of the random number gen.

#ifndef NO_SOUND
    port_init_sound();
#endif

    port_graphics_init();
    /*
    for(t=0; t<200; ++t) {
        port_line(200-t,t,200+t,t);
    }
    for(t=0; t<200; ++t) {
        port_line(200-t+98,t,200+t+98,t);
    }
    
    while(1);*/
    
    
    restart:
    start_game();

    while(TRUE) {
        if(oldx!=positionx || oldy!=positiony || oldo!=orientation) {
            oldx=positionx;
            oldy=positiony;
            oldo=orientation;
            port_clearMazeRegion();
            drawLabyrinthView();
            port_fflushMazeRegion();
            if (positionx==startx && positiony==starty)
                port_printat(40*MULT,100,"step in!");
            if (positionx==exitx && positiony==exity) {
                port_printat(40*MULT,70,"way out!");
                write_time(time_spent,0);
                port_printat(50*MULT,100,time_spent);
                port_loadVICFont(1);
                port_printat(55*MULT, 140,  "press a key");
                port_printat(47*MULT, 150, "to play again");
                port_getch();
                oldx=0;
                goto restart;   // No program for the C64 would be complete
                                // without at least a GOTO statement somewhere.
            }
        }
        do {
            c=port_getch();
            iv=FALSE;
            switch(c) {
                case 'q':   //Exit
                    game_exit();
                    break;
                case 't':   // Forward
                    move_forward();
                    break;
                case 'v':   // Backwards
                    move_backwards();
                    break;
                case 'f':   // Turn left
                    if(orientation==3)
                        orientation=0;
                    else
                        ++orientation;
                    break;
                case 'g':   // Turn right
                    if(orientation==0)
                        orientation=3;
                    else
                        --orientation;
                    break;
                case 'p':   // Show maze and position
                    show_maze();
                    // force a redraw.
                    oldx=0;
                    break;
                case 'a':   // Start again
                    port_clearHGRpage();
                    start_game();
                    oldx=0;
                    break;
#ifndef NO_SOUND
                case 'm':   // Toggle music on/off
                    if(music==TRUE) {
                        port_music_off();
                        music=FALSE;
                    } else {
                        port_music_on();
                        music=TRUE;
                    }
                    break;
#endif
                default:
                    iv=TRUE;
            };
        } while(iv==TRUE);
        if(labyrinth[positionx+positiony*labyrinthSizeX]=='*') {
            //POKE(SCREEN_BORDER,1);
            positionx=oldx;
            positiony=oldy;
        }
    }
}

void game_exit(void)
{
	port_exit();
}
