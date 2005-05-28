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
 
 #ifndef __PLAYLIST_MANAGER_H__
 #define __PLAYLIST_MANAGER_H__
 
 #include <playlist.h>
 #include <xmlprofile.h>
 #include <linklist.h>
 
 #define PL_MAX_PLAYLIST 256
 #define PL_NAME_MAXLEN 256
 
 class PlaylistManager;
 
 class PlaylistManagerEntry {
 
	public:
		PlaylistManagerEntry(XmlTag *tag,int index,PlaylistManager *parent);
		~PlaylistManagerEntry();
		char *name();
		Playlist *playlist();
		bool update(Playlist *playlist);
		
	private:
		int idx;
		PlaylistManager *_parent;
		Playlist *_playlist;
		XmlTag *_tag;
 };
  
 class PlaylistManager {
	public:
		PlaylistManager::PlaylistManager();
		PlaylistManager::~PlaylistManager();
		/* support lockung , useful when we will have a scheduler , 
		 * running in a separate thread that will manage inputchannels and their playlists */
		void lock() { pthread_mutex_lock(&_mutex); };  
		void unlock() { pthread_mutex_unlock(&_mutex); };
		Playlist *load(char *name);
		Playlist *load(int index);
		bool remove(char *name);
		bool remove(int index);
		char *getName(int index);
		bool save(char *name,Playlist *playlist);
		bool update(char *name,Playlist *playlist); /* actually just calls update() for all playlists */
		bool update(int index,Playlist *playlist); /* actually just calls update() for all playlists */
		bool update();
		int len();
		bool isTouched();
		void touch();
		void untouch();
		XmlTag *pl2xml(char *name,Playlist *playlist);
		
	private:
		void PlaylistManager::initPlaylists();
		
		XmlProfile *xml;
		Linklist *playlists;
		char *repository;
		pthread_mutex_t _mutex;
		bool touched;
 };

#endif
