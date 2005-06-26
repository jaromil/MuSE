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
 *
 * "$Id$"
 *
 */


#include <config.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h> /*isdigit*/

#include <httpstream.h>

#define SOCKET_ERROR    (-1)
#define INVALID_SOCKET  (-1)


static int
rread( int fd, void *buf, int len )
{
    int ret;
    
    do {
        ret = read(fd, buf, len);
    } while (ret==-1 && errno==EINTR);
    return ret;
}


static int  
rwrite( int fd, const void *buf, int len )
{
    int ret;
    
    do {
        ret = write(fd, buf, len);
    } while (ret==-1 && errno==EINTR);
    return ret;
}


static int 
rsend( int sock, const char *buf, int len, int flags )
{
    int ret;
    
    do {
        ret = send(sock, buf, len, flags);
    } while (ret==-1 && errno==EINTR);
    return ret;
}


static int  
rrecv( int sock, char *buf, int len, int flags )
{
    int ret;
    
    do {
        ret = recv(sock, buf, len, flags);
    } while (ret==-1 && errno==EINTR);
    return ret;
}



static bool
socket_nonblock( int sock )
{
    int file_flags; 
    int old_errno = errno;

    /* next call sets errno 38 ENOSYS on unix sockets */
    if ( (file_flags = fcntl(sock, F_GETFL, 0)) == -1 ) {
        return false;
    }
    if ( fcntl(sock, F_SETFL, (file_flags | O_NONBLOCK)) ) {
        return false;
    }

    errno = old_errno;
    return true;
}


static bool
socket_block( int sock )
{
    int file_flags; 
    
    if ( (file_flags = fcntl(sock, F_GETFL, 0)) == -1 ) {
        return false;
    }
    if ( fcntl(sock, F_SETFL, (file_flags & ~O_NONBLOCK)) ) {
        return false;
    }
    
    return true;
}


static int
raccept( int s, struct sockaddr *addr, socklen_t *addrlen )
{
    int rc;
    
    do {
        rc = accept( s, addr, addrlen );
    } while ( rc == -1 && errno == EINTR );
    
    return rc;
}


static int
rconnect( int sock, const struct sockaddr *saddr, socklen_t slen )
{
    int rc;
    
    do {
        rc = connect( sock, saddr, slen );
    } while ( rc == -1 && errno == EINTR );
    
    return rc;
}


in_addr_t 
host_to_ip( const char* name )
{
    struct hostent* hostStruct = NULL;
    struct in_addr* hostNode   = NULL;
    in_addr_t       ret = (in_addr_t)0;

    /* FIXME: inet_aton */
    ret = inet_addr( name );
    if ( ret != (in_addr_t)-1 ) 
        return ret;

    hostStruct = gethostbyname( name );
    if ( NULL == hostStruct )
        return( (in_addr_t)0 ); //not found

    /* extract IP */
    hostNode = (struct in_addr*) hostStruct->h_addr;
    return( hostNode->s_addr );
}


int 
fullsend( int sock, const char *buf, int buflen, int flags )
{
    int sent = 0, lastsend = 0;

    while (sent < buflen) {
        errno = 0;
        lastsend  = rsend(sock, buf+sent*sizeof(char), buflen-sent, flags);
        if (lastsend < 0) {
            /* no need for timeout check, when remote side closes the 
               socket, an error will be returned */
            if (errno==EAGAIN || errno==EINTR) {
                continue;
            } else {
                return sent; 
            }
        } else if (lastsend == 0) {
            return sent;
        } else {
            sent += lastsend;
        }
    }

    return sent;
}


int 
fullrecv( int sock, char *buf, int buflen, int flags )
{
    int recvsofar = 0, lastrecv = 0;

    while (recvsofar < buflen) {
        errno = 0;
        lastrecv  = rrecv(sock, buf+recvsofar*sizeof(char), buflen-recvsofar, flags);
        
        /*socket closed*/
        if ( lastrecv == 0 )  {
            return recvsofar;
        } else if (lastrecv < 0) {
            return lastrecv; /*<0*/
        } else {
            recvsofar += lastrecv;
        }
    }

    return recvsofar;
}


/*
 * Restart select if call interrupted. 
 */
static int 
rselect( int n, fd_set *rds, fd_set *wds, fd_set *eds, struct timeval *tout )
{
    int rc;
    
    do {
        rc = select( n, rds, wds, eds, tout );
    } while ( rc == -1 && errno == EINTR );
    
    return rc;
}


/* IP specific.  Do not allow options */
static bool
check_ip_options( int sock )
{
    int             ipproto;
    struct protoent *ip;
    unsigned char   optbuf[BUFSIZ/3];
    int             optsz = sizeof(optbuf);

    if ( (ip=getprotobyname("ip")) != NULL )
        ipproto = ip->p_proto;
    else
        ipproto = IPPROTO_IP;
    
    if (  getsockopt(sock, ipproto, IP_OPTIONS, (char*)optbuf, (socklen_t*)&optsz) == 0 
       && optsz != 0 ) {
       /* turn off options */
       if ( setsockopt(sock, ipproto, IP_OPTIONS, NULL, optsz ) != 0 )
           return false;
    }
    
    return true;
}


static int 
muse_connect( const char* host, int port )
{
    int                rsrv = INVALID_SOCKET;
    struct sockaddr_in rsrvINETAddress;
    struct sockaddr    *rsrvSockAddrPtr = NULL;
    int                selret, conret;
    struct timeval     tval = {5, 0}; 
    fd_set             rset, wset;

    rsrvSockAddrPtr = (struct sockaddr*)&rsrvINETAddress;
    memset( (char*)&rsrvINETAddress, 0, sizeof(rsrvINETAddress) );
    rsrvINETAddress.sin_family      = AF_INET;
    rsrvINETAddress.sin_addr.s_addr = host_to_ip( host );
    rsrvINETAddress.sin_port        = htons( port );
    
    rsrv = socket( AF_INET, SOCK_STREAM, 0 );
    if ( !socket_nonblock(rsrv) ) {
        close( rsrv );
        return INVALID_SOCKET;
    }

    conret = rconnect( rsrv, (struct sockaddr*)rsrvSockAddrPtr, sizeof(rsrvINETAddress) ); 
    if ( conret == 0 )
        goto ok;
    
    if ( conret < 0 && errno != EINPROGRESS ) {
        close( rsrv ); 
        return INVALID_SOCKET;
    }
    errno = 0;
    
    /* 
     * Else wait for connection.  Cannot use cfgst_microsleep 
     */
     
    FD_ZERO( &rset );
    FD_SET( rsrv, &rset );
    wset = rset; 
    selret = rselect( rsrv+1, &rset, &wset, NULL, &tval ); 
    if ( selret == 0 ) {
        close( rsrv );        /* timeout */
        errno = ETIMEDOUT;
        return INVALID_SOCKET;
    }

    if ( FD_ISSET(rsrv, &rset) || FD_ISSET(rsrv, &wset) ) {
        int error=0, optret, len=sizeof(error);
        optret = getsockopt( rsrv, SOL_SOCKET, SO_ERROR, &error, (socklen_t*)&len ); 
        if ( optret < 0 || error != 0 ) {
            /* Solaris pending error */ 
            errno = error; 
            warning("  SOL_SOCKET SO_ERROR=%d", error); 
            /*perror( "cfgst_connect" );*/
            close( rsrv );
            return INVALID_SOCKET;    
        }
    } else {
        warning("  select error: sockfd not set"); 
        close( rsrv );
        return INVALID_SOCKET;
    }
        
    if ( !socket_block(rsrv) ) {
        close( rsrv );
        return INVALID_SOCKET;
    }
ok:
    notice("Connected to %s:%d", host, port); 
    return rsrv;
}


static int  
disconnect( int sock )
{
    /* 2: send and receives disallowed */
    return shutdown(sock, 2);
}


#define HTTP_PREFIX "http://"
FILE *
http_open( const char *url )
{
    int sock = INVALID_SOCKET;
    FILE *fd = NULL;
    char *host=NULL, *file=NULL, *p;
	int port = 0;
	
	host = (char*)calloc( 1, strlen(url)+1 ); 
	file = (char*)calloc( 1, strlen(url)+1 );
	if (!host || !file) goto out;
	
	p = strstr(url, HTTP_PREFIX); if (!p) goto out;
	p += 7;
	strcpy(host, p);
	
	p = strchr(host, '/');
	if (p) {
	    strcpy(file, p);
		*p = '\0';
	} else {
	    *file = '/';
	}
	
	p = strchr(host, ':');
	if (p) {
	    *p++ = '\0';
		port = atoi(p);
	} else {
	    port = 80;
	}
		
	notice("http_open host='%s', port=%d, file='%s'", host, port, file);
	sock = muse_connect(host, port);
	if (sock > 0 ) { 
		fd = fdopen(sock, "rb");
		fullsend(sock, "GET ", 4, 0);
		fullsend(sock, file, strlen(file), 0);
		fullsend(sock, " HTTP/1.0\r\n\r\n", 13, 0);
	}
	
out:
    if (host) free(host);
	if (file) free(file);
    return fd;
}


static void
strip_header( FILE *fd )
{
    int crlf=0;
	while (crlf<2 && !feof(fd)) {
	  int ch = fgetc(fd);
	  if (ch == '\n') crlf++;
	  else if (ch == '\r') /*nothing*/;
	  else crlf = 0;
	}
}


FILE *
hopen( const char *url, const char *mode )
{
    FILE *fd = NULL;
    if (!url) return NULL;
    act("opening stream: '%s'", url);
    if (strstr(url, HTTP_PREFIX)) {
	    fd = http_open(url);
		if (fd) {
			strip_header(fd);
			/*if server replies with err message and closes*/
			if (feof(fd)) {fclose(fd); fd=NULL;}
		}
		return fd;
    } else {
        return fopen(url, mode);
	}
}


/* return \0 terminated string */
static char *
read_header( FILE *fd )
{
    int crlf=0;
	int len=128, used=0;
	char *header = (char*)calloc(1, 128);
	
	if (!header) return NULL;
	
	while (crlf<2 && !feof(fd)) {
		int ch = fgetc(fd);

		if (ch == '\n') crlf++;
		else if (ch == '\r') /*nothing*/;
		else crlf = 0;
	    //printf("%c", ch);
		
		header[used++] = ch;

		if (used == len) {
		    len *= 2;
			header = (char*)realloc(header, len);
			if (!header) return NULL;
		}
	}

    header[used] = '\0';
    return header;
}


hstream 
stream_detect( const char *url )
{
    FILE *fd;
    char *hdr = NULL;
    hstream ret = HS_NONE;
    
    if (!url) return HS_NONE;
    if (!strstr(url, HTTP_PREFIX))  return HS_NONE;
    if ((fd=http_open(url)) == NULL) return HS_NONE;
    
    hdr = read_header(fd);
    func("stream detect got header:\n%s\n",hdr);
    fclose(fd);
    if (!hdr) return HS_NONE;
    
    // simple analysis of Content-Type:
    if (strstr(hdr,"audio/mpeg"))
      ret = HS_MP3;
    else if (strstr(hdr,"application/ogg")) 
      ret = HS_OGG;
    else if (strstr(hdr,"SHOUTcast") || strstr(hdr,"ICY 200")) 
      ret = HS_MP3;
    else if (strstr(hdr,"icy-")) // add for compatibility to proton
      ret = HS_MP3;
    
	 
	 
    free(hdr);
    return ret;
}

