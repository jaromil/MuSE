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

// Muse_console header
//
//

#ifndef __M_CONSOLE__H__
#define __M_CONSOLE__H__

#include "slider.h"
#include "label.h"
#include "screen.h"
#include "entry.h"
#include "fselect.h"
#include "encsetbox.h"
#include "connectbox.h"
#include "streamsetbox.h"
#include "single_channel.h"
#include "about.h"
#include "muse_tui.h"
#include "jmixer.h"

#include <jutils.h>


#define LOGFILE "log.txt"
#define MAX_CHANNELS 6
#define SHORTCUTS_HELP "<C>+</32></B>f<!B><!32>ile +</32></B>u<!B><!32>rl </32></B>t<!B><!32>alk </32></B>c<!B><!32>onnect </32></B>l<!B><!32>ine-in spea</32></B>k<!B><!32>ers </32></B>a<!B><!32>bout </32></B>h<!B><!32>elp </32></B>q<!B><!32>uit"
#define NEWURLENTRY_TITLE "</C>new URL - [ENTER] to confirm [ESC] to cancel"
#define NEWURLENTRY_LABEL "URL: "

#define NEWFILESELECT_TITLE "</C>new file - [ENTER] to confirm [ESC] to cancel"
#define NEWFILESELECT_LABEL "File: "

void error(char *format, ...);
void act(char *format, ...);
void warning(char *format, ...);
void end (void);

#endif
