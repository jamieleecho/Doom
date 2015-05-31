/*
 *  jlcQueue.h
 *  doom
 *
 *  Created by jcho on Sun Oct 14 2001.
 *  Copyright (c) 2001 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _jlcQueue_h
#define _jlcQueue_h

#include <pthread.h>

typedef struct {
    void *buffer;
    int buffSize;
    void *start;
    int count;
    pthread_mutex_t mutex;
    pthread_mutexattr_t mutexAttr;
} *jlcQueue, _jlcQueue;

jlcQueue jlcQueueAlloc(int buffSize);
int jlcQueueRead(jlcQueue q, void *b, int numBytes);
int jlcQueueWrite(jlcQueue q, void *b, int numBytes);
int jlcQueueCount(jlcQueue q);
void jlcQueueClose(jlcQueue q);

#endif