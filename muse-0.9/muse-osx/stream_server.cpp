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

#include "stream_server.h"

/****************************************************************************/
/* CarbonStreamServer class */
/****************************************************************************/

CarbonStreamServer::CarbonStreamServer(CarbonStreamEncoder *enc) {
	encoder=enc;
	memset(_host,0,SERVER_STRING_BUFFER_LEN);
	memset(_mount,0,SERVER_STRING_BUFFER_LEN);
	memset(_name,0,SERVER_STRING_BUFFER_LEN);
	memset(_url,0,SERVER_STRING_BUFFER_LEN);
	memset(_description,0,SERVER_STRING_BUFFER_LEN);
	memset(_username,0,SERVER_STRING_BUFFER_LEN);
	memset(_password,0,SERVER_STRING_BUFFER_LEN);
	_port=0;
	_loginType=0;
	_status=0;
	OutChannel *chan = encoder->getOutChannel();
	iceID=chan->create_ice();
}

CarbonStreamServer::CarbonStreamServer(CarbonStreamEncoder *enc,char *hostName,short tcpPort,
	char *mnt, char *streamName,char *streamUrl,char *descr,char *user,char *pass) 
{
	encoder=enc;
	host(hostName);
	port(tcpPort);
	mount(mnt);
	name(streamName);
	url(streamUrl);
	description(descr);
	username(user);
	password(pass);
	OutChannel *chan = encoder->getOutChannel();
	iceID=chan->create_ice();
}

CarbonStreamServer::~CarbonStreamServer() {
	OutChannel *chan = encoder->getOutChannel();
	chan->delete_ice(iceID);
}

bool CarbonStreamServer::connect() {
	OutChannel *chan = encoder->getOutChannel();
	if(chan) return chan->connect_ice(iceID,true);
	return false;
}

bool CarbonStreamServer::isConnected() {
	Shouter *ice = getIce();
	if(ice) return ice->running;
	return false;
}

bool CarbonStreamServer::disconnect() {
	OutChannel *chan = encoder->getOutChannel();
	if(chan) return chan->connect_ice(iceID,false);
	return false;
}

Shouter *CarbonStreamServer::getIce() {
	OutChannel *chan=encoder->getOutChannel();
	return chan->get_ice(iceID);
}

void CarbonStreamServer::applyIce() {
	OutChannel *chan=encoder->getOutChannel();
	if(!isConnected()) chan->apply_ice(iceID);
}

char *CarbonStreamServer::host() {
	return _host;
}
int CarbonStreamServer::port() {
	return _port;
}
char *CarbonStreamServer::mount() {
	return _mount;
}
char *CarbonStreamServer::name() {
	return _name;
}
char *CarbonStreamServer::url() {
	sprintf(_url,"http://%s:%d/%s",_host,_port,_mount);
	return _url;
}
char *CarbonStreamServer::description() {
	return _description;
}
int CarbonStreamServer::loginType() {
	return _loginType;
}
char *CarbonStreamServer::username() {
	return _username;
}
char *CarbonStreamServer::password() {
	return _password;
}
int CarbonStreamServer::status() {
	return _status;
}

void CarbonStreamServer::host(char *h) {
	strncpy(_host,h,SERVER_STRING_BUFFER_LEN-1);
	_host[SERVER_STRING_BUFFER_LEN-1]=0;
	Shouter *ice=getIce();
	if(ice) {
		ice->host(_host);
		applyIce();
	}
}
void CarbonStreamServer::port(int p) {
	_port=p;
}
void CarbonStreamServer::mount(char *m) {
	//strncpy(_mount,m,SERVER_STRING_BUFFER_LEN-1);
	//_mount[SERVER_STRING_BUFFER_LEN-1]=0;
	memset(_mount,0,SERVER_STRING_BUFFER_LEN);
	int len=strlen(m);
	if(len>SERVER_STRING_BUFFER_LEN-1) len=SERVER_STRING_BUFFER_LEN-1;
	for(int i=0;i<len;i++) {
		if(m[i]=='/') _mount[i]='-';
		else _mount[i]=m[i];
	}
	//_mount[i]=0;
	Shouter *ice=getIce();
	if(ice) {
		ice->mount(_mount);
		applyIce();
	}
}
void CarbonStreamServer::name(char *n) {
	strncpy(_name,n,SERVER_STRING_BUFFER_LEN-1);
	_name[SERVER_STRING_BUFFER_LEN-1]=0;
	Shouter *ice=getIce();
	if(ice) {
		ice->name(_name);
		applyIce();
	}
}
void CarbonStreamServer::url(char *u) {
	strncpy(_url,u,SERVER_STRING_BUFFER_LEN-1);
	_url[SERVER_STRING_BUFFER_LEN-1]=0;
	Shouter *ice=getIce();
	if(ice) {
		ice->url(_url);
		applyIce();
	}
}
void CarbonStreamServer::description(char *d) {
	strncpy(_description,d,SERVER_STRING_BUFFER_LEN-1);
	_description[SERVER_STRING_BUFFER_LEN-1]=0;
	Shouter *ice=getIce();
	if(ice) {
		ice->desc(_description);
		applyIce();
	}
}
void CarbonStreamServer::loginType(int lType) {
	_loginType=lType;
		applyIce();
}
void CarbonStreamServer::username(char *user) {
	strncpy(_username,user,SERVER_STRING_BUFFER_LEN-1);
	_username[SERVER_STRING_BUFFER_LEN-1]=0;
	Shouter *ice=getIce();
	if(ice) {
		ice->user(_username);
		applyIce();
	}
}
void CarbonStreamServer::password(char *pass) {
	strncpy(_password,pass,SERVER_STRING_BUFFER_LEN-1);
	_password[SERVER_STRING_BUFFER_LEN-1]=0;
	Shouter *ice=getIce();
	if(ice) {
		ice->pass(_password);
		applyIce();
	}
}
