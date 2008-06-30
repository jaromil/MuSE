/* A NCURSES/CDK TUI (Text User Interface) for MuSE
 * Copyright (C) 2002 Luca 'rubik' Profico <rubik@olografix.org>
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
 *
 * $Id$
 * 
 */

#ifndef __RUB_SCREEN_H__
#define __RUB_SCREEN_H__

extern "C" {
#include <libcdk/cdk.h>
}

class CDKScreen 
{
  // The window which curses uses.
  WINDOW *cursesWin;
  // The CDKSCREEN struct assigned to this object.
  CDKSCREEN *cdkscreen;
 public:
  // Constructor.
  CDKScreen();
  // Deconstructor.
  ~CDKScreen();
  // Return a pointer to the CDKScreen structure.
  CDKSCREEN *screen(void);
  
  int objcnt (void);
  // Refresh the screen.
  // Note, this function is renamed to avoid clashing with the refresh() macro.
  void refreshscr(void);
  // Erase, but don't destroy, all widgets.
  // Note, this function is renamed to avoid clashing with the erase() macro.
  void erasescr(void);
  int width(void);
  int height(void);
  void refresh(void);
  void empty(void);
};

#endif
