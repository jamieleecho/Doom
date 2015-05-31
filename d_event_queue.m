/*
 *  d_event_queue.m
 *  doom
 *
 *  Created by jamiec on Mon Apr 30 2001.
 *  Copyright (c) 2001 __CompanyName__. All rights reserved.
 *
 */

#include <assert.h>
#include "d_event_queue.h"
#define max_num 64

static event_t my_events[max_num];
static int cnt = 0;
static int start_idx = 0;

void event_queue_put(const event_t *e) {
    assert(cnt < max_num);
    my_events[(cnt++ + start_idx) % max_num] = *e;
}

event_t event_queue_get(void) {
    event_t e;
    assert(cnt > 0);
    e = my_events[start_idx];
    start_idx = (start_idx + 1) % max_num;
    cnt--;
    return e;
}

int event_queue_count(void) { return cnt; }
