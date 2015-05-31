/*
 *  jlcQueue.c
 *  doom
 *
 *  Created by jcho on Sat Oct 13 2001.
 *  Copyright (c) 2001 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdlib.h>
#include <string.h>

#include "jlcQueue.h"

/**
 * Creates and returns a queue with the given buffSize.
 *
 * @param buffSize is the size of the queue in bytes.
 * @return jlcQueue is the returned queue.
 */
jlcQueue jlcQueueAlloc(int buffSize) {
    jlcQueue q = malloc(sizeof(_jlcQueue));
    if (q == NULL) return NULL;
    q->buffer = malloc(buffSize);
    if (q->buffer == NULL) return NULL;
    q->start = q->buffer;
    q->buffSize = buffSize;
    q->count = 0;

    pthread_mutexattr_init(&(q->mutexAttr));
    pthread_mutex_init(&q->mutex, &(q->mutexAttr));

    return q;
}

/**
 * Reads upto numBytes bytes from the given queue and stores
 * the results in b. If there are not numBytes available,
 * stores the number of bytes in the queue in b.
 *
 * @param q is the queue from which to read.
 * @param b is the buffer in which to store the results.
 * @param numBytes is the maximum number of bytes to read.
 * @return the number of bytes read.
 */
int jlcQueueRead(jlcQueue q, void *b, int numBytes) {
    int numBytes1;
    
    pthread_mutex_lock(&(q->mutex));
    
    if (numBytes > q->count)
        numBytes = q->count;
    
    numBytes1 = (q->buffSize - (q->start - q->buffer));
    if (numBytes1 <= numBytes) {
        memcpy(b, q->start, numBytes1);
        memcpy(b + numBytes1, q->buffer, numBytes - numBytes1);
        q->start = q->buffer + numBytes - numBytes1;
    } else {
        memcpy(b, q->start, numBytes);
        q->start += numBytes;
    }

    q->count -=numBytes;
    pthread_mutex_unlock(&(q->mutex));
    return numBytes;
}

/**
 * Writes upto numBytes bytes from the given buffer into the
 * given queue. If the queue is full, will only write the
 * mamximum number of bytes needed to fill the queue.
 *
 * @param q is the queue to write to.
 * @param b is the buffer whose contents are written to the queue.
 * @param numBytes is the maximum number of bytes to write.
 * @return the number of bytes written.
 */
int jlcQueueWrite(jlcQueue q, void *b, int numBytes) {
    int numBytes1;
    void *p;
    
    pthread_mutex_lock(&(q->mutex));

    if (q->count + numBytes > q->buffSize)
        numBytes = q->buffSize - q->count;

    p = ((q->start - q->buffer + q->count) % q->buffSize) + q->buffer;
    numBytes1 = q->buffer + q->buffSize - p;
    if (numBytes1 < numBytes) {
        memcpy(p, b, numBytes1);
        memcpy(q->buffer, b + numBytes1, numBytes - numBytes1);
    } else {
        memcpy(p, b, numBytes);
    }

    q->count += numBytes;
    pthread_mutex_unlock(&(q->mutex));
    return numBytes;
}

/** @return the number of bytes currently in the queue. */
int jlcQueueCount(jlcQueue q) {
    return q->count;
}

/** Closes the given stream. */
void jlcQueueClose(jlcQueue q) {
    pthread_mutex_destroy(&(q->mutex));
    pthread_mutexattr_destroy(&(q->mutexAttr));
    free(q->buffer);
    q->buffer = 0;
    free(q);
}
