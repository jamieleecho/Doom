/*
 *  d_event_queue.h
 *  doom
 *
 *  Created by jamiec on Mon Apr 30 2001.
 *  Copyright (c) 2001 __CompanyName__. All rights reserved.
 *
 */
#ifndef __D_EVENT_QUEUE__
#define __D_EVENT_QUEUE__

#include "d_event.h"

void event_queue_put(const event_t *e);
event_t event_queue_get(void);
int event_queue_count(void);

#endif
