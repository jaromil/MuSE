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
 * "$Id$"
 *
 */

#include <shouter.h>
#include <profile.h>
#include <jutils.h>
#include <generic.h>
#include <config.h>

extern bool got_sigpipe;

Shouter::Shouter()
  : Profile("ice") {

  /*
  int vermaj,vermin,verpat;
  shout_version(&vermaj,&vermin,&verpat);
  func("Shouter::Shouter() using libshout version %i.%i.%i",vermaj,vermin,verpat);
  */

  ice = shout_new();
  profile_changed = true;

  struct settings ice_settings[] = {
    { "host", (void*)&_host, cfgSTR, "localhost" },
    { "ip_address", (void*)&_ip, cfgSTR, "127.0.0.1" },
    { "port", (void*)&_port, cfgINT, "8000" },
    { "pass", (void*)&_pass, cfgSTR, "hackme" },
    { "mount", (void*)&_mount, cfgSTR, "live" },
    //    { "bps", (void*)&_bps, cfgINT, "24" },
    { "login", (void*)&_login, cfgINT, "1" },
    { "name", (void*)&_name, cfgSTR, "Streaming with MuSE" },
    { "url", (void*)&_url, cfgSTR, "http://muse.dyne.org" },
    { "desc", (void*)&_desc, cfgSTR, "Free Software Multiple Streaming Engine" },
    { NULL, NULL, cfgNULL, NULL }
  };
  setup(ice_settings);

  running = false;
  retry = 0;

  if( !load_profile("default") )
    create_default_profile();

  errors = 0;
}

Shouter::~Shouter() {
  func("Shouter::~Shouter");
  stop();
  shout_free(ice);
}

bool Shouter::start() {
  int res;
  char srv[64];
  switch(login()) {
  case SHOUT_PROTOCOL_HTTP: sprintf(srv,"icecast2"); break;
  case SHOUT_PROTOCOL_ICY: sprintf(srv,"shoutcast"); break;
  default: sprintf(srv,"icecast"); break;
  }

  if(running) return(true);
  
  notice("Contacting %s server %s on port %u",srv,host(),port());
  
  res = shout_open(ice);
  func("Shouter::start() shout_open returns %i",res);
  //  shout_sync(ice);
  
  if(res==SHOUTERR_SUCCESS) {
    notice("started streaming on %s",streamurl);
    running = true;
  } else {
    error("shout_open: %s",shout_get_error(ice));
    running = false;
  }
  
  return(running);
}

bool Shouter::stop() {
  if(running) {
    notice("closed stream to %s",streamurl);
    shout_close(ice);
    running = false;
  }
  return true;
}

bool Shouter::apply_profile() {
  char temp[256];
  func("Shouter::apply_profile() on shouter id %i",id);

  bool was_running = running, res = true;
  if(was_running) stop();

  if(shout_set_host(ice,host()))
    error("shout_set_host: %s",shout_get_error(ice));
  
  if( shout_set_port(ice,port()) )
    error("shout_set_port: %s",shout_get_error(ice));
  
  if( shout_set_password(ice,pass()) )
    error("shout_set_password: %s",shout_get_error(ice));

  // === fixes the format of the mountpoint
  if((mount())[0]!='/') {
    char tmp[MAX_VALUE_SIZE];
    sprintf(tmp,"/%s",mount());
    mount(tmp);
  }
  /*
  switch(format) {

  case SHOUT_FORMAT_VORBIS:
    if(!strstr(mount(),".ogg")) {
      char tmp[MAX_VALUE_SIZE];
      sprintf(tmp,"%s.ogg",mount());
      mount(tmp);
    }
    break;

  case SHOUT_FORMAT_MP3:
    if(!strstr(mount(),".mp3")) {
      char tmp[MAX_VALUE_SIZE];
      sprintf(tmp,"%s.mp3",mount());
      mount(tmp);
    }
    break;
  }
  */
  if( shout_set_mount(ice,mount()) )
    error("shout_set_mount: %s",shout_get_error(ice));

  if( shout_set_name(ice,name()) )
    error("shout_set_name: %s",shout_get_error(ice));

  if( shout_set_url(ice,url()) )
    error("shout_set_url: %s",shout_get_error(ice));

  if( shout_set_description(ice,desc()) )
    error("shout_set_description: %s",shout_get_error(ice));

  
  //if( shout_set_bitrate(ice,_bps) )
  //  error("shout_set_bitrate: %s",shout_get_error(ice));
  

  if( shout_set_public(ice,1) )
    error("shout_set_public: %s",shout_get_error(ice));

  if( shout_set_protocol(ice,login()) )
    error("shout_set_protocol: %s",shout_get_error(ice));

  if( shout_set_format(ice,format) )
    error("shout_set_format: %s",shout_get_error(ice));

  if( shout_set_user(ice,"source") )
    error("shout_set_user: %s",shout_get_error(ice));

  snprintf(temp,256,"%s ver. %s",PACKAGE,VERSION);
  if( shout_set_agent(ice,temp) )
    error("shout_set_agent: %s",shout_get_error(ice));
  
  snprintf(streamurl,MAX_VALUE_SIZE,
	   "http://%s:%i%s",host(),port(),mount());
  
  if(was_running) { res = start(); }
  profile_changed = false;
  return res; 
}

int Shouter::send(short int *buf, unsigned int enc) {
  int res = 0;
  if(!running) return(0);
  if(enc<1) return res;

  shout_sync(ice);

  res = shout_send(ice,(unsigned char*) buf, enc);
  if(res) {
    error("shout_send: %s",shout_get_error(ice));
    if (got_sigpipe && (res==SHOUTERR_SOCKET)) {
      errors++;
      got_sigpipe = false;
      if(errors>10) {
	res = -2;
	errors = 0;
      }
    } else res = -1;
  } else errors = 0;

  return(res);
}
