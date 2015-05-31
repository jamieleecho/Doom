//
//  JCImageView.m
//  doom
//
//  Created by jcho on Sat Oct 20 2001.
//  Copyright (c) 2001 __MyCompanyName__. All rights reserved.
//

#import "JCImageView.h"

@implementation JCImageView

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    return self;
}

-(void)mouseMoved:(NSEvent *)theEvent { }

-(void)mouseDragged:(NSEvent *)theEvent { }

-(void)rightMouseDragged:(NSEvent *)theEvent { }

-(void)otherMouseDragged:(NSEvent *)theEvent { }

-(void)mouseDown:(NSEvent *)theEvent { }

-(void)mouseUp:(NSEvent *)theEvent { }

-(void)rightMouseDown:(NSEvent *)theEvent { }

-(void)rightMouseUp:(NSEvent *)theEvent { }

-(void)otherMouseDown:(NSEvent *)theEvent { }

-(void)otherMouseUp:(NSEvent *)theEvent { }

@end
