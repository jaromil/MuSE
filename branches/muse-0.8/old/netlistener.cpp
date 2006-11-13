/* netlistener.cpp - MuSE

   this class is a synchronous object
   comes out a thread launched in the main() (muse.cpp)

   network command are processed here
   commands received from the listening tcp socket looks like:
   CMD\tARG\tARG....\n
   the daemon can be easily overflowed i guess :^)
   that's the right place where to look for discovering
   what they are the commands taken by the interface.

   TAKE CARE THIS FILE IS NOT TESTED AND MOSTLY WILL NOT WORK

*/

#include <iostream.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stream.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>

#include "netlistener.h"

Net_listener::Net_listener(int porta, Stream_mixer *mix) {
  num_porta = porta;
  int one = 1;
  
  if((id_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0)
    perror("Unable to open tcp/ip socket");
  
  if(setsockopt(id_socket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) < 0)
    perror("Unable to set socket parameters");
  
  memset(&nome_socket,0,sizeof(struct sockaddr_in));
  memset(&nome_accept,0,sizeof(struct sockaddr_in));
  
  nome_socket.sin_family = AF_INET;
  nome_socket.sin_port = htons(num_porta);
  nome_socket.sin_addr.s_addr = htonl(INADDR_ANY);
  
  id_accept = -2;
  
  if(bind(id_socket, (struct sockaddr *) &nome_socket, sizeof(nome_socket))==-1)
    perror("Unable to bind tcp/ip connection");
  
  if(listen(id_socket, 5)==-1)
    perror("Cannot listen over tcp/ip");

  mixer = mix;

  disconnect = false;

  mixer->client = &id_accept;

}

void Net_listener::get_connection() {
  bool authorized = false;
  while(!authorized) {
    dim_nome_accept = sizeof(struct sockaddr_in);
    id_accept = accept(id_socket, (struct sockaddr *) &nome_accept, &dim_nome_accept);

    getout("PWD\n");
    getin();

    if(strncmp(PASSWD,buffer,strlen(PASSWD))==0)
      authorized = true;
    else {
      getout("Sorry, authentication failed\n");
      chiudi_socket();
    }
  }
  getout(HELLO);

  /* dirty dirty dirty dirty dirty dirty dirty dirty dirty dirty
     but effective right now.
     in fact we're going to make a playlist selector soon.... 'nuff said */
  getout("201\n");

  /*
    int c;
    for(c=0;c<mixer->mp3ch[0].playlist_len;c++) {
    getout(mixer->mp3ch[0].playlist[c]);
    getout("\n");
    }
  */

  getout("/porcodio/gesu/madonnaputtana.mp3\n");

  getout("202\n");

  disconnect = false;

}    

void Net_listener::run() {
  int res;
  
  while(!mixer->quit) {
    get_connection();
    /* send_playlist(); */
    do {
      if(getin()<=0)
	break;
      buffer[3] = '\0';
      res = esegui_comando(buffer);
      switch(res) {
      case 0:
	getout("400\n"); /* ok, request shall have success */
	break;
      case 1:
	getout("401\n"); /* syntax incorrect */
	break;
      case 2:
	getout("402\n"); /* problem connecting to icecast */
	break;
      default:
	getout("403\n"); /* i got a bad feeling! */
	break;
	/*
	  case 2:
	  getout("402\n\n"); // channel in use 
	  break;
	  case 3:
	  getout("403\n\n"); // channel not allocated
	  break;
	  case 4:
	  getout("404\n\n"); // the request cannot be processed
	  break;
	  case 5:
	  getout("405\n"); // playlist position out of bounds
	  break;
	*/
      }
      
    } while(!disconnect);
    chiudi_socket();
  } /* while(!mixer->quit) */
  
}

int Net_listener::esegui_comando(char *buf) {

  char *p, *pp;
 
  /* 301: start mp3 file on channel with volume & speed
     301'\t'(int)fl'\t'(int)ch'\t'(float)speed'\t'(float)volume'\n' */
    if(strncmp(buf,"301",3)==0) {
      int fnum, ch;
      float speed, vol;
      p = pp = &buf[4];
      while(*p!='\t') p++; *p = '\0';
      fnum = atoi(pp);
      p++; pp = p;
      while(*p!='\t') p++; *p = '\0';
      ch = atoi(pp);
      p++; pp = p;
      while(*p!='\t') p++; *p = '\0';
      speed = atof(pp);
      p++; pp = p;
      while(*p!='\n') p++; *p = '\0';
      vol = atof(pp);
      
      fprintf(stderr,"cmd:301 file:%u channel:%u speed:%f volume:%f\n",ch,fnum,speed,vol);
      
      mixer->set_channel(ch,fnum);
      
      if(vol!=1.0)
	mixer->set_volume(ch,vol);
      
      /*      
	      if(speed!=1.0) {
	      synchronous.select.call(0, mixer, Stream_mixer::id_stream_mixer);
	      synchronous.wait();
	      mixer->set_speed(ch,speed);
	      }
      */
      return(0);
    }
    
    /* 302: stop channel - frees the channel and stops the stream
       302'\t'(int)ch'\n' */
    if(strncmp(buf,"302",3)==0) {
      int ch;
      p = pp = &buf[4];
      while(*p!='\n') p++; *p = '\0';
      ch = atoi(pp);

      fprintf(stderr,"cmd:302 channel:%u\n",ch);

      mixer->stop_channel(ch);
      
      return(0);
    }

    /* 303: pause channel - stop reading from the channel stream (without kill)
       303'\t'(int)ch'\n' */
    if(strncmp(buf,"303",3)==0) {
      int ch;
      p = pp = &buf[4];
      while(*p!='\n') p++; *p = '\0';
      ch = atoi(pp);

      fprintf(stderr,"cmd:303 channel:%u\n",ch);

      mixer->pause_channel(ch);
      
      return(0);
    }

    /* 304: resume channel - cannot be used after killing, shall be use after pause
       304'\t'(int)ch'\n'
       if(strncmp(buf,"304",3)==0) {
       int ch;
       p = pp = &buf[4];
       while(*p!='\n') p++; *p = '\0';
       ch = atoi(pp);
       
       fprintf(stderr,"cmd:304 channel:%u\n",ch);
       
       synchronous.select.call(0, mixer, Stream_mixer::id_resume_channel);
       synchronous.wait();
       mixer->resume_channel(ch);
       
       return(0);
       }
    */

    /* 305: set speed - change the speed of a channel
       305'\t'(int)ch'\t'(float)speed'\n' */
    if(strncmp(buf,"305",3)==0) {
      int ch;
      float speed;
      p = pp = &buf[4];
      while(*p!='\t') p++; *p = '\0';
      ch = atoi(pp);
      p++; pp = p;
      while(*p!='\n') p++; *p = '\0';
      speed = atof(pp);
      
      fprintf(stderr,"cmd:305 channel:%u speed:%f\n",ch,speed);
      /*
	synchronous.select.call(0, mixer, Stream_mixer::id_stream_mixer);
	synchronous.wait();
	mixer->set_speed(ch,speed);
      */
      return(0);
    }

    /* 306: set volume - change the volume of a channel
       306:'\t'(int)ch'\t'(float)volume'\n' */
    if(strncmp(buf,"306",3)==0) {
      int ch;
      float vol;
      p = pp = &buf[4];
      while(*p!='\t') p++; *p = '\0';
      ch = atoi(pp);
      p++; pp = p;
      while(*p!='\n') p++; *p = '\0';
      vol = atof(pp);
      
      fprintf(stderr,"cmd:306 channel:%u volume:%f\n",ch,vol);

      mixer->set_volume(ch,vol);

      return(0);
    }
    
    /* 307: set live - switch live input channel on/off
       307'\t'(int)num'\n' */
    if(strncmp(buf,"307",3)==0) {
      int stat;
      p = pp = &buf[4];
      while(*p!='\n') p++; *p = '\0';
      stat = atoi(pp);

      fprintf(stderr,"cmd:307 status:%u\n",stat);

      if(mixer->set_live(stat))
	return(0);

      return(4);
    }

    /* 308: set dsp out - RESERVED */

    /* 309: set playlist - set/refreshes a playlist for a channel
       309'\t'(int)ch'\t'(char)playlist'\n'
       (char)playlist is the significative part of the standard playlist path
       i.e. .muse/default */
    if(strncmp(buf,"309",3)==0) {
      int ch;
      p = pp = &buf[4];
      while(*p!='\t') p++; *p = '\0';
      ch = atoi(pp);
      p++; pp = p;
      while(*p!='\n') p++; *p = '\0';
      
      mixer->set_playlist(pp, ch);
      return(0);
    }

    /* 310: connect to icecast server
       310'\t'ip address'\t'port'\t'mount'\t'pass'\t'bitrate'\n' */
    if(strncmp(buf,"310",3)==0) {
      char ip[20];
      int port;
      char pass[64];
      char mountpoint[64];
      int bps;

      p = pp = &buf[4];
      while(*p!='\t') p++; *p = '\0';
      strcpy(ip,pp);
      p++; pp = p;
      while(*p!='\t') p++; *p = '\0';
      port = atoi(pp);
      p++; pp = p;
      while(*p!='\t') p++; *p = '\0';
      strcpy(mountpoint,pp);
      p++; pp = p;
      while(*p!='\t') p++; *p = '\0';
      strcpy(pass,pp);
      p++; pp = p;
      while(*p!='\n') p++; *p = '\0';
      bps = atoi(pp);

      if(mixer->set_icecast(ip,port,pass,mountpoint,bps,NULL,NULL,NULL,0))
	return(0);
      else
	return(2);
    }
    
    /* 311: disconnect from icecast server
       311'\n' */
    if(strncmp(buf,"311",3)==0) {
      mixer->stop_icecast();
      return(0);
    }
    
    
    /* 333: dump status - dump channel status on MuSE stderr
       333'\n'
       if(strncmp(buf,"333",3)==0) {
       synchronous.select.call(0, mixer, Stream_mixer::id_stream_mixer);
       synchronous.wait();
       mixer->dump_status();
       return(0);
       }
    */

    /* 666: close connection but leave muse running accepting other connections
       666'\n' */
    if(strncmp(buf,"666",3)==0) {
      disconnect = true;
      return(0);
    }

    /* 999: close connection and stop muse
       999'\n' */
    if(strncmp(buf,"999",3)==0) {
      disconnect = true;
      mixer->quit = true;
      return(0);
    }

    return(1); /* syntax incorrect */
}

void Net_listener::send_playlist() {
  /*
    char temp[MAX_PATH_SIZE+2];
    int c = 0;
    int cc;
    
      for(cc=0;cc<MAX_CHANNELS;cc++) {
      sprintf(temp,"201\t%u\n",cc);
      getout(temp);
      while(mixer->mp3ch[cc].playlist[c][0]!='\0') {
      sprintf(temp,"%s\n",&mixer->mp3ch[cc].playlist[c][0]);
      getout(temp);
      c++;
      }
      }
      
      getout("202\n");
  */
}

void Net_listener::chiudi_socket() {
  close(id_accept);
  id_accept = -2;
}

int Net_listener::getout(const char *buf) {
  return(send(id_accept, buf, strlen(buf), 0x0));
}

int Net_listener::getin() {
  int c;

  c = recv(id_accept, buffer, BUFFERSIZE, 0x0);

  if (c<0) return(c);

  /* make it terminate with a \0 */
  if(c<BUFFERSIZE)
    buffer[c] = '\0';
  else
    /* we have a buffer overflow here */
    cerr << "!! Net_listener's buffer overflowed by a command request\n";

  return(c);
}

Net_listener::~Net_listener() {
  if(id_accept > -1)
    chiudi_socket();
  if(buffer==NULL)
    free(buffer);
}
