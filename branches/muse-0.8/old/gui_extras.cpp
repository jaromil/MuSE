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

#include "gui_extras.h"
#include <FL/fl_draw.H>
#include <iostream.h>

LevelBlock::LevelBlock(int X, int Y, int W, int H) : Fl_Box(X,Y,W,H)
{ 
  Color[0] = 15;
  Color[1] = 15;
  color_index = 0;
  x = X;
  y = Y;
  w = W;
  h = H;
}

void LevelBlock::draw() {
  //  fl_rectf(x, y, w, h);
  draw_box();
}

Level::Level(int X, int Y, int W, int H) : Fl_Group(X,Y,W,H) 
{
  showit = false;

  totalspacing = (unsigned int)(H - (SPACING * NUM_CELLS))/NUM_CELLS; 
  blockheight = (unsigned int)totalspacing - SPACING;

  n = 0;
  i = 0;
  x = PEAK_VOLUME;
  block[NUM_CELLS-1] = new LevelBlock(X,Y,W,blockheight);
  block[NUM_CELLS-1]->Color[1] = FL_RED;
  for (i = NUM_CELLS-2; i>=0; i--) { 
    x = x - (PEAK_VOLUME/NUM_CELLS);
    n  = n + totalspacing;
    block[i] = new LevelBlock(X, Y+n, W, blockheight);
    if (x > (PEAK_VOLUME/NUM_CELLS)*(NUM_CELLS-2))
      block[i]->Color[1] = FL_YELLOW;
    else
      block[i]->Color[1] = FL_GREEN;
  }
  
  n = NUM_CELLS;
  for (i = NUM_CELLS-1; i >= 0; i--) {
    value[i] = (PEAK_VOLUME/NUM_CELLS)*n;
    // cout << "value[" << i << "]:  " << value [i] << endl;
    n--;
  }
  //  end();  // from the FLTK documentation....says this is needed for extending Fl_Group
}

Level::~Level() {
  for(int c=0;c<NUM_CELLS;c++)
    delete block[c];
}

void Level::draw() {
  for (int i = 0; i < NUM_CELLS; i++) {
    fl_color(block[i]->Color[block[i]->color_index]);
    block[i]->draw();
  }
}

void Level::draw_levels(unsigned short int x) {
  
  int i;
  
  for(i=0;i<NUM_CELLS;i++) {
    if(x>value[i])
      block[i]->color_index = 1;
    else
      block[i]->color_index = 0;
  }
  //  redraw(); 
}
