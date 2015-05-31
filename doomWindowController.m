#import <ApplicationServices/ApplicationServices.h>
#include <unistd.h>

#import "doomWindowController.h"

#include "doomstat.h"
#include "v_video.h"
#include "i_video.h"

#include "d_event_queue.h"
#include "s_sound.h"
#include "sounds.h"
#include "m_menu.h"

extern unsigned int imageBuff[SCREENWIDTH * SCREENHEIGHT];
static int doom_init_mode = 0;
int isRunning = 0;

int mouseButtonState = 0;

// Adds a keyup event to the event queue.
static void add_key_up_event(int d_key) {
    event_t e;
    e.type = ev_keyup;
    e.data1 = d_key;
    event_queue_put(&e);
}

// Adds a keydown event to the event queue
static void add_key_down_event(int d_key) {
    event_t e;
    e.type = ev_keydown;
    e.data1 = d_key;
    event_queue_put(&e);
}

// Adds a mousemove event to the event queue
void add_mouse_move_event(int deltaX, int deltaY) {
    event_t e;
    e.type = ev_mouse;
    e.data1 = mouseButtonState;
    e.data2 = deltaX;
    e.data3 = deltaY;
    event_queue_put(&e);
}

// Adds a modifier key event to the event queue if the modifier changed.
static unsigned int old_modifiers = 0;
static void add_modifier_event(unsigned int modifiers, unsigned int mask, int d_key) {
    if (old_modifiers & mask)
        if (modifiers & mask) return; // no change
        else add_key_up_event(d_key); // key was lifted
    else
        if (modifiers & mask) add_key_down_event(d_key); // key was pressed
        else return; // no change
}

// Hack - X11 macros that don't have an equivalent Cocoa definition.
#define XK_Escape 27
#define XK_Return 13
#define XK_Tab '\t'
#define XK_Backspace 127
#define XK_equal '='
#define XK_plus '+'
#define XK_minus '-'
#define XK_space ' '
#define XK_asciitilde '~'


static unichar xlatekey(unichar rc) {
    switch(rc)
    {
      case NSLeftArrowFunctionKey:	rc = KEY_LEFTARROW;	break;
      case NSRightArrowFunctionKey:	rc = KEY_RIGHTARROW;	break;
      case NSDownArrowFunctionKey:	rc = KEY_DOWNARROW;	break;
      case NSUpArrowFunctionKey:	rc = KEY_UPARROW;	break;
      case XK_Escape:	rc = KEY_ESCAPE;	break;
      case XK_Return:	rc = KEY_ENTER;		break;
      case XK_Tab:	rc = KEY_TAB;		break;
      case NSF1FunctionKey:	rc = KEY_F1;		break;
      case NSF2FunctionKey:	rc = KEY_F2;		break;
      case NSF3FunctionKey:	rc = KEY_F3;		break;
      case NSF4FunctionKey:	rc = KEY_F4;		break;
      case NSF5FunctionKey:	rc = KEY_F5;		break;
      case NSF6FunctionKey:	rc = KEY_F6;		break;
      case NSF7FunctionKey:	rc = KEY_F7;		break;
      case NSF8FunctionKey:	rc = KEY_F8;		break;
      case NSF9FunctionKey:	rc = KEY_F9;		break;
      case NSF10FunctionKey:	rc = KEY_F10;		break;
      case NSF11FunctionKey:	rc = KEY_F11;		break;
      case NSF12FunctionKey:	rc = KEY_F12;		break;
	
      case XK_Backspace:
      case NSDeleteFunctionKey:	rc = KEY_BACKSPACE;	break;

      case NSF15FunctionKey:
      case NSPauseFunctionKey:	rc = KEY_PAUSE;		break;

      case XK_plus:
      case XK_equal:	rc = KEY_EQUALS;	break;

      case XK_minus:	rc = KEY_MINUS;		break;

      default:
	if (rc >= XK_space && rc <= XK_asciitilde)
	    rc = rc - XK_space + ' ';
	if (rc >= 'A' && rc <= 'Z')
	    rc = rc - 'A' + 'a';
	break;
    }

    return rc;
}


@implementation doomWindowController

-(id)init {
    [super init];
    [[super window] setPreferredBackingLocation:NSWindowBackingLocationVideoMemory];
       
    return self;
}

-(void) awakeFromNib {	
    static char buffer[1024];
    const char *tmp = [[[NSBundle mainBundle] resourcePath] UTF8String];
    sprintf(buffer, "%s/../../..",tmp);
    chdir(buffer);
	
	if ([super.window respondsToSelector:@selector(setPreferredBackingLocation:)])
		[super.window setPreferredBackingLocation:NSWindowBackingLocationVideoMemory];
    [NSTimer scheduledTimerWithTimeInterval:.01666666
                        target: self
                        selector: @selector(tick:)
                        userInfo: @"Invaders Framerate Timer"
                        repeats: YES];
    [[super window] setAcceptsMouseMovedEvents: YES];
    [[super window] center];
    [myImageView setRefusesFirstResponder: YES];
    [myImageView setIgnoresMultiClick: YES];
}

void D_DoomIter(void);
void D_DoomMain(void);
void IdentifyVersion(void);

-(void)tick:(NSTimer *)theTimer {
    NSUserDefaults *myDefaults;
    NSSize sz;

    if (doom_init_mode < 2) {
        myDefaults = [NSUserDefaults standardUserDefaults];
        switch (doom_init_mode) {
        case 0:
            //strDoomDir = [myDefaults stringForKey: @"DOOMWADDIR"];
            //if (strDoomDir != nil)
            //    setenv("DOOMWADDIR", [strDoomDir cString], YES);
            IdentifyVersion ();
            if (gamemode != indetermined) {
                doom_init_mode = 1;
                return;
            } else {
				NSRunAlertPanel(@"WAD File Not Found", @"Please copy the WAD file to the Doom executable directory!", @"Quit", nil, nil);
				[[NSApplication sharedApplication] terminate:self];
				return;
			}

        case -1:        
        doom_init_mode = 1;
        
        default:
            [myDefaults synchronize];
            D_DoomMain();
            doom_init_mode++;
        }
    }
    
    sz = [[super window] frame].size;
    unsigned char *planes[] = {(unsigned char *)imageBuff, NULL};
	NSSize imageSize = {SCREENWIDTH,  SCREENHEIGHT};
	NSBitmapImageRep *myImageRep = [[NSBitmapImageRep alloc]	
					initWithBitmapDataPlanes: planes
					pixelsWide: imageSize.width
					pixelsHigh: imageSize.height
					bitsPerSample: 4
					samplesPerPixel: 3
					hasAlpha: NO
					isPlanar: NO
					colorSpaceName: NSCalibratedRGBColorSpace
					bytesPerRow: imageSize.width * 2
					bitsPerPixel: 16];
	NSImage *image = [[NSImage alloc] initWithSize:imageSize];
	while([[image representations] count] > 0)
		[image removeRepresentation:[[image representations] objectAtIndex:0]];
	[image addRepresentation:myImageRep];
    [myImageRep release];
	[myImageView setImage:image];
	[image release];
	
    D_DoomIter();
    [myImageView setNeedsDisplay: YES];
    [[super window] makeFirstResponder: self];
}

-(void)mouseMoved:(NSEvent *)theEvent {
    CGPoint newCursorPosition;
    int32_t deltaX, deltaY;
    NSRect r, sr;
    static BOOL mouseOn = YES;
    
    CGGetLastMouseDelta( &deltaX, &deltaY );
    add_mouse_move_event(deltaX * 10, deltaY * -10);
    r = [[super window] frame];
    sr = [[[super window] screen] frame];
    if (!menuactive) {
        if (mouseOn) {
            CGAssociateMouseAndMouseCursorPosition(NO);
            mouseOn = NO;
            [NSCursor hide];
            newCursorPosition.x = r.origin.x + r.size.width/2;
            newCursorPosition.y = sr.size.height - (r.origin.y + r.size.height/2);
            CGWarpMouseCursorPosition( newCursorPosition );
        }
    } else {
        if (!mouseOn) {
            CGAssociateMouseAndMouseCursorPosition(YES);
            mouseOn = YES;
            [NSCursor unhide];
        }
    }
}

-(void)mouseDragged:(NSEvent *)theEvent {
    [self mouseMoved:theEvent];
	[super mouseDragged:theEvent];
}
-(void)rightMouseDragged:(NSEvent *)theEvent {
    [self mouseMoved:theEvent];
}
-(void)otherMouseDragged:(NSEvent *)theEvent {
    [self mouseMoved:theEvent];
}

-(void)keyDown:(NSEvent *)theEvent {
    NSString *str;
    int len, ii;
    if (doom_init_mode <= 0) {
        doom_init_mode = -1;
        return;
    }
    str = [theEvent charactersIgnoringModifiers];
    len = [str length];
    for (ii=0; ii<len; ii++)
        add_key_down_event(xlatekey([str characterAtIndex: ii]));
}

-(void)keyUp:(NSEvent *)theEvent {
    NSString *str;
    int len, ii;
    if (doom_init_mode <= 0) {
        doom_init_mode = -1;
        return;
    }
    str = [theEvent charactersIgnoringModifiers];
    len = [str length];
    for (ii=0; ii<len; ii++)
        add_key_up_event(xlatekey([str characterAtIndex: ii]));
}

-(void)mouseDown:(NSEvent *)theEvent {
    mouseButtonState |= LEFT_MOUSE;
    add_mouse_move_event(0, 0);
	[super mouseDown:theEvent];
}

-(void)mouseUp:(NSEvent *)theEvent {
    mouseButtonState &= ~LEFT_MOUSE;
    add_mouse_move_event(0, 0);
	[super mouseUp:theEvent];
}

-(void)rightMouseDown:(NSEvent *)theEvent {
    mouseButtonState |= RIGHT_MOUSE;
    add_mouse_move_event(0, 0);
}
-(void)rightMouseUp:(NSEvent *)theEvent {
    mouseButtonState &= ~RIGHT_MOUSE;
    add_mouse_move_event(0, 0);
}
-(void)otherMouseDown:(NSEvent *)theEvent {
    mouseButtonState |= OTHER_MOUSE;
    add_mouse_move_event(0, 0);
}
-(void)otherMouseUp:(NSEvent *)theEvent {
    mouseButtonState &= ~OTHER_MOUSE;
    add_mouse_move_event(0, 0);
}

-(void)flagsChanged:(NSEvent *)theEvent {
    unsigned int modifiers = [theEvent modifierFlags];
    add_modifier_event(modifiers, NSShiftKeyMask, KEY_RSHIFT);
    add_modifier_event(modifiers, NSControlKeyMask, KEY_RCTRL);
    add_modifier_event(modifiers, NSAlternateKeyMask, KEY_RALT);
    old_modifiers = modifiers;
}

-(void)normalSize:(id)sender {
    NSSize imageSize = {SCREENWIDTH, SCREENHEIGHT};
    [[super window] setContentSize: imageSize];
    [[super window] center];
}

-(void)doubleSize:(id)sender {
    NSSize imageSize = {SCREENWIDTH*2, SCREENHEIGHT*2};
    [[super window] setContentSize: imageSize];
    [[super window] center];
}

-(void)copy:(id)sender {
    NSRect rect = [[super window] frame];
    rect.origin.x = 0;
    rect.origin.y = 0;
}

// Must figure out a better way to do this...
-(void)newGame:(id)sender {
    M_StartControlPanel();
    S_StartSound(NULL,sfx_swtchn);
    M_NewGame(0);
}

-(void)loadGame:(id)sender {
    M_StartControlPanel();
    S_StartSound(NULL,sfx_swtchn);
    M_LoadGame(0);
}

-(void)saveGameAs:(id)sender {
    M_StartControlPanel();
    S_StartSound(NULL,sfx_swtchn);
    M_SaveGame(0);
}

-(void)quickSaveGame:(id)sender {
    S_StartSound(NULL,sfx_swtchn);
    M_QuickSave();
}

-(void)quickLoadGame:(id)sender {
    S_StartSound(NULL,sfx_swtchn);
    M_QuickLoad();
}

-(void)performClose:(id)sender {
    [[super window] performClose: sender];
}

-(void)performMiniaturize:(id)sender {
    [[super window] performMiniaturize: sender];
}

-(void)performZoom:(id)sender {
    [[super window] performZoom: sender];
}

-(void)showOptions:(id)sender {
    M_StartControlPanel();
    S_StartSound(NULL,sfx_swtchn);
    M_Options(0);
}

-(void)print:(id)sender {
    [[super window] print: sender];
}

-(void)showHelp:(id)sender {
    M_StartHelp();
}

-(BOOL)windowShouldClose:(id)sender {
    M_StartControlPanel();
    S_StartSound(NULL,sfx_swtchn);
    M_QuitDOOM(0);
    return NO;
}

-(void)quitDoom:(id)sender {
    [self windowShouldClose: sender];
}

@end
