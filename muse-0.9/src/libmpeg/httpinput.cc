/* MPEG Sound library

   (C) 1997 by Woo-jae Jung */

// Httpinputstream.cc
// Inputstream for http

// It's from mpg123

#include <sys/stat.h>
#include <unistd.h>

#include "mpegsound.h"

#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>

#include <config.h>

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

static const char *httpstr="http://";

static int  
rwrite( int fd, const void *buf, int len )
{
    int ret;
    
    do {
        ret = write(fd, buf, len);
    } while (ret==-1 && errno==EINTR);
    return ret;
}

bool Soundinputstreamfromhttp::writestring(int fd, char *string)
{
  int result,bytes=strlen(string);

  while(bytes)
  {
    if((result=rwrite(fd,string,bytes))<0)
    {
      seterrorcode(SOUND_ERROR_HTTPWRITEFAIL);
      return false;
    }
    else if(result==0)
    {
      seterrorcode(SOUND_ERROR_HTTPWRITEFAIL);
      return false;
    }
    string += result;
    bytes -= result;
  }

  return true;
}

static int  
rgetc( FILE *f )
{
    int ret;
    
    do {
        ret = getc(f);
    } while (ret==EOF && errno==EINTR);
    return ret;
}

static char *  
rfgets( char *s, int size, FILE *f )
{
    char *ret = s;
    
    for (; size>0; size--, s++) {
        *s = rgetc(f);
	    if (*s == '\n') break;
    }
    *s = '\0';
    return ret;
}

bool Soundinputstreamfromhttp::readstring(char *string,int maxlen,FILE *f)
{
  char *result;

  result=fgets(string,maxlen,f);
  if(!result)
  {
    seterrorcode(SOUND_ERROR_FILEREADFAIL);
    return false;
  }

  return true;
}

static char *strndup(char *src,int num)
{
  char *dst;

  if(!(dst=(char *)malloc(num+1)))return NULL;
  dst[num]='\0';

  return strncpy(dst, src, num);
}

static char *url2hostport(char *url,char **hname,
			  unsigned long *hip,unsigned int *port)
{
  char *cptr;
  struct hostent *myhostent;
  struct in_addr myaddr;
  int isip=1;

  if(!(strncmp(url,httpstr,7)))url+=7;
  cptr=url;
  while(*cptr && *cptr!=':' && *cptr!='/')
  {
    if((*cptr<'0' || *cptr>'9') && *cptr!='.')isip=0;
    cptr++;
  }
  if(!(*hname=strndup(url,cptr-url)))
  {
    *hname=NULL;
    return NULL;
  }
  if(!isip)
  {
    if (!(myhostent=gethostbyname(*hname)))return NULL;
    memcpy(&myaddr,myhostent->h_addr,sizeof(myaddr));
    *hip=myaddr.s_addr;
  }
  else if((*hip=inet_addr(*hname))==INADDR_NONE)return NULL;
  if(!*cptr || *cptr=='/')
  {
    *port=80;
    return cptr;
  }
  *port=atoi(++cptr);
  while(*cptr && *cptr!='/')cptr++;
  return cptr;
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

char *proxyurl=NULL;
unsigned long proxyip=0;
unsigned int proxyport;

FILE *Soundinputstreamfromhttp::http_open(char *url)
{
  char *purl=NULL,*host,*request,*sptr;
  char agent[50];
  int linelength;
  unsigned long myip;
  unsigned int myport;
  int relocate=0,numrelocs=0;
  struct sockaddr_in server;
  FILE *myfile;

  close();

  if(!proxyip)
  {
    if(!proxyurl)
      if(!(proxyurl=getenv("MP3_HTTP_PROXY")))
	if(!(proxyurl=getenv("http_proxy")))
	  proxyurl = getenv("HTTP_PROXY");
    if (proxyurl && proxyurl[0] && strcmp(proxyurl, "none"))
    {
      if (!(url2hostport(proxyurl, &host, &proxyip, &proxyport)))
      {
	seterrorcode(SOUND_ERROR_UNKNOWNPROXY);
	return NULL;
      }
      if(host)free(host);
    }
    else
      proxyip = INADDR_NONE;
  }
  
  if((linelength=strlen(url)+100)<1024)
    linelength=1024;
  if(!(request=(char *)malloc(linelength)) || !(purl=(char *)malloc(1024))) 
  {
    seterrorcode(SOUND_ERROR_MEMORYNOTENOUGH);
    return NULL;
  }
  strncpy(purl,url,1023);
  purl[1023]='\0';
  do{
    strcpy(request,"GET ");
    if(proxyip!=INADDR_NONE) 
    {
      if(strncmp(url,httpstr,7))
	strcat(request,httpstr);
      strcat(request,purl);
      myport=proxyport;
      myip=proxyip;
    }
    else
    {
      if(!(sptr=url2hostport(purl,&host,&myip,&myport)))
      {
	seterrorcode(SOUND_ERROR_UNKNOWNHOST);
	return NULL;
      }
      if (host)
	free (host);
      strcat (request, sptr);
    }
    sprintf (agent, " HTTP/1.0\r\nUser-Agent: %s/%s\r\n\r\n",
	     PACKAGE,VERSION);
    strcat (request, agent);
    server.sin_family = AF_INET;
    server.sin_port = htons(myport);
    server.sin_addr.s_addr = myip;
    if((sock=socket(PF_INET,SOCK_STREAM,6))<0)
    {
      seterrorcode(SOUND_ERROR_SOCKET);
      return NULL;
    }
    if(rconnect(sock,(struct sockaddr *)&server,sizeof(server)))
    {
      seterrorcode(SOUND_ERROR_CONNECT);
      return NULL;
    }
	socket_block(sock);
    if(!writestring(sock,request))return NULL;
    if(!(myfile=fdopen(sock, "rb")))
    {
      seterrorcode(SOUND_ERROR_FDOPEN);
      return NULL;
    };
    relocate=false;
    purl[0]='\0';
    if(!readstring(request,linelength-1,myfile))return NULL;
    if((sptr=strchr(request,' ')))
    {
      switch(sptr[1])
      {
        case '3':relocate=true;
        case '2':break;
        default: seterrorcode(SOUND_ERROR_HTTPFAIL);
	         return NULL;
      }
    }
    do{
      if(!readstring(request,linelength-1,myfile))return NULL;
      if(!strncmp(request,"Location:",9))
	strncpy (purl,request+10,1023);
    }while(request[0]!='\r' && request[0]!='n');
  }while(relocate && purl[0] && numrelocs++<5);
  if(relocate)
  { 
    seterrorcode(SOUND_ERROR_TOOMANYRELOC);
    return NULL;
  }
  free(purl);
  free(request);
  return myfile;
}

Soundinputstreamfromhttp::Soundinputstreamfromhttp()
{
  fp=NULL;
  sock=-1;
}

Soundinputstreamfromhttp::~Soundinputstreamfromhttp()
{
  close();
}

bool Soundinputstreamfromhttp::open(char *url)
{
  if((fp=http_open(url))==NULL)
  {
    seterrorcode(SOUND_ERROR_FILEOPENFAIL);
    return false;
  }

  return true;
}

void Soundinputstreamfromhttp::close() {
  if(fp) {
    fclose(fp);
    fp = NULL;
  }
  if(sock>0) {
    shutdown(sock,2);
    sock = -1;
  }
}

int Soundinputstreamfromhttp::getbytedirect(void)
{
  int c;

  if((c=rgetc(fp))<0)
  {
    seterrorcode(SOUND_ERROR_FILEREADFAIL);
    return -1;
  }

  return c;
}

bool Soundinputstreamfromhttp::_readbuffer(char *buffer,int size)
{
  int len=0;
  do {
    len += fread(buffer+len,1,size-len,fp);
  } while (len<size && !feof(fp) && !ferror(fp) && errno==EINTR);
  
  if(len!=size)
  {
    seterrorcode(SOUND_ERROR_FILEREADFAIL);
    return false;
  }
  return true;
}

bool Soundinputstreamfromhttp::eof(void)
{
  return feof(fp);
};

int Soundinputstreamfromhttp::getblock(char *buffer,int size)
{
  int l=0;
  do {
    l += fread(buffer+l,1,size-l,fp);
  } while (l<size && !feof(fp) && !ferror(fp) && errno==EINTR);
  
  if(l!=size) seterrorcode(SOUND_ERROR_FILEREADFAIL);
  return l;
}

int Soundinputstreamfromhttp::getsize(void)
{
  return 0;
}

void Soundinputstreamfromhttp::setposition(int)
{
}

int  Soundinputstreamfromhttp::getposition(void)
{
  return 0;
}

