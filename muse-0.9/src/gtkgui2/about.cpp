#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <gen.h>

#include <gtk/gtk.h>

#include <utils.h>
#include <xpm2/asbepallo.h>

GtkWidget*
about_win(void)
{
	GtkWidget *window1;
	GtkWidget *vbox1, *hbox1;
	GtkWidget *aboutframe;
	GtkWidget *img;
	GtkWidget *label1, *fixed1;
	GtkWidget *scrolledwindow1;
	GtkWidget *view;
	GtkTextBuffer *buffer;
	GdkPixbuf *pixbuf;

	window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	/* look ad Gnome Human Interface Guidelines ;) */
	gtk_container_set_border_width(GTK_CONTAINER(window1), 12);
	//gtk_widget_set_size_request(window1, 550, 300);
	gtk_window_set_title (GTK_WINDOW (window1), _("about MuSE: the Multiple Streaming Engine"));
	gtk_window_set_resizable(GTK_WINDOW(window1), FALSE);
	g_signal_connect(G_OBJECT(window1), "delete_event",
		  G_CALLBACK(gtk_widget_destroy), NULL);

	aboutframe = gtk_frame_new(_("MuSE: Codename COTURNIX"));
	gtk_container_add(GTK_CONTAINER(window1), aboutframe);
	//gtk_container_set_border_width(GTK_CONTAINER(aboutframe), 6);
	
	fixed1 = gtk_fixed_new();	

	hbox1 = gtk_hbox_new(FALSE, 6);
	vbox1 = gtk_vbox_new(FALSE, 6);
	
	gtk_container_add(GTK_CONTAINER(aboutframe), hbox1);
	gtk_box_pack_start(GTK_BOX(hbox1), vbox1, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(vbox1), fixed1);

	label1 = gtk_label_new (_("development team for this release:\n"
			  "\n"
			  "jaromil aka Denis Rojo - main coder and mantainer\n"
			  "nightolo aka Antonino Radici - GTK user interface\n"
			  "rubik aka Luca Profico - console user interface\n"
			  "godog aka Filippo Giunchedi - docu and organizer\n"));

	//gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
	//gtk_box_pack_start(GTK_BOX(vbox1), label1, FALSE, FALSE, 0);
	gtk_fixed_put(GTK_FIXED(fixed1), label1, 6, 6);

	scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	//gtk_box_pack_start(GTK_BOX(vbox1), scrolledwindow1, FALSE, FALSE, 12);
	gtk_fixed_put(GTK_FIXED(fixed1), scrolledwindow1, 6, 120);

	gtk_widget_set_size_request(scrolledwindow1, 400, 153);
	
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);


	view = gtk_text_view_new();
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	gtk_container_add(GTK_CONTAINER(scrolledwindow1), view);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
	gtk_text_buffer_set_text(buffer,_( 
		   "MuSE is an application for the mixing, encoding, and\n"
		   "streaming of sound: is an engine that can simultaniously\n"
		   "mix up to 6 separate MP3 or OggVorbis audio files from\n"
		   "the hard drive or the network, where each channel of\n"
		   "audio can be dynamicly adjusted for speed and volume\n"
		   "plus a soundcard line-in channel. The resulting stream\n"
		   "can be played locally on the sound card and/or\n"
		   "encoded as an mp3 network stream to an icecast or\n"
		   "shoutcast server (ready to be mixed and played\n"
		   "again by other muses... ;)\n"
		   "\n"
		   "MuSE as it is now would have never existed without\n"
		   "the contributions of (in reverse chronological order):\n"
		   "= Matteo Nastasi aka Mop ( alternativeoutput.it )\n"
		   "= Lobo for herbivore and free open air open source\n"
		   "= PUBLIC VOICE Lab ( pvl.at ) support for development\n"
		   "= SERVUS.AT trusting this project since its beginning\n"
		   "= Asbesto Molesto ( freaknet.org ) testing and docu\n"
		   "= Alex, Rasty and Martinez ( maria libera! ) good vibes\n"
		   "= August Black ( aug.ment.org ) original GUI scheme\n"
		   "= Markus Seidl ( funda.ment.org ) vision of such a tool\n"
		   "\n"
		   "Also waves and thanks to:\n"
		   "voyager, void, blicero, sandolo, eni,\n"
		   "the Freaknet Medialab, the ASCII squat,\n"
		   "henk, the imc-audio collective, jeff,\n"
		   "the LOA hacklab, the TPO, bundes & didi,\n"
		   "indymedia italy, neural.it, autistici.org,\n"
		   "Mag-One, radio Ondarossa, bomboclat, newmark\n"
		   "c1cc10, vanguardist, janki, godog, kysucix\n"
		   "and all the others i'm forgetting here!\n"
		   "\n"
		   "\nMuSE redistributes statically, the following libraries:\n= libmpeg by Woo-jae Jung and Mikael Hedin\n= libshout by Jack Moffit and others\n= libcdk by Mike Glover\n\n\nMuSE can link dinamically to the following libraries:\n= libogg, libvorbis, libvorbisfile - www.xiph.org\n= glib, libgdk, libgtk - www.gtk.org\n= liblame - www.mp3dev.org/mp3\n= libX11, libXext - www.xfree86.org\n= other common GNU libraries\n\n\nMuSE sourcecode also got inspirations and code\nsnippets from the stream mixer sourcecode by\nScott Manley, the buffered FIFO pipe source\nby Charles Samuels, the GTK knob widget by\nAlexander Koenig, GNOME icons by Jakub Steiner\n\n"
		   "MuSE Streamer is copyleft (c)\n"
		   "2000-2003 by Denis \"jaromil\" Rojo\n"
		   "\n"
		   "MuSE's GTK+ GUI is copyleft (c)\n"
		   "2001-2003 by Antonino \"nightolo\" Radici\n"
		   "\n"
		   "MuSE's NCURSES GUI is copyleft (c)\n"
		   "2002-2003 by Luca \"rubik\" Profico\n"
		   "\n"
		   "DSP resampling routines are copyleft (c)\n"
		   "2002 by Matteo \"MoP\" Nastasi\n"
		   "\n"
		   "MuSE's first GUI scheme is copyleft (c)\n"
		   "2000-2001 by August Black\n"
		   "\n"
		   "part of the included code is copyright by the\n"
		   "respective authors, please refer to the supplied\n"
		   "sourcecode for further informations.\n"
		   "\n-----------------------------------------------------------\n"
		   "\nThis source code is free software; you can redistribute\nit and/or modify it under the terms of the GNU Public\nLicense as published by the Free Software Foundation;\neither version 2 of the License, or (at your option) any\nlater version.\n\nThis source code is distributed in the hope that it will\nbe useful, but WITHOUT ANY WARRANTY; without\neven the implied warranty of MERCHANTABILITY or\nFITNESS FOR A PARTICULAR PURPOSE.\nPlease refer to the GNU Public License for more details.\n\nYou should have received a copy of the GNU Public\nLicense along with this source code; if not, write to:\nFree Software Foundation, Inc., 675 Mass Ave,\nCambridge, MA 02139, USA\n\n\n"), 
	-1);
	
	
	pixbuf = gdk_pixbuf_new_from_inline(-1, asbepallo, 0, NULL);
	img = gtk_image_new_from_pixbuf(pixbuf);
	gtk_box_pack_start(GTK_BOX(hbox1), img, FALSE, FALSE, 0);
	
	gtk_widget_show_all(window1);

	return window1;
	  
}
