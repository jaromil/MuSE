
/* $Id$ */

/**
   @mainpage MuSE :: Multiple Streaming Engine



   @section Introduction
   
   MuSE is an application for mixing, encoding, and network streaming of
   sound. It relies code that has been written with modularity and reusability
   in mind. Actually the engine can play MP3 or OGG files, mix them together
   up to an amount of 6 input streams and then reencode them together and stream
   them to the network again.
   
   It can also produce multiple encoded streams at the same time, and save the
   encoded audio to a local file.
   
   Streams produced by MuSE can run on various servers: icecast 1 and 2,
   litestream, darwin (with icecast emulation), shoutcast and theoretically
   any other protocol supported by libshout.
   
   MuSE can read its own MP3 streams (and mix them, and restream them, and...),
   but also a large number of audio players can do: xmms, freeamp, winamp,
   itunes, winzozz media player and probably more.
   
   In its application form, MuSE offers two allready implemented interfaces
   to be operated in realtime and a slick commandline interface.
   This documentation is useful to who wants to reuse or tweak MuSE's code,
   if you are not a programmer (and you don't want to become one) you are
   not really interested in all the details documented here.
   Nevertless, reading here might give you a picture about some unexplored
   possibility of audio streaming ;)

   A good place to start from is the Stream_mixer class, which is the main
   interface to interact and send asynchronous commands to the engine while
   its running. In fact the API declared in the jmixer.h file is the one
   used by the interactive user interfaces.

   A useful implementation example can be found in muse.cpp
   
   MuSE engine is being developed and hereby documented in the hope to
   provide the Free Software community with user friendly tool for
   network audio streaming and a high level interface for programming
   automatic radio tools.




   @section Authors

   the MuSE Engine is Copyright (C) 2000-2004
   Denis Rojo aka jaromil - http://rastasoft.org

   the GTK-2 MuSE interface is Copyright (C) 2002-2004
   Antonio Radici aka nightolo - http://freaknet.org

   the Ncurses MuSE interface is Copyright (C) 2002-2004
   Luca Profico aka rubik - http://olografix.org

   MuSE and all its interface source code is free software; you can
   redistribute it and/or modify it under the terms of the GNU Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   MuSE source code is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  Please refer
   to the GNU Public License for more details.
   
   You should have received a copy of the GNU Public License along with
   this source code; if not, write to:
   Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.   

*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <assert.h>

#include <config.h>

#include <jutils.h>
#include <generic.h>
#include <jmixer.h>
#include <gui.h>
#include <out_lame.h>
#include <out_vorbis.h>

#ifdef GUI_NIGHTOLO
#include <gtkgui/gtk_gui.h>
#endif

#ifdef GUI_NIGHTOLO2
#include <gtkgui2/gtk2_gui.h>
#endif

#ifdef GUI_RUBIK
#include <ncursesgui/ncurses_gui.h>
#endif

#ifdef UI_XMLRPC
#include <xmlrpc/xmlrpc_ui.h>
#endif

#ifdef HAVE_SCHEDULER
#include "radiosched.h"
#endif


/* command line stuff */

char *version =
"%s version %s http://muse.dyne.org";

char *help =
"Usage: muse [generic options] [-e [encoder options] [stream options] ] [files]\n"
":: generic options:\n"
" -h --help         this help\n"
" -v --version      version information\n"
" -D --debug [1-3]  debug verbosity level       - default 1\n"
" -o --dspout       disable souncard output     - default on\n"
" -C --cli          command line input (no GUI)\n"
" -g --gui          specify GUI to use (-g list)\n"
":: input channels options\n"
" -i --live         mic/line soundcard input    - default off\n"
" -I --liveamp      mic/line volume (1 - 32)    - default 1\n"
" -N --number       channel number              - default 1\n"
" -V --volume       channel volume              - default 1.0\n"
" -S --position     channel starting position   - default 0.0\n"
" -P --playmode     playmode: play, cont, loop  - default cont\n"
":: output encoders options:\n"
" -e --encoder      codec to use [ogg|mp3]      - default ogg\n"
" -b --bitrate      codec bitrate in Kbit/s     - default 24\n"
" -r --frequency    encoding frequency          - default auto\n"
" -q --quality      encoding quality (1-9)      - default 4\n"
" -c --channels     number of audio channels    - default 1\n"
" -f --filedump     dump stream to file\n"
":: broadcast stream options:\n"
" -s --server       stream to server[:port]     - default port 8000\n"
" -m --mount        mounpoint on server         - default live\n"
" -l --login        login type [ice1|ice2|icy]  - default ice1\n"
" -p --pass         encoder password on server\n"
" -n --name         name of the stream\n"
" -u --url          descriptive url of the stream\n"
" -d --desc         description of the stream\n"
"\n";

static const struct option long_options[] = {
  { "help", no_argument, NULL, 'h' },
  { "version", no_argument, NULL, 'v' },
  { "debug", required_argument, NULL, 'D' },
  { "live", no_argument, NULL, 'i' },
  { "liveamp", required_argument, NULL, 'I' },
  { "dspout", no_argument, NULL, 'o' },
  { "cli", no_argument, NULL, 'C' },
  { "number", required_argument, NULL, 'N' },
  { "volume", required_argument, NULL, 'V' },
  { "position", required_argument, NULL, 'S' },
  { "playmode", required_argument, NULL, 'P' },
  { "encoder", required_argument, NULL, 'e' },
  { "bitrate", required_argument, NULL, 'b' },
  { "frequency", required_argument, NULL, 'r' },
  { "quality", required_argument, NULL, 'q' },
  { "channels", required_argument, NULL, 'c' },
  { "filedump", required_argument, NULL, 'f' },
  { "gui", required_argument, NULL, 'g' },
  { "server", required_argument, NULL, 's' },
  { "port", required_argument, NULL, 'p' },
  { "mount", required_argument, NULL, 'm' },
  { "login", required_argument, NULL, 'l' },
  { "pass",required_argument, NULL, 'p' },
  { "name",required_argument, NULL, 'n' },
  { "url",required_argument, NULL, 'u' },
  { "desc", required_argument, NULL, 'd' },
  { 0, 0, 0, 0 }
};

char *short_options = "-hvD:ioCN:V:S:P:e:b:r:q:c:f:g:s:m:l:p:n:u:d:";

/* misc settings */
#define MAX_CLI_CHARS 9182
int debug = 1;
bool daemon_mode = false;
char *queue_file = NULL;

int lfreq = 0;
float quality = 1.0f;
int channels = 1;

int thegui = -1; /* no gui */
enum interface { CLI, GTK1, GTK2, NCURSES };

// channel options
int number = 0;
int playmode = PLAYMODE_CONT;

bool has_playlist = false;
bool dspout = true;
bool micrec = false;
int micvol = 16;
bool snddev = false;

Stream_mixer *mix = NULL;
GUI *gui = NULL;

OutChannel *outch = NULL;
int encid = 0;

Shouter *ice = NULL;
int iceid = 0;

/* declare the sighandlers */
void quitproc (int Sig);
void fsigpipe (int Sig);
bool got_sigpipe;
/* ---------------------- */

bool take_args(int argc, char **argv) {
  int res;
  
  MuseSetDebug(1);
  
  do {
    res = getopt_long(argc, argv, short_options, long_options, NULL);
    
    switch(res) {

    case 'h':
      fprintf(stderr,"%s",help);
      exit(0);

    case 'v':
      
      notice("GTK2 GUI by Antonino \"nightolo\" Radici <night@freaknet.org>");
      notice("Ncurses console by Luca \"rubik\" Profico <rubik@olografix.org>");
      act(" ");
      notice("BIG UP \\o/  dyne.org / FreakNet / RASTASOFT / Metro Olografix");
      act(" ");
      act("part of the redistributed code is copyright by the respective authors,");
      act("please refer to the AUTHORS file and to the sourcecode for complete");
      act("information.");
      act(" ");
      act("This source code is free software; you can redistribute it and/or");
      act("modify it under the terms of the GNU Public License as published");
      act("by the Free Software Foundation; either version 2 of the License,");
      act("or (at your option) any later version.");
      act(" ");
      act("This source code is distributed in the hope that it will be useful,");
      act("but WITHOUT ANY WARRANTY; without even the implied warranty of");
      act("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.");
      act("Please refer to the GNU Public License for more details.");
      act(" ");
      act("You should have received a copy of the GNU Public License along with");
      act("this source code; if not, write to:");
      act("Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.");
      exit(0);

    case 'D':
      MuseSetDebug( atoi(optarg) );
      break;

    case 'o':
      /* if( !mix->set_lineout(false) )
	error("soundcard not present");
      else {
	act("CLI: soundcard disabled");
	dspout = false;
	} */
      dspout = false;
      break;

    case 'i':
      /* if( !mix->set_live(true) )
	error("soundcard not present");
      else {
	act("CLI: recording from mic/linein");
	micrec = true;
	} */
      micrec = true;
      mix->set_mic_volume(micvol);
      notice("CLI: recording from mic/linein");
      break;
      
    case 'I':
      micvol = atoi(optarg);
      if(micvol < 1 || micvol > 32)
	error("invalid mic/line volume value, must be a approx between 1 and 32");
      else {
	mix->set_mic_volume(micvol);
	act("CLI: mic/linein volume gain %i",micvol);
      }
      break;

    case 'e':
      encid = 0;

#ifdef HAVE_VORBIS
      if (strncasecmp("ogg",optarg,3) == 0) {
	encid = mix->create_enc(OGG);
	notice("CLI: created Ogg encoder");
      }
#endif
      
#ifdef HAVE_LAME
      if (strncasecmp("mp3",optarg,3) == 0) {
	encid = mix->create_enc(MP3);
	notice("CLI: created Mp3 encoder");
      }
#endif

      if(encid>0) outch = mix->get_enc(encid);
      if(outch) break;

      error("you can specify an encoder with the -e option");
      act("supported encoders are:");
#ifdef HAVE_VORBIS
      act("  OGG - Ogg/Vorbis codec");
#endif
#ifdef HAVE_LAME
      act("  MP3 - Lame MP3 codec");
#endif
      exit(0);

    case 'b':
      if(!outch) {
	error("invalid command line argument: bps");
	error("you must specify a codec first with the -e option");
	break;
      }
      outch->bps( atoi(optarg) );
      act("CLI: bitrate set to %iKbit/s",outch->bps());
      break;

    case 'r':
      if(!outch) {
	error("invalid command line argument: frequency");
	error("you must specify a codec first with the -e option");
	break;
      }
      lfreq = atoi(optarg);
      if(lfreq != 0 &&
	 lfreq != 11000 &&
	 lfreq != 16000 &&
	 lfreq != 22050 &&
	 lfreq != 32000 &&
	 lfreq != 44100) {
	error("invalid frequency %i",lfreq);
        error("must be 0, 11000, 16000, 22050, 32000 or 44100 Hz!");
        act("CLI: falling back to auto");
        lfreq = 0;
      }
      outch->freq(lfreq);
      act("CLI: frequency set to %iKhz",outch->freq());
      break; 

    case 'q':
      if(!outch) {
	error("invalid command line argument: quality");
	error("you must specify a codec first with the -e option");
	break;
      }
      if( sscanf(optarg,"%f",&quality) ) {
	
	/*
	  if(quality<0.1f) quality = 0.1f;
	  if(quality>9.0f) quality = 9.0f;
	*/
	//	outch->quality(quality);
	act("CLI: quality set to %s",outch->quality(quality));
      } else error("invalid quality value");
      break;
	
    case 'c':
      if(!outch) {
        error("invalid command line argument: channels");
        error("you must specify a codec first with the -e option");
        break;
      }
      
      channels = atoi(optarg);
      if(channels>2 | channels<1) {
	error("audio channels can be only 1 (mono) or 2 (stereo)");
	act("falling back to default: 1 (mono)");
	channels = 1;
      }
      outch->channels(channels);
      act("CLI: encoding %i channel(s)",channels);
      break;

    case 'f':
      if(!outch) {
	error("invalid command line argument: file dump");
	error("you must specify a codec first with the -e option");
	break;
      }
      outch->dump_start( optarg );
      act("CLI: file saving to %s",optarg);
      break;

    case 'C':
      thegui = 0;
      break;

    case 'N':
      number = atoi(optarg);
      number = (number<0) ? 0 : (number>=MAX_CHANNELS) ? MAX_CHANNELS-1 : number;
      //      number--;
      if(!mix->chan[number]) {
	if(!mix->create_channel(number)) {
	  error("got problems creating channel %i",number);
	} else {
	  notice("CLI: created channel %i",number);
	  mix->set_playmode(number,playmode);
	}
      }
      break;

    case 'V':
      float vol;
      if(sscanf(optarg,"%f",&vol)==EOF)
	error("CLI: invalid volume for channel %i",number);
      else {
	vol = (vol>1.0) ? 1.0 : (vol < 0.0) ? 0.0 : vol;
	mix->set_volume(number,vol);
	act("CLI: volume set to %.2f",vol);
      }
      break;
      
    case 'S':
      float pos;
      if(sscanf(optarg,"%f",&pos)==EOF)
	error("CLI: invalid position for channel %i",number);
      else {
	pos = (pos>1.0) ? 1.0 : (pos < 0.0) ? 0.0 : pos;
	mix->set_position(number,pos);
	act("CLI: starting from position %.2f",pos);
      }
      break;

    case 'P':
      playmode = 0;
      if(strncasecmp(optarg,"play",4)==0) playmode=PLAYMODE_PLAY;
      if(strncasecmp(optarg,"cont",4)==0) playmode=PLAYMODE_CONT;
      if(strncasecmp(optarg,"loop",4)==0) playmode=PLAYMODE_LOOP;
      if(!playmode)
	error("invalid playmode %s",optarg);
      else
	//	mix->set_playmode(number,playmode);
	act("CLI: set playmode \"%s\" for following channels",optarg);
      break;

    case 'g':
#ifdef GUI_NIGHTOLO
      if(strcasecmp(optarg,"gtk")==0) thegui=GTK1;
#endif
#ifdef GUI_NIGHTOLO2
      if(strcasecmp(optarg,"gtk2")==0) thegui=GTK2;
#endif
#ifdef GUI_RUBIK
      if(strcasecmp(optarg,"ncurses")==0) thegui=NCURSES;
#endif
      if(strcasecmp(optarg,"cli")==0) thegui=CLI;

      if(thegui>=0) break;

      notice("listing available user interfaces:");
#ifdef GUI_NIGHTOLO
      act("[gtk] - Xwin graphical interactive clicky clicky");
#endif
#ifdef GUI_NIGHTOLO2
      act("[gtk2] - new graphical interactive, experimental with languages");
#endif
#ifdef GUI_RUBIK
      act("[ncurses] - 0ld sch00l l33t console display");
#endif
      act("[cli] - command line interface, not interactive");
      exit(0);

    case 's':
      if(!outch) {
        error("invalid command line argument: server");
        error("you must specify a codec first with the -e option");
        break;
      }

      iceid = outch->create_ice();
      if(iceid<0) {
	error("could'nt create icecast shouter");
	break;
      } else func("created icecast shouter ID %i",iceid);
      ice = outch->get_ice(iceid);

      char *p;
      p = strstr(optarg,":");
      if(p) { 
	ice->port( atoi(p+1) );
	*p = '\0';
      } else ice->port(8000);
      ice->host(optarg);
      notice("CLI: created streamer to %s %i",
	     ice->host(), ice->port());
      
      break;

    case 'l':
      if(!outch) {
        error("invalid command line argument: server login type");
        error("you must specify a codec first with the -e option");
        break;
      }
      if(!iceid) {
        error("invalid command line argument: server login type");
        error("you must specify a server first with the -s option");
        break;
      }
      int ltype;
      if(strncasecmp(optarg,"ice1",4)==0)
	ltype = SHOUT_PROTOCOL_XAUDIOCAST;
      else if(strncasecmp(optarg,"ice2",4)==0)
	ltype = SHOUT_PROTOCOL_HTTP;
      else if(strncasecmp(optarg,"icy",4)==0)
	ltype = SHOUT_PROTOCOL_ICY;
      else {
	error("unrecognized login type: %s",optarg);
	error("please use one of the following:");
	error("ice1 = icecast 1, darwin and litestream");
	error("ice2 = icecast 2 server");
	error("icy  = shoutcast server");
	break;
      }
      ice = outch->get_ice(iceid);
      ice->login(ltype);
      act("CLI: login type set to %s",optarg);
      break;

    case 'p':
      if(!outch) {
        error("invalid command line argument: server password");
        error("you must specify a codec first with the -e option");
        break;
      }
      if(!iceid) {
        error("invalid command line argument: server password");
        error("you must specify a server first with the -s option");
        break;
      }
      ice = outch->get_ice(iceid);
      ice->pass(optarg);
      act("CLI: stream password set");
      break;

    case 'm':
      if(!outch) {
        error("invalid command line argument: server mountpoint");
        error("you must specify a codec first with the -e option");
        break;
      }
      if(!iceid) {
        error("invalid command line argument: server mountpoint");
        error("you must specify a server first with the -s option");
        break;
      }
      ice = outch->get_ice(iceid);
      ice->mount(optarg);
      act("CLI: stream mountpoint %s",ice->mount());
      break;

    case 'n':
      if(!outch) {
        error("invalid command line argument: stream name");
        error("you must specify a codec first with the -e option");
        break;
      }
      if(!iceid) {
        error("invalid command line argument: stream name");
        error("you must specify a server first with the -s option");
        break;
      }
      ice = outch->get_ice(iceid);
      ice->name(optarg);
      act("CLI: stream descriptive name: %s",ice->name());
      break;

    case 'u':
      if(!outch) {
        error("CLI: invalid command line argument: stream url");
        error("you must specify a codec first with the -e option");
        break;
      }
      if(!iceid) {
        error("CLI: invalid command line argument: stream url");
        error("you must specify a server first with the -s option");
        break;
      }
      ice = outch->get_ice(iceid);
      ice->url(optarg);
      act("CLI: stream descriptive: url %s",ice->url());
      break;

    case 'd':
      if(!outch) {
        error("CLI: invalid command line argument: stream description");
        error("you must specify a codec first with the -e option");
        break;
      }
      if(!iceid) {
        error("CLI: invalid command line argument: stream description");
        error("you must specify a server first with the -s option");
        break;
      }
      ice = outch->get_ice(iceid);
      ice->desc(optarg);
      act("CLI: stream description: %s",ice->desc());
      break;

    case 1:
      act("CLI: queue %s on channel %i",optarg,number);
      if(!mix->chan[number])
	if(!mix->create_channel(number)) {
	  error("CLI: can't create channel %i",number);
	  break;
	} else {
	  notice("CLI: created channel %i",number);
	  mix->set_playmode(number,playmode);
	}
      if(!mix->add_to_playlist(number,optarg))
	error("CLI: can't add %s to channel %1",optarg,number);
      break;

    default:
      break;
    }
  } while(res > 0);  
  return true;
}

bool check_config() {
  /* checking config directory
     TODO: FIXME PLEASE */
  char temp[MAX_PATH_SIZE];
  char *home = getenv("HOME");
  sprintf(temp,"%s/.muse",home);
  mkdir(temp,0744);
  return(true);
}

int main(int argc, char **argv) {

  notice(version,PACKAGE,VERSION);
  act("by Denis \"jaromil\" Rojo http://rastasoft.org");
  act("--");

  /* register signal traps */
  if (signal (SIGINT, quitproc) == SIG_ERR) {
    error ("Couldn't install SIGINT handler"); exit (0);
  }

  if (signal (SIGQUIT, quitproc) == SIG_ERR) {
    error ("Couldn't install SIGQUIT handler"); exit (0);
  }

  if (signal (SIGTERM, quitproc) == SIG_ERR) {
    error ("Couldn't install SIGTERM handler"); exit (0);
  }
  got_sigpipe = false;
  if (signal (SIGPIPE, fsigpipe) == SIG_ERR) {
    error ("Couldn't install SIGPIPE handler"); exit (0);
  }
  
  mix = new Stream_mixer();
#ifdef HAVE_SCHEDULER
  rscheduler = new Scheduler_xml( sched_file_path() );
  mix->register_sched( rscheduler );
  rscheduler->register_mixer(mix);
  rscheduler->lock();
  rscheduler->start();
  rscheduler->wait();
  rscheduler->unlock();
  //rscheduler->play();
#endif
    
  if( !take_args(argc, argv) ) goto QUIT;

  check_config();

  // setup soundcard
  mix->set_live(micrec);
  mix->set_lineout(dspout);
  
  //  if(!snddev) {
  //    warning("no soundcard found");
  //    act("line-in and speaker out deactivated");
  //  }

  if(thegui==GTK1 || thegui==GTK2)
    if(!getenv("DISPLAY")) { /* no graphical environment */
      error("DISPLAY not found, falling back to console");
      thegui=-1;
    }
  
  if(thegui<0) { /* select default gui */
    if(getenv("DISPLAY")) { /* we are in a graphical environment */
#ifdef GUI_NIGHTOLO
      thegui=GTK1;
#elif GUI_NIGHTOLO2
      thegui=GTK2;
#endif
    }
    if(thegui<0) { /* if GUI is still not selected */
#ifdef GUI_RUBIK
      thegui = NCURSES;
#else
      thegui=CLI;
#endif
    }
  }

  switch(thegui) {
  case GTK1:
#ifdef GUI_NIGHTOLO
    notice("spawning the GTK-1.2 GUI");
    act("by nightolo <night@freaknet.org>");
    warning("GTK-1.2 should be upgraded to GTK 2 on your system");
    warning("this old version of the interface is much unstable");
    gui = new GTK_GUI(argc,argv,mix);
#else
    error("the Gtk-1.2 interface is not compiled in");
#endif
    break;
  case GTK2:
#ifdef GUI_NIGHTOLO2
    notice("spawning the GTK-2 GUI");
    act("by Antonino \"nightolo\" Radici <night@freaknet.org>");
    gui = new GTK2_GUI(argc,argv,mix);
#else
    error("the Gtk2 interface is not compiled in");
#endif
    break;
  case NCURSES:
#ifdef GUI_RUBIK
    notice("spawning the NCURSES console user interface");
    act("by Luca \"rubik\" Profico <rubik@olografix.org>");

    gui = new NCURSES_GUI(argc,argv,mix);
    MuseSetLog("muse.log");
#else
    error("the ncurses console interface is not compiled in");
#endif
    break;
  default:
    notice("using commandline interface (non interactive)");
    thegui=CLI;
  }

  if(thegui<0) error("no interface selected, this should never happen");
 
  if(thegui!=CLI) {
    gui->start();
    mix->register_gui(gui);
    set_guimsg(gui);
    notice("%s version %s",PACKAGE,VERSION);
  }

  /* apply configuration and startup all registered encoders */
  outch = (OutChannel*)mix->outchans.begin();
  while(outch) {
    
    if( (outch->icelist.len() == 0)
	&& !outch->fd) { /* check if its worth to encode */
      error("codec %s [%u] has no server neither filedump configured",
	    outch->name,outch->id);
      mix->delete_enc(outch->id);
      error("encoder removed");
      
    } else if(mix->apply_enc( outch->id )) outch->start();

    if(!outch) break;
    outch = (OutChannel*) outch->next;
  }

  if(thegui==CLI) { /* CLI interface logics ======================= */
  
    int c;
    for(c=0;c<MAX_CHANNELS;c++)
      if(mix->chan[c])
	if(!mix->set_channel(c,1))
	  error("CLI: error in set_channel(%i,1)",c);	  
	else
	  if(!mix->play_channel(c))
	    error("CLI: error in play_channel(%i)",c);
	  else
	    has_playlist = true;
  } /* === END CLI === */

  if((!has_playlist && !micrec) && (thegui==CLI)) {
    warning("nothing to play, you must specify at least songs or live input");
    act("see --help switch for more information");
    goto QUIT;
  }


  /* MAIN LOOP */
  while(!mix->quit)
    mix->cafudda();
 
  /* simple isn't it? */

  QUIT:
  notice("quitting MuSE");

  if(thegui!=CLI) gui->quit = true;

  set_guimsg(NULL);
  
#ifdef HAVE_SCHEDULER
  rscheduler->stop(); 
  rscheduler->quit = true; 
  jsleep(0,50);
  delete rscheduler; rscheduler = NULL;
#endif
  if(mix) {
    act("stopping mixer...");  
  /* piglia o'tiemp e sputa in terra
     senza fa' troppo casino a segnala' ai canali */
    jsleep(0,50);
    delete mix;
  }

  if(thegui!=CLI) {
    act("quitting graphic interface");
    delete gui;
  }
  
  act("cya on http://muse.dyne.org");
  MuseCloseLog();
  exit(0);
}

/* signal handling */
void quitproc (int Sig) {
  func("received signal %u on process %u",Sig,getpid());
  if(thegui!=CLI) gui->quit = true;
  mix->quit = true;  
}

void fsigpipe (int Sig) {
  warning("received signal SIGPIPE (%u) on process %u",Sig,getpid());
  got_sigpipe = true;
}
