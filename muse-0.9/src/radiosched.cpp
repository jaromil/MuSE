/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2000-2004 Denis Roio aka jaromil <jaromil@dyne.org>
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

 "$Id$"
 
 */

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <stdarg.h>

#include <config.h>

#include <jutils.h>
#include <audioproc.h>
#include <jmixer.h>
#include <playlist.h>
#include <inchannels.h>

#ifdef HAVE_VORBIS
#include <out_vorbis.h>
#endif

#ifdef HAVE_LAME
#include <out_lame.h>
#endif

#include <glib.h> 

#include <radiosched.h>



//typedef void* (kickoff)(void*);

Basic_scheduler *rscheduler = NULL;

static char m_sched_file[FILENAME_MAX+1] = {0};


/*Tags, attributes and values*/
#define SF_S      "sched"
#define SFI_S     "sched:item"
# define SFIA_S    "start"
# define SFIA_E    "end"
# define SFIA_WKD  "weekday" //0-6
#  define SUN     "Sun"      //0 
#  define MON     "Mon"
#  define TUE     "Tue"
#  define WEN     "Wen"
#  define THU     "Thu"
#  define FRI     "Fri"
#  define SAT     "Sat"
# define SFIA_DAY  "day"   //1-31
# define SFIA_MTH  "month" //1-12
#  define JAN     "Jan"
#  define FEB     "Feb"
#  define MAR     "Mar"
#  define APR     "Apr"
#  define MAI     "Mai"
#  define JUN     "Jun"
#  define JUL     "Jul"
#  define AUG     "Aug"
#  define SEP     "Sep"
#  define OCT     "Oct"
#  define NOV     "Nov"
#  define DEC     "Dec"
# define SFIA_SRC  "source"
# define SFIA_CMNT "comment"



static void 
_dbg(char *error, ...)
{
  va_list args;

  va_start(args, error);
  vfprintf(stderr, error, args);
  va_end(args);
}


const char *
sched_file_path(void)
{
  if (*m_sched_file) return m_sched_file;
  //else fill it
  char *home = getenv("HOME");
  if (home) {
      snprintf(m_sched_file, FILENAME_MAX, "%s/.muse/" SCHEDFILE, home);
  }
  
  return m_sched_file;
}


static const char *
_wkd(const char *d)
{
    if (0==strcmp(d, SUN) || 0==strcmp(d, "0")) return "0";
    if (0==strcmp(d, MON) || 0==strcmp(d, "1")) return "1";
    if (0==strcmp(d, TUE) || 0==strcmp(d, "2")) return "2";
    if (0==strcmp(d, WEN) || 0==strcmp(d, "3")) return "3";
    if (0==strcmp(d, THU) || 0==strcmp(d, "4")) return "4";
    if (0==strcmp(d, FRI) || 0==strcmp(d, "5")) return "5";
    if (0==strcmp(d, SAT) || 0==strcmp(d, "6")) return "6";
	return NULL;
}

static const char *
_mth(const char *d)
{
    if (0==strcmp(d, JAN) || 0==strcmp(d, "1"))  return "1";
    if (0==strcmp(d, FEB) || 0==strcmp(d, "2"))  return "2";
    if (0==strcmp(d, MAR) || 0==strcmp(d, "3"))  return "3";
    if (0==strcmp(d, APR) || 0==strcmp(d, "4"))  return "4";
    if (0==strcmp(d, MAI) || 0==strcmp(d, "5"))  return "5";
    if (0==strcmp(d, JUN) || 0==strcmp(d, "6"))  return "6";
    if (0==strcmp(d, JUL) || 0==strcmp(d, "7"))  return "7";
    if (0==strcmp(d, AUG) || 0==strcmp(d, "8"))  return "8";
    if (0==strcmp(d, SEP) || 0==strcmp(d, "9"))  return "9";
    if (0==strcmp(d, OCT) || 0==strcmp(d, "10")) return "10";
    if (0==strcmp(d, NOV) || 0==strcmp(d, "11")) return "11";
    if (0==strcmp(d, DEC) || 0==strcmp(d, "12")) return "12";
	return NULL;
}

/* Extract next token from str and put it in buf.  Transform it in appropriate
   digit form.  Return new pointer in str.  Separators: -,:  */
static const char *
_token(const char *str, char *buf, int len)
{
    const char *digits = NULL;
	char *b = buf;
	
    memset(buf, '\0', len);
	len--; //leave space for ending 0
	while (*str!='\0' && *str!='-' && *str!=',' && *str!=':' && len-->0) {
	    *buf++ = *str++;
	}
	buf = b;
	//_dbg("  _token: %s\n", buf);
	
	if (digits=_wkd(buf)) {
	    strcpy(buf, digits);
	}
	if (digits=_mth(buf)) {
	    strcpy(buf, digits);
	}
	//_dbg("  _token: %s str '%s'\n", buf, str);
	return str;
}

/*atoi...*/
static inline int
_number(const char *str)
{
  register int n;
  register char c;

  n = 0;
  while ((c = *str++) && (c >= '0') && (c <= '9')) n = (n * 10) + c - '0';
  return n;
}


Basic_scheduler::Basic_scheduler( const void *sched ) {
  func("Basic_scheduler::Basic_scheduler()\n");
  
  prev_time = (time_t) 0;    /* timekeeper: previous wakeup  */
  
  playlist = new Playlist();
  
  on = false;
  running = false;
  quit = true;

  playing   = NULL;
  
  channel = new Channel();
  channel->lock();
  channel->start();
  channel->wait();
  channel->unlock();
  
  _schedule = sched;
  mixer = NULL;

  _thread_init();
}

Basic_scheduler::~Basic_scheduler() {
  func("Basic_scheduler::~Basic_scheduler()\n");

  if (channel) {
      stop_channel();
      channel->quit = true;
      //FIXME
      jsleep(0,50);
      delete channel; channel = NULL;
  }

  /* paranoia */
  //stop();
  //  clean();
  quit = true;
  
  while(running) jsleep(0,20);
  
  delete playlist; 

  _thread_destroy();
}


void Basic_scheduler::_thread_init() {

  func("Basic_scheduler::thread_init()\n");
  if(pthread_mutex_init (&_mutex,NULL) == -1)
    error("error initializing POSIX thread mutex");
  if(pthread_cond_init (&_cond, NULL) == -1)
    error("error initializing POSIX thread condition"); 
  if(pthread_attr_init (&_attr) == -1)
    error("error initializing POSIX thread attribute");
  
  /* set the thread as detached
     see: man pthread_attr_init(3) */
  pthread_attr_setdetachstate(&_attr,PTHREAD_CREATE_DETACHED);
}

void Basic_scheduler::_thread_destroy() {
  /* we signal and then we check the thread
     exited by locking the conditional */
  while(running) {
    signal();
    lock(); unlock();
  }

  if(pthread_mutex_destroy(&_mutex) == -1)
    error("error destroying POSIX thread mutex");
  if(pthread_cond_destroy(&_cond) == -1)
    error("error destroying POSIX thread condition");
  if(pthread_attr_destroy(&_attr) == -1)
    error("error destroying POSIX thread attribute");
}


void Basic_scheduler::run() {

  register struct tm *tm;
  int status, i, pid;
  time_t clk;
  unsigned left; //time left to sleep
  
  func("Basic_scheduler::run()\n");
  lock();
  running = true;
  unlock();
  signal(); // signal to the parent thread we are born!

  quit = false;

  while(!quit) {
    if(on) {
      on_wakeup();
      (void) time(&clk);
	  left = (unsigned) (60 - clk % 60); 
	  while (left--) {
          sleep(1);
		  if (quit) break;
	  }
    } else { // if(on)
      // just hang on
      sleep(1);
    }
  } // while(!quit)
  running = false;
}


bool Basic_scheduler::play() {
  if(on) return(true);

  if(!running) {
    error("%i:%s %s channel thread not launched",
	  __LINE__,__FILE__,__FUNCTION__);
    return(false);
  }
  channel->play();

  on = true;
  return(on);
}

bool Basic_scheduler::stop() {
  //  lock();
  stop_channel();
  jsleep(0,50);
  on = false;
  return(!on);
}


void Basic_scheduler::dump() {
  if (!playlist) return;
  
  for (int i=1; i<=playlist->len(); i++ ) {
     Url *p = (Url*)playlist->pick(i);
     notice( "schedule_record: \n" 
          "  Min: %s, Hr: %s, Day: %s, Mon: %s, WDay: %s \n"
          "  MnPlay %d,\n"
          "  Url %s, commnent '%s'\n",
          p->mn, p->hr, p->day, p->mon, p->wkd,
          p->mnplay, 
          p->path, p->comment
        );
  }
}


void Basic_scheduler::on_wakeup( void )
{
    register struct tm *tm;
    time_t cur_time;
    int st, pid;

    func("Basic_scheduler::on_wakeup\n");
    
    if (playing) {
      playing->mnleft--;
      if (playing->mnleft <= 0) {
          stop_channel(); 
      }
    }

    load_schedule();

    time(&cur_time);
    tm = localtime(&cur_time);

    for (int i=1; i<=playlist->len(); i++) {
	    Url *this_entry = (Url*)playlist->pick(i); 
        if ( match(this_entry->mn, tm->tm_min) 
          && match(this_entry->hr, tm->tm_hour) 
          && match(this_entry->day, tm->tm_mday) 
          && match(this_entry->mon, tm->tm_mon + 1) 
          && match(this_entry->wkd, tm->tm_wday)) {
              notice("==> Basic_scheduler: %02d/%02d-%02d:%02d  %s\n",
                   tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,
                   this_entry->path);
              start_channel(this_entry);
        }
    }
}


void Basic_scheduler::start_inner_channel( Url *rec )
{
  stop_channel();
  func("Basic_scheduler::start_channel '%s'\r\n", rec->path);
  playing = rec;
  playing->mnleft = playing->mnplay; 
  channel->playlist->addurl( playing->path );
  channel->playlist->sel(1); //FIXME: channel complaints no song selected 
  channel->play();
}


void Basic_scheduler::stop_inner_channel(void)
{
  func("Basic_scheduler::stop_channel\r\n");
  channel->lock();
  channel->stop();
  channel->playlist->cleanup();
  channel->unlock();
  //clean will delete the decoder, so wait for a while for channel's thread 
  //to finish.  Got some SIGSEGV
  jsleep(0,50);
  channel->clean(); 
  channel->report();
}

#define MUSE_URL "muse://channel"
void Basic_scheduler::start_mixer_channel( Url *rec )
{
    char *p;
	int  chan;
	
    if (!rec || !rec->path || !mixer) return;
	
	p = strstr(rec->path,MUSE_URL); 
	if (!p) return;
	p += strlen(MUSE_URL);
	chan = atoi(p) -1 ;
	notice("Basic_scheduler::start_mixer_channel %d", chan);
	
	if (chan>0 &&chan<MAX_CHANNELS) {
        mixer->set_channel(chan, 1/*pos*/);
        mixer->play_channel(chan);
    }
}

void Basic_scheduler::stop_mixer_channel( Url *rec )
{
    char *p;
	int  chan;
	
    if (!rec || !rec->path || !mixer) return;
	
	p = strstr(rec->path,MUSE_URL); 
	if (!p) return;
	p += strlen(MUSE_URL);
	chan = atoi(p) -1 ;
	notice("Basic_scheduler::stop_mixer_channel %d", chan);
	
	if (chan>0 &&chan<MAX_CHANNELS) {
        mixer->stop_channel(chan);
    }
}

void Basic_scheduler::start_channel( Url *rec )
{
    if (strstr(rec->path,MUSE_URL)) {
        start_mixer_channel(rec);
    } else {
        start_inner_channel(rec);
    }
}

void Basic_scheduler::stop_channel(void)
{
    if (playing && strstr(playing->path,MUSE_URL)) {
        stop_mixer_channel(playing);
    } else {
        stop_inner_channel();
    }
}

bool Basic_scheduler::match( const char *left, int right )
{
  register int n;
  register char c;
  char tok[16];

  //notice("match '%s' %d \n", left, right);
  if (!left) return false;

  if (!strcmp(left, "*") || !strcmp(left, "any")) return true;

  left = _token(left, tok, 16); 
  n = _number(tok);

  switch (*left) {
      case '\0':
          return( right == n );
          /*NOTREACHED*/
          break;
      case ',':
          if (right == n) return true;
          do {
              n = 0;
			  left = _token(++left, tok, 16); 
			  n = _number(tok);
              if (right == n) return(1);
          } while (*left == ',');
          return false;
          /*NOTREACHED*/
          break;
      case '-':
          if (right < n) return(0);
          n = 0;
          left = _token(++left, tok, 16);  
		  n = _number(tok);
          return( right <= n );
          /*NOTREACHED*/
          break;
      default:
          break;
  }
  return false;
}

/*------------------------------------------------------------------*/

const char empty_text_sched_file[] = 
"#\n"
"# $HOME/.muse/schedule\n"
"# Mn  Hr Day Mth WDay MnPlay Url comment\n"
"#\n"
"# Sample: start at every minute\n"
"#* * * * * 2 http://wwww.none.com:888/x.ogg comment starts here \n"
"\n"
"\n"
;


void Scheduler_text::on_load_schedule() {
  int len, pos;
  FILE *cfp;
  struct stat buf;
  const char *cronf = (const char *)_schedule;

  func("Scheduler_text::on_load_schedule \n");
  if (!_schedule) return;

  if (stat(cronf, &buf)) {
      if (prev_time == (time_t) 0) warning("Can't stat crontab %s\n", cronf);
      prev_time = (time_t) 0;
      return;
  }

  if (buf.st_mtime <= prev_time) return;

  if ((cfp = fopen(cronf, "r")) == NULL) {
      if (prev_time == (time_t) 0) warning("Can't open crontab %s\n", cronf);
      prev_time = (time_t) 0;
      return;
  }
  prev_time = buf.st_mtime;
  len = pos = 0;
  playlist->cleanup();

  stop_channel();
  
  while (fgets(&crontab[pos], CRONSIZE - pos, cfp) != NULL) {
      if (  crontab[pos] == '#' || crontab[pos] == '\n' 
         || crontab[pos] == '\r') 
          continue;
      len = strlen(&crontab[pos]);
      if (crontab[pos + len - 1] == '\n') {
          len--;
          crontab[pos + len] = '\0';
      }

      addentry(&crontab[pos]);

      pos += ++len;
      if (pos >= CRONSIZE) break;
  }
  (void) fclose(cfp);

 // while (rec_ptr) {
 //     rec_ptr->mn = NULL;
 //     rec_ptr = rec_ptr->next;
 // }
  dump(); //FIXME
}


/* Assign the field values to the crontab entry. */
//# Mn  Hr Day Mth WDay MnPlay Url comment
void Scheduler_text::addentry( char *line )
{
    const char *mn, *hr, *day, *mon, *wkd, *cmnplay, *path, *comment;
	
    mn     = strtok(line, TSEPARATOR);
    hr     = strtok( (char *)NULL, TSEPARATOR);
    day    = strtok( (char *)NULL, TSEPARATOR);
    mon    = strtok( (char *)NULL, TSEPARATOR);
    wkd    = strtok( (char *)NULL, TSEPARATOR);
    cmnplay= strtok( (char *)NULL, TSEPARATOR);
    path   = strtok( (char *)NULL, TSEPARATOR);
    if (path) { 
	    comment= strchr(path, '\0') + 1;
	    playlist->addurl(path, mn, hr, day, mon, wkd, cmnplay, comment);
	}
}


/*------------------------------------------------------------------*/

const char empty_xml_sched_file_header[] = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" ?> \n"
"<sched xmlns:sched=\"/\">\n"
"<!--\n"
"  $HOME/.muse/schedule \n"
"  Attributes: \n"
"      start=\"hour:min\" in 24-hours format \n"
"      end=\"hour:min\" \n"
"      weekday=0-6 |  Sun(0), Mon, Tue, Wen, Thu, Fri, Sat \n"
"      day=1-31 \n"
"      month=1-12 | Jan,Feb,Mar,Apr,Mai,Jun,Jul,Aug,Sep,Oct,Nov,Dec \n"
"  Wildcards: \n"
"      * ex: start=\"*:*\" \n"
"      periods: Sun-Sat \n"
"      enumerations: Jan,Feb,Dec \n"
"  Source: file.ogg, file.mp3, http://source, muse://channelX with X=1..6\n"
"  Example: \n"
"  <sched:item start=\"10:00\"  end=\"13:00\" weekday=\"Mon-Fri\" day=\"*\" month=\"*\" source=\"http://aud-one.kpfa.org:8090/kpfa.mp3\" comment=\"KPFA Democracy Now\" /> \n"
"-->\n"
"\n"
;

const char empty_xml_sched_file_footer[] = 
"</sched>"
"\n"
;

typedef struct _xml_user_data {
    sched_rec_callb callb; //GUI func
	void *udata;           //passed from GUI
	sched_rec *sr;         //filled by xml parser
} xml_user_data;


int
create_xml_schedule_file(void)
{
	gchar *home, *dir; 
	int ret;
	int fd;

	if(!(home = getenv("HOME"))) {
		error("no $HOME found");
		return FALSE;
	}
	
	dir = g_strconcat(home, "/.muse", NULL);
	if(!g_file_test(dir, (GFileTest) (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
		if((fd = creat(dir, S_IRWXU)) < 0) {
			error("error during $HOME./muse directory creation");
			return FALSE;
		}
		close(fd);
	}
	g_free(dir);
	
	if(g_file_test(sched_file_path(), (GFileTest) (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR))) { 
		return TRUE;
	}

    notice("create_xml_schedule_file '%s'", m_sched_file);	
	ret = write_xml_schedule_file( "" ); 
	return ret;
}

char *
format_xml_sched_rec(const sched_rec *rec)
{
    gchar *ret = g_strconcat(
	        "<" SFI_S, 
			" " SFIA_SRC "=\"", rec->src, "\"",
			" " SFIA_CMNT "=\"", rec->comment, "\"",
			" " SFIA_WKD "=\"", rec->wkd, "\"",
			" " SFIA_S "=\"", rec->stime, "\"",
			" " SFIA_E "=\"", rec->etime, "\"",
			" " SFIA_DAY "=\"*\"",
			" " SFIA_MTH "=\"*\"",
			"/>\n", NULL
	        );

}

int 
write_xml_schedule_file(const char *content)
{
	FILE *f;
	
	if(!(f = fopen(sched_file_path(), "w"))) {
		error("problem opening " SCHEDFILE " for writing");
		return FALSE;
	}
	
	fputs(empty_xml_sched_file_header, f);
	fputs(content, f);
	fputs(empty_xml_sched_file_footer, f);

    fclose(f);
	
    return TRUE;
}

static void
start_e(GMarkupParseContext *c, const gchar *ename,
        const gchar **anames, const gchar **avalues,
		gpointer udata, GError **err)
{
    xml_user_data *ud = (xml_user_data*)udata;
	
    //notice("<%s>", ename);
	//for (int i=0; anames[i]; i++) {
	//    notice("  '%s' == '%s'", anames[i], avalues[i]);
	//}
	
	if (0==strcmp(ename, SFI_S)) {
	    for (int i=0; anames[i]; i++) {
	        if (0==strcmp(anames[i], SFIA_CMNT)) ud->sr->comment = avalues[i];
	        if (0==strcmp(anames[i], SFIA_SRC))  ud->sr->src = avalues[i];
	        if (0==strcmp(anames[i], SFIA_MTH))  ud->sr->month = avalues[i];
	        if (0==strcmp(anames[i], SFIA_DAY))  ud->sr->day = avalues[i];
	        if (0==strcmp(anames[i], SFIA_WKD))  ud->sr->wkd = avalues[i];
	        if (0==strcmp(anames[i], SFIA_S))    ud->sr->stime = avalues[i];
	        if (0==strcmp(anames[i], SFIA_E))    ud->sr->etime = avalues[i];
	    }
		//"fix" it
		if (!ud->sr->comment) ud->sr->comment = "";
		if (!ud->sr->src)     ud->sr->src = "";
		if (!ud->sr->month)   ud->sr->month = "";
		if (!ud->sr->day)     ud->sr->day = "";
		if (!ud->sr->wkd)     ud->sr->wkd = "";
		if (!ud->sr->stime)   ud->sr->stime = "";
		if (!ud->sr->etime)   ud->sr->etime = "";
		(*ud->callb)(ud->udata, ud->sr); 
		//cleanup for next call
		ud->sr->comment = NULL;
		ud->sr->src = NULL;
		ud->sr->month = NULL;
		ud->sr->day = NULL;
		ud->sr->wkd = NULL;
		ud->sr->stime = NULL;
		ud->sr->etime = NULL;
	}
}


static void
end_e(GMarkupParseContext *c, const gchar *ename,
		gpointer udata, GError **err)
{
    //notice("</%s>", ename);
}

static void
on_err(GMarkupParseContext *c, GError *err, gpointer udata)
{
    notice("<on_err>");
}

int 
parse_xml_sched_file( sched_rec_callb callb, void *udata, sched_rec *sr )
{
    gchar *buf=NULL; 
	guint len;
	xml_user_data calldata = {callb, udata, sr};
	
	g_file_get_contents(sched_file_path(), &buf, &len, NULL); 
	if (len>0) {
	    GMarkupParser xp = {start_e, end_e, NULL, NULL, on_err};
	    GError *err = NULL; 
	    GMarkupParseContext *parser = NULL;
	    parser = g_markup_parse_context_new(&xp, (GMarkupParseFlags)0, &calldata, NULL);
		if (!g_markup_parse_context_parse(parser, buf, len, &err)) {
		    error("parse_xml_sched_file g_markup_parse_context_parse error: %s\n", err->message);
		}
		g_markup_parse_context_end_parse(parser, &err);
		g_markup_parse_context_free(parser);
		if (err) {
		    warning("parse_xml_sched_file: '%s'", err->message);
			//should free err?
		}
	}
	
	g_free(buf); 
	return TRUE; 
}

void Scheduler_xml::on_load_schedule() {
  sched_rec sr = {0};
  struct stat buf;
  const char *cronf = (const char *)_schedule;

  func("Scheduler_xml::on_load_schedule \n");
  if (!_schedule) return;

  if (stat(cronf, &buf)) {
      if (prev_time == (time_t) 0) warning("Can't stat crontab %s\n", cronf);
      prev_time = (time_t) 0;
      return;
  }

  if (buf.st_mtime <= prev_time) return;
  prev_time = buf.st_mtime;

  playlist->cleanup();
  stop_channel();
  parse_xml_sched_file( addentry, this, &sr );
  
  dump(); //FIXME
}


/* Assign the field values to the crontab entry. */
//# Mn  Hr Day Mth WDay MnPlay Url comment
int Scheduler_xml::addentry( void *instance, sched_rec *sr )
{
    char mn[16]={0}, hr[16]={0}, cmnplay[16]={0}; 
	int emn, smn, ehr, shr, mnplay;
    char tok[16];
	const char *p;

    //start hr
    p = _token(sr->stime, tok, 16); 
	strcpy(hr, tok);
    shr = _number(tok);
    //start mn
    p = _token(++p, tok, 16); 
	strcpy(mn, tok);
    smn = _number(tok);
    //end hr
    p = _token(sr->etime, tok, 16); 
    ehr = _number(tok);
    //end mn
    p = _token(++p, tok, 16); 
    emn = _number(tok);
	
	//calculate (c)mnplay
	if (*mn=='*') {
	    mnplay = 1;
	} else if (*hr=='*') {
	    mnplay = 60;
	} else {
	    mnplay = 60*(ehr-shr) + (emn-smn);
	}
	sprintf(cmnplay, "%d", mnplay);

    if (sr->src) { 
	    ((Scheduler_xml*)instance)->playlist->addurl(sr->src, mn, hr, sr->day, 
		    sr->month, sr->wkd, cmnplay, sr->comment);
	}
}
