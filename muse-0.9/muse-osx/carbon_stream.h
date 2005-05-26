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
#include <xmlprofile.h>

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
		int addStream();
		void deleteStream(); /* delete selected stream */
		void deleteStream(int idx); /* delete stream at index idx */
		int addServer(); /* just an accessor to addServer(int strIdx) that uses selectedServer as strIdx */
		int addServer(int strIdx);
		CarbonStreamEncoder *selectedStream();
		CarbonStreamEncoder *getStream(int idx);
		bool updateStreamTab();
		void deleteServer(); /* delete selected server for selected stream */
		void deleteServer(int streamIndex,int serverIndex); /* delete a specific server in a specific stream */
		CarbonStreamServer *selectedServer();
		CarbonStreamServer *getServer(int strIdx,int srvIdx);
		int countServers(int strIdx);
		bool updateServerTab();
		bool changeServerTab();
		bool doConnect();
		bool connectServer(CarbonStreamServer *server);
		bool disconnectServer(CarbonStreamServer *server);
		bool updateQuality(); 
		bool updateBitrate();
		bool updateFrequency();
		bool updateMode();
		bool updateFilteringMode();
		void recordStreamPath(char *path);
		void recordStream();
		void activateMenuBar();
		void codecChange();
		bool loadPreset(int idx);
		bool deletePreset(int idx);
		bool savePreset(char *name);
		void savePresetDialog();
		void confirmSavePreset();
		void cancelSavePreset();
		
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
		void delTab(SInt32 controlID,int idx);
		int nextTabIndex(SInt32 controlID);
		void saveServerInfo(CarbonStreamServer *server);
		void saveStreamInfo(CarbonStreamEncoder *encoder);
		void updateServerInfo(CarbonStreamServer *server);
		void updateStreamInfo(CarbonStreamEncoder *encoder);
		void updatePresetControls();
		
		IBNibRef nibRef;
		CarbonStreamEncoder *enc[MAX_STREAM_ENCODERS];
		CarbonStreamServer *servers[MAX_STREAM_ENCODERS][MAX_STREAM_SERVERS];
		MenuRef streamMenu;
		WindowGroupRef streamGroup;
		XmlProfile *presets;
		WindowRef savePresetWindow;
	protected:

};

#endif