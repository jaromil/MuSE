/* Amuse GUI
 * Copyright (C) 2000-2001 August Black <august@alien.mur.at>
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

#ifndef __LEVEL_H
#define __LEVEL_H

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
//#include <FL/Fl_Button.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Box.H>

#include "generic.h"

class LevelBlock : public Fl_Box {
 public:
  LevelBlock(int X,int Y,int W,int H); 
  void draw();
  int Color[2];
  bool color_index;
  
  int x, y, h, w;

};

class Level : public Fl_Group {
 public:
  Level(int X, int Y, int W, int H);
  ~Level();
  void draw();
  void draw_levels(unsigned short int x);
  bool showit;
 private:
  int value[NUM_CELLS];
  LevelBlock *block[NUM_CELLS];
  unsigned int totalspacing, blockheight;
  int n,i;
  unsigned int x;
};
#endif
