/*  Fast atomical linklist class
 *  (c) Copyright 2001 Denis Roio aka jaromil <jaromil@dyne.org>
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

#ifndef __linklist_h__
#define __linklist_h__

#include <pthread.h>

class Entry;

class Linklist {
 public:
  Linklist();
  virtual ~Linklist();

  Entry *begin() { return(first); };
  Entry *end() { return(last); };
  int len() { return(length); };
  
  void append(Entry *addr);
  void add(Entry *addr) { append(addr); }; /* lazy backward compatibility */
  void prepend(Entry *addr);
  void insert(Entry *addr, int pos);
  
  bool rem(int pos);
  bool sel(int pos);
  bool clear();
  bool moveup(int pos);
  bool movedown(int pos);
  bool moveto(int num, int pos);
  Entry *pick(int pos);  
  Entry *Linklist::selected();

  Entry *operator[](int pos) { return pick(pos); };

  /* don't touch these directly */
  Entry *first;
  Entry *last;
  int length;

  /* deprecated, here just for MuSE */
  Entry *pick_id(int id);
  int selected_pos();

  /* thread stuff */
  void lock();
  void unlock();
 private:
  pthread_mutex_t _mutex;

};

class Entry {
 public:
  Entry();
  ~Entry();
  
  Entry *next;
  Entry *prev;

  Linklist *list;

  bool up();
  bool down();
  bool move(int pos);
  void rem();
  void sel(bool on);

  int id;  /* deprecated, here just for MuSE */
  bool select;
};

#endif
