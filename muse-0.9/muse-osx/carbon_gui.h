/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2005 xant <xant@dyne.org>
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
 */
#ifndef __CARBON_GUI_H__
#define __CARBON_GUI_H__

#include <gui.h>
#include "carbon_common.h"
#include "playlist_manager.h"
#include <carbon_channel.h>
#include <carbon_stream.h>
#include <carbon_message.h>
#include <credits.h>
#include <buffer_inspector.h>

//OSStatus startCarbon(void *arg);
class CarbonChannel;

/* 
 * Main class for the Carbon Interface.
 * This class extends the base one GUI and both implements the interface 
 * used by jmixer and handles other carbon related objects 
 * (channel and stream windows, vumeters, etc )
 */
class CARBON_GUI : public GUI {

 public:
  CARBON_GUI(int argc, char **argv, Stream_mixer *mix);
  ~CARBON_GUI();

  void run(); /* the mainLoop of the carbon gui */
  void set_pos(unsigned int chan, float pos) {new_pos[chan] = true;};
  void set_lcd(unsigned int chan, char *lcd) {new_lcd[chan] = true;};
  void set_title(char *txt);
  void set_status(char *txt); 
  void add_playlist(unsigned int ch, char *txt);
  void sel_playlist(unsigned int ch, int row);
  void bpsmeter_set(int n) { vuband = n; };
  void vumeter_set(int n) { vumeter = n; };
  bool meter_shown() { return meterShown(); } /* just an accessor to meterShown ... here for muse API compatibility */
  void stop();
  void showStreamWindow();
  bool new_channel();
  bool remove_channel(int idx);
  bool attract_channels(int chIndex,AttractedChannel *neigh);
  void showVumeters(bool flag);
  void showStatus(bool flag);
  void showBufferInspector(bool flag);
  void toggleStatus();
  void toggleVumeters();
  void toggleBufferInspector();
  void clearStatus();
  bool meterShown();
  bool statusShown(); 
  void bringToFront();
  void activatedChannel(int idx);
  void credits();
 // void start();

  WindowRef 	window; /* the main window */
  WindowGroupRef mainGroup; /* the main window group */
  IBNibRef 		nibRef; /* nibRef with resources for all windows,controls and menu */
  Stream_mixer *jmix; /* the Stream_mixer object AKA the MuSE core */
  CarbonMessage *msg; /* a simple message object to let us notify errors to user trough the gui */
  PlaylistManager *playlistManager; /* an object to handle load&save of playlists */
  BufferInspector *bufferInspector;

private:
  bool init_controls();
  void setupVumeters();
  void updateVumeters();
  void setupStatusWindow();
  bool new_channel(int idx);
  void msgOut(char *txt);

  bool new_pos[MAX_CHANNELS];
  bool new_lcd[MAX_CHANNELS];
  int myPos[MAX_CHANNELS];
  char myLcd[MAX_CHANNELS][255];
  unsigned int new_sel[MAX_CHANNELS];

  int vuband, vumeter;
  WindowRef vumeterWindow;
  WindowRef statusWindow;
  MenuRef mainMenu;
  OSStatus		err;
  CarbonChannel	*channel[MAX_CHANNELS];
  CarbonChannel *selectedChannel;
  HIViewRef statusTextView;
  TXNObject statusText;
  CarbonStream *streamHandler;
  AboutWindow *aboutWindow;
  Linklist *msgList;
  
  pthread_mutex_t _statusLock;
  void statusLock() { pthread_mutex_lock(&_statusLock); };
  void statusUnlock() { pthread_mutex_unlock(&_statusLock); };
  
};

#endif
