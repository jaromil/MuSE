/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2005 xant <xant@dyne.org>
 *
 * This sourcCARBONe code is free software; you can redistribute it and/or
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

#include <Carbon/Carbon.h>

#ifndef __STREAM_SERVER_H__
#define __STREAM_SERVER_H__

#define SERVER_STRING_BUFFER_LEN 256
#include <outchannels.h>
#include "stream_encoder.h"

class CarbonStreamEncoder;

class CarbonStreamServer {
	public:
		CarbonStreamServer(CarbonStreamEncoder *enc);
		CarbonStreamServer(CarbonStreamEncoder *enc,char *hostName,short tcpPort, char *mnt, 
			char *streamName,char *streamUrl,char *descr,char *user,char *pass);
		~CarbonStreamServer();
		bool connect();
		bool isConnected();
		bool disconnect();
		
		/* get mathods */
		char *host();
		int port();
		char *mount();
		char *name();
		char *url();
		char *description();
		int loginType();
		char *username();
		char *password();
		int status();
		
		/* set methods */
		void host(char *h);
		void port(int p);
		void mount(char *m);
		void name(char *n);
		void url(char *u);
		void description(char *d);
		void loginType(int lType);
		void username(char *user);
		void password(char *pass);
		
	private:
		Shouter *getIce();
		void applyIce();
	
		CarbonStreamEncoder *encoder;
		char _host[SERVER_STRING_BUFFER_LEN];
		int _port;
		char _mount[SERVER_STRING_BUFFER_LEN];
		char _name[SERVER_STRING_BUFFER_LEN];
		char _url[SERVER_STRING_BUFFER_LEN];
		char _description[SERVER_STRING_BUFFER_LEN];
		int _loginType;
		char _username[SERVER_STRING_BUFFER_LEN];
		char _password[SERVER_STRING_BUFFER_LEN];
		int _status;
		int iceID;
};


#endif
