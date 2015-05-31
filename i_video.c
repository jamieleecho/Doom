// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <errno.h>
#include <signal.h>

#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"

#include "d_event_queue.h"

#define POINTER_WARP_COUNTDOWN	1

// Display*	X_display=0;
// Window		X_mainWindow;
// Colormap	X_cmap;
// Visual*		X_visual;
// GC		X_gc;
// XEvent		X_event;
// int		X_screen;
// XVisualInfo	X_visualinfo;
// XImage*		image;
// int		X_width;
// int		X_height;

// MIT SHared Memory extension.
// boolean2		doShm;

// XShmSegmentInfo	X_shminfo;
// int		X_shmeventtype;

// Fake mouse handling.
// This cannot work properly w/o DGA.
// Needs an invisible mouse cursor at least.
// boolean2		grabMouse;
// int		doPointerWarp = POINTER_WARP_COUNTDOWN;

// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
// static int	multiply=1;

// imageBuff contains the rgb version of Doom's ouput
unsigned short imageBuff[SCREENWIDTH * SCREENHEIGHT * 4];

/*
 * Warp the mouse cursor to the desired position in global
 * coordinates without generating events
 */
//CG_EXTERN CGEventErr CGWarpMouseCursorPosition( CGPoint newCursorPosition );

void I_ShutdownGraphics(void)
{
}


//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?

}

boolean2		mousemoved = false;
boolean2		shmFinished;

void I_GetEvent(void)
{
    int ii = event_queue_count();
    for (; ii>0; ii--) {
        event_t e = event_queue_get();
        D_PostEvent(&e);
    }
}

//
// I_StartTic
//
void I_StartTic (void)
{
    I_GetEvent();
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}

struct {
#ifdef __BIG_ENDIAN__
    unsigned short red0:4,
                   green0:4,
                   blue0:4,
                   alpha0:4;
    unsigned short red1:4,
                   green1:4,
                   blue1:4,
                   alpha1:4;
#else
    unsigned short green0:4,
                   red0:4,
                   alpha0:4,
                   blue0:4;
    unsigned short green1:4,
                   red1:4,
                   alpha1:4,
                   blue1:4;
#endif
} colors[256];

extern int wipestart;
//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{
    unsigned char *src = screens[(wipestart == -1) ? 0 : 0];
    unsigned char *end = src + SCREENWIDTH * SCREENHEIGHT;
    unsigned *ib = (unsigned *)imageBuff;
    unsigned short *ibs = (unsigned short *)imageBuff;    
    
	for (; src<end; ib+=SCREENWIDTH)
		*ibs++ = *(unsigned short*)&colors[*src++];
}


//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    int idxEnd = SCREENWIDTH * SCREENHEIGHT / sizeof(long);
    long *src = (long *)screens[0];
    long *dest = (long *)scr;
    int ii;
    for (ii=0; ii<idxEnd; ii++)
        dest[ii] = src[ii]; 
}

//
// I_SetPalette
//
void I_SetPalette (byte* palette)
{
    int i, c;
    for (i=0 ; i<256 ; i++) {
        c = gammatable[usegamma][*palette++];
        colors[i].red0 = colors[i].red1 = c >> 4;
        c = gammatable[usegamma][*palette++];
        colors[i].green0 = colors[i].green1 =c >> 4;
        c = gammatable[usegamma][*palette++];
        colors[i].blue0 = colors[i].blue1 = c >> 4;
        colors[i].alpha0 = colors[i].alpha1 = 0;
    }
}

void I_InitGraphics(void)
{
}


unsigned	exptable[256];

void InitExpand (void)
{
    int		i;
	
    for (i=0 ; i<256 ; i++)
	exptable[i] = i | (i<<8) | (i<<16) | (i<<24);
}

double		exptable2[256*256];

void InitExpand2 (void)
{
    int		i;
    int		j;
    // UNUSED unsigned	iexp, jexp;
    double*	exp;
    union
    {
	double 		d;
	unsigned	u[2];
    } pixel;
	
    printf ("building exptable2...\n");
    exp = exptable2;
    for (i=0 ; i<256 ; i++)
    {
	pixel.u[0] = i | (i<<8) | (i<<16) | (i<<24);
	for (j=0 ; j<256 ; j++)
	{
	    pixel.u[1] = j | (j<<8) | (j<<16) | (j<<24);
	    *exp++ = pixel.d;
	}
    }
    printf ("done.\n");
}

int	inited;

void
Expand4
( unsigned*	lineptr,
  double*	xline )
{
    double	dpixel;
    unsigned	x;
    unsigned 	y;
    unsigned	fourpixels;
    unsigned	step;
    double*	exp;
	
    exp = exptable2;
    if (!inited)
    {
	inited = 1;
	InitExpand2 ();
    }
		
		
    step = 3*SCREENWIDTH/2;
	
    y = SCREENHEIGHT-1;
    do
    {
	x = SCREENWIDTH;

	do
	{
	    fourpixels = lineptr[0];
			
	    dpixel = *(double *)( (int)exp + ( (fourpixels&0xffff0000)>>13) );
	    xline[0] = dpixel;
	    xline[160] = dpixel;
	    xline[320] = dpixel;
	    xline[480] = dpixel;
			
	    dpixel = *(double *)( (int)exp + ( (fourpixels&0xffff)<<3 ) );
	    xline[1] = dpixel;
	    xline[161] = dpixel;
	    xline[321] = dpixel;
	    xline[481] = dpixel;

	    fourpixels = lineptr[1];
			
	    dpixel = *(double *)( (int)exp + ( (fourpixels&0xffff0000)>>13) );
	    xline[2] = dpixel;
	    xline[162] = dpixel;
	    xline[322] = dpixel;
	    xline[482] = dpixel;
			
	    dpixel = *(double *)( (int)exp + ( (fourpixels&0xffff)<<3 ) );
	    xline[3] = dpixel;
	    xline[163] = dpixel;
	    xline[323] = dpixel;
	    xline[483] = dpixel;

	    fourpixels = lineptr[2];
			
	    dpixel = *(double *)( (int)exp + ( (fourpixels&0xffff0000)>>13) );
	    xline[4] = dpixel;
	    xline[164] = dpixel;
	    xline[324] = dpixel;
	    xline[484] = dpixel;
			
	    dpixel = *(double *)( (int)exp + ( (fourpixels&0xffff)<<3 ) );
	    xline[5] = dpixel;
	    xline[165] = dpixel;
	    xline[325] = dpixel;
	    xline[485] = dpixel;

	    fourpixels = lineptr[3];
			
	    dpixel = *(double *)( (int)exp + ( (fourpixels&0xffff0000)>>13) );
	    xline[6] = dpixel;
	    xline[166] = dpixel;
	    xline[326] = dpixel;
	    xline[486] = dpixel;
			
	    dpixel = *(double *)( (int)exp + ( (fourpixels&0xffff)<<3 ) );
	    xline[7] = dpixel;
	    xline[167] = dpixel;
	    xline[327] = dpixel;
	    xline[487] = dpixel;

	    lineptr+=4;
	    xline+=8;
	} while (x-=16);
	xline += step;
    } while (y--);
}


