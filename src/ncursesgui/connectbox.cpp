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

#include "connectbox.h"
#include "muse_console.h"

ConnectBox::ConnectBox (Stream_mixer *m)
{

	// int i;
	mixer = m ;

	func ("ConnectBox::ConnectBox");

	ybase = (LINES-CONNBOX_HEIGHT)/2;
	xbase = (COLS-CONNBOX_WIDTH)/2;
	subwin = newwin (CONNBOX_HEIGHT, CONNBOX_WIDTH, ybase, xbase);

	warning ("ConnBox pars: %d, %d, %d, %d",xbase, ybase,CONNBOX_WIDTH, CONNBOX_HEIGHT);

	cdkscreen = initCDKScreen (subwin);
	box (subwin, ACS_VLINE, ACS_HLINE);
	wrefresh (subwin);


	/* labels */
	bottom.setscreen (cdkscreen);

	enc = (encdata *) malloc (sizeof (encdata));
	enc->numchan=0;
	warning ("mixer = %p", mixer);
	encsetbox.setmixer (mixer, enc);
	encsetbox.setscreen (cdkscreen);
	
	streamsetbox.setmixer (mixer, enc);
	streamsetbox.setscreen (cdkscreen);
	warning ("mixer = %p", mixer);



	/* load default profiles */

	/* nuova api
	mixer->ice.load_profile("last_active");
	mixer->lame.load_profile("last_active");
	*/
}

void ConnectBox::setparm (void)
{
	bottom.box=TRUE;
	bottom.setparm (xbase +3 , ybase + CONNBOX_HEIGHT,CONNBOX_WIDTH -6 ,1);
	bottom.llhook (ACS_BTEE);
	bottom.lrhook (ACS_BTEE);
	bottom.setvalue ("<C>Connbox");


	encsetbox.setparm(xbase, ybase);
	streamsetbox.setparm(xbase, ybase);
	//showcurrval();
	enc->lameid = mixer->create_enc(MP3);
	enc->oggid = mixer->create_enc(OGG);

	//lame
	enc->outchan=mixer->get_enc (enc->lameid);
	enc->iceid[enc->numchan]=enc->outchan->create_ice();
	enc->coreice=enc->outchan->get_ice(enc->iceid[enc->numchan]);
	enc->outchan->start();
	//ogg
	enc->outchan=mixer->get_enc (enc->oggid);
	enc->iceid[enc->numchan]=enc->outchan->create_ice();
	enc->coreice=enc->outchan->get_ice(enc->iceid[enc->numchan]);
	enc->outchan->start();
}

void ConnectBox::draw (void)
{

	bottom.draw();

	refresh();
}

void ConnectBox::refresh (void)
{
	func ("ConnectBox::refresh()");
	refreshCDKScreen (cdkscreen);
}

void ConnectBox::erase (void)
{
	bottom.destroy();
	//encsetbox.cleanup();
}

void ConnectBox::activate (void)
{
	int quit=0, c, k;
	Shouter *ice;

	while (!quit)
	{
		k = 1;
		//bottom.setvalue ("<C>Use fields' shortcuts, set ic</48>e<!48>cast, set l</48>a<!48>me, or e</48></B>x<!B><!48>it");
		bottom.setvalueNR ("<C>Set La</48>m<!48>e/</48>O<!48>GG, edit </48>s<!48>ervers or e</48>x<!48>it");
		refresh();
		c=getch();
		switch (c) {
			case 'x':
				quit=1;
				break;
			case 'm':
			case 'M':
				encsetbox.reg();
				encsetbox.showcurrval(enc->lameid);

				while (k<999) {
					bottom.setvalue ("<C></48>S<!48>ave/</48>L<!48>oad profiles, </48>A<!48>pply or </48>C<!48>ancel");
					refresh();
					k = getch();
					switch (k) {
						case 'S':
							break;
						case 'L':
							break;
						case 'A':
							k=999;
							encsetbox.setval(enc->lameid);
							break;
						case 'C':
							k=999;
							break;
						default:
							bottom.setvalue ("<C>Edit values using TAB. Enter to confirm");
							encsetbox.tabentry(k);
							break;
					}
				}
				encsetbox.unreg();
				bottom.erase();
				break;
			case 'o':
			case 'O':
				encsetbox.reg();
				encsetbox.showcurrval(enc->oggid);

				while (k<999) {
					bottom.setvalue ("<C></48>S<!48>ave/</48>L<!48>oad profiles, </48>A<!48>pply or </48>C<!48>ancel");
					refresh();
					k = getch();
					switch (k) {
						case 'S':
							break;
						case 'L':
							break;
						case 'A':
							k=999;
							encsetbox.setval(enc->oggid);
							break;
						case 'C':
							k=999;
							break;
						default:
							bottom.setvalue ("<C>Edit values using TAB. Enter to confirm");
							encsetbox.tabentry(k);
							break;
					}
				}
				encsetbox.unreg();
				bottom.erase();
				break;
			case 's':
			case 'S':
				bottom.setvalue ("<C>+</48>L<!48>ameSRV +</48>O<!48>ggSRV edit:[l</48>a<!48>me|O</48>g<!48>g]SRV </48>b<!48>ack");
				c = getch();
				switch (c) {
					case 'l':
					case 'L':
							streamsetbox.setdefaultval();
							if ((enc->numchan) == (MAX_CHANNELS - 1)) {
									bottom.setvalue ("<C>Too many channels configured");
									k = 999;
									getch();
							}
							
							while (k<999){
									bottom.erase();
									bottom.setvalue ("<C></48>A<!48>pply or </48>C<!48>ancel");
									streamsetbox.draw();
									k=getch();
									switch (k) {
											case 'A':
													enc->outchan=mixer->get_enc (enc->lameid);
													// enc->iceid[enc->numchan]=enc->outchan->create_ice();
													// enc->coreice=enc->outchan->get_ice(enc->iceid[enc->numchan]);
													// enc->outchan->start();

													streamsetbox.setval(enc->numchan);
													ice = enc->outchan->get_ice(enc->iceid[enc->numchan]);
													enc->outchan->apply_ice(enc->iceid[enc->numchan]);
													mixer->apply_enc(enc->lameid);
													enc->outchan->connect_ice(enc->iceid[enc->numchan], true);
													enc->numchan++;
													k=999;
													break;
											default:
													bottom.setvalue ("<C>Edit values using TAB. Enter to confirm");
													streamsetbox.tabentry(k);
													break;
											case 'C':
													k=999;
													break;
									}
							}
						streamsetbox.unreg();
						break;
					case 'o':
					case 'O':
						streamsetbox.setdefaultval();
							if ((enc->numchan) == (MAX_CHANNELS - 1)) {
									bottom.setvalue ("<C>Too many channels configured");
									k = 999;
									getch();
							}
							
							while (k<999){
									bottom.erase();
									bottom.setvalue ("<C></48>A<!48>pply or </48>C<!48>ancel");
									streamsetbox.draw();
									k=getch();
									switch (k) {
											case 'A':
													enc->outchan=mixer->get_enc (enc->oggid);
													// enc->iceid[enc->numchan]=enc->outchan->create_ice();
													// enc->coreice=enc->outchan->get_ice(enc->iceid[enc->numchan]);
													// enc->outchan->start();
													
													streamsetbox.setval(enc->numchan);
													ice = enc->outchan->get_ice(enc->iceid[enc->numchan]);
													enc->outchan->apply_ice(enc->iceid[enc->numchan]);
													mixer->apply_enc(enc->oggid);
													enc->outchan->connect_ice(enc->iceid[enc->numchan], true);
													enc->numchan++;
													k=999;
													break;
											default:
													bottom.setvalue ("<C>Edit values using TAB. Enter to confirm");
													streamsetbox.tabentry(k);
													break;
											case 'C':
													k=999;
													break;
									}
							}
						streamsetbox.unreg();
						break;
					case 'a':
					case 'A':
						break;
					case 'g':
					case 'G':
						break;
					case 'b':
					case 'B':
						break;
				}
				bottom.erase();
				break;
			default:
				//quit=tabentry(c);
				break;
		}
	}
}

	
ConnectBox::~ConnectBox ()
{
	warning ("ConnectBox::~ConnectBox objcnt = %d [BEGIN]", cdkscreen->objectCount); 

	//bottom.destroy();

	encsetbox.cleanup();

	warning ("ConnectBox::~ConnectBox objcnt = %d ", cdkscreen->objectCount); 
	if (cdkscreen) {
		destroyCDKScreen(cdkscreen);
		cdkscreen=NULL;
	}
	warning ("ConnectBox::~ConnectBox objcnt [END]"); 
}
