/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2000-2003 Denis Rojo aka jaromil <jaromil@dyne.org>
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
 * here command line is parsed
 * synchronous threads are launched depending on mode selection
 * signals are trapped for nice&clean exiting
 *
 * "$Id$"
 *
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

#include <jutils.h>
#include <generic.h>
#include <jmixer.h>
#include <gui.h>
#include <out_lame.h>
#include <out_vorbis.h>

#include <config.h>

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

#define CLI 1
#define GTK1 2
#define GTK2 3
#define NCURSES 4

/* command line stuff */

char *version =
"%s version %s [ http://muse.dyne.org ]";

char *help =
"Usage: muse [generic options] [-e [encoder options] [stream options] ] [files]\n"
":: generic options:\n"
" -h --help         this help\n"
" -v --version      version information\n"
" -D --debug [1-3]  debug verbosity level       - default 1\n"
" -i --live         mix soundcard live input\n"
" -o --dspout       disable souncard output\n"
" -C --cli          command line input (no GUI)\n"
" -g --gui          specify GUI to use (-g list)\n"
" -P --playmode     playmode: play, cont, loop  - default cont\n"
":: encoder options:\n"
" -e --encoder      codec to use [ogg|mp3]      - default ogg\n"
" -b --bitrate      codec bitrate in Kbit/s     - default 24\n"
" -r --frequency    encoding frequency          - default auto\n"
" -q --quality      encoding quality (0.1-9.0)  - default 1.0\n"
" -c --channels     number of audio channels    - default 1\n"
" -f --filedump     dump stream to file\n"
":: stream options:\n"
" -s --server       stream to server[:port]     - default port 8000\n"
" -m --mount        mounpoint on server         - default live\n"
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
  { "dspout", no_argument, NULL, 'o' },
  { "cli", no_argument, NULL, 'C' },
  { "playmode", required_argument, NULL, 'P' },
  { "encoder", no_argument, NULL, 'e' },
  { "bitrate", required_argument, NULL, 'b' },
  { "frequency", required_argument, NULL, 'r' },
  { "quality", required_argument, NULL, 'q' },
  { "channels", required_argument, NULL, 'c' },
  { "filedump", required_argument, NULL, 'f' },
  { "gui", required_argument, NULL, 'g' },
  { "server", required_argument, NULL, 's' },
  { "port", required_argument, NULL, 'p' },
  { "mount", required_argument, NULL, 'm' },
  { "pass",required_argument, NULL, 'p' },
  { "name",required_argument, NULL, 'n' },
  { "url",required_argument, NULL, 'u' },
  { "desc", required_argument, NULL, 'd' },
  { 0, 0, 0, 0 }
};

char *short_options = "-hvD:ioCP:e:b:r:q:c:f:g:s:m:p:n:u:d:";

/* misc settings */
#define MAX_CLI_CHARS 9182
int debug = 1;
bool daemon_mode = false;
char *queue_file = NULL;

int lfreq = 0;
float quality = 1.0f;
int channels = 1;

int thegui = -1; /* no gui */
int playmode = PLAYMODE_CONT;
bool live = false;
bool has_playlist = false;
bool dspout = true;
bool snddev = false;
char files[MAX_CLI_CHARS];
int cli_chars = 0;

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
  memset(files,0,MAX_CLI_CHARS);
  char *fp = files;
  
  MuseSetDebug(1);
  
  do {
    res = getopt_long(argc, argv, short_options, long_options, NULL);
    
    switch(res) {

    case 'h':
      fprintf(stderr,"%s",help);
      exit(0);

    case 'v':
      act("MuSE is copyright (c) 2000-2003 by jaromil");
      act("MuSE's GTK+ GUI is copyright (c) 2001, 2002 by nightolo");
      act("MuSE's NCURSES GUI is copyright (c) 2002 by rubik");
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
      dspout = false;
      break;

    case 'i':
      live = true;
      break; 

    case 'e':
      encid = 0;

#ifdef HAVE_VORBIS
      if (strncasecmp("ogg",optarg,3) == 0) {
	encid = mix->create_enc(OGG);
      }
#endif
      
#ifdef HAVE_LAME
      if (strncasecmp("mp3",optarg,3) == 0) {
	encid = mix->create_enc(MP3);
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
      act("bitrate set to %iKbit/s",outch->bps());
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
        act("falling back to auto");
        lfreq = 0;
      }
      outch->freq(lfreq);
      act("frequency set to %iKhz",outch->freq());
      break; 

    case 'q':
      if(!outch) {
	error("invalid command line argument: quality");
	error("you must specify a codec first with the -e option");
	break;
      }
      sscanf(optarg,"%f",&quality);
      /*
      if(quality<0.1f) quality = 0.1f;
      if(quality>9.0f) quality = 9.0f;
      */
      outch->quality(quality);
      act("quality set to %.1f",outch->quality());
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
	act("falling back to 1 channel default");
	channels = 1;
      }
      outch->channels(channels);
      break;

    case 'f':
      if(!outch) {
	error("invalid command line argument: file dump");
	error("you must specify a codec first with the -e option");
	break;
      }
      outch->dump_start( optarg );
      act("file saving set to %s",optarg);
      break;

    case 'C':
      thegui = 0;
      break;
      
    case 'P':
      if(strcasecmp(optarg,"play")==0) playmode=PLAYMODE_PLAY;
      if(strcasecmp(optarg,"cont")==0) playmode=PLAYMODE_CONT;
      if(strcasecmp(optarg,"loop")==0) playmode=PLAYMODE_LOOP;
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
      break;

    case 'u':
      if(!outch) {
        error("invalid command line argument: stream url");
        error("you must specify a codec first with the -e option");
        break;
      }
      if(!iceid) {
        error("invalid command line argument: stream url");
        error("you must specify a server first with the -s option");
        break;
      }
      ice = outch->get_ice(iceid);
      ice->url(optarg);
      break;

    case 'd':
      if(!outch) {
        error("invalid command line argument: stream description");
        error("you must specify a codec first with the -e option");
        break;
      }
      if(!iceid) {
        error("invalid command line argument: stream description");
        error("you must specify a server first with the -s option");
        break;
      }
      ice = outch->get_ice(iceid);
      ice->desc(optarg);
      break;

    case 1:
      {
	int optlen = strlen(optarg);
	has_playlist = true;
	if( (cli_chars+optlen) < MAX_CLI_CHARS ) {
	  sprintf(fp,"%s#",optarg);
	  cli_chars+=optlen+1;
	  fp+=optlen+1;
	} else {
	  warning("too much files on commandline to insert in a playlist");
	  warning("list truncated, consider including them into a .pls file");
	}
      }
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
  act("by Denis Rojo aka jaromil http://rastasoft.org");
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

  if( !take_args(argc, argv) ) goto QUIT;

  check_config();

  if(dspout||live) snddev = mix->open_soundcard();
  if(!snddev) {
    warning("no soundcard found");
    act("line-in and speaker out deactivated");
  }

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
    act("by nightolo <night@dyne.org>");
    gui = new GTK_GUI(argc,argv,mix);
#else
    error("the Gtk-1.2 interface is not compiled in");
#endif
    break;
  case GTK2:
#ifdef GUI_NIGHTOLO2
    notice("spawning the GTK-2 GUI");
    act("by nightolo <night@dyne.org>");	  
    gui = new GTK2_GUI(argc,argv,mix);
#else
    error("the Gtk2 interface is not compiled in");
#endif
    break;
  case NCURSES:
#ifdef GUI_RUBIK
    notice("spawning the NCURSES console user interface");
    act("by Luca Profico aka rubik <rubik@olografix.org>");
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

  if((!has_playlist && !live) && (thegui==CLI)) {
    warning("nothing to play, you must specify at least songs or live input");
    act("see --help switch for more information");
    goto QUIT;
  }


  if(thegui<0) error("no interface selected, this should never happen");
 
  if(thegui!=CLI) {
    gui->start();
    mix->register_gui(gui);
    set_guimsg(gui);
    notice("%s version %s",PACKAGE,VERSION);
  } else {
    if(!mix->create_channel(0))
      error("CLI: strange! i got problems creating a channel");
  }
  
  has_playlist = false;
  if(cli_chars>0) {
    char *p, *pp = files;
    while(cli_chars>0) {
      p = pp; while(*p!='#' && cli_chars>0) { p++; cli_chars--; }
      if(cli_chars<=0) break; *p='\0';
      if(!mix->add_to_playlist(0,pp))
	warning("can't add %s into playlist",pp);
      else
	has_playlist = true;
      pp = p+1;
    }
  }

  if((!has_playlist && !live) && (thegui==CLI)) {
    warning("nothing to play, you must specify at least songs or live input");
    act("see --help switch for more information");
    goto QUIT;
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
      
    } else {
      func("configuring %s Q%i freq%i chan%i",
	   outch->name,
	   (int)fabs(outch->quality()),
	   outch->freq(),
	   outch->channels());
      
      if(mix->apply_enc( outch->id )) outch->start();
    }
    if(!outch) break;
    outch = (OutChannel*) outch->next;
  }

  if(thegui==CLI) { /* CLI interface logics ======================= */
  
    /* lame guesses for best frequency
       hardcoded mono channel (mode 3)
       lame guesses for best low/highpass filtering
    */

    if(snddev) {
      mix->set_lineout(false); /* linein has priority */
      if( mix->set_live(live) ) notice("CLI: recording from mic/linein");
      if( mix->set_lineout(dspout) ) notice("CLI: playing on soundcard");
    }


    if(!mix->set_channel(0,1)) {
      error("CLI: fatal error setting channel!");
      goto QUIT;
    }
    mix->set_playmode(0,playmode); /* CONTINUOUS playing mode, cycle thru playlist */
    if(!mix->play_channel(0)) {
      error("CLI: some error playing on channel");
      goto QUIT;
    }
	 
  } /* === END CLI === */




  /* MAIN LOOP */
  while(!mix->quit)
    mix->cafudda();
 
  /* simple isn't it? */

  QUIT:
  if(mix) mix->quit = true;
  notice("quitting MuSE");
  set_guimsg(NULL);
  if(thegui!=CLI) gui->quit = true;
  
  /* piglia o'tiemp e sputa in terra
     senza fa' troppo casino a segnala' ai canali */

  if(mix) {
    act("stopping mixer...");
    jsleep(2,0);
    delete mix;
  }

  if(thegui!=CLI) {
    act("quitting graphic interface");
    delete gui;
  }
  
  act("cya!");
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
