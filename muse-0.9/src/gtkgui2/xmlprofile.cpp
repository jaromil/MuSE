/* A Xml profile manager for MuSE - Multiple Streaming Engine
 * Copyright (C) 2002-2004 nightolo <night@autistici.org>
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <gtk/gtk.h>

#include <jmixer.h>
#include <jutils.h>

#include <gen.h>
#include <xmlprofile.h>

#include <config.h>

IceState icest = STATE_NULL;
EncState encst = STATE_NULL1;

struct encprof *encprof_tmp;
struct iceprof *iceprof_tmp;
const gchar *home;

gboolean profile_init(void)
{
	gchar *dir;
	int fd;

	if(!(home = g_getenv("HOME"))) {
		error(_("no $HOME found"));
		return FALSE;
	}
	
	dir = g_strconcat(home, "/.muse", NULL);
	if(!g_file_test(dir, (GFileTest) (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
		if((fd = creat(dir, S_IRWXU)) < 0) {
			error(_("error during directory creation"));
			return FALSE;
		}
		close(fd);
	}
	
	g_free(dir);
	/* files will be created at writing */
	return TRUE;
}

/* three profile_something_load zone */
void profile_lame_load(void)
{
	gchar *filepath, *buf;
	GMarkupParser mp;
	GMarkupParseContext *mpc;

	mp.start_element = enc_get_element;
	mp.end_element = lame_end_element;
	mp.text = enc_get_value;
	mp.passthrough = NULL;
	mp.error = NULL;

	filepath = g_strconcat(home, "/.muse/lame.xml", NULL);
	g_file_get_contents(filepath, &buf, NULL, NULL);
	g_free(filepath);
	if(!buf)
		return;
	
	/* fuck C++ and his cast */
	mpc = g_markup_parse_context_new(&mp, (GMarkupParseFlags) 0, NULL,  NULL);
	g_markup_parse_context_parse(mpc, buf, -1, NULL);
	g_markup_parse_context_free(mpc);

}	
void profile_vorbis_load(void)
{
	gchar *filepath, *buf;
	GMarkupParser mp;
	GMarkupParseContext *mpc;

	mp.start_element = enc_get_element;
	mp.end_element = vorbis_end_element;
	mp.text = enc_get_value;
	mp.passthrough = NULL;
	mp.error = NULL;

	filepath = g_strconcat(home, "/.muse/vorbis.xml", NULL);
	g_file_get_contents(filepath, &buf, NULL, NULL);
	g_free(filepath);
	if(!buf)
		return;

	mpc = g_markup_parse_context_new(&mp, (GMarkupParseFlags) 0, NULL, NULL);
	g_markup_parse_context_parse(mpc, buf, -1, NULL);
	g_markup_parse_context_free(mpc);
	func("finito il parsing");

}	

void profile_ice_load(void)
{
	gchar *filepath, *buf;
	GMarkupParser mp;
	GMarkupParseContext *mpc;
	
	mp.start_element = ice_get_element;
	mp.end_element = ice_end_element;
	mp.text = ice_get_value;
	mp.passthrough = NULL;
	mp.error = NULL;

	filepath = g_strconcat(home, "/.muse/ice.xml", NULL);
	g_file_get_contents(filepath, &buf, NULL, NULL);
	func("testo del file %s %s", filepath, buf);
	g_free(filepath);
	if(!buf)
		return;
	

	mpc = g_markup_parse_context_new(&mp, (GMarkupParseFlags) 0, NULL, NULL);
	g_markup_parse_context_parse(mpc, buf, -1, NULL);
	g_markup_parse_context_free(mpc);
}	

/* enc parse zone */

void enc_get_element(GMarkupParseContext *context, const gchar *element,
		const gchar **attr_names, const gchar **attr_values,
		gpointer user_data, GError **error)
{
	if(!strcmp(element, "profile")) {
		encprof_tmp = (struct encprof *) malloc(sizeof(struct encprof));
		encprof_tmp->name = g_strdup(attr_values[0]);
		encst = STATE_NAME1;
	}
	else if(element[0] == 'q')
		encst = STATE_QUALITY;
	else if(element[0] == 'm')
		encst = STATE_MODE;
	else if(element[0] == 'b')
		encst = STATE_BITRATE;
	else if(!strcmp(element, "frequency"))
		encst = STATE_FREQUENCY;
	else if(!strcmp(element, "freqfil"))
		encst = STATE_FREQFIL;
	else if(element[0] == 'l')
		encst = STATE_LOWPASS;
	else if(element[0] == 'h')
		encst = STATE_HIGHPASS;

}



void enc_get_value(GMarkupParseContext *context, const gchar *text, gsize size,
		gpointer user_data, GError **error)
{
	switch(encst) {
		case STATE_QUALITY:
			encprof_tmp->quality = atof(text);
			break;
		case STATE_MODE:
			encprof_tmp->mode = g_strdup(text);
			break;
		case STATE_BITRATE:
			encprof_tmp->bitrate = g_strdup(text);
			break;
		case STATE_FREQUENCY:
			encprof_tmp->frequency = g_strdup(text);
			break;
		case STATE_FREQFIL:
			encprof_tmp->freqfil = g_strdup(text);
			break;
		case STATE_HIGHPASS:
			encprof_tmp->highpass = g_strdup(text);
			break;
		case STATE_LOWPASS:
			encprof_tmp->lowpass = g_strdup(text);
			break;
		default:
			break;
	}

}

void lame_end_element(GMarkupParseContext *context, const gchar *element,
		gpointer user_data, GError **error)
{
	if(!strcmp(element, "profile"))
		lameprof = g_list_append(lameprof, (void *) encprof_tmp);
	encst = STATE_NULL1;
}

void vorbis_end_element(GMarkupParseContext *context, const gchar *element,
		gpointer user_data, GError **error)
{
	if(!strcmp(element, "profile"))
		vorbisprof = g_list_append(vorbisprof, (void *) encprof_tmp);
	encst = STATE_NULL1;
}

/* ice parse zone */
void ice_get_element(GMarkupParseContext *context, const gchar *element,
		const gchar **attr_names, const gchar **attr_values,
		gpointer user_data, GError **error)
{
	if(!strcmp(element, "profile")) {
		iceprof_tmp = (struct iceprof *) malloc(sizeof(struct iceprof));
		iceprof_tmp->name = g_strdup(attr_values[0]);
		icest = STATE_NAME;
	}
	else if(element[0] == 'h')
		icest = STATE_HOST;
	else if(!strcmp(element, "port"))
		icest = STATE_PORT;
	else if(element[0] == 'm')
		icest = STATE_MNT;
	else if(element[0] == 'u')
		icest = STATE_URL;
	else if(element[0] == 'n')
		icest = STATE_STREAM_NAME;
	else if(element[0] == 'd')
		icest = STATE_DESCRIPTION;
	else if(element[0] == 'l')
		icest = STATE_LOGINTYPE;
	else if(element[0] == 'p')
		icest = STATE_PASSWORD;

}

void ice_get_value(GMarkupParseContext *context, const gchar *text, gsize size,
		gpointer user_data, GError **error)
{
	switch (icest) {
		case STATE_HOST:
			iceprof_tmp->host = g_strdup(text);
			break;
		case STATE_PORT:
			iceprof_tmp->port = g_strdup(text);
			break;
		case STATE_MNT:
			iceprof_tmp->mnt = g_strdup(text);
			break;
		case STATE_URL:
			iceprof_tmp->url = g_strdup(text);
			break;
		case STATE_STREAM_NAME:
			iceprof_tmp->stream_name = g_strdup(text);
		case STATE_DESCRIPTION:
			iceprof_tmp->desc = g_strdup(text);
			break;
		case STATE_LOGINTYPE:
			iceprof_tmp->logintype = g_strdup(text);
			break;
		case STATE_PASSWORD:
			iceprof_tmp->password = g_strdup(text);
			break;
		default:
			break;
	}

}

void ice_end_element(GMarkupParseContext *context, const gchar *element,
		gpointer user_data, GError **error)
{
	if(!strcmp(element, "profile"))
		iceprof = g_list_append(iceprof, (void *) iceprof_tmp);
	icest = STATE_NULL;
}

/* write zone */
/* these three functions rewrite _all_ files */
void profile_lame_write(void)
{
	gchar *file;
	FILE *fp;
	GList *listrunner;
	struct encprof *tmp;

	file = g_strconcat(home, "/.muse/lame.xml", NULL);
	
	if(!(fp = fopen(file, "w"))) {
		error(_("problem opening lame.xml for writing"));
		return;
	}

	if(!(listrunner = g_list_first(lameprof))) {
	  func("gtkgui2/xmlprofile.cpp ERROR listrunner is (null)");
	}
	while(listrunner) {
		tmp = (struct encprof *) listrunner->data;
		
		  fprintf(fp, "<profile name=%s>\n"
		  "\t<quality>%f</quality>\n"
		  "\t<mode>%s</mode>\n"
		  "\t<bitrate>%s</bitrate>\n"
		  "\t<frequency>%s</frequency>\n"
		  "\t<freqfil>%s</freqfil>\n"
		  "\t<lowpass>%s</lowpass>\n"
		  "\t<highpass>%s</highpass>\n"
		  "</profile>\n\n",
		  tmp->name, tmp->quality, tmp->mode, tmp->bitrate,
		  tmp->frequency, tmp->freqfil, tmp->lowpass,
		  tmp->highpass);
		
		listrunner = g_list_next(listrunner);
	}
	fclose(fp);

}

void profile_vorbis_write(void)
{
	gchar *file;
	FILE *fp;
	GList *listrunner;
	struct encprof *tmp;

	file = g_strconcat(home, "/.muse/vorbis.xml", NULL);
	
	if(!(fp = fopen(file, "w"))) {
		error(_("problem opening vorbis.xml for writing"));
		return;
	}

	if(!(listrunner = g_list_first(vorbisprof))) {
	  func("gtkgui2/xmlprofile.cpp ERROR listrunner is (null)");
	}
	while(listrunner) {
		tmp = (struct encprof *) listrunner->data;
		
		  fprintf(fp, "<profile name=\"%s\">\n"
		  "\t<quality>%f</quality>\n"
		  "\t<mode>%s</mode>\n"
		  "\t<bitrate>%s</bitrate>\n"
		  "\t<frequency>%s</frequency>\n"
		  "\t<freqfil>%s</freqfil>\n"
		  "\t<lowpass>%s</lowpass>\n"
		  "\t<highpass>%s</highpass>\n"
		  "</profile>\n\n",
		  tmp->name, tmp->quality, tmp->mode, tmp->bitrate,
		  tmp->frequency, tmp->freqfil, tmp->lowpass,
		  tmp->highpass);
		
		listrunner = g_list_next(listrunner);
	}
	fclose(fp);


}

void profile_ice_write(void)
{
	gchar *file;
	FILE *fp;
	GList *listrunner;
	struct iceprof *tmp;

	file = g_strconcat(home, "/.muse/ice.xml", NULL);
	
	if(!(fp = fopen(file, "w"))) {
		error(_("problem opening ice.xml for writing"));
		return;
	}

	listrunner = g_list_first(iceprof);

	while(listrunner) {
		tmp = (struct iceprof *) listrunner->data;
		
		  fprintf(fp, "<profile name=\"%s\">\n"
		  "\t<host>%s</host>\n"
		  "\t<port>%s</port>\n"
		  "\t<mnt>%s</mnt>\n"
		  "\t<name>%s</name>\n"
		  "\t<url>%s</url>\n"
		  "\t<desc>%s</desc>\n"
		  "\t<logintype>%s</logintype>\n"
		  "\t<password>%s</password>\n"
		  "</profile>\n\n",
		  tmp->name, tmp->host, tmp->port, tmp->mnt,
		  tmp->stream_name, tmp->url, tmp->desc,
		  tmp->logintype, tmp->password);
		
		listrunner = g_list_next(listrunner);
	}
	fclose(fp);	
}

/* remove zone */
void profile_lame_remove(gchar *name)
{
	GList *listrunner = g_list_first(lameprof);
	struct encprof *tmp;

	while(listrunner) {
		tmp = (struct encprof *) listrunner->data;
		if(!strcmp(name, tmp->name)) {
			profile_enc_free(tmp);
			lameprof = g_list_remove(lameprof, tmp);
			break;
		}
		listrunner = g_list_next(listrunner);
	}

}

void profile_vorbis_remove(gchar *name)
{
	GList *listrunner = g_list_first(vorbisprof);
	struct encprof *tmp;

	while(listrunner) {
		tmp = (struct encprof *) listrunner->data;
		if(!strcmp(name, tmp->name)) {
			profile_enc_free(tmp);
			vorbisprof = g_list_remove(vorbisprof, tmp);
			break;
		}
		listrunner = g_list_next(listrunner);
	}
}
void profile_ice_remove(gchar *name)
{
	GList *listrunner = g_list_first(iceprof);
	struct iceprof *tmp;

	while(listrunner) {
		tmp = (struct iceprof *) listrunner->data;
		if(!strcmp(name, tmp->name)) {
			profile_ice_free(tmp);
			iceprof = g_list_remove(iceprof, tmp);
			break;
		}
		listrunner = g_list_next(listrunner);
	}
}

/* utility functions */
void profile_ice_free(struct iceprof *i)
{
	g_free(i->name);
	g_free(i->host);
	g_free(i->port);
	g_free(i->mnt);
	g_free(i->url);
	g_free(i->stream_name);
	g_free(i->desc);
	g_free(i->logintype);
	g_free(i->password);
}

void profile_enc_free(struct encprof *e)
{
	g_free(e->name);
	g_free(e->mode);
	g_free(e->bitrate);
	g_free(e->frequency);
	g_free(e->freqfil);
	g_free(e->lowpass);
	g_free(e->highpass);
}

