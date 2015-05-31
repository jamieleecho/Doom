#import <Cocoa/Cocoa.h>

#include "doomdef.h"
#include "m_argv.h"
#include "d_main.h"
#include "d_event_queue.h"
#include "jlcQueue.h"

int main(int argc, char **argv)
{
    myargc = argc; 
    myargv = argv; 

    // Do all of the UI event stuff here
    return NSApplicationMain(argc, (const char **)argv);
}
