#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*
 * $Id$
 * ringbuffer.h
 * Ring Buffer utility..
 *
 * Author: Phil Burk, http://www.softsynth.com
 *
 * This program is distributed with the PortAudio Portable Audio Library.
 * For more information see: http://www.audiomulch.com/portaudio/
 * Copyright (c) 1999-2000 Ross Bencina and Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ringbuffer.h"
#include <string.h>

typedef struct
{
    int32_t   bufferSize; /* Number of bytes in FIFO. Power of 2. Set by RingBuffer_Init. */
/* These are declared volatile because they are written by a different thread than the reader. */
    volatile int32_t   writeIndex; /* Index of next writable byte. Set by RingBuffer_AdvanceWriteIndex. */
    volatile int32_t   readIndex;  /* Index of next readable byte. Set by RingBuffer_AdvanceReadIndex. */
    int32_t   bigMask;    /* Used for wrapping indices with extra bit to distinguish full/empty. */
    int32_t   smallMask;  /* Used for fitting indices to buffer. */
    char *buffer;
}
RingBuffer;
/*
 * Initialize Ring Buffer.
 * numBytes must be power of 2, returns -1 if not.
 */
int32_t RingBuffer_Init( RingBuffer *rbuf, int32_t numBytes, void *dataPtr );

/* Clear buffer. Should only be called when buffer is NOT being read. */
void RingBuffer_Flush( RingBuffer *rbuf );

/* Return number of bytes available for writing. */
int32_t RingBuffer_GetWriteAvailable( RingBuffer *rbuf );
/* Return number of bytes available for read. */
int32_t RingBuffer_GetReadAvailable( RingBuffer *rbuf );
/* Return bytes written. */
int32_t RingBuffer_Write( RingBuffer *rbuf, void *data, int32_t numBytes );
/* Return bytes read. */
int32_t RingBuffer_Read( RingBuffer *rbuf, void *data, int32_t numBytes );

/* Get address of region(s) to which we can write data.
** If the region is contiguous, size2 will be zero.
** If non-contiguous, size2 will be the size of second region.
** Returns room available to be written or numBytes, whichever is smaller.
*/
int32_t RingBuffer_GetWriteRegions( RingBuffer *rbuf, int32_t numBytes,
                                 void **dataPtr1, int32_t *sizePtr1,
                                 void **dataPtr2, int32_t *sizePtr2 );
int32_t RingBuffer_AdvanceWriteIndex( RingBuffer *rbuf, int32_t numBytes );

/* Get address of region(s) from which we can read data.
** If the region is contiguous, size2 will be zero.
** If non-contiguous, size2 will be the size of second region.
** Returns room available to be read or numBytes, whichever is smaller.
*/
int32_t RingBuffer_GetReadRegions( RingBuffer *rbuf, int32_t numBytes,
                                void **dataPtr1, int32_t *sizePtr1,
                                void **dataPtr2, int32_t *sizePtr2 );

int32_t RingBuffer_AdvanceReadIndex( RingBuffer *rbuf, int32_t numBytes );

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _RINGBUFFER_H */
