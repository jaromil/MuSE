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

#include "menu.h"
#include "muse_console.h"

CDKMenu::CDKMenu() {

}

CDKMenu::~CDKMenu() {

}

void CDKMenu::setscreen (CDKSCREEN *s) {
	cdkscreen = s;
}

void CDKMenu::setparm (char *menulist[MAX_MENU_ITEMS][MAX_SUB_ITEMS], int menuListLength, int *submenuListLength, int *menuLocation, int menuPos, chtype titleattr, chtype subtitleattr) {
	func ("CDKMenu::setparm");
	menu = newCDKMenu (
			cdkscreen,
			menulist,
			menuListLength,
			submenuListLength,
			menuLocation,
			menuPos,
			titleattr,
			subtitleattr
			);
}

void CDKMenu::draw (bool b) {
	func ("CDKMenu::draw");
	box = b;
	drawCDKMenu (menu, box);
}

void CDKMenu::draw (void) {
	drawCDKMenu (menu, box);
}

void CDKMenu::activate (void) {
	activateCDKMenu (menu, NULL);
}

