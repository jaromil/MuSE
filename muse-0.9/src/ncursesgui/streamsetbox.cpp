/* A NCURSES/CDK TUI (Text User Interface) for MuSE
 * Copyright (C) 2003 Luca 'rubik' Profico <rubik@olografix.org>
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

#include "streamsetbox.h"
#include "muse_console.h"

StreamSetBox::StreamSetBox ()
{
	logtypelist[0] = strdup ("ICE");
	logtypelist[1] = strdup ("X-Audiocast");
	logtypelist[2] = strdup ("ICY");
	logtypelist[3] = strdup ("HTTP");
}

void StreamSetBox::setscreen (CDKSCREEN *screen)
{
	cdkscreen = screen;
}

void StreamSetBox::setmixer (Stream_mixer *m, encdata *e)
{
	mix = m;
	enc = e;
}

void StreamSetBox::setparm (int x, int y)
{
	xbase = x; ybase = y;
	host.setscreen (cdkscreen); host.box=FALSE;
	port.setscreen (cdkscreen); port.box=FALSE;
	mnt.setscreen (cdkscreen); mnt.box=FALSE;
	name.setscreen (cdkscreen); name.box=FALSE;
	url.setscreen (cdkscreen); url.box=FALSE;
	description.setscreen (cdkscreen); description.box=FALSE;
	logintype.setscreen (cdkscreen); logintype.box=FALSE;
	pass.setscreen (cdkscreen); pass.box=FALSE;
	label1.setscreen (cdkscreen);

	host.setparm (NULL, "</48>h<!48></32>ost:<!32>", 20, xbase + 2, ybase + 8, false, '_');
	port.setparm (NULL, "</32>p<!32></48>o<!48></32>rt:<!32>", 5, xbase + 32, ybase + 8, false, '_');
	mnt.setparm (NULL, "</48>m<!48></32>nt:<!32>", 15, xbase + 45, ybase + 8, false, '_');
	
	name.setparm (NULL, "</48>n<!48></32>ame:<!32>", 27, xbase + 2, ybase + 10, false, '_');
	url.setparm (NULL, "</48>u<!48></32>rl:<!32>", 22, xbase + 38, ybase + 10, false, '_');
	
	description.setparm (NULL, "</48>d<!48></32>escription:<!32>", 50, xbase + 2, ybase + 12, false, '_');
	
	logintype.setparm (NULL, "</32>logint<!32></48>y<!48></32>pe:<!32>", xbase + 1, ybase + 14, logtypelist, 4, 0, false);
	pass.setparm (NULL, "</48>p<!48></32>ass:<!32>", 18, xbase + 29, ybase + 14, false, '_', A_REVERSE, vHMIXED);
	label1.setparm (xbase, ybase,20, 1);
	label1.setvalue ("Stream settings:");

	host.unreg();
	port.unreg();
	mnt.unreg();
	name.unreg();
	url.unreg();
	description.unreg();
	logintype.unreg();
	pass.unreg();
	label1.unreg();

}

void StreamSetBox::draw (void)
{
	host.draw();
	port.draw();
	mnt.draw();
	name.draw();
	url.draw();
	description.draw();
	logintype.draw();
	pass.draw();
	label1.draw();

}

void StreamSetBox::unreg (void)
{
	host.unreg(); host.erase();
	port.unreg(); port.erase();
	mnt.unreg(); mnt.erase();
	name.unreg(); name.erase();
	url.unreg(); url.erase();
	description.unreg(); description.erase();
	logintype.unreg(); logintype.erase();
	pass.unreg(); pass.erase();
	label1.unreg(); label1.erase();
}

void StreamSetBox::erase (void)
{
	host.destroy();
	port.destroy();
	mnt.destroy();
	name.destroy();
	url.destroy();
	description.destroy();
	logintype.destroy();
	pass.destroy();
	label1.destroy();
}

int StreamSetBox::tabentry (int k)
{
	int retval = 0;
	int loop=0;

	while (!loop) {
		loop=1;

		switch (k) {
			case 'h': //host
				host.activate();
				if (host.lastkey() != 9) break;
			case 'o': //port
				port.activate();
				if (port.lastkey() != 9) break;
			case 'm': //mnt
				mnt.activate();
				if (mnt.lastkey() != 9) break;
			case 'n': //name
				name.activate();
				if (name.lastkey() != 9) break;
			case 'u': //url
				url.activate();
				if (url.lastkey() != 9) break;
			case 'd': //description
				description.activate();
				if (description.lastkey() != 9) break;
			case 'y': //logintype
				logintype.activate();
				if (logintype.lastkey() != 9) break;
			case 'p': //pass
				pass.activate();
				if (pass.lastkey() != 9) break;
			case NULL:
				k='h'; // start from first case
				loop = 0;
				break;
			default:
				warning ("k = %d", k);
				retval = 0;
				break;
		}
	}
	return retval;
}

void StreamSetBox::showcurrval (int serverid)
{
	Shouter *ice;
	ice = enc->coreice;
	char temp[32];
	
	host.setvalue (ice->host());
	snprintf (temp, 32, "%d", ice->port());
	port.setvalue (temp);
	mnt.setvalue (ice->mount());
	name.setvalue (ice->name());
	url.setvalue (ice->url());
	description.setvalue (ice->desc());
	logintype.setitem (ice->login());
	pass.setvalue (ice->pass());
}

void StreamSetBox::setval (int serverid)
{
		func ("Streamsetbox:setval (int serverid)");
	Shouter *ice;
	ice = enc->outchan->get_ice(enc->iceid[serverid]);

	ice->host(host.getvalue());
	ice->port(atoi(port.getvalue()));
	ice->mount(mnt.getvalue());
	ice->name(name.getvalue());
	ice->url(url.getvalue());
	ice->desc(description.getvalue());
	ice->login(logintype.getcurritem());
	warning ("logintype = %d", logintype.getcurritem());
	ice->pass(pass.getvalue());
	warning ("pass = %s", pass.getvalue());
}

void StreamSetBox::setdefaultval (void)
{
	char temp[32];
	
	host.setvalue ("localhost");
	snprintf (temp, 32, "%d", 8000);
	port.setvalue (temp);
	mnt.setvalue ("/live.mp3");
	name.setvalue ("MuSE");
	url.setvalue ("http://muse.dyne.org");
	description.setvalue ("Free Software Multiple Streaming Engine");
	logintype.setitem (2);
}

StreamSetBox::~StreamSetBox ()
{
	label1.destroy();
}

