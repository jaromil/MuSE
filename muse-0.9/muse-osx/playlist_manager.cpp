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

#include "playlist_manager.h"
#include <jutils.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/****************************************************************************/
/* PlaylistManager class */
/****************************************************************************/

PlaylistManager::PlaylistManager() {
	struct stat st;
	char *home=NULL;
	playlists=NULL;
	touched=false;
	if(pthread_mutex_init (&_mutex,NULL))
		error("%i:%s error initializing POSIX thread mutex",
		__LINE__,__FILE__);
	if(!(home = getenv("HOME"))) {
        error(_("no $HOME found"));
		repository=(char *)malloc(3);
		sprintf(repository,"./");
    }
	else {
		repository=(char *)malloc(strlen(home)+8);
		sprintf(repository,"%s/.muse/",home);
		if(stat(repository,&st)!=0) {
			if(mkdir(repository,S_IRWXU)!=0) {
				error("Can't create preferences directory %s !!",repository);
				/* TODO - should printout errno */
			}
		}
	}
	xml=new XmlProfile("playlist",repository);
	if(xml) initPlaylists();
}

PlaylistManager::~PlaylistManager() {
	if(pthread_mutex_destroy (&_mutex))
		error("%i:%s error destroying POSIX thread mutex",
		__LINE__,__FILE__);
	if(xml) delete xml;
}

void PlaylistManager::initPlaylists() {
	int i;
	lock();
	int plNum = xml->numBranches();
	if(playlists) delete playlists;
	playlists = new Linklist;
	for(i=1;i<=plNum;i++) {
		XmlTag *playlistTag=xml->getBranch(i);
		if(playlistTag) {
			if(strcmp(playlistTag->name(),"playlist")!=0) {
				error("Bad xml node. Expected 'playlist' found '%s'",playlistTag->name());
				continue; /* skip this node */
			}
			func("new playlist found \"%s\" \n",playlistTag->value());
			PlaylistManagerEntry *newPlEntry = new PlaylistManagerEntry(playlistTag,i,this);
			Entry *newEntry=new Entry((void *)newPlEntry);
			playlists->append(newEntry);
		}
	}
	unlock();
}

Playlist *PlaylistManager::load(char *name) {
	lock();
	for(int i=1;i<=playlists->len();i++) {
		Entry *tmp=playlists->pick(i);
		if(tmp) {
			PlaylistManagerEntry *entry=(PlaylistManagerEntry *)tmp->get_value();
			if(entry) {
				if(strcmp(entry->name(),name)==0) {
					unlock();
					return entry->playlist();
				}
			}
		}
	}
	unlock();
	return NULL;
}

Playlist *PlaylistManager::load(int index) {
	lock();
	Entry *tmp=playlists->pick(index);
	if(tmp) {
		PlaylistManagerEntry *entry=(PlaylistManagerEntry *)tmp->get_value();
		if(entry) {
			unlock();
			return entry->playlist();
		}
	}
	unlock();
	return NULL;
}

bool PlaylistManager::remove(char *name) {
	lock();
	if(name) {
		for(int i=1;i<=playlists->len(); i++) {
			char *eName=getName(i);
			if(eName && strcmp(eName,name)==0) {
				unlock();
				return remove(i);
			}
		}
	}
	unlock();
	return false;
}

bool PlaylistManager::remove(int index) {
	lock();
	Entry *tmp=playlists->pick(index);
	if(tmp) {
		PlaylistManagerEntry *entry=(PlaylistManagerEntry *)tmp->get_value();
		if(entry) {
			delete entry;
		}
		playlists->rem(index);
		xml->removeBranch(index);
		if(xml->update()) {
			unlock();
			return true;
		}
	}
	unlock();
	return false;
}

bool PlaylistManager::isTouched() {
	return touched;
}

void PlaylistManager::touch() {
	lock();
	touched=true;
	unlock();
}

void PlaylistManager::untouch() {
	lock();
	touched=false;
	unlock();
}

char *PlaylistManager::getName(int index) {
	lock();
	Entry *tmp=playlists->pick(index);
	if(tmp) {
		PlaylistManagerEntry *entry=(PlaylistManagerEntry *)tmp->get_value();
		if(entry) {
			unlock();
			return entry->name();
		}
	}
	unlock();
	return NULL;
}

bool PlaylistManager::save(char *name,Playlist *playlist) {
	lock();
	XmlTag *playlistTag=pl2xml(name,playlist);
	if(xml->addRootElement(playlistTag) && xml->update()) {
		PlaylistManagerEntry *newPlEntry = new PlaylistManagerEntry(playlistTag,playlists->len()+1,this);
		Entry *newEntry=new Entry((void *)newPlEntry);
		playlists->append(newEntry);
		unlock();
		return true;
	}
	unlock();
	return false;
}

bool PlaylistManager::update(char *name,Playlist *playlist) {
}

bool PlaylistManager::update(int index, Playlist *playlist) {
	lock();
	Entry *tmp=playlists->pick(index);
	if(tmp) {
		PlaylistManagerEntry *entry=(PlaylistManagerEntry *)tmp->get_value();
		if(entry->update(playlist)) {
			bool res=xml->update();
			unlock();
			return res;
		}
		else {
			warning("Can't update playlist entry at index %d",index);
		}
	}
	unlock();
	return false;
}

int PlaylistManager::len() {
	return playlists->len();
}

XmlTag *PlaylistManager::pl2xml(char *name,Playlist *playlist) {
	int i;
	XmlTag *playlistTag = new XmlTag("playlist",name,NULL);
	for(i=1;i<=playlist->len();i++) {
		Url *entry=(Url *)playlist->pick(i);
		playlistTag->addChild("url",entry->path);
	}
	return playlistTag;
}

/****************************************************************************/
/* PlaylistManagerEntry class */
/****************************************************************************/

PlaylistManagerEntry::PlaylistManagerEntry(XmlTag *tag,int index,PlaylistManager *parent) {
	int i;
	_parent=parent;
	idx=index;
	_tag=tag;
	_playlist=new Playlist;
	int numEntries=_tag->numChildren();
	for(i=1;i<=numEntries;i++) {
		XmlTag *xmlEntry=_tag->getChild(i);
		if(xmlEntry) _playlist->addurl(xmlEntry->value());
	}
}

PlaylistManagerEntry::~PlaylistManagerEntry() {
	delete _playlist;
}

char *PlaylistManagerEntry::name() {
	return _tag->value();
}

Playlist *PlaylistManagerEntry::playlist() {
	return _playlist;
}

bool PlaylistManagerEntry::update(Playlist *newpl) {
	int i;
	if(_playlist->clear() && _tag->removeChildren()) {
		for(i=1;i<=newpl->len();i++) {
			Url *url=(Url *)newpl->pick(i);
			if(url) {
				_playlist->addurl(url->path);
				if(!_tag->addChild("url",url->path)) 
					warning("Can't add %s to the new playlist tag",url->path);
			}
		}
		return true;
	}
	else {
		warning("Can't clean old playlist");
	}
	return false;
}
