/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2002-2004 jaromil <jaromil@dyne.org>
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

#ifndef __CARBON_STREAM_H__
#define __CARBON_STREAM_H__

#include <Carbon/Carbon.h>
#include <carbon_common.h>
#include <jmixer.h>
#include "stream_server.h"
#include "stream_encoder.h"
#include <carbon_message.h>

#define MAX_STREAM_SERVERS 8
#define MAX_STREAM_ENCODERS 8

#define CS_NOT_CONNECTED 0
#define CS_CONNECTED 1



class CarbonStream {
	public:
		CarbonStream(Stream_mixer *mix,WindowRef mainWin,IBNibRef nib);
		~CarbonStream();
		void show();
		void hide();
		void addStream();
		void deleteStream(); /* delete selected stream */
		void deleteStream(unsigned int idx); /* delete stream at index idx */
		void addServer();
		void deleteServer(); /* delete selected server for selected stream */
		void deleteServer(unsigned int streamIndex,unsigned int serverIndex); /* delete a specific server in a specific stream */
		bool updateStreamTab();
		bool updateServerTab();
		bool changeServerTab();
		bool connectServer();
		bool connectServer(unsigned int streamIndex,unsigned int serverIndex);
		CarbonStreamEncoder *selectedStream();
		CarbonStreamServer *selectedServer();
		
		WindowRef window;
		WindowRef parent;
		Stream_mixer *jmix;
		CarbonMessage *msg;
	
	private:
		ControlRef streamTabControl;
		ControlRef serverTabControl;
		int getTabValue(SInt32 controlID,int tab);
		void setTabValue(SInt32 controlID,int tab,int val);
		short _selectedStream;
		short _selectedServer;
		bool addTab(SInt32 controlID);
		void delTab(SInt32 controlID,unsigned int idx);
		int nextTabIndex(SInt32 controlID);
		void saveServerInfo(CarbonStreamServer *server);
		void updateServerInfo(CarbonStreamServer *server);
		IBNibRef nibRef;
		CarbonStreamEncoder *enc[MAX_STREAM_ENCODERS];
		CarbonStreamServer *servers[MAX_STREAM_ENCODERS][MAX_STREAM_SERVERS];
		
		
	protected:

};

#endif