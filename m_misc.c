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
//
// $Log:$
//
// DESCRIPTION:
//	Main loop menu stuff.
//	Default Config File.
//	PCX Screenshots.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: m_misc.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <ctype.h>


#include "doomdef.h"

#include "z_zone.h"

#include "m_swap.h"
#include "m_argv.h"

#include "w_wad.h"

#include "i_system.h"
#include "i_video.h"
#include "v_video.h"

#include "hu_stuff.h"

// State.
#include "doomstat.h"

// Data.
#include "dstrings.h"

#include "m_misc.h"

//
// M_DrawText
// Returns the final X coordinate
// HU_Init must have been called to init the font
//
extern patch_t*		hu_font[HU_FONTSIZE];

int
M_DrawText
( int		x,
  int		y,
  boolean2	direct,
  char*		string )
{
    int 	c;
    int		w;

    while (*string)
    {
	c = toupper(*string) - HU_FONTSTART;
	string++;
	if (c < 0 || c> HU_FONTSIZE)
	{
	    x += 4;
	    continue;
	}
		
	w = SHORT (hu_font[c]->width);
	if (x+w > SCREENWIDTH)
	    break;
	if (direct)
	    V_DrawPatchDirect(x, y, 0, hu_font[c]);
	else
	    V_DrawPatch(x, y, 0, hu_font[c]);
	x+=w;
    }

    return x;
}




//
// M_WriteFile
//
#ifndef O_BINARY
#define O_BINARY 0
#endif

boolean2
M_WriteFile
( char const*	name,
  void*		source,
  int		length )
{
    int		handle;
    int		count;
	
    handle = open ( name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);

    if (handle == -1)
	return false;

    count = write (handle, source, length);
    close (handle);
	
    if (count < length)
	return false;
		
    return true;
}


//
// M_ReadFile
//
int
M_ReadFile
( char const*	name,
  byte**	buffer )
{
    int	handle, count, length;
    struct stat	fileinfo;
    byte		*buf;
	
    handle = open (name, O_RDONLY | O_BINARY, 0666);
    if (handle == -1)
	I_Error ("Couldn't read file %s", name);
    if (fstat (handle,&fileinfo) == -1)
	I_Error ("Couldn't read file %s", name);
    length = fileinfo.st_size;
    buf = Z_Malloc (length, PU_STATIC, NULL);
    count = read (handle, buf, length);
    close (handle);
	
    if (count < length)
	I_Error ("Couldn't read file %s", name);
		
    *buffer = buf;
    return length;
}


//
// DEFAULTS
//
int		usemouse;
int		usejoystick;

extern int	key_right;
extern int	key_left;
extern int	key_up;
extern int	key_down;

extern int	key_strafeleft;
extern int	key_straferight;

extern int	key_fire;
extern int	key_use;
extern int	key_strafe;
extern int	key_speed;

extern int	mousebfire;
extern int	mousebstrafe;
extern int	mousebforward;

extern int	joybfire;
extern int	joybstrafe;
extern int	joybuse;
extern int	joybspeed;

extern int	viewwidth;
extern int	viewheight;

extern int	mouseSensitivity;
extern int	showMessages;

extern int	detailLevel;

extern int	screenblocks;

extern int	showMessages;

// machine-independent sound params
extern	int	numChannels;


// UNIX hack, to be removed.
#ifdef SNDSERV
extern char*	sndserver_filename;
extern int	mb_used;
#endif

#ifdef LINUX
char*		mousetype;
char*		mousedev;
#endif

extern const char*	const chat_macros[];



typedef struct
{
    char*	name;
    int*	location;
    int		defaultvalue;
    int		scantranslate;		// PC scan code hack
    int		untranslated;		// lousy hack
} default_t;

default_t	defaults[100];

int	numdefaults;
char*	defaultfile;


//
// M_SaveDefaults
//
void M_SaveDefaults (void)
{
    int		i;
    int		v;
    FILE*	f;

    f = fopen (defaultfile, "w");
    if (!f)
	return; // can't write the file, but don't complain
		
    for (i=0 ; i<numdefaults ; i++)
    {
	if (defaults[i].defaultvalue > -0xfff
	    && defaults[i].defaultvalue < 0xfff)
	{
	    v = *defaults[i].location;
	    fprintf (f,"%s\t\t%i\n",defaults[i].name,v);
	} else {
	    fprintf (f,"%s\t\t\"%s\"\n",defaults[i].name,
		     * (char **) (defaults[i].location));
	}
    }
	
    fclose (f);
}

static default_t makeDefault(char *name, int *location, int value) {
    default_t defaultValue = { name, location, value };
    return defaultValue;
}

static void initDefaults() {
    // Make sure this only gets called once    
    if (numdefaults != 0) return;
    
    defaults[numdefaults++] = makeDefault("mouse_sensitivity",&mouseSensitivity, 5);    
    defaults[numdefaults++] = makeDefault("sfx_volume",&snd_SfxVolume, 8);
    defaults[numdefaults++] = makeDefault("music_volume",&snd_MusicVolume, 8);
    defaults[numdefaults++] = makeDefault("show_messages",&showMessages, 1);
    
    
#ifdef NORMALUNIX
    defaults[numdefaults++] = makeDefault("key_right",&key_right, KEY_RIGHTARROW);
    defaults[numdefaults++] = makeDefault("key_left",&key_left, KEY_LEFTARROW);
    defaults[numdefaults++] = makeDefault("key_up",&key_up, KEY_UPARROW);
    defaults[numdefaults++] = makeDefault("key_down",&key_down, KEY_DOWNARROW);
    defaults[numdefaults++] = makeDefault("key_strafeleft",&key_strafeleft, ',');
    defaults[numdefaults++] = makeDefault("key_straferight",&key_straferight, '.');
    
    defaults[numdefaults++] = makeDefault("key_fire",&key_fire, KEY_RCTRL);
    defaults[numdefaults++] = makeDefault("key_use",&key_use, ' ');
    defaults[numdefaults++] = makeDefault("key_strafe",&key_strafe, KEY_RALT);
    defaults[numdefaults++] = makeDefault("key_speed",&key_speed, KEY_RSHIFT);
    
    // UNIX hack, to be removed. 
#ifdef SNDSERV
    defaults[numdefaults++] = makeDefault("sndserver", (int *) &sndserver_filename, (int) "sndserver");
    defaults[numdefaults++] = makeDefault("mb_used", &mb_used, 6);
#endif
    
#endif
    
#ifdef LINUX
    defaults[numdefaults++] = makeDefault("mousedev", (int*)&mousedev, (int)"/dev/ttyS0");
    defaults[numdefaults++] = makeDefault("mousetype", (int*)&mousetype, (int)"microsoft");
#endif
    
    defaults[numdefaults++] = makeDefault("use_mouse",&usemouse, 1);
    defaults[numdefaults++] = makeDefault("mouseb_fire",&mousebfire,0);
    defaults[numdefaults++] = makeDefault("mouseb_strafe",&mousebstrafe,1);
    defaults[numdefaults++] = makeDefault("mouseb_forward",&mousebforward,2);
    
    defaults[numdefaults++] = makeDefault("use_joystick",&usejoystick, 0);
    defaults[numdefaults++] = makeDefault("joyb_fire",&joybfire,0);
    defaults[numdefaults++] = makeDefault("joyb_strafe",&joybstrafe,1);
    defaults[numdefaults++] = makeDefault("joyb_use",&joybuse,3);
    defaults[numdefaults++] = makeDefault("joyb_speed",&joybspeed,2);
    
    defaults[numdefaults++] = makeDefault("screenblocks",&screenblocks, 9);
    defaults[numdefaults++] = makeDefault("detaillevel",&detailLevel, 0);
    
    defaults[numdefaults++] = makeDefault("snd_channels",&numChannels, 3);
    
    
    
    defaults[numdefaults++] = makeDefault("usegamma",&usegamma, 0);
    
    defaults[numdefaults++] = makeDefault("chatmacro0", (int *) &chat_macros[0], (int) HUSTR_CHATMACRO0 );
    defaults[numdefaults++] = makeDefault("chatmacro1", (int *) &chat_macros[1], (int) HUSTR_CHATMACRO1 );
    defaults[numdefaults++] = makeDefault("chatmacro2", (int *) &chat_macros[2], (int) HUSTR_CHATMACRO2 );
    defaults[numdefaults++] = makeDefault("chatmacro3", (int *) &chat_macros[3], (int) HUSTR_CHATMACRO3 );
    defaults[numdefaults++] = makeDefault("chatmacro4", (int *) &chat_macros[4], (int) HUSTR_CHATMACRO4 );
    defaults[numdefaults++] = makeDefault("chatmacro5", (int *) &chat_macros[5], (int) HUSTR_CHATMACRO5 );
    defaults[numdefaults++] = makeDefault("chatmacro6", (int *) &chat_macros[6], (int) HUSTR_CHATMACRO6 );
    defaults[numdefaults++] = makeDefault("chatmacro7", (int *) &chat_macros[7], (int) HUSTR_CHATMACRO7 );
    defaults[numdefaults++] = makeDefault("chatmacro8", (int *) &chat_macros[8], (int) HUSTR_CHATMACRO8 );
    defaults[numdefaults++] = makeDefault("chatmacro9", (int *) &chat_macros[9], (int) HUSTR_CHATMACRO9 );
}

//
// M_LoadDefaults
//
extern byte	scantokey[128];

void M_LoadDefaults (void)
{
    int		i;
    int		len;
    FILE*	f;
    char	def[80];
    char	strparm[100];
    char*	newstring;
    int		parm;
    boolean2	isstring;
    
    // set everything to base values
    initDefaults();
    for (i=0 ; i<numdefaults ; i++)
	*defaults[i].location = defaults[i].defaultvalue;
    
    // check for a custom default file
    i = M_CheckParm ("-config");
    if (i && i<myargc-1)
    {
	defaultfile = myargv[i+1];
	printf ("	default file: %s\n",defaultfile);
    }
    else
	defaultfile = basedefault;
    
    // read the file in, overriding any set defaults
    f = fopen (defaultfile, "r");
    if (f)
    {
	while (!feof(f))
	{
	    isstring = false;
	    if (fscanf (f, "%79s %[^\n]\n", def, strparm) == 2)
	    {
		if (strparm[0] == '"')
		{
		    // get a string default
		    isstring = true;
		    len = strlen(strparm);
		    newstring = (char *) malloc(len);
		    strparm[len-1] = 0;
		    strcpy(newstring, strparm+1);
		}
		else if (strparm[0] == '0' && strparm[1] == 'x')
		    sscanf(strparm+2, "%x", &parm);
		else
		    sscanf(strparm, "%i", &parm);
		for (i=0 ; i<numdefaults ; i++)
		    if (!strcmp(def, defaults[i].name))
		    {
			if (!isstring)
			    *defaults[i].location = parm;
			else
			    *defaults[i].location =
				(int) newstring;
			break;
		    }
	    }
	}
		
	fclose (f);
    }
}


//
// SCREEN SHOTS
//


typedef struct
{
    char		manufacturer;
    char		version;
    char		encoding;
    char		bits_per_pixel;

    unsigned short	xmin;
    unsigned short	ymin;
    unsigned short	xmax;
    unsigned short	ymax;
    
    unsigned short	hres;
    unsigned short	vres;

    unsigned char	palette[48];
    
    char		reserved;
    char		color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    
    char		filler[58];
    unsigned char	data;		// unbounded
} pcx_t;


//
// WritePCXfile
//
void
WritePCXfile
( char*		filename,
  byte*		data,
  int		width,
  int		height,
  byte*		palette )
{
    int		i;
    int		length;
    pcx_t*	pcx;
    byte*	pack;
	
    pcx = Z_Malloc (width*height*2+1000, PU_STATIC, NULL);

    pcx->manufacturer = 0x0a;		// PCX id
    pcx->version = 5;			// 256 color
    pcx->encoding = 1;			// uncompressed
    pcx->bits_per_pixel = 8;		// 256 color
    pcx->xmin = 0;
    pcx->ymin = 0;
    pcx->xmax = SHORT(width-1);
    pcx->ymax = SHORT(height-1);
    pcx->hres = SHORT(width);
    pcx->vres = SHORT(height);
    memset (pcx->palette,0,sizeof(pcx->palette));
    pcx->color_planes = 1;		// chunky image
    pcx->bytes_per_line = SHORT(width);
    pcx->palette_type = SHORT(2);	// not a grey scale
    memset (pcx->filler,0,sizeof(pcx->filler));


    // pack the image
    pack = &pcx->data;
	
    for (i=0 ; i<width*height ; i++)
    {
	if ( (*data & 0xc0) != 0xc0)
	    *pack++ = *data++;
	else
	{
	    *pack++ = 0xc1;
	    *pack++ = *data++;
	}
    }
    
    // write the palette
    *pack++ = 0x0c;	// palette ID byte
    for (i=0 ; i<768 ; i++)
	*pack++ = *palette++;
    
    // write output file
    length = pack - (byte *)pcx;
    M_WriteFile (filename, pcx, length);

    Z_Free (pcx);
}


//
// M_ScreenShot
//
void M_ScreenShot (void)
{
    int		i;
    byte*	linear;
    char	lbmname[12];
    
    // munge planar buffer to linear
    linear = screens[2];
    I_ReadScreen (linear);
    
    // find a file name to save it to
    strcpy(lbmname,"DOOM00.pcx");
		
    for (i=0 ; i<=99 ; i++)
    {
	lbmname[4] = i/10 + '0';
	lbmname[5] = i%10 + '0';
	if (access(lbmname,0) == -1)
	    break;	// file doesn't exist
    }
    if (i==100)
	I_Error ("M_ScreenShot: Couldn't create a PCX");
    
    // save the pcx file
    WritePCXfile (lbmname, linear,
		  SCREENWIDTH, SCREENHEIGHT,
		  W_CacheLumpName ("PLAYPAL",PU_CACHE));
	
    players[consoleplayer].message = "screen shot";
}

