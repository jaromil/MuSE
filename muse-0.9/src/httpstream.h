/*  Streaming over http tools. 
 *  (c) Copyright 2001 Eugen Melinte <ame01@gmx.net>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef HTTPSTREAM_H
#define HTTPSTREAM_H


#include <stdio.h>

#include <outchannels.h> //codec

typedef enum {
    HS_MP3,
	HS_OGG,
	HS_NONE, //cannot detect
} hstream;

/** fopen() syntax.  If @param url is an url, it will open the stream, 
    strip the header and return a valid FILE*, otherwise it will return
	whatever fopen() returns.  */
FILE *hopen( const char *url, const char *mode );

/** Will open a connection to url, analyse header and close connection.  */
hstream stream_detect( const char *url );


#endif //HTTPSTREAM_H
