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

#include "muse_console.h"
#include "panel.h"
#include <iostream.h>
#include <fstream.h>
#include <stdlib.h>

//ofstream logfile (LOGFILE, ios::app );

muse_tui *t = NULL;

int main (int argc, char **argv)
{
	atexit(end); // function end() will close logfile
	warning ("------------- Start ---------------");

	t = new muse_tui;
	t->start();

	if (t) delete t;
	return 0;
}

void end(void)
{
	warning ("------------- End ---------------");
	//logfile.close();
}

/*
void error(char *format, ...) 
{
	char msg[255];
	va_list arg;
	va_start(arg, format);
	
	vsnprintf(msg, 254, format, arg);
	logfile << "[!] " << msg << endl;
	
	va_end(arg);
}

void act(char *format, ...) 
{
	char msg[255];
	va_list arg;
	va_start(arg, format);
	
	vsprintf(msg, format, arg);
	logfile << " .  " << msg << endl;
	
	va_end(arg);
}

void warning(char *format, ...) 
{
	char msg[255];
	va_list arg;
	va_start(arg, format);
	
	vsprintf(msg, format, arg);
	logfile << "[W] " << msg << endl;
	
	va_end(arg);
}

*/
