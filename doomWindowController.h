#import <Cocoa/Cocoa.h>
#import "JCImageView.h"

@interface doomWindowController : NSWindowController
{
    JCImageView *myImageView;
}

-(id)init;
-(void)tick:(NSTimer *)theTimer;
-(void)doubleSize:(id)sender;
-(void)copy:(id)sender;
-(void)newGame:(id)sender;
-(void)loadGame:(id)sender;
-(void)saveGameAs:(id)sender;
-(void)quickSaveGame:(id)sender;
-(void)quickLoadGame:(id)sender;
-(void)performMiniaturize:(id)sender;
-(void)showOptions:(id)sender;
-(void)print:(id)sender;

@end
